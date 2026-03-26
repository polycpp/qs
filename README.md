# polycpp/qs

C++ port of [npm qs](https://github.com/ljharb/qs) for [polycpp](https://github.com/enricohuang/polycpp).

A query string parsing and stringifying library with support for nested objects, arrays, and configurable encoding/decoding.

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

## License

MIT - see [LICENSE](LICENSE).
