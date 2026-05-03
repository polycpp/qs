# Research

- package: qs
- npm url: https://www.npmjs.com/package/qs
- source url: https://github.com/ljharb/qs.git
- upstream version basis: 6.15.1
- upstream revision analyzed: a0a81ea2071acce3eff41a040f719ac8f5c4f64c
- upstream default branch: main
- license: BSD-3-Clause
- license evidence: package.json license field and upstream `LICENSE.md`
- category: foundational query-string parser and serializer utility

## Package purpose

`qs` parses URL query strings into nested object and array structures and serializes nested values back to query strings. The compatibility value is package-specific behavior: bracket parsing, dot notation, duplicate handling, depth and parameter limits, comma arrays, null handling, and RFC1738/RFC3986 formatting.

## Runtime assumptions

- browser: upstream is browser-compatible JavaScript with no DOM dependency.
- node.js: upstream is CommonJS and supports Node.js engines `>=0.6`; runtime dependency `side-channel` is used by stringify cycle detection and parse overflow metadata.
- filesystem: none in runtime code.
- network: none.
- crypto: none.
- terminal: none.

## Dependency summary

- package.json present: yes
- package main: `lib/index.js`
- package exports: none declared
- package types: none shipped in the package
- package bin: none
- hard dependencies: `side-channel@^1.1.0`
- peer dependencies: none detected in package.json
- optional dependencies: none detected in package.json
- published npm artifact path: `.tmp/npm-package`
- dependency analysis report: `docs/dependency-analysis.md`

## Upstream repo layout summary

Clone path used for analysis: `<repo>/.tmp/upstream/qs`

Top-level runtime and validation layout:

- `lib/index.js`: CommonJS entry exporting parse, stringify, and formats.
- `lib/parse.js`: parse defaults, option normalization, value splitting, key segmentation, array/object construction, depth and parameter limits.
- `lib/stringify.js`: stringify defaults, option normalization, array-prefix generation, side-channel cycle detection, date/filter/sort/custom encoder handling.
- `lib/utils.js`: percent encode/decode, array/object merge and compaction, overflow tracking through side-channel, Buffer detection.
- `lib/formats.js`: RFC1738 and RFC3986 format constants and formatter functions.
- `test/parse.js`: main parse compatibility suite.
- `test/stringify.js`: main stringify compatibility suite.
- `test/utils.js`: merge, compact, encode/decode helper behavior.
- `test/empty-keys-cases.js`: shared empty-key fixtures.
- `README.md`: user-facing option and behavior examples.

## Entry points used by consumers

- `require('qs')` via package main `lib/index.js`.
- `qs.parse(str, opts)`.
- `qs.stringify(obj, opts)`.
- `qs.formats` constants: `RFC1738` and `RFC3986`.
- TypeScript declarations inspected: upstream ships no declarations and the published artifact has no `.d.ts`; API evidence came from package runtime files, README, and tests.

## Important files and why they matter

- `.tmp/upstream/qs/lib/parse.js`: source of parse option defaults and edge-case semantics.
- `.tmp/upstream/qs/lib/stringify.js`: source of stringify option defaults and callback/value surfaces such as custom filters, sort callbacks, Date, Buffer, Symbol, and BigInt handling.
- `.tmp/upstream/qs/lib/utils.js`: source of encoding, decoding, merge, compaction, and overflow semantics.
- `.tmp/upstream/qs/lib/formats.js`: maps format names to space handling.
- `.tmp/upstream/qs/test/parse.js`: primary compatibility evidence for parse behavior.
- `.tmp/upstream/qs/test/stringify.js`: primary compatibility evidence for stringify behavior.
- `.tmp/upstream/qs/test/empty-keys-cases.js`: exact empty-key fixture source.

## Files likely irrelevant to the C++ port

- `logos/*`: branding assets only.
- `bower.json`, `component.json`, `tea.yaml`: package ecosystem metadata not needed by the C++ API.
- `eslint.config.mjs`, npm lifecycle scripts, bundling and lint dev dependencies: JavaScript project tooling only.

## Test directories worth mining first

- `test/parse.js`: parse defaults, dot/bracket behavior, limit enforcement, charset behavior, sparse array behavior, duplicate handling.
- `test/stringify.js`: array formats, null handling, encoding, option validation, sorting/filtering behavior, circular reference behavior.
- `test/utils.js`: encode/decode and merge helper behavior.
- `test/empty-keys-cases.js`: empty-key fixture table.
- README examples: public behavior examples to keep docs synchronized.

