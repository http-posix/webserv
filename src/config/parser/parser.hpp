#pragma once
#include <string>
#include <vector>

// FREE FUNCTION:
Config ParseConfig(std::string config_path);

class ConfigParser {
	private:
		Config config_;
		std::string	file_string_;
		ConfigTokenizer tokenizer_;
		ConfigToken pos_;

		// Collects token values up to (and consuming) `stop`.
		// Throws ConfigException on end of file.
		std::vector<std::string> collectUntil(const std::string& stop);
		// Expects the current token to equal `value`, then advances.
		void expect(const std::string& value);
		static std::string join(const std::vector<std::string>& values);
		static int parseInt(const std::string& value);
		static size_t parseSize(const std::string& value);
		static size_t applyUnit(size_t value, const std::string& unit);

	public:
		ConfigParser();
		ConfigParser(const std::string& filename);
		~ConfigParser(void);

		int readFile(const std::string& filename);
		int removeComments();
		void parseFromString();
		const Config& getConfig() const;
		void createServerConfig();
		void createLocationConfig(LocationConfig& location);
};
