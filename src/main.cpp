#include "utils/app_exception/app_exception.hpp"
#include "utils/logger/logger.hpp"
#include "io/event_loop/signal_handler.hpp"

#include <iostream>

// Later on move to the Server header ?
void run_app(int argc, char **argv);

int main(int argc, char **argv){
	
	int	status = 1;
	
	Signal::SetupSignalHandlers();

	try{
		run_app(argc, argv);
		status = 0;
	}
	catch(const std::bad_alloc& error){
		std::cerr << "Error: can't allocate memory " << error.what() << "\n";
	}
	catch(const AppException& error){
		LOG_ERROR(std::string("AppException: ") + error.what());
	}
	catch (const std::exception& error){
		LOG_ERROR(std::string("Unexpected exception: ") + error.what());
	}
	return status;
}
