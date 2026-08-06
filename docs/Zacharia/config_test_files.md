## Config Test Files (10-07)
Random config files to understand how workflow of ConfigParser will be:
### 1. 
```
server {
    listen 8002;
	server_name localhost;
    host 127.0.0.1;
    root docs/fusion_web/;
    # client_max_body_size 3000000;
	index index.html;
    error_page 404 error_pages/404.html;

    location / {
        allow_methods  DELETE POST GET;
        autoindex off;
    }
    
    location /tours {
        autoindex on;
        index tours1.html;
        allow_methods GET POST PUT HEAD;
    }

	location /red {
		return /tours;
	}

    location /cgi-bin {
        root ./;
        allow_methods GET POST DELETE;
        index time.py;
        cgi_path /usr/bin/python3 /bin/bash;
        cgi_ext .py .sh;
    }
}
```

### 2.
```
server {
	listen 3333;
	server_name fury.com;
	error_page 404 error_page/404.html;
	error_page 403 error_page/403.html;
	error_page 400 error_page/400.html;
	error_page 405 error_page/405.html;
	error_page 500 502 error_page/50x.html;

	root #<path/to/42webserv> + /www/Groco;
	client_max_body_size 3000000;

	location / {
		method GET;
		autoindex off;
		index index.html;
	}
	location /upload { 
		root #<path/to/42webserv> + /;
		index index.html;
		method GET POST;
		upload_path #<path/to/42webserv> + /upload;
	}
	location /cgi-bin {
		root #<path/to/42webserv> + /;
		autoindex on;
		method GET POST;
		cgi_path /usr/bin/python;
		# cgi_path /usr/local/bin/node;
	}
}

server {
	listen 2222;
	server_name localhost;
	error_page 404 error_page/404.html;
	error_page 400 error_page/400.html;
	error_page 405 error_page/405.html;
	error_page 500 502 error_page/50x.html;

	root #<path/to/42webserv> + /www/cookie;
	client_max_body_size 100000;

	location / {
		method GET;
		autoindex off;
		index index.html;
	}
	location /cgi-bin {
		root #<path/to/42webserv> + /;
		method GET POST;
		cgi_path /usr/bin/python3;
	}
}

server {
	listen 5555;
	server_name localhost;
	error_page 404 error_page/404.html;
	error_page 400 error_page/400.html;
	error_page 405 error_page/405.html;
	error_page 500 502 error_page/50x.html;

	root #<path/to/42webserv> + /www/Image_editor;
	client_max_body_size 100000;

	location / {
		method GET;
		autoindex off;
		index index.html;
	}
	location /cgi-bin {
		root #<path/to/42webserv> + /;
		method GET POST;
		cgi_path /usr/bin/python3;
	}
}

server {
	listen 9090;
	server_name localhost;
	error_page 404 error_page/404.html;
	error_page 400 error_page/400.html;
	error_page 405 error_page/405.html;
	error_page 500 502 error_page/50x.html;

	root #<path/to/42webserv> + /www/Memory_game;
	client_max_body_size 100000;

	location / {
		method GET;
		autoindex off;
		index index.html;
	}
	location /cgi-bin {
		root #<path/to/42webserv> + /;
		method GET POST;
		cgi_path /usr/bin/python3;
	}
}

server {
	listen 6262;
	server_name localhost;
	error_page 404 error_page/404.html;
	error_page 400 error_page/400.html;
	error_page 405 error_page/405.html;
	error_page 500 502 error_page/50x.html;

	root #<path/to/42webserv> + /www/Quiz_App;
	client_max_body_size 100000;

	location / {
		method GET;
		autoindex off;
		index index.html;
	}
	location /cgi-bin {
		root #<path/to/42webserv> + /;
		method GET POST;
		cgi_path /usr/bin/python3;
	}
}

server {
	listen 7070;
	server_name localhost;
	error_page 404 error_page/404.html;
	error_page 400 error_page/400.html;
	error_page 405 error_page/405.html;
	error_page 500 502 error_page/50x.html;

	root #<path/to/42webserv> + /www/;
	client_max_body_size 100000;

	location / {
		method GET;
		autoindex off;
		index index.html;
	}
	location /cgi-bin {
		root #<path/to/42webserv> + /;
		method GET POST;
		cgi_path /usr/bin/python3;
	}
}

server {
	listen 8081;
	server_name localhost;
	error_page 404 error_page/404.html;
	error_page 400 error_page/400.html;
	error_page 403 error_page/403.html;
	error_page 405 error_page/405.html;
	error_page 500 502 error_page/50x.html;

	root #<path/to/42webserv> + /www/tmp;
	client_max_body_size 100000000;


	location / {
		method GET POST DELETE;
		autoindex on;
	}
	location /cgi-bin {
		autoindex on;
		root #<path/to/42webserv> + /;
		method GET POST;
		cgi_path /usr/local/bin/node;
	}
	location /1337 {
		method GET;
		return 302 http://www/ar;
    }
	location palestine {
		method GET;
		return 302 https://www.youtube.com/watch?v=l8qay1Al7Dc;
	}
}
```

