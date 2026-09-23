# Instruction manual for setting up configuration files.

Configurations can be set for servers and locations.  

## Server Settings
Multiple servers are allowed and are identified by the `server` keyword. 
Each `server` keyword must be followed by a code block (`{`, `}`) to mark
the start and end of the configuration for the specific server. 
Obviously, a server block within a server block is invalid.

> [!caution]
> Mandatory Settings.
Each `server` block must contain at least the following keywords (settings):
- `hostname`:
    The IP-adress on which the server shall be running.
    Current implementation allows only a single IPv4 adress per server block.
    Usage: `hostname 127.0.0.1;`
- `listen`: 
    The port on which the server will be listening.
    The port must be fit within the range.
    Usage: `listen 8080;`
- `root`:
    The default folder in which the server shall process all (default) requests.
    Usage: `root www;`

Combining all lists from above a valid config file would be:
```
server
{
    hostname 0.0.0.0;
    listen 8080;
    root www;
}
server
{
    hostname 127.0.0.1;
    listen 8082;
    root temp/
}
```

The following keywords are optional and allow for customization:
- `error_page`:
    Allows for returning specific text in case of certain errors.
    Usage: `error_page 404 /error/404.html;`



- server
- location

- hostname
- listen
- error\_page
- client\_max\_body\_size
- methods
- return
- root
- autoindex
- index
- upload\_enable
- upload\_store
