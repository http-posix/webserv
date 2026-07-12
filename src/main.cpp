#include "utils/app_exception/app_exception.hpp"
#include "utils/logger/logger.hpp"
#include <iostream>
#include "event_loop/signal_handler.hpp"


// Later on move to the Server header ?
void run_app(int argc, char **argv);

int main(int argc, char **argv){
	
	int	status = 0;
	
	Logger::GetInstance().PrintMsg("Hello user!");
	Signal::SetupSignalHandlers();

	try{
		run_app(argc, argv);
	}
	catch(const std::bad_alloc& error){
		status = 1;
		std::cerr << "Error: can't allocate memory " << error.what() << "\n";
	}
	catch(const AppException& error){
		status = 1;
		LOG_ERROR(std::string("AppException: ") + error.what());
	}
	return status;
}
