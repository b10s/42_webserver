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