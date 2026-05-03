# Divergences From Upstream

## Adapted Features

- Parse `charsetSentinel`, `interpretNumericEntities`, and custom `decoder` are implemented through typed `ParseOptions` fields.
- Stringify `charsetSentinel`, custom `encoder`, `formatter`, `filter`, `sort`, and deprecated `indices` are implemented through typed `StringifyOptions` fields.
- `serializeDate` is implemented for explicit Date adaptation through `qs::toQsValue(date, opts)` because `JsonValue` cannot store Date objects directly.
- Upstream Buffer stringify is implemented through `qs::toQsValue(buffer)` because `JsonValue` cannot store Buffer objects directly.
- polycpp classes with a JavaScript-like `toString()` can be adapted with the generic `qs::toQsValue(value)` helper.
- A public `formats` object equivalent to `qs.formats` is not exposed; `Format` covers the supported selector.

## Deliberate Behavior Changes

- The C++ API uses typed option structs and scoped enums instead of JavaScript option dictionaries and string literal unions.
- Parse returns `polycpp::JsonValue` with `JsonObject`/`JsonArray` values, not mutable JavaScript objects.
- Stringify accepts `JsonValue`; JavaScript values such as function, Symbol, BigInt, `undefined`, and arbitrary object instances are outside v0 unless callers adapt them to a `JsonValue` first.
- JavaScript option validation errors for invalid dynamic option types are compile-time type issues in C++.
- `allowPrototypes`, `plainObjects`, and `strictMerge` are not exposed because JavaScript prototype mutation and null-prototype object shapes are not part of the C++ object model.
- The C++ implementation does not ship upstream `side-channel`; it uses clean-room value-tree helpers for supported behavior. Non-throwing duplicate overflow does not retain hidden overflow metadata for every merge shape.
- Comma-format stringify does not coerce nested array/object elements to JavaScript's `[object Object]` string form; scalar `JsonValue` entries are emitted and nested values are skipped.

## Unsupported Runtime-Specific Features

- CommonJS module identity, browser package metadata, npm lifecycle scripts, and bundler behavior are not part of the C++ companion surface.
- JavaScript prototype pollution controls are adapted to C++ object semantics; `__proto__` remains dropped defensively, but there is no C++ prototype chain.
- Upstream runtime behavior for Symbol, BigInt, functions, and `undefined` is unsupported in v0 because those values are not representable in `JsonValue`; Date and Buffer use explicit adapters.

## Audit findings (libgen catch-up)

| ID | Severity | Status | Description | Resolution |
|---|---|---|---|---|
| AF-2026-05-03-A | medium | fixed | Array indexes equal to `arrayLimit` were parsed as arrays, but upstream `qs@6.15.1` treats index `arrayLimit` as over limit and as an object key or `RangeError` when `throwOnLimitExceeded` is enabled. | Parser now requires numeric indexes to be `< arrayLimit`; tests cover `a[2]=b` with `arrayLimit=2` in object and throw modes. |
| AF-2026-05-03-B | medium | fixed | `decodeDotInKeys` decoded `%2E` before dot splitting, losing the distinction between separator dots and double-encoded in-key dots such as `name%252Eobj.first`. | Parser now decodes keys once before dot splitting and restores `%2E` inside key segments; tests cover double-encoded in-key dots. |
| AF-2026-05-03-C | medium | fixed | Comma parsing split values without enforcing `arrayLimit`; upstream throws or converts to an overflow object when comma parts exceed the limit. | Comma values longer than `arrayLimit` now throw or become numeric-key overflow objects; tests cover both paths, plus duplicate implicit-array limit throws. |
| AF-2026-05-03-D | low | documented divergence | Comma-format stringify ignores object/array elements inside arrays, while upstream JavaScript joins them through string conversion such as `[object Object]`. | Retained as a typed `JsonValue` behavior difference and documented in the public known-differences guide. |
| AF-2026-05-03-E | low | fixed | The decode-dot round-trip test comment said the single-encoded `%2E` behavior was the full npm round-trip story; upstream only preserves in-key dots on the default encoded stringify path, which emits `%252E`. | Stringify now emits `%252E` for encoded keys, and tests cover both encoded round-trip preservation and `encode=false` single-encoded nesting behavior. |
| AF-2026-05-03-F | medium | fixed | Deferred upstream parity notes predated libgen and missed current polycpp Date/Buffer/toString support. | Added charset sentinel and numeric entity parsing, custom decoder/encoder/formatter/filter/sort hooks, deprecated `indices`, stringify charset sentinels, and explicit Date/Buffer/toString adapters with tests. |
