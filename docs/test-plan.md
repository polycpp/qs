# Test Plan

## Unit tests

- `tests/test_qs.cpp` currently contains 92 GoogleTest cases covering basic parse/stringify behavior, nested objects, arrays, sparse compaction, duplicate keys, parameter limits, delimiters, null handling, numbers, booleans, encoding, dot notation, charsets, and round trips.
- Focused audit tests cover `arrayLimit` equality, comma-array limit overflow, duplicate implicit-array limit throws, double-encoded `decodeDotInKeys`, and default encoded `encodeDotInKeys` round-trips.
- Keep private helper behavior covered through public parse/stringify tests unless a helper becomes complex enough to justify a separate `tests/test_<helper>.cpp`.

## Integration tests

- Keep `examples/parse_roundtrip.cpp` as the public workflow test for parse -> stringify -> parse.
- Keep `examples/array_formats.cpp` as the public workflow test for the four array formats and comma round-trip option.
- Downstream integration is exercised by `express`, which includes `<polycpp/qs/qs.hpp>` for request query parsing and urlencoded body parsing; qs should remain dependency-free so downstream `FetchContent` stays simple.

## Compatibility tests adapted from upstream

- upstream compatibility layout: aggregate C++ file `tests/test_qs.cpp` currently adapts representative cases from upstream `test/parse.js`, `test/stringify.js`, `test/utils.js`, README examples, and `test/empty-keys-cases.js`.
- upstream-to-local coverage map:
  - `test/parse.js` simple strings, nesting, arrays, sparse arrays, depth, duplicates, limits, delimiters, empty values, and encoded brackets -> `tests/test_qs.cpp` `QsParseTest` and `QsEdgeCaseTest`.
  - `test/stringify.js` primitive values, nested objects, array formats, null handling, encoding, delimiters, and comma round trip -> `tests/test_qs.cpp` `QsStringifyTest`.
  - README parse/stringify examples -> `tests/test_qs.cpp` round-trip tests plus `examples/parse_roundtrip.cpp` and `examples/array_formats.cpp`.
  - `test/empty-keys-cases.js` default empty-key dropping -> partially covered by leading/trailing delimiter tests; full fixture table should be added.
- omitted upstream cases:
  - Invalid option type tests are not meaningful because C++ option fields are typed.
  - `allowPrototypes` and `plainObjects` tests are runtime-object-shape behavior and are intentionally omitted.
  - Custom encoder/decoder/filter/sort/serializeDate cases are deferred.
  - Date, Buffer, Symbol, BigInt, function, and `undefined` stringify cases are omitted for v0 because `JsonValue` cannot represent those JavaScript values.
  - Browser bundling and npm packaging tests are not applicable to the C++ companion.

## Security and fail-closed tests

- Existing coverage includes `strictDepth`, `parameterLimit` with `throwOnLimitExceeded`, `arrayLimit` overflow greater than and equal to the limit, comma-array overflow with `throwOnLimitExceeded`, duplicate implicit-array overflow with `throwOnLimitExceeded`, sparse compaction, and `__proto__` dropping.
- Add abuse-oriented tests for very deep input with `strictDepth=false` and `strictDepth=true`, large parameter lists at the exact limit boundary, malformed percent escapes, and prototype-like keys such as `constructor` and `toString` with documented C++ semantics.

## Protocol/client tests

Not applicable because qs is not a database, cache, queue, cloud-service, or wire-protocol client.

- service-backed e2e matrix: not applicable.
- transport matrix, including TLS/compression decisions: not applicable.
- auth or credential flow matrix: not applicable.
- malformed packet / unsupported mode tests: not applicable.
- binary payload type-mapping tests: not applicable.
- stateful parser/session-state tests: not applicable.
- server/listener response writer loopback tests: not applicable.
- multi-result or unread-packet drain behavior: not applicable.
- pool/session lifecycle tests: not applicable.

## Release-blocking behaviors

- Public headers compile with only `<polycpp/qs/qs.hpp>`.
- `cmake -B build -G Ninja -DFETCHCONTENT_SOURCE_DIR_POLYCPP=<polycpp-checkout>` configures.
- `cmake --build build -j$(nproc)` builds library, tests, and examples when enabled.
- `ctest --test-dir build --output-on-failure` passes.
- `python3 docs/build.py` builds Doxygen/Sphinx docs without warnings.
- All known upstream parity gaps are either fixed or recorded in `docs/divergences.md`.

## Current validation

Record exact commands run, service versions, and notable environment variables.

- 2026-05-03: `python3 <libgen>/scripts/check-port-readiness.py --baseline <repo>` passed.
- 2026-05-03: `python3 <libgen>/scripts/intake-upstream.py <repo> https://github.com/ljharb/qs.git` cloned/updated upstream at revision `a0a81ea2071acce3eff41a040f719ac8f5c4f64c`.
- 2026-05-03: `python3 <libgen>/scripts/intake-npm-package.py <repo> qs` extracted published `qs@6.15.1`.
- 2026-05-03: `python3 <libgen>/scripts/analyze-upstream-js.py <repo> <repo>/.tmp/npm-package` wrote `.tmp/dependency-analysis.json` and emitted one handled warning.
- 2026-05-03: `python3 <libgen>/scripts/check-port-readiness.py --strict <repo>` passed after private repository visibility was verified.
- 2026-05-03: `cmake -B build-libgen -G Ninja -DFETCHCONTENT_SOURCE_DIR_POLYCPP=<polycpp-checkout> -DPOLYCPP_QS_BUILD_EXAMPLES=ON` passed after changing the GoogleTest FetchContent URL to HTTPS. GCC 11.4.0 emitted the base polycpp warning that GCC 12+ is recommended.
- 2026-05-03: `cmake --build build-libgen -j$(nproc)` passed. Fetched `zstd` legacy C sources emitted `-Wmaybe-uninitialized` warnings; qs sources built cleanly.
- 2026-05-03: `ctest --test-dir build-libgen --output-on-failure` passed: 92/92 tests after the parser limit and encoded-dot compatibility fixes.
- 2026-05-03: `./build-libgen/examples/array_formats` and `./build-libgen/examples/parse_roundtrip 'a[b]=1&a[c]=2&tags[]=c&tags[]=cpp'` passed and matched docs examples.
- 2026-05-03: `python3 docs/build.py` passed with Doxygen and Sphinx `-W --keep-going`.
- 2026-05-03: `python3 <libgen>/scripts/check-port-validation.py <repo>` passed after the docs build.
