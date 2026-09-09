#include "utils/logger/logger.hpp"
#include "utils/logger/colors.hpp"

#include <iostream>
#include <ctime>
#include <cstdio>
#include <sys/stat.h>
#include <chrono>
#include <iomanip>

namespace {

	std::string CurrentTime() {

		auto time_point = std::chrono::system_clock::now();
 		std::time_t seconds = std::chrono::system_clock::to_time_t(time_point);
		auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(time_point.time_since_epoch()).count() % 1000;

		auto local_tm = std::localtime(&seconds);
		if (!local_tm)
			return "??:??:??.???";

		char timestamp[16];
		std::snprintf(timestamp, sizeof(timestamp), "%02d:%02d:%02d.%03d",
					local_tm->tm_hour,
					local_tm->tm_min,
					local_tm->tm_sec,
					static_cast<int>(ms));

		return timestamp;
	}
}

Logger::Logger(){
	#ifdef LOG_TO_FILE
		InitLogFile();
	#endif
}

Logger&  Logger::GetInstance(){
	static Logger instance;
	return instance;
}

void Logger::InitLogFile(){
	mkdir("log", 0755);

	std::time_t		now = std::time(nullptr);
	std::tm*		tm = std::localtime(&now);
	if (!tm){
		std::cerr << "[Logger] localtime failed." << "\n";
		return;
	}

	char	timestamp[32];
	std::strftime(timestamp, sizeof(timestamp), "%m-%d_%H-%M", tm);
	
	std::string	path = std::string("log/") + timestamp + ".log";
	file_.open(path, std::ios::out);
	if (file_.is_open())
		use_file_ = true;
	else
		std::cerr << "[Logger] Cannot open log file: " << path << "\n";
}

void Logger::Log(LogLevel level, const char *file, int line, const std::string &msg, const std::string& ctx){
	const char *label = nullptr;
	const char* color = nullptr;

	switch (level){
		case LogLevel::DEBUG:	label = "[--DEBUG--]"; color=clrs::kCyan; break;
		case LogLevel::INFO:	label = "[--INFO---]"; color=clrs::kGreen; break;
		case LogLevel::WARN:	label = "[--WARN---]"; color=clrs::kYellow; break;
		case LogLevel::ERROR:	label = "[--ERROR--]"; color=clrs::kRed; break;
	}

	std::string file_short = std::string(file);
	size_t i = file_short.find_last_of('/');
	std::string location = file_short.substr(i+1) + ":" + std::to_string(line) + ": ";
	std::string time = CurrentTime();

	std::cerr	<< time << ' '
				<< color << label << ' ' << clrs::kReset
				<< clrs::kBold << std::left << std::setw(20) << location << clrs::kReset
				<< clrs::kDim << ctx << clrs::kReset << ' '
				<< msg << '\n';

	#ifdef LOG_TO_FILE
		if (use_file_){
			file_	<< time << ' ' << label << ' '
					<< std::left << std::setw(20) << location
					<< ctx << ' ' 
					<< msg << '\n';
			if (level == LogLevel::WARN || level == LogLevel::ERROR)
				file_ << std::flush;
		}
	#endif
}

void	Logger::PrintMsg(const std::string& msg){
	std::cout << msg << std::endl;
}
