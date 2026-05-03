# API Mapping

| Upstream symbol | C++ symbol | Status | Notes |
|---|---|---|---|
| `qs.parse(str, opts)` | `polycpp::qs::parse(const std::string&, const ParseOptions&)` | direct | Synchronous parse into a nested object; C++ returns `polycpp::JsonValue` with a top-level `JsonObject`. |
| `qs.stringify(obj, opts)` | `polycpp::qs::stringify(const JsonValue&, const StringifyOptions&)` | direct | Synchronous stringify for `JsonValue` object roots with supported option fields. |
| parse options object | `polycpp::qs::ParseOptions` | adapted | Typed struct preserves supported option names and defaults; JavaScript-only dynamic fields are unavailable. |
| stringify options object | `polycpp::qs::StringifyOptions` | adapted | Typed struct preserves supported option names and defaults, including callback hooks that operate on `JsonValue` strings/trees. |
| `duplicates` values `combine`, `first`, `last` | `polycpp::qs::Duplicates` | direct | Closed string union maps to a scoped enum. |
| `arrayFormat` values `indices`, `brackets`, `repeat`, `comma` | `polycpp::qs::ArrayFormat` | adapted | Closed string union maps to a scoped enum; scalar array behavior matches, while comma-format nested object/array coercion is a documented C++ divergence. |
| `formats.RFC3986` and `formats.RFC1738` | `polycpp::qs::Format` | adapted | C++ exposes the format selector as an enum instead of a mutable `formats` object. |
| `allowDots`, `decodeDotInKeys`, bracket depth parsing | `ParseOptions::allowDots`, `ParseOptions::decodeDotInKeys`, `ParseOptions::depth` | direct | Supported, including double-encoded in-key dot preservation with `%252E` and single-encoded separator dots with `%2E`. |
| `parameterLimit`, `throwOnLimitExceeded` | matching `ParseOptions` fields | direct | Supported for parameter caps and fail-closed parser limit errors. |
| `arrayLimit` | `ParseOptions::arrayLimit` | adapted | Explicit index overflow, comma-array overflow, and duplicate implicit-array throw behavior match `qs@6.15.1`; non-throwing duplicate overflow does not retain upstream's hidden side-channel metadata for every merge shape. |
| `charset` values `utf-8`, `iso-8859-1` | `ParseOptions::charset`, `StringifyOptions::charset` | direct | Static charset selection is supported for parse and stringify. |
| custom parse `decoder` | `ParseOptions::decoder` | adapted | C++ callback receives the raw component plus `DecodeContext`; `defaultDecode()` is available for upstream-style fallback decoding. |
| `charsetSentinel`, `interpretNumericEntities` | matching `ParseOptions` and `StringifyOptions::charsetSentinel` fields | direct | Sentinel-driven charset switching/output and ISO-8859-1 numeric entity interpretation are supported. |
| `plainObjects`, `allowPrototypes`, `strictMerge` | none | omitted | These are JavaScript object-shape and prototype-chain controls that do not map to `JsonValue`; `__proto__` is still dropped defensively. |
| custom stringify `encoder`, `formatter`, `filter`, `sort`, `serializeDate` | matching `StringifyOptions` fields | adapted | Encoder/formatter/filter/sort operate during `JsonValue` traversal; `serializeDate` is honored by the explicit `toQsValue(Date, opts)` adapter. |
| deprecated stringify `indices` | `StringifyOptions::indices` | adapted | Compatibility switch selects repeat arrays when false while `arrayFormat` is left at its default. |
| Date and Buffer stringify values | `qs::toQsValue(const Date&, opts)`, `qs::toQsValue(const buffer::Buffer&, encoding)`, `toString()` adapter | adapted | `JsonValue` cannot carry arbitrary objects, so Date/Buffer and other polycpp `toString()` objects are converted explicitly before `stringify()`. |
| Symbol, BigInt, `undefined`, function stringify values | none | omitted | These JavaScript-only runtime values are not representable in `JsonValue`. |
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
- Declaration-only globals, caches, deprecated fields, or runtime-specific surfaces mapped as unsupported/not-applicable: deprecated `indices` was found in `lib/stringify.js` runtime defaults and is implemented as a compatibility field.

## Framework object boundary review

- Upstream reads or mutates framework/request/response/context objects: none.
- Upstream fields or methods read: none.
- Upstream fields or methods written: none.
- C++ adapter boundary: pure functions over `std::string`, `ParseOptions`, `JsonValue`, and `StringifyOptions`; downstream `express` owns request/response integration.
- Partial mutation risk on validation failure: none because parse/stringify do not mutate external framework objects.

## Node parity surface review

- Callback APIs: custom parse/stringify callbacks are adapted as typed `std::function` fields.
- Promise APIs: none.
- EventEmitter APIs: none.
- Server/listener APIs: none; base polycpp listener primitives were inspected before marking this not applicable.
- Diagnostic/tracing APIs: none.
- Stream APIs: none.
- Buffer and binary APIs: upstream Buffer stringify is available through the explicit `toQsValue(buffer)` adapter because `JsonValue` has no Buffer payload.
- URL, timer, process, and filesystem APIs: none.
- Crypto, compression, TLS, network, and HTTP APIs: none.
- Unsupported or non-meaningful Node-specific APIs and audit reason: CommonJS packaging, browser metadata, JS dynamic type validation, prototype-shape options, and Symbol/BigInt/function/`undefined` values are not meaningful in the typed C++ API. Date/Buffer and callback hooks are adapted explicitly.
