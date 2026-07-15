#include "logger/logger.hpp"
#include "logger/colors.hpp"
#include <iostream>
#include <ctime>
#include <sys/stat.h>

Logger::Logger(){
	#ifdef LOG_TO_FILE
		InitLogFile();
	#endif
}

Logger::~Logger(){
	if (file_.is_open())
		file_.close();
}

Logger&  Logger::GetInstance(){
	static Logger instance;
	return instance;
}

void Logger::InitLogFile(){
	mkdir("log", 0755);

	std::time_t		now = std::time(nullptr);
	std::tm*		tm = std::localtime(&now);

	char	timestamp[32];
	std::strftime(timestamp, sizeof(timestamp), "%m-%d_%H-%M", tm);
	
	std::string	path = std::string("log/") + timestamp + ".log";
	file_.open(path, std::ios::out);
	if (file_.is_open())
		use_file_ = true;
	else
		std::cerr << "[Logger] Cannot open log file: " << path << "\n";
}

void Logger::Log(LogLevel level, const std::string &msg, const char *file, int line, const char *func){
	const char *label = nullptr;
	const char* color = clrs::kReset;
	switch (level){
		case LogLevel::DEBUG:	label = "--DEBUG--"; color=clrs::kCyan; break;
		case LogLevel::INFO:	label = "--INFO---"; color=clrs::kGreen; break;
		case LogLevel::WARN:	label = "--WARN---"; color=clrs::kYellow; break;
		case LogLevel::ERROR:	label = "--ERROR--"; color=clrs::kRed; break;
		default: std::cerr << "[Logger] Unknown log level\n"; return;
	}
	std::string suffix_str = std::string(file) + ": " + "line " + std::to_string(line)
						+ " (" + func + "): " + msg + "\n";
	std::cerr << color << "[" << label << "] " << clrs::kReset << suffix_str;
	#ifdef LOG_TO_FILE
		if (use_file_)
			file_ << "[" << label << "]" << suffix_str << std::flush;
	#endif
}

void	Logger::PrintMsg(const std::string& msg){
	std::cout << msg << std::endl;
}