### 3.
https://github.com/marbenMB/WebServ
```
server 
{
    listen 0.0.0.0:8000;
    server_name 127.0.0.1;

    root ./public;
    index  index.html;
    error_page 404 /errors/40x.html;
    error_page 500 502 503 504 errors/50x.html;
    client_max_body_size 2G;

    location  %.go$ {
        allow_methods [GET POST];
        fastcgi_pass ./source/cgi_files/cgi_script;
    }
    location %.py$ {
       allow_methods [GET POST];
        fastcgi_pass ./source/cgi_files/cgi_script;
    }

    location / {
        allow_methods GET POST;
        autoindex on;
    }
    
    location /upload {
        allow_methods [GET POST DELETE];
        autoindex on;
    }
 
    location /assets {
        allow_methods GET;
        autoindex on;
    }
    location /yo {
        allow_methods [GET POST DELETE];
        autoindex on;
        return 301 https://www.youtube.com/watch?v=eLaIVnI-N6Q;
    }
}
```

### 4.
https://github.com/harshkumbhani/42webserv
```
# General configuration file
http {
	server {
		keepalive_timeout 	15s; # in seconds
		send_timeout		10s; # in seconds
		listen				8000;
		server_name			localhost;
		root				www;
		autoindex			on;
		#index				index.html;
		directory_listing	on;
		client_body_size	3000000; # in bytes
		location / {
			methods			GET DELETE;
		}
		location /upload {
			methods			GET POST;
		}

		location /redirect {
			methods			GET;
			redirect		surf.inc/de/;
		}

		location /cgi {
			methods			GET POST;
		}
	}
	server {
		listen 8080;
		root www;
		server_name			localhost;
		index	main.py;
		location / {
			methods GET POST DELETE;
		}
		location /upload {
			methods			GET POST DELETE;
		}
	}
	server {
		listen 8080;
		root www;
		server_name	127.0.0.1;
		index	main.py;
		location / {
			methods GET;
		}
	}
}
```

### 5.
https://github.com/swangarch/42webserv
```
server {
    listen 127.0.0.1:8080;
    server_name webserv.com;
    root /www/site/web1;
    client_max_body_size 500000;
    autoindex on;
    index index.html;
    error_page 404 error_pages/404.html;
    error_page 403 error_pages/403.html;
    method GET POST DELETE HEAD;
    cgi_extention .py;
    cgi_path /usr/bin/python3;
    cgi_extention .php;
    cgi_path /usr/bin/php;
    cgi_extention .pl;
    cgi_path /usr/bin/perl;

    location / {
        upload_destination /data;
    }

    location /redir {
        method GET POST HEAD;
        error_page 404 /another.html;
        return 308 /data;
    }

    location /getnotallowed {
        method DELETE;
        error_page 404 /another.html;
        return 307 /data;
    }

    location /redirexample {
        method GET POST;
        error_page 404 /another.html;
        return 307 https://example.com/;
    }

    location /data {
        method GET DELETE;
        index ;
        error_page 404 /404.html;
        autoindex on;
    }

    location /data/users {
        method ;
        autoindex off;
    }

    location /data/sessions {
        method ;
        autoindex off;
    }

    location /upload {
        method GET POST DELETE;
        client_max_body_size 50000000;
        error_page 404 /404.html;
        autoindex on;
        upload_destination /data;
    }

    location /cgi {
        method GET DELETE POST;
        error_page 404 /404.html;
        autoindex on;
        upload_destination /data;
    }

    location /assets {
        method GET POST;
        index ;
        error_page 404 /another.html;
        autoindex off;
    }
}
```

