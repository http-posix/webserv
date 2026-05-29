#include <cassert>
#include <iostream>
#include <string>
#include <vector>

#include "../../src/http_response/http_response_state.hpp"

StatusCode check_method_error(const HttpRequest& request);
StatusCode check_file_error(const HttpRequest& request);
std::vector<unsigned char> read_file(const HttpRequest request);
std::string parse_type(const HttpRequest& request);

namespace {
const bool kEnableStrictContentLengthHeaderAssert = false;
const bool kEnableStrictContentTypeHeaderAssert = false;
const bool kEnableStrictHeaderBodyConsistencyAssert = false;
const bool kEnableFutureSpecEdgeCaseAsserts = false;

HttpRequest make_request(const std::string& path, const std::string& method) {
  HttpRequest req;

  req.path_ = path;
  req.method_ = method;
  return (req);
}

std::size_t find_separator(const std::string& wire) {
  return (wire.find("\r\n\r\n"));
}

std::string extract_body_from_wire(const std::string& wire) {
  std::size_t sep = find_separator(wire);
  if (sep == std::string::npos)
    return ("");
  return (wire.substr(sep + 4));
}

bool has_header_line(const std::string& wire, const std::string& name,
                     const std::string& value) {
  const std::string needle = name + ": " + value + "\r\n";
  return (wire.find(needle) != std::string::npos);
}

std::string extract_header_value(const std::string& wire,
                                 const std::string& name) {
  const std::size_t sep = find_separator(wire);
  if (sep == std::string::npos)
    return ("");
  const std::string prefix = name + ": ";
  const std::size_t pos = wire.find(prefix);
  if (pos == std::string::npos || pos >= sep)
    return ("");
  const std::size_t value_start = pos + prefix.size();
  const std::size_t line_end = wire.find("\r\n", value_start);
  if (line_end == std::string::npos || line_end > sep)
    return ("");
  return (wire.substr(value_start, line_end - value_start));
}

bool parse_content_length(const std::string& wire, std::size_t& out_length) {
  const std::string value = extract_header_value(wire, "Content-Length");
  if (value.empty())
    return (false);
  std::size_t parsed = 0;
  for (std::size_t i = 0; i < value.size(); ++i) {
    const char c = value[i];
    if (c < '0' || c > '9')
      return (false);
    parsed = parsed * 10 + static_cast<std::size_t>(c - '0');
  }
  out_length = parsed;
  return (true);
}

HttpResponseState build_response_for_request(const HttpRequest& request) {
  HttpResponseState response;

  response.set_statusCode(request);
  if (response.get_statusCode() == OK) {
    response.set_body(request);
    response.set_headers(request);
  }
  return (response);
}

std::string build_wire_for_request(const HttpRequest& request) {
  HttpResponseState response = build_response_for_request(request);
  return (response.serialize());
}
}  // namespace

