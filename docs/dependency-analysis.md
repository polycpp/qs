# Dependency and JavaScript API Analysis

- package: qs
- package version: 6.15.1
- package root: `.tmp/npm-package`
- analyzer json: `.tmp/dependency-analysis.json`
- published npm artifact path: `.tmp/npm-package`
- published npm artifact analyzed: yes; analyzer was run against the published package artifact after `npm pack qs`.
- include dev dependencies: no
- dependency source install used: yes, production install with scripts disabled by the analyzer
- companion root checked: local polycpp companion workspace

## Package entry metadata

- main: `lib/index.js`
- module: none
- types: none
- exports: none
- bin: none
- missing declared entries in repo clone: none
- TypeScript source files detected: 0
- TypeScript declarations reviewed: none shipped by upstream or the npm artifact
- declaration-source decision: runtime JavaScript, README, and upstream tests are the API evidence source.
- source-vs-published artifact decision: source tag `v6.15.1` and published artifact `6.15.1` align; the published artifact is the exact runtime analysis basis.

## Direct dependencies

- `side-channel@^1.1.0` is the only direct runtime dependency in upstream. It supports stringify cycle detection and parse overflow metadata in JavaScript.

## Dependency ownership decisions

| Package | Kind | Requested | Installed | License | License evidence | License impact | License strategy | Affects repo license | Deps | Source files | Node API calls | JS API calls | Recommendation | Rationale |
|---|---|---|---|---|---|---|---|---|---:|---:|---:|---:|---|---|
| side-channel | hard | ^1.1.0 | 1.1.0 | MIT | package.json license field in installed package | permissive | clean-room replacement | no | 5 | 3 | 0 | 0 | do not create a companion dependency for qs v0 | C++ value semantics and private helpers cover the supported cycle/overflow needs; no `side-channel` source is linked or shipped. |

## License impact summary

- upstream package license: BSD-3-Clause
- repo license decision: keep the companion repo MIT while retaining the upstream BSD-3-Clause notice in `THIRD_PARTY_LICENSES.md`.
- GPL/AGPL dependencies: none.
- LGPL/MPL dependencies: none.
- permissive dependencies requiring notices: upstream qs BSD-3-Clause notice is present; `side-channel` is not shipped or copied into the C++ target.
- dev/test-only dependencies excluded from shipped artifacts: upstream dev dependencies such as `tape`, `iconv-lite`, `object-inspect`, lint, bundling, and audit tools are not shipped.
- dependency license notices to add to `THIRD_PARTY_LICENSES.md`: none beyond the existing qs notice unless future code copies from `side-channel` or its dependency graph.

## Transitive dependency summary

Analyzer install found `side-channel` with transitive runtime helpers `es-errors`, `object-inspect`, `side-channel-list`, `side-channel-map`, `side-channel-weakmap`, `call-bound`, `get-intrinsic`, `call-bind-apply-helpers`, `function-bind`, `gopd`, `get-proto`, `dunder-proto`, `es-define-property`, `es-object-atoms`, `has-symbols`, `hasown`, and `math-intrinsics`. All are MIT per package metadata and none are linked or shipped by this C++ port.

## Runtime API usage

### Target package

- entry points analyzed: analyzer emitted no target entry points, so manual inspection covered `lib/index.js`, `lib/parse.js`, `lib/stringify.js`, `lib/utils.js`, and `lib/formats.js`.
- source files analyzed by analyzer: 0 target files due analyzer warning `qs: no entry points found for qs`.
- source files manually inspected: upstream and npm artifact `lib/index.js`, `lib/parse.js`, `lib/stringify.js`, `lib/utils.js`, `lib/formats.js`, `test/parse.js`, `test/stringify.js`, `test/utils.js`, and `test/empty-keys-cases.js`.
- external imports seen from target: `./parse`, `./stringify`, `./formats`, `./utils`, and `side-channel`.

### Analyzer porting gates

- polycpp reuse hints consumed: analyzer emitted none; manual reuse selected `JsonValue`, `JsonObject`, `JsonArray`, `RangeError`, and `Number` helpers from base polycpp.
- Node parity hints consumed: one callback signal came from installed helper dependencies, not from the target package; target qs exposes no callback API.
- security hints consumed: analyzer classified the package as not security-sensitive; manual review still treats parser limits and user-controlled query input as abuse-relevant.
- security-sensitive package: no for auth/crypto, moderate for parser resource limits.
- polycpp capability snapshot consumed: `75bc07dfca6ac0aaca07c8748476246e8c18df74` from the local polycpp checkout on 2026-05-03.
- transport/listener capability hints consumed: base transport/listener primitives were inspected and recorded as not applicable to qs.

### Node.js API usage

- target package: no Node built-in APIs are required for runtime parse/stringify behavior.
- installed dependencies: analyzer reported no Node built-in API calls in shipped runtime dependencies.

### Node parity surface usage

- callbacks: none in target; dependency-only analyzer callback signal is not exposed.
- Promise APIs: none.
- EventEmitter APIs: none.
- server/listener APIs: none.
- diagnostic/tracing APIs: none.
- streams: none.
- Buffer and binary data: upstream `utils.isBuffer` makes Buffer stringify as a primitive; C++ v0 omits Buffer because `JsonValue` does not carry binary values.
- URL/timer/process/filesystem APIs: none.
- crypto/compression/TLS/network/HTTP APIs: none.

### JavaScript API usage

Analyzer totals across installed packages included `TypeError`, `Array.prototype.push`, `Object.getOwnPropertyDescriptor`, `Object.prototype.toString.call`, `Map`, `Set`, `Object.keys`, and `parseInt`. Target manual inspection found JavaScript-specific dynamic typing, object/prototype checks, callbacks for custom encoder/decoder/filter/sort, and side-channel object identity tracking. The C++ port adapts these to typed options, `JsonValue`, `std::unordered_set`/value-tree traversal, and `polycpp::Number`.

### Framework object boundary usage

- analyzer-reported target-package framework object accesses: 0
- analyzer-reported dependency framework object accesses: 0
- manual review decision: qs does not read or mutate request, response, context, or framework objects; downstream frameworks call it as a pure helper.

## Porting decisions

- Keep the C++ port dependency-free apart from base `polycpp`; do not add a `side-channel` companion for v0.
- Preserve package-specific parser and serializer semantics through private helpers and compatibility tests.
- Use typed C++ option structs instead of JavaScript option dictionaries and runtime type validation.
- Keep `JsonValue` as the public data model because query shapes are open and downstream `express` already consumes it.
- Record unimplemented JavaScript value surfaces and callbacks as deferred or omitted divergences, not as hidden behavior.

## Analyzer warnings

- `qs: no entry points found for qs`: agent response: manually inspected `package.json` main `lib/index.js`, runtime `lib/*.js`, and upstream tests; analyzer dependency data was still consumed for `side-channel` and transitive license/API review.
