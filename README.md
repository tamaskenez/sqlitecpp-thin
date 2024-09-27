# SQLITECPP-THIN

Thin C++ wrapper for the SQLite C API.

The goal of this wrapper is to expose the SQLite API to C++ in a straightforward way, staying as close as possible to the original C API. There are no fancy C++ abstractions, even the SQLite C API documentation remains applicable.

Key features:

- Support for C++ types.
- C++ error handling patterns: exceptions and `std::expected`
- RAII on SQLite objects.

## Error handling

`std::expected` is in many ways superior to exceptions. However, due to the lack of native language support, `std::expected` can be cumbersome to use.

For this reason, the library is available in two versions — one using `std::expected` and the other using exceptions.

## Status

Currently, the library covers only the SQLite functionality I’ve used in my own projects. However, expanding it is straightforward since the C to C++ API mapping is one-to-one, and the fundamental patterns are already in place. Contributions are welcome!