int main(void) {
  HttpRequest get_req;
  HttpRequest post_req;
  HttpRequest missing_req;
  HttpResponseState response = build_response_for_request(get_req);
  HttpRequest empty_method_req;
  HttpRequest lowercase_get_req;
  HttpRequest directory_req;
  StatusCode directory_status;
  HttpRequest html_type_req;
  HttpRequest uppercase_type_req;
  HttpRequest no_extension_req;

  const std::string existing_path = "test/response_test/mock_site/simple.html";
  const std::string missing_path =
      "test/response_test/mock_site/does_not_exist.html";
  // 1) Method validation
  get_req = make_request(existing_path, "GET");
  post_req = make_request(existing_path, "POST");
  assert(check_method_error(get_req) == OK && "GET should be accepted");
  assert(check_method_error(post_req) == NOT_FOUND &&
         "Non-GET should be rejected for now");
  // 2) File validation
  assert(check_file_error(get_req) == OK &&
         "Existing readable file should return (OK");
  missing_req = make_request(missing_path, "GET");
  assert(check_file_error(missing_req) == NOT_FOUND &&
         "Missing file should return (NOT_FOUND");
  // 3) Body loading
  std::vector<unsigned char> body = read_file(get_req);
  assert(!body.empty() && "Body should not be empty for an existing file");
  std::string body_text(body.begin(), body.end());
  assert(body_text.find("Simple Mock Page") != std::string::npos &&
         "Expected known body content");
  // 4) Serialization sanity (Step 1)
  assert(response.get_statusCode() == OK &&
         "Status should be OK for GET + existing file");
  std::string wire = response.serialize();
  assert(wire.find("HTTP/1.1 200 OK\r\n") == 0 &&
         "Wire response should start with a valid status line");
  assert(wire.find("Simple Mock Page") != std::string::npos &&
         "Wire response should include body content");
  // 5) Serialization checks (Step 2)
  std::size_t separator_pos = find_separator(wire);
  assert(separator_pos != std::string::npos &&
         "Wire response must contain CRLF CRLF header/body separator");
  std::string wire_body = extract_body_from_wire(wire);
  assert(wire_body == body_text && "Wire body must match file body exactly");
  assert(wire_body.size() == body.size() &&
         "Wire body size must match loaded body size");
  if (kEnableStrictContentLengthHeaderAssert) {
    assert(
        has_header_line(wire, "Content-Length", std::to_string(body.size())) &&
        "Expected Content-Length header");
  }
  if (kEnableStrictContentTypeHeaderAssert) {
    assert(has_header_line(wire, "Content-Type", "text/html") &&
           "Expected MIME Content-Type header");
  }
  if (kEnableStrictHeaderBodyConsistencyAssert) {
    std::size_t declared_length = 0;
    assert(parse_content_length(wire, declared_length) &&
           "Expected parseable Content-Length header");
    assert(declared_length == wire_body.size() &&
           "Content-Length must match serialized body size");
  }
  // 6) Serialized error status checks (Step 3)
  std::string missing_wire = build_wire_for_request(missing_req);
  assert(missing_wire.find("HTTP/1.1 404 ") == 0 &&
         "Missing file wire response should start with 404 status line");
  assert(find_separator(missing_wire) != std::string::npos &&
         "404 wire response must contain CRLF CRLF separator");
  assert(extract_body_from_wire(missing_wire).empty() &&
         "404 wire response body is expected to be empty for now");
  std::string post_wire = build_wire_for_request(post_req);
  assert(post_wire.find("HTTP/1.1 -1 ") == 0 &&
         "Unsupported method wire response should start with OTHER status code "
         "for now");
  assert(find_separator(post_wire) != std::string::npos &&
         "Unsupported method wire response must contain CRLF CRLF separator");
  assert(
      extract_body_from_wire(post_wire).empty() &&
      "Unsupported method wire response body is expected to be empty for now");
  // 7) Edge-case matrix (Step 5)
  empty_method_req = make_request(existing_path, "");
  assert(check_method_error(empty_method_req) == NOT_FOUND &&
         "Empty method should be rejected for now");
  lowercase_get_req = make_request(existing_path, "get");
  assert(check_method_error(lowercase_get_req) == NOT_FOUND &&
         "Lowercase get should be rejected for now");
  directory_req = make_request("test/response_test/mock_site", "GET");
  directory_status = check_file_error(directory_req);
  assert((directory_status == FORBIDDEN || directory_status == OK) &&
         "Directory path behavior is platform-dependent for now");
  html_type_req = make_request(existing_path, "GET");
  assert(parse_type(html_type_req) == "html" &&
         "Expected lowercase extension parsing for .html");
  uppercase_type_req =
      make_request("test/response_test/mock_site/SIMPLE.HTML", "GET");
  assert(parse_type(uppercase_type_req) == "HTML" &&
         "Expected extension parsing to preserve case for now");
  no_extension_req = make_request("test/response_test/mock_site/README", "GET");
  assert(parse_type(no_extension_req).empty() &&
         "Expected empty extension for paths without dot extension");
  if (kEnableFutureSpecEdgeCaseAsserts) {
    assert(check_method_error(lowercase_get_req) == OK &&
           "Future spec: method parsing may become case-insensitive");
    assert(parse_type(uppercase_type_req) == "html" &&
           "Future spec: extension normalization may become lowercase");
  }
  std::cout
      << "Step 1-5 assert tests passed (strict feature asserts disabled).\n";
  return (0);
}
