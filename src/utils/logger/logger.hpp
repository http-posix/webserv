#pragma once

#include <fstream>
#include <string>

enum class LogLevel {
	DEBUG,
	INFO,
	WARN,
	ERROR
};

// Static local variable – Meyers Singleton
class Logger {
	public:
		static Logger& GetInstance();
		Logger(const Logger&) 				= delete;
		Logger& operator=(const Logger&)	= delete;
		void Log(LogLevel level, const std::string& msg, const char *file, int line, const char *func);
		static void PrintMsg(const std::string& msg);

	private:
		Logger();
		~Logger();
		void InitLogFile();

		std::ofstream	file_;
		bool			use_file_ = false;

};

// Standart log levels, we'll see do we need all of them
#ifdef DEBUG_MODE
	#define LOG_DEBUG(msg) Logger::GetInstance().Log(LogLevel::DEBUG, msg, __FILE__, __LINE__, __func__)
	#define LOG_INFO(msg) Logger::GetInstance().Log(LogLevel::INFO, msg, __FILE__, __LINE__, __func__)
	#define LOG_WARN(msg) Logger::GetInstance().Log(LogLevel::WARN, msg, __FILE__, __LINE__, __func__)
	#define LOG_ERROR(msg) Logger::GetInstance().Log(LogLevel::ERROR, msg, __FILE__, __LINE__, __func__)
#else
	#define LOG_DEBUG(msg)
	#define LOG_INFO(msg)
	#define LOG_WARN(msg)
	#define LOG_ERROR(msg)
#endif