### 6.
https://github.com/FreddyMSchubert/42_webserv
```
server {
	listen	127.0.0.1:8080;
	server_name test.wooo yourmum.gay kindergartenhumore.true clicker.com;
	root /www/clicker/;
	index test.php noidea.html index.html;
	max_package_size 10MB;
	error_page 404 /error_pages/404.html;
	client_timeout 60s;

	location /{
		allowed_methods GET POST DELETE;
		cgi_extension py "python3 [SCRIPT]";
		cgi_extension php "php-cgi [SCRIPT]";
	}

	location /assets {
		allowed_methods GET POST DELETE;
		autoindex off;
		upload_dir /uploads;
	}

	# location /outcommmented {
	# 	randombs;
	# 	but shouldnt matter;
	# 	cause its outcommmented;
	# }
}

server {
	listen	127.0.0.1:8081;
	server_name kindergartenhumore.false platformer.com testing.yourmum;
	root /www/platformer/;
	index test.php noidea.html index.html;
	max_package_size 10mb;
	error_page 404 /404/404.html;
	client_timeout 60000ms;

	location / {
		allowed_methods GET POST DELETE;
	}

	location /assets {
		allowed_methods GET POST DELETE;
		autoindex off;
	}

	location /test {
		return 518 /404.html;
	}

	# location /outcommmented {
	# 	randombs;
	# 	but shouldnt matter;
	# 	cause its outcommmented;
	# }
}

server {
	listen	127.0.0.1:4242;
	server_name tetris.com kindergartenhumore.true;
	root /www/tetris/;
	index test.php noidea.html index.html;
	max_package_size 10mb;
	error_page 404 404.html;
	client_timeout 60000ms;

	location / {
		allowed_methods GET POST DELETE;
		autoindex off;
	}

	# location /outcommmented {
	# 	randombs;
	# 	but shouldnt matter;
	# 	cause its outcommmented;
	# }
}

server {
	listen	127.0.0.1:4243;
	root /www/cgi;
	index index.html;
	max_package_size 10mb;

	location / {
		allowed_methods GET POST DELETE;
		autoindex off;
		cgi_extension py "/Users/fschuber/.brew/bin/python3 [SCRIPT]";
	}
}
```

### 7.
https://github.com/RealConrad/42webserv
```
http {
	server_timeout_time       10000
	server {
		index                   index.html
		server_name             localhost
		listen                  8080
		root                    www
		keepalive_timeout		31
		send_timeout			11
		max_body_size           100000000
		directory_listing       true
		location / {
			request_types       GET POST
		}
		location /google {
			request_types       GET
			redirection			www.google.com
		}
		location /uploads {
			request_types       GET POST DELETE
		}
		location /topsecret {
			redirection			https://www.topsecret.pl/
		}
	}
	server {
		index                   index.html
		server_name             127.0.0.1
		listen                  8080
		root                    www
		keepalive_timeout		61
		send_timeout			31
		max_body_size           200
		directory_listing       false
		location / {
			request_types       GET POST
		}
		location /google {
			request_types       GET
			redirection			www.google.com
		}
		location /uploads {
			request_types       GET POST
		}
		location /topsecret {
			redirection			https://www.topsecret.pl/
		}
	}
	server {
		index                   index.html
		server_name             localhost
		listen                  8081
		root                    www
		keepalive_timeout		91
		send_timeout			91
		max_body_size           100000
		directory_listing       true
		location / {
			request_types       GET
		}
		location /google {
			request_types       GET
			redirection			www.google.com
		}
		location /topsecret {
			redirection			https://www.topsecret.pl/
		}
	}
	server {
		index                   better_index.html
		server_name             127.0.0.1
		listen                  8081
		root                    www
		keepalive_timeout		121
		send_timeout			999
		max_body_size           200
		directory_listing       false
		location / {
			request_types       GET
		}
		location /google {
			request_types       GET
			redirection			www.google.com
		}
		location /topsecret {
			redirection			https://www.topsecret.pl/
		}
	}
}
```

