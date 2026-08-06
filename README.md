![Build and Test](https://github.com/tamaskenez/sqlitecpp-thin/actions/workflows/build-and-test.yml/badge.svg)

# SQLITECPP-THIN

Thin C++ wrapper for the SQLite C API.

The goal of this wrapper is to expose the SQLite API to C++ in a straightforward way, staying as close as possible to the original C API. There are no fancy C++ abstractions, even the SQLite C API documentation remains applicable.

## Key features:

- Support for C++ types.
- Error handling with `std::expected`, no exceptions.
- RAII on SQLite objects.
- The wrapper does not try to abstract away the C API: for example, `bind_blob` and `bind_text` remain two separate functions.

### Support for C++ types.

`bind_*` and `column_*` functions accept and return `std::string`, `std::string_view`, `std::span<const std::byte>`

### Error handling

Every fallible function returns `std::expected<T, E>`, the library never throws.

The error type is `sqlite::current_error` for functions which report the error on an existing database connection. It's a lightweight object which holds the primary error code and the database handle, and reads the remaining error fields (extended error code, message, offset) from the database on demand. That's why it must be processed right away and never stored: a subsequent operation on the same database overwrites those fields. Call `get_error()` to obtain a self-contained `sqlite::error` if you need to keep it.

Functions which have no database connection to report on, like `sqlite::open()`, return `sqlite::error` directly.

Both error types have a `format()` function returning a human-readable message.

### Straightforward mapping to the C API

It's always obvious which underlying SQLite-C function gets called. Unlike in other SQLite/C++ wrappers, overloads don't obscure important details, like difference between `bind_blob` and `bind_text`.

## Status

Currently, the library covers only the SQLite functionality I’ve used in my own projects. However, expanding it is straightforward since the C to C++ API mapping is one-to-one, and the fundamental patterns are already in place. Contributions are welcome!
