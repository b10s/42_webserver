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
