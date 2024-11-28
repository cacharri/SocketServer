#!/usr/bin/python3

import os
import socket
from time import sleep

current_user = socket.gethostname()

html_content = f"""
<!DOCTYPE html>
<html lang="en">
<head>
    <title>User</title>
</head>
<body>
    <h1>Hello, {current_user}!</h1>
</body>
</html>
"""


sleep(2)
print(html_content)