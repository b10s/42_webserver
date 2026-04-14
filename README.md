*This project has been created as part of the 42 curriculum by ssoeno, takitaga, aenshin.*

[![cpp-linter](https://github.com/b10s/42_webserver/actions/workflows/cpp-linter.yml/badge.svg)](https://github.com/b10s/42_webserver/actions/workflows/cpp-linter.yml)

## Description

This project implements an HTTP server in C++98. It receives requests from clients (such as browsers) and returns appropriate responses. It supports methods such as GET, POST, and DELETE, serves static files, executes CGI scripts, and returns proper HTTP status codes and headers.

The server uses an event-driven architecture based on `epoll` to efficiently handle multiple connections concurrently. `epoll` monitors multiple file descriptors and notifies the server when they are ready for I/O operations, allowing non-blocking and scalable request handling.

### Core Architecture

- **Single event loop** handling:
  - client connections
  - request parsing
  - response sending

- **Non-blocking I/O**:
  - I/O operations are performed only when `epoll` signals readiness
  - Read/write operations continue until `EAGAIN` or `EWOULDBLOCK`, then control returns to the event loop

- **Main loop design**:
  - `epoll` is placed in the main loop to continuously monitor all file descriptors for both read and write events without blocking

### CGI

- CGI is handled using `fork()` and `execve()`

### Error Handling

All I/O operations (`read`, `recv`, `write`, `send`) properly check return values:

- `> 0`: success  
- `0`: connection closed  
- `-1`: error (handled appropriately, e.g., closing the connection)

Control flow is driven primarily by syscall return values. When an operation fails, the implementation may also consult `errno` to distinguish error conditions (for example, retryable non-blocking cases versus other failures) and to map filesystem or syscall errors to appropriate HTTP status codes. 

### How it works
1. epoll_wait monitors all file descriptors
2. On read event:
   - parse HTTP request incrementally
3. On write event:
   - send response buffer
4. CGI:
   - fork + execve
   - communicate via pipes

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
- RFC9110, 9112, 3875
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

## Configuration

The server supports:

- multiple ports
- route-based configuration
- allowed methods per route
- file upload directory
- CGI execution by extension
- autoindex on/off

## Testing with curl
Open different terminal and run following curl commands to test the server in a dev container:

### Basic tests
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
./webserv demo/conf/webserv_eval_browser.conf
# Terminal B: start another instance using same interface:port
./webserv demo/conf/webserv_eval_browser.conf
# Expect: clean error (bind failure), no crash, clear message.

# Must not crash / never hang indefinitely”
# Unknown method: expect 501(not implemented)
printf "PUT / HTTP/1.1\r\nHost: localhost\r\n\r\n" | nc -v 127.0.0.1 <PORT>
# Slow client / partial request should not hang
# send headers slowly
{ printf "GET / HTTP/1.1\r\nHost: localhost\r\n"; sleep 5; printf "\r\n"; } | nc -v 127.0.0.1 <PORT>

# GET / POST / DELETE work
# GET a file
curl -v http://127.0.0.1:<PORT>/index.html
# GET a directory (index vs autoindex)
curl -v http://127.0.0.1:<PORT>/
# Autoindex listing
curl -v http://127.0.0.1:8080/dirlist

# chunked transfer encoding
(
printf "POST /upload/test.txt HTTP/1.1\r\nHost: localhost\r\nTransfer-Encoding: chunked\r\n\r\n5\r\nHello\r\n"
sleep 1
printf "6\r\n World\r\n0\r\n\r\n"
) | nc -v 127.0.0.1 8080

# POST upload (file)
curl -X POST --data-binary @cat.jpg http://localhost:8080/upload/cat.jpg

# DELETE a resource
curl -X DELETE -v http://localhost:8080/upload/cat.jpg
# Expect: 200 + file removed.
```

### Verifying Accurate Status Codes and Default Error Pages

#### 200 OK
Test a valid static file:
```bash
curl -i http://127.0.0.1:8080/static/a.txt
```
Expected result:
* `200 OK`
* File content is returned

#### 201 Created
Test file upload via POST:
```bash
curl -i -X POST http://127.0.0.1:8080/upload/test.txt \
  -H "Content-Type: text/plain" \
  --data "hello world"
```
Expected result:
* `201 Created`
* `Location` header is set (if implemented)
* File is created on the server

#### 301 / 302 Redirect
Test redirection:
```bash
curl -i http://127.0.0.1:8080/redirect
```
Expected result:
* `301 Moved Permanently` or `302 Found`
* `Location` header is present
Follow redirect:
```bash
curl -L http://127.0.0.1:8080/redirect
```

#### 400 Bad Request
Send an invalid HTTP request:
```bash
printf "GET / HTTP/1.1\r\n\r\n" | nc 127.0.0.1 8080
```
Expected result:
* `400 Bad Request`
* Server does not crash

#### 403 Forbidden
Test a directory with no index file and autoindex disabled:
```bash
curl -i http://127.0.0.1:8080/static/
```
Expected result:
* `403 Forbidden`

#### 404 Not Found

Test a non-existent path:
```bash
curl -i http://127.0.0.1:8080/this_does_not_exist
```
Expected result:
* `404 Not Found`
* Default or custom error page is returned

#### 405 Method Not Allowed
Test a disallowed method:
```bash
curl -i -X DELETE http://127.0.0.1:8080/
```
Expected result:
* `405 Method Not Allowed`
* `Allow` header should be present

#### 411 Length Required
Send a POST request without `Content-Length`:
```bash
printf "POST /upload HTTP/1.1\r\nHost: localhost\r\n\r\n" | nc 127.0.0.1 8080
```
* `411 Length Required` would normally be appropriate, 
but the tester expects POST requests without body headers to be treated as having a content length of 0. 
Therefore, we handle them accordingly.

#### 413 Payload Too Large
Send a request body exceeding `client_max_body_size`:
```bash
python3 - <<'PY' | curl -i -X POST http://127.0.0.1:8080/upload \
  -H "Content-Type: text/plain" \
  --data-binary @-
print("A" * 2000000)
PY
```
Expected result:
* `413 Payload Too Large`

#### Directory Redirect (Trailing Slash)
Test missing trailing slash:
```bash
curl -i http://127.0.0.1:8080/dirlist
```
Expected result:
* `301 Moved Permanently`
* Redirects to `/dirlist/`

### Redirection works
```bash
curl -v http://127.0.0.1:8080/redirect
```
Expected result:
* 301/302 + Location header
Follow redirect:
* curl -v -L http://127.0.0.1:8080/redirect

### CGI (GET, POST, error handling, timeout)
curl localhost:8080/cgi/hello.py
curl localhost:8080/cgi/hello_post.py -d "test data"
curl localhost:8080/cgi/endless.py -d "test data"

### Siege stress test + availability
#### Install and run:
sudo apt install siege
#### create file for test
echo "Hello World" > demo/static_sites/demo_site/empty.html
#### basic benchmark mode (-b), 50 clients, delay 1s, 10 repetitions
siege -b -c50 -d1 -r10 http://127.0.0.1:8080/empty.html
#### longer run
siege -b -c50 -d1 -t30S http://127.0.0.1:8080/empty.html
#### Check memory doesn’t grow indefinitely:
watch -n 1 "ps -o pid,rss,vsz,command -p \$(pgrep webserv)"

## Limitations

- multipart/form-data is not supported
- absolute-form requests are not supported
- percent-decoding is not implemented
    printf "GET http://127.0.0.1:8080/cgi/hello.py HTTP/1.1\r\nHost: 127.0.0.1:8080\r\n\r\n" \
    | nc -v 127.0.0.1 8080
    -> Bad request
