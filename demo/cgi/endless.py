#!/usr/bin/env python3
import sys
import os

print("Content-Type: text/plain; charset=utf-8\r\n\r\n", end='')

content_length = os.environ.get('CONTENT_LENGTH')

# 無限ループでタイムアウトをテスト
while True:
    pass