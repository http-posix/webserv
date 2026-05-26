#include <sys/stat.h>

#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

struct Request {
  std::string method;
  std::string uri;
  std::map<std::string, std::string> headers;
  std::string body;
};

struct Config {
  std::string root;
};

struct Response {
  int status_code = 200;
  std::string reason = "OK";
  std::map<std::string, std::string> headers;
  std::string body;

  std::string serialize() const {
    std::ostringstream out;
    out << "HTTP/1.1 " << status_code << " " << reason << "\r\n";
    for (auto const &h : headers) out << h.first << ": " << h.second << "\r\n";
    out << "\r\n";
    out << body;
    return out.str();
  }
};

bool file_exists(const std::string &path) {
  struct stat st;
  return stat(path.c_str(), &st) == 0 && S_ISREG(st.st_mode);
}

std::string read_file(const std::string &path) {
  std::ifstream ifs(path, std::ios::in | std::ios::binary);
  if (!ifs) return std::string();
  std::ostringstream ss;
  ss << ifs.rdbuf();
  return ss.str();
}

std::string get_mime_type(const std::string &path) {
  auto pos = path.rfind('.');
  if (pos == std::string::npos) return "application/octet-stream";
  std::string ext = path.substr(pos);
  if (ext == ".html" || ext == ".htm") return "text/html";
  if (ext == ".css") return "text/css";
  if (ext == ".js") return "application/javascript";
  if (ext == ".json") return "application/json";
  if (ext == ".png") return "image/png";
  if (ext == ".jpg" || ext == ".jpeg") return "image/jpeg";
  return "application/octet-stream";
}

Response build_404() {
  Response r;
  r.status_code = 404;
  r.reason = "Not Found";
  r.body = "<html><body><h1>404 Not Found</h1></body></html>";
  r.headers["Content-Type"] = "text/html";
  r.headers["Content-Length"] = std::to_string(r.body.size());
  r.headers["Server"] = "Webserv-Test";
  return r;
}

Response handle_get(const Request &req, const Config &cfg) {
  std::string uri = req.uri;
  if (uri.empty()) uri = "/";
  // simple index rewrite
  if (uri.back() == '/') uri += "index.html";

  // naive sanitize: reject paths containing ".."
  if (uri.find("..") != std::string::npos) return build_404();

  std::string full = cfg.root + uri;

  if (!file_exists(full)) return build_404();

  std::string body = read_file(full);

  Response res;
  res.status_code = 200;
  res.reason = "OK";
  res.body = body;
  res.headers["Content-Type"] = get_mime_type(full);
  res.headers["Content-Length"] = std::to_string(body.size());
  res.headers["Server"] = "Webserv-Test";
  return res;
}

int main(int argc, char **argv) {
  // read config: first token = root path
  std::string config_path = "mock.conf";
  if (argc > 1) config_path = argv[1];

  std::ifstream cfgin(config_path);
  Config cfg;
  if (!cfgin || !(cfgin >> cfg.root)) {
    std::cerr << "Failed to read config (expected single token root path)\n";
    return 2;
  }

  // simulate parsed request
  Request req;
  req.method = "GET";
  req.uri = "/";  // request index

  Response res = handle_get(req, cfg);

  // print serialized response to stdout
  std::string out = res.serialize();
  std::cout << out;
  return 0;
}