## Implementation risks discovered from the source layout

- Upstream behavior is concentrated in small files but has many option interactions; compatibility gaps can hide in boundary values such as `arrayLimit`, `decodeDotInKeys`, and comma arrays.
- Upstream uses `side-channel` to distinguish overflow objects and detect stringify cycles; the C++ port uses value trees and local helpers, so the same observable behavior must be tested explicitly.
- Upstream accepts JavaScript values that do not map cleanly to `JsonValue`, including Symbol, BigInt, `undefined`, functions, and arbitrary object instances. Date, Buffer, and `toString()`-capable polycpp objects are adapted explicitly before stringification.
- Upstream option validation throws `TypeError` for invalid dynamic option types; the C++ API uses typed option fields, making those tests runtime-only.
- Upstream has no TypeScript declarations, so tests and README are the API source of truth.

## Companion repo alignment

- companion repos inspected: `cookie`, `ini`, `yaml`, and `express`; `express` is a downstream user of `polycpp::qs` for request query and urlencoded body parsing.
- CMake target and alias pattern: existing `polycpp_qs` and `polycpp::qs` match the companion baseline; `test_qs` is an older generic test target and is recorded as a compatibility-era naming deviation.
- public header layout: `include/polycpp/qs/qs.hpp` is the only public header, matching small utility companions.
- detail/private header strategy: `include/polycpp/qs/detail/{utils,parse,stringify,aggregator}.hpp` keeps split implementation helpers private.
- aggregator header strategy: `detail/aggregator.hpp` is used only by `src/qs.cpp`; no top-level `include/polycpp/qs.hpp` exists, matching this repo's established public include path.
- examples strategy: two user-workflow examples are present under `examples/`; CMake exposes them behind `POLYCPP_QS_BUILD_EXAMPLES`.
- documentation site strategy: Doxygen/Breathe/Sphinx tree exists with real qs pages; catch-up adds the libgen-standard `docs/build.py` entry point and updates the Pages workflow.
- deliberate deviations from existing companions: repo predates libgen, GitHub repo is already public, tests were written before strict planning, and the generic `test_qs` executable name remains for continuity.

## Polycpp ecosystem reuse analysis

- polycpp core paths inspected: `<polycpp-checkout>/include/polycpp/core/json.hpp`, `<polycpp-checkout>/include/polycpp/core/error.hpp`, `<polycpp-checkout>/include/polycpp/core/number.hpp`, `<polycpp-checkout>/include/polycpp/core/date.hpp`, `<polycpp-checkout>/include/polycpp/buffer.hpp`, plus `<polycpp-checkout>/include/polycpp/{http,https,net,tls,io,stream,events}` for Node parity surfaces.
- polycpp capability snapshot: `75bc07dfca6ac0aaca07c8748476246e8c18df74` from `git -C <polycpp-checkout> rev-parse HEAD` on 2026-05-03.
- transport/listener capability review: current base polycpp has `io::TcpSocket`, `io::TcpAcceptor`, `io::PipeSocket`, `io::PipeAcceptor`, `io::StreamSocket`, `io::StreamAcceptor`, `net::Server`, `net::NativeListenHandle`, `http::Server`, `https::Server`, `tls::TLSSocket`, `io::TlsStream`, and `tls::Server`; qs does not expose any transport or listener surface, so none are used.
- polycpp core types/functions selected: `polycpp::JsonValue`, `JsonObject`, and `JsonArray` for open nested query data; `polycpp::RangeError` for limit/depth failures; `polycpp::Number::parseInt`, `Number::toString`, `Number::isNaN`, and `Number::isFinite` for JavaScript-compatible numeric parsing and formatting.
- polycpp core types/functions rejected: `polycpp::http::Headers`, request/response, URL, streams, events, timers, sockets, TLS, crypto, filesystem, and Promise primitives are not part of the qs public contract.
- public polycpp interop review: the public data model should continue to accept and return `polycpp::JsonValue` trees because query objects are schema-open and downstream `express` already consumes that shape.
- string policy: public query strings and keys use `std::string` with UTF-8 text; `polycpp::String` is not needed because the supported API does not promise JavaScript UTF-16 code-unit indexing.
- JsonValue/Object/Array policy: dynamic query data is represented directly with `JsonValue`, `JsonObject`, and `JsonArray`; parse always returns a top-level object; stringify accepts a `JsonValue` but only object roots produce output.
- Date/time interop policy: `polycpp::Date` is supported through explicit `qs::toQsValue(date, opts)` adaptation. `StringifyOptions::serializeDate` is honored there because `JsonValue` does not carry Date instances during recursive traversal.
- diagnostic/config object policy: `ParseOptions` and `StringifyOptions` are typed structs; no `toObject()`/`toJSON()` adapter is needed for diagnostics in the current API.
- toJSON/stringify policy: qs owns query serialization through `qs::stringify`; direct `polycpp::JSON::stringify(value)` support is not expected to emit query strings.
- companion libs inspected for reusable APIs: `cookie`, `ini`, `yaml`, `express`, `content-type`, and `path-to-regexp` public headers/CMake were inspected for repo patterns and reusable query behavior.
- companion libs selected for reuse: none as dependencies; `express` reuses `qs`, not the other direction.
- companion libs rejected or deferred: no existing companion owns the package-specific qs parse/stringify semantics.
- new local abstractions introduced: `ParseOptions`, `StringifyOptions`, `Duplicates`, `ArrayFormat`, `Format`, callback context types, Date/Buffer/toString adapters, and private helpers for percent encoding/decoding, merge, compaction, and key segmentation; these are package-specific and justified because base polycpp has JSON and numeric primitives but not qs semantics.
- reuse risks or integration gaps: the C++ port clean-room reimplements upstream `side-channel`-backed overflow and cycle behavior; audit findings resolved parser limit and encoded-dot edge gaps, with comma-format object coercion retained as a documented C++ behavior difference.

