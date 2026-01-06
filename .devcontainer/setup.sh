#!/bin/bash
set -e

# ディレクトリの作成
sudo mkdir -p /var/www/html/site_8080/data
sudo mkdir -p /var/www/html/site_8081/static
sudo mkdir -p /var/www/html/site_8081/scripts

# welcome.html の作成
sudo tee /var/www/html/site_8080/data/welcome.html > /dev/null <<EOF
<!DOCTYPE html>
<html>
<body>
    <h1>Welcome to Site 8080</h1>
</body>
</html>
EOF

# homepage.html の作成
sudo tee /var/www/html/site_8081/static/homepage.html > /dev/null <<EOF
<!DOCTYPE html>
<html>
<body>
    <h1>Static Homepage - Site 8081</h1>
</body>
</html>
EOF

# hello.py の作成
sudo tee /var/www/html/site_8081/scripts/hello.py > /dev/null <<EOF
#!/usr/bin/env python3
print("Content-Type: text/html\r\n\r\n", end='')
print("Hello, CGI!")
EOF

# hello_post.py の作成
sudo tee /var/www/html/site_8081/scripts/hello_post.py > /dev/null <<EOF
#!/usr/bin/env python3
import sys
import os

print("Content-Type: text/plain; charset=utf-8\r\n\r\n", end='')

content_length = os.environ.get('CONTENT_LENGTH')

if content_length:
    length = int(content_length)
    input_data = sys.stdin.read(length)
    print(f"Received data: {input_data}")
else:
    print("There is no data received.")
EOF

# endless.py の作成
sudo tee /var/www/html/site_8081/scripts/endless.py > /dev/null <<EOF
#!/usr/bin/env python3
import sys
import os

print("Content-Type: text/plain; charset=utf-8\r\n\r\n", end='')

content_length = os.environ.get('CONTENT_LENGTH')

while True:
    pass
EOF

# 権限の設定
sudo chmod +x /var/www/html/site_8081/scripts/hello.py
sudo chmod +x /var/www/html/site_8081/scripts/hello_post.py
sudo chmod +x /var/www/html/site_8081/scripts/endless.py
sudo chown -R vscode:vscode /var/www/html/site_8080 /var/www/html/site_8081
