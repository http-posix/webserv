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

		void Log(LogLevel level, const char* file, int line, const std::string& msg, const std::string& ctx="");
		static void PrintMsg(const std::string& msg);

	private:
		Logger();
		~Logger() = default;
		void InitLogFile();

		std::ofstream	file_;
		bool			use_file_ = false;

};

#define LOG_INFO(...) Logger::GetInstance().Log(LogLevel::INFO, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_WARN(...) Logger::GetInstance().Log(LogLevel::WARN, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_ERROR(...) Logger::GetInstance().Log(LogLevel::ERROR, __FILE__, __LINE__, __VA_ARGS__)

#ifdef DEBUG_MODE
	#define LOG_DEBUG(...) Logger::GetInstance().Log(LogLevel::DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#else
	#define LOG_DEBUG(...) (void(0))
#endif
