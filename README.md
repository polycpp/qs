# polycpp/qs

C++ port of [npm qs](https://github.com/ljharb/qs) for [polycpp](https://github.com/enricohuang/polycpp).

A query string parsing and stringifying library with support for nested objects, arrays, and configurable encoding/decoding.

Port version: `0.1.0`
Initial port based on upstream version: `6.15.1`

## Prerequisites

- C++20 compiler (GCC 13+ or Clang 16+)
- CMake 3.20+
- Ninja (recommended)

## Build

```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j$(nproc)
cd build && ctest --output-on-failure
```

Build runnable examples with:

```bash
cmake -B build -G Ninja -DPOLYCPP_QS_BUILD_EXAMPLES=ON
cmake --build build --target polycpp_qs_example_parse_roundtrip
```

## Usage

```cpp
#include <polycpp/qs/qs.hpp>
#include <iostream>

int main() {
    using namespace polycpp;

    // Parse a query string
    auto result = qs::parse("a=b&c[d]=e&f[]=g&f[]=h");
    // result["a"]          -> "b"
    // result["c"]["d"]     -> "e"
    // result["f"]          -> ["g", "h"]

    // Parse with options
    qs::ParseOptions opts;
    opts.allowDots = true;
    auto dotted = qs::parse("a.b.c=d", opts);
    // dotted["a"]["b"]["c"] -> "d"

    // Stringify
    JsonValue obj = JsonObject{
        {"a", "b"},
        {"c", JsonArray{1, 2, 3}}
    };
    auto str = qs::stringify(obj);
    // str == "a=b&c%5B0%5D=1&c%5B1%5D=2&c%5B2%5D=3"

    // Stringify with options
    qs::StringifyOptions sopts;
    sopts.addQueryPrefix = true;
    sopts.arrayFormat = qs::ArrayFormat::brackets;
    auto bracketed = qs::stringify(obj, sopts);
    // bracketed == "?a=b&c%5B%5D=1&c%5B%5D=2&c%5B%5D=3"

    return 0;
}
```

## API

### `qs::parse(str, opts)`

Parse a URL query string into a nested `JsonValue` object.

### `qs::stringify(obj, opts)`

Serialize a `JsonValue` object into a URL query string.

See [qs documentation](https://github.com/ljharb/qs) for detailed option descriptions.

## Status

Implemented: parse and stringify for `JsonValue` objects; bracket and dot notation; duplicate handling; array formats; depth, parameter, and array limits; null handling; RFC1738/RFC3986 encoding; UTF-8 and ISO-8859-1 charset selection.

Known differences from upstream: JavaScript callback hooks, Date/Buffer/Symbol/BigInt values, `charsetSentinel`, `interpretNumericEntities`, `plainObjects`, and `allowPrototypes` are not part of the current C++ API. See `docs/sphinx/guides/known-differences.rst` for the public compatibility notes and `docs/divergences.md` for the libgen catch-up audit.

## License

MIT - see [LICENSE](LICENSE).