### 8.
https://github.com/angirov/42webserv
```
timeout: 20
max_clients: 10
client_max_body_size: 100000

<server>
    listen: 80
    server_name: default.local
    error_page: /data/errorPages/defaultServer/404.html

    <location>
        route: /
        root: /data/eval/
        index: index.html
        methods: GET, POST, DELETE
        return:
        returnCode:
        autoindex: off
        cgi: py
        uploadDir: /data/uploads
    </location>

    <location>
        route: /colorful
        root: /data/root1
        index: index.html
        methods: GET
        return:
        returnCode:
        autoindex: off
        cgi: py
        uploadDir: /data/uploads
    </location>

    <location>
        route: /uploads
        root: /data
        index:
        methods: GET, POST, DELETE
        return:
        returnCode:
        autoindex: on
        cgi: py
        uploadDir: /data/uploads
    </location>

    <location>
        route: /cgi-bin
        root: /webserv
        index:
        methods: GET, POST
        return:
        returnCode:
        autoindex: off
        cgi: py
        uploadDir: /data/uploads
    </location>

    <location>
        route: /blog
        root: /data
        index:
        methods: GET
        return:
        returnCode:
        autoindex: on
        cgi: py
        uploadDir: /data/uploads
    </location>
</server>

<server>
    listen: 8080
    server_name: redirect.local
    error_page: /data/errorPages/redirectServer/404.html

    <location>
        route: /
        root: /data/eval/
        index: index.html
        methods: GET, POST
        return:
        returnCode:
        autoindex: on
        cgi: py
        uploadDir: /data/uploads
    </location>

    <location>
            route: /git_vladimir
            root:
            index:
            methods: GET
            return: https://github.com/angirov
            returnCode: 301
            autoindex: off
            cgi:
            uploadDir:
    </location>

    <location>
            route: /git_max
            root:
            index:
            methods: GET
            return: https://github.com/mwagner86
            returnCode: 301
            autoindex: off
            cgi:
            uploadDir:
    </location>

    <location>
            route: /git_kolja
            root:
            index:
            methods: GET
            return: https://github.com/Apelidas
            returnCode: 301
            autoindex: off
            cgi:
            uploadDir:
    </location>

    <location>
            route: /42wolfsburg
            root:
            index:
            methods: GET
            return: https://42wolfsburg.de/
            returnCode: 301
            autoindex: off
            cgi:
            uploadDir:
    </location>

    <location>
            route: /columbia301
            root:
            index:
            methods: GET
            return: http://www.columbia.edu/~fdc/sample.html
            returnCode: 301
            autoindex: off
            cgi:
            uploadDir:
    </location>

    <location>
            route: /columbia302
            root:
            index:
            methods: GET
            return: http://www.columbia.edu/~fdc/sample.html
            returnCode: 302
            autoindex: off
            cgi:
            uploadDir:
    </location>

    <location>
            route: /columbia303
            root:
            index:
            methods: GET
            return: http://www.columbia.edu/~fdc/sample.html
            returnCode: 303
            autoindex: off
            cgi:
            uploadDir:
    </location>

    <location>
            route: /columbia304
            root:
            index:
            methods: GET
            return: http://www.columbia.edu/~fdc/sample.html
            returnCode: 304
            autoindex: off
            cgi:
            uploadDir:
    </location>

    <location>
            route: /columbia307
            root:
            index:
            methods: GET
            return: http://www.columbia.edu/~fdc/sample.html
            returnCode: 307
            autoindex: off
            cgi:
            uploadDir:
    </location>

</server>

<server>
    listen: 8040
    server_name: indexserver.local
    error_page: /data/errorPages/defaultServer/404.html

    <location>
        route: /
        root: /data/eval/
        index: index.html
        methods:
        return:
        returnCode:
        autoindex: on
        cgi: py
        uploadDir: /data/uploads
    </location>

    <location>
        route: /colorful
        root: /data/root1
        index: index.html
        methods: GET
        return:
        returnCode:
        autoindex: on
        cgi: py
        uploadDir: /data/uploads
    </location>

    <location>
        route: /colorfulaion
        root: /data/root1
        index:
        methods: GET
        return:
        returnCode:
        autoindex: on
        cgi: py
        uploadDir: /data/uploads
    </location>

    <location>
        route: /colorfulaioff
        root: /data/root1
        index:
        methods: GET, POST, DELETE
        return:
        returnCode:
        autoindex: off
        cgi: py
        uploadDir: /data/uploads
    </location>      

</server>
```
### 9.
https://github.com/PepeSegura/42webserv
```
server {
    listen localhost:8080;
    server_name hola.com;
	root $PWD/html/;
	index index.html index.php;
    location / {
        index index.html index.php;
    }
}

server {
    listen localhost:8080;
    server_name adios.com;
    autoindex true;
    client_max_body_size 15K;
	root $PWD;
    cgi_pass pl /usr/bin/perl;
    location /hola {
		root /hola;
        index index.html index.php;
		cgi_pass py /bin/py;
    }
}
```
### 10.
https://github.com/MarkosComK/42Webserver
```
server {
	listen 127.0.0.1:8080;
	listen 127.0.0.2:8081;
	listen 8082; # defaults host to 0.0.0.0 (nginx behavior), making the server accessible from any IP address on the machine
	#listen 127.0.0.1; # no port = default to 80 (nginx behavior)
	#listen 127.0.0.1:4242; # trying a closed port or one already in use returns a bind error, but the server still starts and listens on the other valid ports, which is the expected behavior

	# Set up default error pages
	error_page 404 /errors/404.html;

	# Set the maximum allowed size for client request bodies.
	client_max_body_size 10M;

	# Default root for the server, used if no root is specified in a location block
	root www;

	# Specify rules or configurations on a URL/route (no regex required here), for a website, among the following:
	location / {
		# List of accepted HTTP methods for the route.
		methods GET;
		# Directory where the requested file should be located (e.g., if URL /kapouet is rooted to /tmp/www, URL /kapouet/pouic/toto/pouet will search for /tmp/www/pouic/toto/pouet).
		#root www;
		# Default file to serve when the requested resource is a directory.
		index index.html;
		# Enabling or disabling directory listing.
		autoindex on;
	}

	# Uploading files from the clients to the server is authorized, and storage location is provided.
	location /upload {
		methods GET POST DELETE;
		root www;
		index upload.html;
		upload_store www/uploads;
	}

	location /uploads {
		methods GET;
		root www/uploads;
		autoindex on;
	}

	location /old-page {
		# HTTP redirection
		return 301 /new-page;
	}

	# Execution of CGI, based on file extension (for example .php).
	location /cgi-bin {
		methods GET POST;
		root www/cgi-bin;
		cgi .php /usr/bin/php-cgi;
		cgi .py /usr/bin/python3;
	}

}

server {
	listen 127.0.0.1:9999;
	listen 127.0.0.1:8080;
	error_page 404 /errors/404.html;
	client_max_body_size 1M;

	location / {
		methods GET;
		root www;
		index index.html;
		autoindex on;
	}
}
```
