// nginx – https://docs.nginx.com/nginx/admin-guide/basic-functionality/managing-configuration-files/
// Two servers on DIFFERENT ports → "multiple servers" without virtual hosts.

#pragma once
#include <string>
#include <vector>
#include <cstdint>


// uint16_t - max port size 0–65535
struct ServerConfig {
	std::string				host;
	std::vector<uint16_t>	listen_ports;
};

struct Config {
	std::vector<ServerConfig> servers;
};

struct MockConfig {
	static Config SingleServer(const std::string& host, const std::vector<uint16_t>& ports) {
		Config cfg;
		cfg.servers.push_back({host, ports});
		return cfg;
	}

	static Config TwoServersSameSettings(const std::string& host, uint16_t port) {
		Config cfg;
		cfg.servers.push_back({host, {port}});
		cfg.servers.push_back({host, {port}}); // Duplicate host:port for validation check
		return cfg;
	}

	static Config Empty() {
		return Config{}; // Empty config
	}
};

inline Config ParseConfig(const std::string& /*config_path*/) {
	Config obj;
	obj.servers.push_back({ "127.0.0.1", { 8080, 8081 } });
	obj.servers.push_back({ "10.1.1.1", { 8082 } });
	return obj;
}


