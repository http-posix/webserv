```text
The logger writes to the file only what passes through Logger::Log.
External stderr output does not end up there. 
To intercept it at startup:
```

`./webserv config/cgi 2>&1 | tee -a run.log`
