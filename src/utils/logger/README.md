## Logger

A simple singleton logger for development and debugging.  
Uses the Meyers Singleton patter

**Build flags:**  
make DBG=1 — logs to terminal  
make DBG=1 LOG=1 — logs to terminal and to log/ directory  

**Macros:**  
LOG_DEBUG("message") 
LOG_INFO("message")  
LOG_WARN("message")  
LOG_ERROR("message")  

Each macro automatically records at the call site:
 - file name
 - line number
 - function name

**PrintMsg:**  
Logger::PrintMsg("message") — static helper for simple output, no instance required.