## Summary. (What has been done).
This Pull Request adds a working parser for configuring settings for both the server and settings for specific (file)locations the server might be interacting with.

- Introduces `src/config/parser/*` and `src/config/tokenizer/*`
- Adds a working parser creating a structure the server can use to set settings.
- Currently does not integrate with any other modules. (Integration is WIP)
- Adds tests for configuration parser that can later on be used for server life cycles as well.

#### TLDR; Workflow
The following diagram displays from a high level overview how the entire config parser module operates.
```
config file (text)
      │
      ▼
  readFile()        ── read entire file into a string
      │
      ▼
  removeComments()  ── strip every "# ... \n" line
      │
      ▼
  ConfigTokenizer   ── split the text into a stream of ConfigToken
      │
      ▼
  ConfigParser      ── recursively descend, matching tokens into directives,
      │                building ServerConfig / LocationConfig objects
      ▼
  Config (object tree)  ── returned via getConfig()
```

## Design implementation. (How has this been done)
The following paragraphs explain which elements have been added in order to create the final parser. The main elements discussed are; 
1. Implemented Structures. 
	   These structures define the end product of the parser. A necessary first step to understand *what* we must create.
2. Tokenizer.
	   The first step *how* we will create *what* we must create. The tokenizer transforms raw characters from a configuration file into a token; something that the parser can understand. 
3. Parser.
	   Parsing the actual configuration file into a structure that the program can later use. The parser uses the tokenizer and a custom set of rules to set all of the settings in the `ServerConfig` structure.

### Configuration Structure(s).
The goal of this product is to create and initialize a configuration structure that the program can use to set configurations (for multiple servers) when booting. The first step was to decide on the final Configuration structure.

*--from* `src/config/parser/parser.hpp`
```
``` c++
struct Config
{
	std::vector<struct ServerConfig> servers;
};
```
This `struct Config` is a simple vector that contains at least one `ServerConfig` (NB: We can have multiple servers). The `ServerConfig` was more complex and required many design choices. Currently this structure looks as follows;

*--from* `src/config/parser/parser.hpp`
```c++
struct ServerConfig
{
	// Non-virtual so we only have one hostname (ip adress)
	std::string hostname;

	// Ports to listen to. Multiple ports are allowed.
	// Ports are like gateways; our castle can have multiple entrances.
	std::vector<uint16_t> listen_port;

	// Html error pages to return in case of specific error pages.
	// Most browser have their own error page if non are provided,
	// however, subject defines we must give our own error_pages.
	std::unordered_map<int, std::string> error_pages;

	// Default root (directory) used by every location that does not
	// provide its own.
	std::string root;

	// Default index file used by every location that does not provide its own.
	std::string index;

	// Settings for a specific location/directory
	std::vector<struct LocationConfig> locations;

	// Defines what the maximum size of a request should be.
	size_t client_max_body_size = 0;
};
```
Most of these settings are defined and required by the subject. What needs further implementation is the `LocationConfig`. This structure holds custom settings for a specific location that the user might be requesting from the server.

*--from* `src/config/parser/parser.hpp`
```c++
struct LocationConfig
{
	// URI: Uniform Resource Identifier -> path to the location
	std::string uri_path;

	// What methods are allowed in the (sub)directory
	// GET, POST, DELETE
	std::vector<std::string> allowed_methods;

	// Each return codenumber has their own redirection page
	// Somewhat similar to error_pages
	// TODO: Needs to be std::vector<std::pair<int, std::string>>!!!
	std::pair<int, std::string> redirections;

	// root (directory) to override default root of server
	// If non is provided, the server's default is used
	std::string root;

	// Default index file to serve inside this location
	std::string index;

	// Whether to generate a directory listing when no index is present
	bool autoindex = false;

	// Whether file uploads are allowed in this location
	bool upload_enable = false;

	// Location where uploaded files while be uploaded.
	std::string upload_location;
};
```
Again, settings here are required and defined by the subject. Comments in code give a short explanation of the variables and what they are used for. Currently these comments are left in the code intentionally for readability and future understanding.
### Tokenizer.
The main function of the tokenizer is to read a configuration file word for word and separate these words into specific tokens. Currently different types of `tokens` are determined via `enums`.

*--from* `src/config/tokenizer/tokenizer.hpp`
```c++
enum TokenType
{
	Keyword,
	Identifier,
	Symbol,
	Number,
	String,
	EndOfFile
}
```
Alongside the type of a token, the `ConfigToken` holds the actual value of the word:

*from:* `src/config/tokenizer/tokenizer.hpp`
```c++
struct ConfigToken
{
	enum TokenType
	{
		Keyword,
		Identifier,
		Symbol,
		Number,
		String,
		EndOfFile
	} type;
	std::string value;
};
```
`Keywords` are intricately linked with the `struct ServerConfig` and `struct LocationConfig`. They are keywords that identify a specific setting will be set.

Current keywords are established by the `isKeyword()` function:

*from* `src/config/tokenizer/tokenizer.cpp`
```c++
bool ConfigTokenizer::isKeyword(std::string word)
{
	if (word == "server" || word == "location" || word == "listen" || word == "error_page" || word == "client_max_body_size" || word == "methods" || word == "return" || word == "root" || word == "autoindex" || word == "index" || word == "upload_enable" || word == "upload_store")
		return (true);
	return (false);
}
```

Using tokens like these significantly eases up parsing lines for the configuration. It allows for custom rule setting and eases up the workflow of the parser. In Pseudocode, a "rule" can look like this:
```c++
if (Token.type == Keyword)
{
	if (Token.value == "listen")
	// Set specific rules for collecting information on the listen keyword
		while (Token.value != ";") //All lines must end with a ";"
			if (Token.type != Number)
				// Listen keyword should only expect numbers. Thus any
				// type of information is invalid. Throw an Error
}
```

Just like in the HTTP Request parser, a state machine is used. State Machines are identified by keeping a certain state. Meaning it is not a function you call and returns everything all at once but rather reads in chunks and keeps track of the position.

Steps can be made through the configuration using the following member function:
`ConfigToken ConfigTokenizer::next()`

### Parser.
This parser is where tokens get translated into Data Object the program can later use. Initialization, validation and error management happens here.

The `ConfigParser` holds a `ConfigTokenizer` as an object in itself. It can therefore call the `next()` function and iterate through the configuration file word by word.

Because of the `ConfigToken` structure, rule sets for specific keywords is extremely simplified in syntax. An example of a rule can be seen in the snippet below:

*--from:* `src/config/parser/parser.cpp` *in* `ConfigParser::createServerConfig()`
```c++
		else if (pos_.value == "error_page")
		{
			pos_ = tokenizer_.next();
			if (pos_.type != ConfigToken::Number)
				throw ConfigException("Expected a status code after 'error_page'.");

			int code = parseInt(pos_.value);
			if (code < 400 || code > 599)
				throw ConfigException("Error page: " + pos_.value + " out of range (400-599)");

			ConfigToken temp_pos;
			temp_pos = tokenizer_.checkNext();
			if (temp_pos.type == ConfigToken::Number)
				throw ConfigException("Error number cannot be followed by another error number.");

			std::string path = join(collectUntil(";"));
			if (path.empty())
				throw ConfigException("Empty path in 'error_page' directive.");
			server_config.error_pages[code] = path;
		}

```