## Node parity surface audit

- callback APIs: custom decoder, encoder, formatter, filter, sort, and date serialization are adapted as typed C++ callbacks.
- Promise APIs: none, because qs is synchronous CPU-local parsing/formatting.
- EventEmitter APIs: none.
- server/listener APIs: none; transport/listener snapshot was still recorded before marking this surface not applicable.
- stream APIs: none; upstream parses whole strings and stringifies whole objects.
- Buffer and binary APIs: upstream stringify treats Buffer as a primitive string-like value; C++ supports this through `qs::toQsValue(buffer)` because `JsonValue` does not carry Buffer instances.
- URL, timer, process, and filesystem APIs: none in runtime target package.
- crypto, compression, TLS, network, and HTTP APIs: none in runtime target package.
- unsupported Node-specific APIs and audit reason: CommonJS packaging, dynamic option type validation, JS prototype/`plainObjects` behavior, Symbol, BigInt, function/`undefined` values, and runtime dependency objects are JS-runtime-only; Date, Buffer, and callbacks are intentionally adapted to typed C++ options and `JsonValue`.

## External SDK and native driver strategy

- upstream external services/protocols: not applicable because qs is a local string parser/serializer, not a service client or protocol implementation.
- native SDKs/client libraries to use: none.
- SDKs/protocols explicitly not reimplemented: none.
- adapter/linking strategy: no external SDK or native driver linkage; CMake links only `polycpp`.
- test environment needs: local compiler, CMake, GoogleTest via FetchContent, and optionally Doxygen/Sphinx for docs.

## Compatibility foundation review

- downstream dependency role: foundational query parsing and serialization utility used by downstream frameworks such as `express` for request query and urlencoded body parsing.
- native substitution risk: high; no native SDK should replace qs semantics because downstream packages depend on package-specific nesting, duplicate, limit, and encoding behavior.
- upstream implementation data to preserve: option defaults, enum/string option sets, percent-encoding tables, merge/compact rules, array-limit overflow behavior, dot/bracket segmentation, and fixture vectors from upstream tests.
- generated or vendored data plan: no generated or vendored data is required; compatibility vectors are represented as C++ tests and documented provenance, with upstream BSD-3-Clause notice retained.
- compatibility fixture strategy: continue adapting targeted cases from upstream `test/parse.js`, `test/stringify.js`, `test/utils.js`, `test/empty-keys-cases.js`, and README examples into focused GoogleTest cases.

## Security and fail-closed review

