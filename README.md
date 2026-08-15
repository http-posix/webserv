# webserv

## Architecture & Ownership

- **Event Loop & Connection Handling** — `Tetiana`
- **CGI Execution** — `Tetiana`
- **Build System & Conventions** — `Tetiana`
- **HTTP Parsing** — `Zach`
- **Request Routing & Response Generation** — `Zach`
- **Configuration** — `Zach`

## Build

**make**              — build project  
**make DBG=1**        — build with debug logging  
**make DBG=1 LOG=1**  — build with debug logging and log file  
**make test**         — run all tests in tests/  
**make demo**         — run demos in tests/demo  
**make test_clean**   — remove test object files and binary  
**make re**           — full rebuild  
**make clean**        — remove object files  
**make fclean**       — remove object files and binary  

