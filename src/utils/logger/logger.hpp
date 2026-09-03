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
		void Log(LogLevel level, const std::string& msg, const char *file, int line);
		static void PrintMsg(const std::string& msg);

	private:
		Logger();
		~Logger() = default;
		void InitLogFile();

		std::ofstream	file_;
		bool			use_file_ = false;

};

#define LOG_INFO(msg) Logger::GetInstance().Log(LogLevel::INFO, msg, __FILE__, __LINE__)
#define LOG_WARN(msg) Logger::GetInstance().Log(LogLevel::WARN, msg, __FILE__, __LINE__)
#define LOG_ERROR(msg) Logger::GetInstance().Log(LogLevel::ERROR, msg, __FILE__, __LINE__)

#ifdef DEBUG_MODE
	#define LOG_DEBUG(msg) Logger::GetInstance().Log(LogLevel::DEBUG, msg, __FILE__, __LINE__)
#else
	#define LOG_DEBUG(msg) = (void(0));

#endif