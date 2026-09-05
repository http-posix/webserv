#include "utils/app_exception/app_exception.hpp"
#include "utils/logger/logger.hpp"
#include "app/server/server.hpp"
#include "io/event_loop/event_loop.hpp"



void run_app(int argc, char **argv){
	(void)argv;
	(void)argc;

	// throw (ConfigException("test_Conf"));
	Logger::GetInstance().PrintMsg("Welcome to Webserv of the http-posix team!");

	// Config mock_obj = MockConfig::SingleServer("127.0.0.1", {8080}); // Create only one Server object
	Config mock_obj = MockConfig::SingleServer("127.0.0.1", {8080, 8081}); // Creates two Server objects

	std::vector<Server> listeners = CreateListeners(mock_obj);
	EventLoop	loop(std::move(listeners));
	loop.run();
	Logger::GetInstance().PrintMsg("\n Server stopped. Have a good day ^_^");
}


// {
// 	Logger::GetInstance().PrintMsg("Welcome to Webserv of the http-posix team!");

// 	if (argc > 2){
// 		throw ConfigException("Too many arguments. Maximum one configuration file is allowed");
// 	}
// 	const std::string config_path = (argc == 2) ? argv[1] : kDefaultConfigPath;

// 	Config	config_data = ParseConfig(config_path);
// 	std::vector<Server> listeners = CreateListeners(config_data);
// 	EventLoop	loop(std::move(listeners));
// 	loop.run();
// }