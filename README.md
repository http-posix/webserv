*This project has been created as part of the 42 curriculum by tchernia and zivanov*

# webserv

## Architecture & Ownership

Design decisions are documented as ADRs in [`adr/`](adr/).

### ➡️ Tetiana
- Build System & Conventions
- Event Loop & Connection Handling 
- CGI Execution 
- Logging

### ➡️ Zach
- HTTP Parsing 
- Request Routing & Response Generation 
- HTTP Error Responses & Error Pages
- Configuration

### 🔥 Shared Responsibilities 🔥
- Planning & Backlog Management
- Testing
- Integration & Code Review

## Build

```text
make                # build the project
make re             # full rebuild
make DBG=1          # build with debug logging
make DBG=1 LOG=1    # build with debug logging and log file
```

## Run

```text
./webserv                 # default: config/default_config
./webserv [config_file]   # e.g. config/<name>
```

## Test

```text
make test           # run all tests in tests/
make demo           # run demos in tests/demo
```

## Clean

```text
make clean          # remove object files
make fclean         # remove object files and binary
make test_clean     # remove test object files and test binary
```