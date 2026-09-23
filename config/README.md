# Configuration Manual
This manual describes the configuration syntax and available directives for configuring the web server.

Configuration is organized into two types of blocks:

- **Server blocks** define settings that apply to an individual server, such as the listening address, port, and default document root.

- **Location blocks** define settings that apply to requests matching a specific URI.

The configuration syntax is inspired by the style used by nginx.

>[!TIP]
>Comments can be added using the `#` character. Everything from `#` to the end of the line is ignored by the configuration parser.
>
>Comments may appear on their own line or after a configuration directive.

## Configuration Syntax
Configuration directives consist of a keyword followed by one or more arguments and must be terminated with a semicolon (`;`).

For example:
```
listen 8080;
root www;
```
Configuration blocks group directives that apply to a particular server or location:
```
server
{
    listen 8080;
    root www;
}
```
Opening and closing braces (`{` and `}`) define the beginning and end of a block.

# Server Settings
Multiple `server` blocks may be defined in a configuration file.

Each `server` keyword must be followed by a configuration block containing the settings for that server.

A `server` block cannot be nested inside another `server` block.
```
server
{
    hostname 127.0.0.1;
    listen 8080;
    root www;
}
```
## Mandatory Server Settings
Each server block must contain the following directives.

### - `hostname`
Specifies the IP address on which the server accepts connections.

The current implementation supports a single IPv4 address per server block.

**Syntax:**
```
hostname <IPv4-address>;
```
---
### - `listen`
Specifies the TCP port on which the server listens for incoming connections.

The port must be within the range supported by the web server.

**Syntax:**
```
listen <port>;
```
---
### - `root`
Specifies the default directory from which files are served.

Unless overridden by a location-specific root directive, requests handled by the server use this directory as their document root.

**Syntax:**
```
root <directory>;
```
---
## Complete Server Example
The following configuration defines two independent servers:
```
# First server
server
{
    hostname 0.0.0.0;
    listen 8080;
    root www;
}

# Second server
server
{
    hostname 127.0.0.1;
    listen 8082;
    root temp/;
}
```
## Optional Server Settings
The following directives can be used to customize the behavior of a server block.

### - `error_page`
Specifies the resource to return when a particular HTTP error occurs.

**Syntax:**
```
error_page <status-code> <URI>;
```
Example:
```
error_page 404 /error/404.html;
```
In this example, requests resulting in a 404 Not Found error are handled using /error/404.html.

---
### - `client_max_body_size`
Sets the maximum size of the HTTP request body accepted by the server.

The value may be specified using the following units:
- `kb` for kilobytes
- `mb` for megabytes
- `gb` for gigabytes

**Syntax:**
```
client_max_body_size <size>;
```
**Example:**
```
client_max_body_size 1gb;
```
---
### - `index`
Specifies the default file to serve when a request targets a directory.

If no index directive is specified, `index.html` is used.

**Syntax:**
```
index <filename>;
```
---
# Location Settings
A location block defines configuration for requests matching a specific URI.

The location keyword must be followed by a URI and a configuration block.

**Syntax:**
```
location <URI>
{
    <directives>
}
```
**Example:**
```
location /temp
{
    # Location-specific directives can be added here.
}
```
Location settings can override applicable server-level settings for matching requests.

## Optional Location Settings
### - `methods`
Specifies which HTTP methods are allowed for requests matching the location.

The currently supported methods are:
- GET
- POST
- DELETE

**Syntax:**
```
methods <method> [<method> ...];
```
**Example:**
```
methods GET POST;
```
This configuration allows both GET and POST requests while rejecting other methods.

---
### - `return`
Returns an HTTP response with the specified status code and, optionally, a target URI.

This can be used to redirect requests from one URI to another.

**Syntax:**
```
return <status-code> <URI>;
```
**Example:**
```
return 301 /new_path_to_take;
```
In this example, a request matching the location is redirected to /new_path_to_take using HTTP status code 301.

---
### - `root`
Overrides the server-level document root for the specified location.

This is useful when a particular URI should serve files from a different directory.

For example:
```
server
{
    hostname 127.0.0.1;
    listen 8080;
    root www;

    location /image
    {
        root /data;
    }
}
```
With the configuration above, a request for:
```
/image/someimage.png
```
is mapped to:
```
/data/someimage.png
```
**Syntax:**
```
root <directory>;
```
**Example:**
```
root /data;
```
---
### - `index`
Specifies the default index file for a particular location.

When a request targets a directory, the configured index filename is appended to the requested path.

**Syntax:**
```
index <filename>;
```
**Example:**
```
index elementary.html;
```
This directive overrides the server-level index setting for the matching location.

---
# Complete Configuration Example
The following example demonstrates server-level and location-level configuration together:
```
server
{
    hostname 127.0.0.1;
    listen 8080;
    root www;

    client_max_body_size 1gb;
    index index.html;

    error_page 404 /error/404.html;

    location /images
    {
        root /data;
        methods GET;
        index images.html;
    }

    location /old
    {
        return 301 /new;
    }
}
```
In this example:
- The server listens on 127.0.0.1:8080.
- www is used as the default document root.
- Request bodies are limited to 1gb.
- index.html is used as the default index file.
- A custom page is configured for 404 errors.
- Requests under /images use /data as their root and only allow GET.
- Requests under /old are redirected to /new.

# Future Implementations
The following directives are planned for future versions:

- upload_enable — Enable file uploads.
- upload_store — Configure where uploaded files are stored.
- auto_index — Automatically generate directory listings.

These directives are currently not supported and must not be used in configuration files.