- security-sensitive behavior: moderate parser abuse surface, not auth or crypto; relevant risks are unbounded input expansion, prototype-pollution parity, excessive parameter counts, deep nesting, and malformed percent escapes.
- trust boundary: callers commonly parse user-controlled URL query strings or request bodies.
- supported protocol or algorithm matrix: no protocol; supported algorithms are query parse/stringify with bracket/dot syntax, duplicate strategies, array formats, depth/parameter/array limits, charset selection, and RFC1738/RFC3986 formatting.
- unsupported behavior and fail-closed policy: unsupported JS runtime options are unavailable at compile time; `throwOnLimitExceeded` and `strictDepth` should throw `RangeError` for configured parser limits; malformed percent escapes decode conservatively without throwing.
- result-set/framing drain policy, if protocol client: not applicable.
- binary payload type-mapping policy, if protocol client: not applicable; query input/output is text.
- stateful parser/session-state policy, if protocol client/server: not applicable; parser is stateless per call.
- server/listener response writer matrix, if protocol server surface exists: not applicable.
- key, secret, credential, or user-controlled input handling: query strings may contain user-controlled and sensitive values; examples and docs should not log secrets; limit options should be used for untrusted input.
- misuse cases that must be tested: excessive depth, parameter limit exceeded, array limit exceeded including equality boundary, sparse array compaction, malformed bracket groups, encoded dot keys, comma arrays over limit, duplicate keys, and `__proto__` keys.

## Core use cases

- Parse a REST API query string such as `filter[status]=published&tags[]=cpp` into a nested `JsonValue` tree.
- Parse Express-style urlencoded form bodies with nested fields.
- Serialize a `JsonObject` with nested arrays and objects into a query string.
- Select array output shape for downstream consumers: indices, brackets, repeat, or comma.
- Guard untrusted input with `parameterLimit`, `arrayLimit`, `depth`, `strictDepth`, and `throwOnLimitExceeded`.

## Key features to port first

- `parse()` for basic key/value pairs, nested brackets, dot notation, duplicate keys, arrays, sparse compaction, null handling, comma values, charsets, depth, and parameter/array limits.
- `stringify()` for nested objects, arrays, nulls, booleans, numbers, strings, RFC1738/RFC3986 encoding, query prefix, delimiter selection, dot notation, and empty arrays.
- Public option structs and enums that keep upstream option names recognizable.
- Compatibility tests from upstream parse/stringify fixtures for supported behavior.

## Parity outcome and omitted surfaces

- Completed upstream parity work: parse `charsetSentinel`, `interpretNumericEntities`, and custom `decoder`; stringify `charsetSentinel`, custom `encoder`, `formatter`, `filter`, `sort`, `serializeDate` through Date adaptation, deprecated `indices`, and Date/Buffer/toString adapters.
- Omitted JS-runtime surfaces: `allowPrototypes`, `plainObjects`, `strictMerge`, Symbol, BigInt, function, and `undefined` handling because they depend on JavaScript object/value shapes that `JsonValue` does not expose.
- Omitted test classes: upstream invalid option type tests for dynamic JavaScript values because C++ option structs are typed.
- Omitted package identity surface: a separate mutable `qs.formats` public object; current C++ API exposes the supported selector as `Format`.

## Features to defer

- No upstream parity items from the prior deferred list remain deferred after the libgen catch-up update.
- Remaining omitted surfaces are JavaScript runtime object/value-shape behavior: `allowPrototypes`, `plainObjects`, `strictMerge`, Symbol, BigInt, function, and `undefined`.
- A mutable `qs.formats` object remains omitted because the C++ API exposes the supported selector as the `Format` enum.

## Non-parity extension candidates

- `std::string_view` overloads for parse/stringify inputs when lifetime and ABI policy are settled.
- Optional helpers to parse directly from URL objects once a stable base `polycpp::url` public API is selected.
- Optional JSON adapter utilities for callers that want sorted stable output independent of `JsonObject` ordering.

## v0 scope

- port version: 1.0.0
- versioning note: port version is independent from upstream versioning
- supported APIs: `polycpp::qs::parse`, `polycpp::qs::stringify`, `ParseOptions`, `StringifyOptions`, `Duplicates`, `ArrayFormat`, and `Format`.
- unsupported APIs: Symbol/BigInt/function/undefined value handling, CommonJS/browser package artifacts, and `qs.formats` object identity.
- dependency plan: link only base `polycpp`; do not ship `side-channel` or its transitive JavaScript helpers because the C++ implementation uses value semantics and private helpers.
- polycpp modules to use: `polycpp/core/json.hpp`, `polycpp/core/error.hpp`, `polycpp/core/number.hpp`, `polycpp/core/date.hpp`, and `polycpp/buffer.hpp`.
- missing polycpp primitives: none for the supported qs scope; transport, listener, stream, and event primitives exist in base polycpp but are not needed by qs.
