import os
for k in ("GATEWAY_INTERFACE", "REQUEST_METHOD", "QUERY_STRING", "SCRIPT_NAME",
          "SERVER_PORT", "HTTP_HOST", "HTTP_AUTHORIZATION", "HTTP_PROXY"):
    print(k + "=" + os.environ.get(k, "<unset>"))
print("CWD=" + os.getcwd())
