*This project has been created as part of the 42 curriculum by ssoeno, takitaga, aenshin.*

[![cpp-linter](https://github.com/b10s/42_webserver/actions/workflows/cpp-linter.yml/badge.svg)](https://github.com/b10s/42_webserver/actions/workflows/cpp-linter.yml)

## Description
- an HTTP server in C++ 98.
- epoll based
- GET/POST/DELETE
- CGI support

## Instructions

### Build Requirements
- **C++ Compiler**: g++ (>= 9.0) or clang++ (>= 10.0)
- **Build System**: GNU Make
- **Standard Libraries**: C++98 compatible standard library

### System Requirements
- **Operating System**: Linux (epoll support required)
- **Architecture**: x86_64 or compatible

### Installation on Ubuntu/Debian
```bash
sudo apt update
sudo apt install build-essential g++ make clang-format clang-tidy python3 python3-pip
# if needed, sudo apt install -y netcat-openbsd
```
### Execution
```bash
make
./webserv demo/conf/webserv_eval.conf
```

### Test Command
```bash
make test
```
- Source files are located in the `srcs` directory, and corresponding test files should be placed under the `tests` directory.
- Name all test files with the `.test.cpp` extension.

## Resources
- RFC9110, 9112
- nginx documentation https://nginx.org/en/docs/
- How AI was used
    - to review pull requests
    - to clarify RFC interpretation
    - for documentation improvement

## Coding Tools Usage

### Setup

Install **clang-format** using your package manager such as `brew` or `apt`.

### Formatting Command

```bash
make format
```

### Linting Command

```bash
make tidy
```

If you want to fix lint errors automatically, use following command.

```bash
make tidy-fix
```

## Testing with curl
Open different terminal and run following curl commands to test the server in a dev container:

```bash
# Confirm it listens
lsof -nP -iTCP -sTCP:LISTEN | grep webserv
# or
netstat -an | grep LISTEN | grep <PORT>

# Basic checks
# listens on two ports, e.g. 8080 and 8081:
curl -v http://127.0.0.1:8080/
curl -v http://127.0.0.1:8081/

# Port conflict behavior (coherent and non-crashing)
# Terminal A
./webserv demo/conf/webserv_eval.conf
# Terminal B: start another instance using same interface:port
./webserv demo/conf/webserv_eval.conf
# Expect: clean error (bind failure), no crash, clear message.

# Must not crash / never hang indefinitely”
# Unknown method: expect 501(not implemented)
printf "PUT / HTTP/1.1\r\nHost: localhost\r\n\r\n" | nc -v 127.0.0.1 <PORT>
# Slow client / partial request should not hang
# send headers slowly
{ printf "GET / HTTP/1.1\r\nHost: localhost\r\n"; sleep 5; printf "\r\n"; } | nc -v 127.0.0.1 <PORT>
# Client disconnect during response
( printf "GET /bigfile HTTP/1.1\r\nHost: localhost\r\n\r\n"; sleep 0.1 ) | nc -v 127.0.0.1 <PORT> >/dev/null

# GET / POST / DELETE work
# GET a file
curl -v http://127.0.0.1:<PORT>/index.html
curl -I http://127.0.0.1:<PORT>/index.html   # headers only
# GET a directory (index vs autoindex)
curl -v http://127.0.0.1:<PORT>/
# curl -v http://127.0.0.1:8080/dirlist

# chuncked transfer encoding
(
printf "POST /upload/test.txt HTTP/1.1\r\nHost: localhost\r\nTransfer-Encoding: chunked\r\n\r\n5\r\nHello\r\n"
sleep 1
printf "6\r\n World\r\n0\r\n\r\n"
) | nc -v 127.0.0.1 8080

# POST upload (file)
curl -X POST --data-binary @README.md http://localhost:8080/upload/readme.txt

# DELETE a resource
curl -X DELETE -v http://localhost:8080/upload/readme.txt
# Expect: 200 + file removed.

# Accurate status codes” + default error pages
# 404 (and custom error page if configured)
curl -v http://127.0.0.1:8080/this_does_not_exist
# Expect: 404 and your default/custom error page body.
# 405 (method not allowed for a route)
curl -v -X DELETE http://127.0.0.1:<PORT>/
# Expect: 405
# 413 (client_max_body_size)
# generate 2MB body (change size to exceed your configured limit)
python3 - <<'PY'
print("A"*2_000_000)
PY | curl -v -X POST http://127.0.0.1:8080/upload \
      -H "Content-Type: text/plain" \
      --data-binary @-
# Expect: 413 when above limit.

# Redirection works”
curl -v http://127.0.0.1:8080/redirect
# Expect: 301/302 + Location header
# Follow redirect:
curl -v -L http://127.0.0.1:8080/redirect

# CGI (GET, POST, error handling, timeout)
curl localhost:8080/cgi/hello.py
curl localhost:8080/cgi/hello_post.py -d "test data"
curl localhost:8080/cgi/endless.py -d "test data"

# Siege stress test + availability
# Install and run:
sudo apt install siege
# create file for test
echo "Hello World" > demo/static_sites/demo_site/empty.html
# basic benchmark mode (-b), 50 clients, delay 1s, 10 repetitions
siege -b -c50 -d1 -r10 http://127.0.0.1:8080/empty.html
# longer run
siege -b -c50 -d1 -t30S http://127.0.0.1:8080/empty.html
# Check memory doesn’t grow indefinitely:
watch -n 1 "ps -o pid,rss,vsz,command -p \$(pgrep webserv)"
```

testerでのチェック
( printf "GET / HTTP/1.1\r\nHost: localhost:8080\r\n\r\nPOST / HTTP/1.1\r\nHost: localhost:8080\r\nContent-Length: 0\r\n\r\n" ) | nc -v 127.0.0.1 8080
testerは1接続で複数リクエストを処理する（レスポンス送信後に HttpRequest をリセットする）ことを求めている

スコープ外にするか相談
- multipart/form-dataのアップロードは対応しない
- absolute-form も対応しない
    printf "GET http://127.0.0.1:8080/cgi/hello.py HTTP/1.1\r\nHost: 127.0.0.1:8080\r\n\r\n" \
    | nc -v 127.0.0.1 8080
    -> Bad request
    subjectには"The HTTP 1.0 is suggested as a reference point, but not enforced."とあるのでdefence可能？
