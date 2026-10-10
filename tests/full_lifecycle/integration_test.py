import subprocess
import time
import socket

class WebservRunner:
    def __init__(self, config, host, port):
        self.cmd = ["./webserv", config]
        self.host = host
        self.port = port
        self.proc = None

    def __enter__(self):
        self.proc = subprocess.Popen(self.cmd, cwd = "../../")
        start = time.time()
        while time.time() - start < 3.0:
            try:
                s = socket.create_connection([host, port])
                s.close()
                return self
            except:
                time.sleep(0.02)
        self.proc.terminate()

    def __exit__(self, exc_type, exc_value, exc_traceback):
        self.proc.terminate()
        self.proc.wait()

config = "config/default_config"
host = "127.0.0.1"
port = 8080
resource = "about.html"
url = f"{host}:{port}/{resource}"

with WebservRunner(config, host, port):
    subprocess.run(["curl", "-v", url])

