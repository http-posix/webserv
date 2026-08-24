#pragma once

//if I want to define colors during compile time I need to use constexpr const char* 
// because untill c++20 it wasn't allowed to define dynamic resources (string is new())
// if I'm ok to define during run time than I can use const string - they would be heavier than char*
//inline const std::string

namespace clrs {
	// Colors
	constexpr const char *kRed = "\033[31m";
	constexpr const char *kGreen = "\033[32m";
	constexpr const char *kYellow = "\033[93m";
	constexpr const char *kBlue = "\033[34m";
	constexpr const char *kMagenta = "\033[35m";
	constexpr const char *kBlack = "\033[30m";
	constexpr const char *kCyan = "\033[36m";
	
	// Formatting
	constexpr const char *kReset = "\033[0m";
	constexpr const char *kBold = "\033[1m";
}
