# API Mapping

| Upstream symbol | C++ symbol | Status | Notes |
|---|---|---|---|
| `qs.parse(str, opts)` | `polycpp::qs::parse(const std::string&, const ParseOptions&)` | direct | Synchronous parse into a nested object; C++ returns `polycpp::JsonValue` with a top-level `JsonObject`. |
| `qs.stringify(obj, opts)` | `polycpp::qs::stringify(const JsonValue&, const StringifyOptions&)` | direct | Synchronous stringify for `JsonValue` object roots with supported option fields. |
| parse options object | `polycpp::qs::ParseOptions` | adapted | Typed struct preserves supported option names and defaults; JavaScript-only dynamic fields are unavailable. |
| stringify options object | `polycpp::qs::StringifyOptions` | adapted | Typed struct preserves supported option names and defaults; callback/value-type options are deferred or omitted. |
| `duplicates` values `combine`, `first`, `last` | `polycpp::qs::Duplicates` | direct | Closed string union maps to a scoped enum. |
| `arrayFormat` values `indices`, `brackets`, `repeat`, `comma` | `polycpp::qs::ArrayFormat` | adapted | Closed string union maps to a scoped enum; scalar array behavior matches, while comma-format nested object/array coercion is a documented C++ divergence. |
| `formats.RFC3986` and `formats.RFC1738` | `polycpp::qs::Format` | adapted | C++ exposes the format selector as an enum instead of a mutable `formats` object. |
| `allowDots`, `decodeDotInKeys`, bracket depth parsing | `ParseOptions::allowDots`, `ParseOptions::decodeDotInKeys`, `ParseOptions::depth` | direct | Supported, including double-encoded in-key dot preservation with `%252E` and single-encoded separator dots with `%2E`. |
| `parameterLimit`, `throwOnLimitExceeded` | matching `ParseOptions` fields | direct | Supported for parameter caps and fail-closed parser limit errors. |
| `arrayLimit` | `ParseOptions::arrayLimit` | adapted | Explicit index overflow, comma-array overflow, and duplicate implicit-array throw behavior match `qs@6.15.1`; non-throwing duplicate overflow does not retain upstream's hidden side-channel metadata for every merge shape. |
| `charset` values `utf-8`, `iso-8859-1` | `ParseOptions::charset`, `StringifyOptions::charset` | adapted | Static charset selection is supported; sentinel-driven charset switching is deferred. |
| custom parse `decoder` | none | deferred | JavaScript callback hook is not exposed in v0. |
| `charsetSentinel`, `interpretNumericEntities` | none | deferred | Sentinel-driven charset switching and numeric entity interpretation are upstream parse/stringify parity surfaces deferred behind explicit charset selection. |
| `plainObjects`, `allowPrototypes`, `strictMerge` | none | omitted | These are JavaScript object-shape and prototype-chain controls that do not map to `JsonValue`; `__proto__` is still dropped defensively. |
| custom stringify `encoder`, `formatter`, `filter`, `sort`, `serializeDate` | none | deferred | JavaScript callback hooks and Date handling do not map to the current typed API. |
| Date, Buffer, Symbol, BigInt, `undefined` stringify values | `JsonValue` subset | omitted | `JsonValue` supports null, bool, number, string, array, and object; other JS values are not representable in v0. |
| CommonJS package surface and browser metadata | CMake target `polycpp::qs` and public header `<polycpp/qs/qs.hpp>` | adapted | C++ package boundary follows companion-library conventions. |

Status values:

- `direct`: same behavior with an idiomatic C++ spelling.
- `compatibility layer`: same user-facing behavior through a different implementation shape.
- `adapted`: preserves the upstream intent with a typed C++ API.
- `deferred`: planned future work for an upstream surface or explicitly accepted C++ extension that is intentionally not implemented yet.
- `omitted`: deliberately not part of this port.

## TypeScript Declaration Review

- Declaration source used: none; upstream `qs@6.15.1` does not ship TypeScript declarations in source or the published npm package.
- Public APIs, overloads, options, callbacks, streams, or literal unions found only or most clearly in declarations: none.
- Declaration-only globals, caches, deprecated fields, or runtime-specific surfaces mapped as unsupported/not-applicable: not applicable; deprecated `indices` was found in `lib/stringify.js` runtime defaults and is deferred.

## Framework object boundary review

- Upstream reads or mutates framework/request/response/context objects: none.
- Upstream fields or methods read: none.
- Upstream fields or methods written: none.
- C++ adapter boundary: pure functions over `std::string`, `ParseOptions`, `JsonValue`, and `StringifyOptions`; downstream `express` owns request/response integration.
- Partial mutation risk on validation failure: none because parse/stringify do not mutate external framework objects.

## Node parity surface review

- Callback APIs: none in target; dependency-only analyzer signal is not exposed.
- Promise APIs: none.
- EventEmitter APIs: none.
- Server/listener APIs: none; base polycpp listener primitives were inspected before marking this not applicable.
- Diagnostic/tracing APIs: none.
- Stream APIs: none.
- Buffer and binary APIs: upstream Buffer stringify is omitted because `JsonValue` has no Buffer payload.
- URL, timer, process, and filesystem APIs: none.
- Crypto, compression, TLS, network, and HTTP APIs: none.
- Unsupported or non-meaningful Node-specific APIs and audit reason: CommonJS packaging, browser metadata, JS dynamic type validation, prototype-shape options, Date/Buffer/Symbol/BigInt values, and callback hooks are not meaningful in the typed v0 C++ API or are deferred explicitly.
