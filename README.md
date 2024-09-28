![Build and Test](https://github.com/tamaskenez/sqlitecpp-thin/actions/workflows/build-and-test.yml/badge.svg)

# SQLITECPP-THIN

Thin C++ wrapper for the SQLite C API.

The goal of this wrapper is to expose the SQLite API to C++ in a straightforward way, staying as close as possible to the original C API. There are no fancy C++ abstractions, even the SQLite C API documentation remains applicable.

## Key features:

- Support for C++ types.
- C++ error handling patterns: exceptions and `std::expected`
- RAII on SQLite objects.
- The wrapper does not try to abstract away the C API: for example, `bind_blob` and `bind_text` remain two separate functions.

### Support for C++ types.

`bind_*` and `column_*` functions accept and return `std::string`, `std::string_view`, `std::span<const std::byte>`

### Error handling

`std::expected` is in many ways superior to exceptions. However, due to the lack of native language support, `std::expected` can be cumbersome to use.

For this reason, the library is available in two versions — one using `std::expected` and the other using exceptions.

They are compiled from the very same sources, the `std::expected<T, E>` return types become simply `T` in the exception-throwing version.

### Straightforward mapping to the C API

It's always obvious which underlying SQLite-C function gets called. Unlike in other SQLite/C++ wrappers, overloads don't obscure important details, like difference between `bind_blob` and `bind_text`.

## Status

Currently, the library covers only the SQLite functionality I’ve used in my own projects. However, expanding it is straightforward since the C to C++ API mapping is one-to-one, and the fundamental patterns are already in place. Contributions are welcome!
