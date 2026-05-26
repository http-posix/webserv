#include "app_exception/app_exception.hpp"
#include "logger/logger.hpp"
#include <iostream>

// Later on move to the Server header ?
void run_app(int argc, char **argv);

<<<<<<< HEAD
int main(int argc, char **argv){
	
	int	status = 0;
	
	Logger::GetInstance().PrintMsg("Hello user!");
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
=======
int main()
{
	HttpRequest request;
	PrintMsg("Hello user!");
	fill_response(request);
	
	return 0;
}
>>>>>>> 14524b6 (progress - implemented serialize method to concatenate response object into an actual response)
