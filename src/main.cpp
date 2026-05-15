#include "app_exception/app_exception.hpp"
#include "logger/logger.hpp"
#include <iostream>

// Later on move to the Server header ?
void run_app(int argc, char **argv);

int main(int argc, char **argv){
	
	int	status = 0;
	
	PrintMsg("Hello user!");
	try{
		run_app(argc, argv);
	}
	catch(const AppException& error){
		status = 1;
		std::cerr << error.what() << std::endl;
	}
	catch(const std::bad_alloc& error){
		status = 1;
		std::cerr << "Error: can't allocate memory " << error.what() << std::endl;
	}
	return status;
}
