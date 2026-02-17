*This project has been created as part of the 42 curriculum by ssoeno, takiga, aenshin.*

[![cpp-linter](https://github.com/b10s/42_webserver/actions/workflows/cpp-linter.yml/badge.svg)](https://github.com/b10s/42_webserver/actions/workflows/cpp-linter.yml)

## Description
- HTTP/1.1 server
- epoll based
- GET/POST/DELETE
- CGI support

## Instructions

```bash
make
./webserv [config file]
```
依存関係も記載が必要

## Testing strategy

### Test Command

```bash
make test
```
- Source files are located in the `srcs` directory, and corresponding test files should be placed under the `tests` directory.
- Name all test files with the `.test.cpp` extension.

## Resources
参考にしたドキュメント
RFC
記事
チュートリアル
How AI was used
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
```bash
make
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
