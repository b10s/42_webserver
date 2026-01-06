# WEBSERV

[![cpp-linter](https://github.com/b10s/42_webserver/actions/workflows/cpp-linter.yml/badge.svg)](https://github.com/b10s/42_webserver/actions/workflows/cpp-linter.yml)

## How to Run the program

```bash
make
./webserv [config file]
```

## How to Run Tests

### Test Command

```bash
make test
```

### Notes

- Create one test file per source file.
- Source files are located in the `srcs` directory, and corresponding test files should be placed under the `tests` directory.
- Name all test files with the `.test.cpp` extension.

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
## Sample Configuration and Testing with curl
```bash
./webserv
```

Open different terminal and run following curl commands to test the server in a dev container:

```bash
curl localhost:8080
curl localhost:8080/welcome.html
curl localhost:8081
curl localhost:8081/cgi/hello.py
curl localhost:8081/cgi/hello_post.py -d "test data"
curl localhost:8081/cgi/endless.py -d "test data"
```
