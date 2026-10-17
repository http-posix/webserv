import os
try:
    os.fstat(int(os.environ["QUERY_STRING"]))
    print("open")
except OSError:
    print("closed")
