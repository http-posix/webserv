#include "utils/app_exception/app_exception.hpp"
#include "utils/logger/logger.hpp"

void run_app(int argc, char **argv){
	(void)argv;
	(void)argc;
	// throw (ConfigException("test_Conf"));
	LOG_INFO("We are runnig app!");
}
