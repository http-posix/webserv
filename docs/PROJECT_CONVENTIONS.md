
## 1. Coding Standards

This project follows Google-inspired C++ naming and formatting conventions.

The project uses the C++17 standard exclusively.

## 2. File Structure

Header and source files should be grouped into folders based on their
module or class.

All module folders must be placed inside the `src/` directory.

Include paths must be relative to `src/` to match the Makefile
configuration.

Example:

```cpp
#include "module_name/class_name.hpp"
```

The directory structure is organized as follows:

```
src/
├── main.cpp         # Entry point (only try-catch and run_app call)
├── app/             # Application startup and lifecycle orchestration
│   ├── run_app.cpp  # Initialization logic called by main
│   └── server/      # Server orchestration class
│       ├── server.cpp
│       └── server.hpp
├── config/          # Parsing and validation of configuration files
├── http/            # HTTP protocol logic (parsing, request, response, routing)
├── io/              # Network mechanisms and I/O handling (config-agnostic)
└── utils/           # Shared utilities (Logger, helper templates)
```

## 3. C++ Style Guide Summary

This project uses a consistent set of Google-inspired C++ naming and
formatting conventions adapted for the project's requirements.

### 3.1. Filenames

Use lowercase letters only and separate words with underscores (`_`).

Header files use the `.hpp` extension.  
Implementation files use the `.cpp` extension.

Examples:

```text
http_server.cpp
request_handler.hpp
```

### 3.2. PascalCase

PascalCase is used for:

* classes
* structs
* enums
* type aliases
* complex or non-trivial accessor/mutator functions

Examples:

```cpp
class HttpServer { ... };
struct RequestParams { ... };

int GetProcessedRequestCount() const;
void SetConnectionConfiguration();
```

### 3.3. snake_case

snake_case is used for:

* local variables
* function parameters
* namespaces
* simple accessor and mutator functions

Examples:

```cpp
int port_number = 8080;
std::string remote_address;

namespace web_server { ... }

int count() const;
void set_count(int count);
```

### 3.4. snake_case with trailing underscore

snake_case with a trailing underscore (`_`) is used for:

* private member variables
* protected member variables

Examples:

```cpp
class Parser {
 private:
  int status_code_;
  std::string buffer_;
};
```

Public member variables do not use a trailing underscore.

### 3.5. Constants

Constants with a fixed value during program execution use a leading `k`
followed by PascalCase.

Examples:

```cpp
const int kMaxConnections = 1000;
constexpr int kDefaultPort = 8080;
```

### 3.6. Comments

Use `//` for comments.

Start comments with a capital letter and leave a space after `//`.

Example:

```cpp
// Initialize the socket connection
```

## 4. Git Commit Convention

Commit messages follow a simplified conventional format:

```text
type(scope): short description
```

The scope should represent the module or directory being changed (e.g., io, http, config, app, utils, readme).

Examples:

```text
feat(http): add HTTP request parser
fix(io): resolve socket cleanup issue
docs(readme): update formatting section
refactor(app): extract application initialization logic
```

Common commit types:

- `feat` — add new functionality or a new feature
- `fix` — fix a bug or incorrect behaviour
- `refactor` — improve or reorganize code without changing behaviour
- `docs` — update documentation or comments
- `test` — add or improve tests
- `progress` — partial work that is not yet finished but should be saved or shared

## 5. Branch Naming Convention

Branch names should use lowercase letters and hyphens (-).

Recommended format:

```text
type/short-description
```

Examples:

```text
feat/http-parser
fix/socket-cleanup
refactor/request-handler
docs/readme-update
test/parser-tests
```


## References

- [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html)
- [Conventional Commits](https://www.conventionalcommits.org/en/v1.0.0/)
- [Conventional Git Branches](https://conventional-branch.github.io/)