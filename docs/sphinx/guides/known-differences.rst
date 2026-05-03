Known differences from npm qs
=============================

polycpp qs follows the supported npm qs parse/stringify behavior, but the
C++ API is intentionally typed. These surfaces are not present in v0:

- JavaScript callback hooks such as custom parse ``decoder`` and stringify
  ``encoder``, ``formatter``, ``filter``, ``sort``, and ``serializeDate``.
- JavaScript-only value types such as Date, Buffer, Symbol, BigInt,
  functions, and ``undefined``; the C++ surface accepts
  :cpp:class:`polycpp::JsonValue`.
- Runtime object-shape options such as ``plainObjects``,
  ``allowPrototypes``, and ``strictMerge``. ``__proto__`` keys are still
  dropped defensively.
- Charset sentinel parsing/output and numeric HTML entity interpretation;
  callers choose the charset directly through the option structs.

One stringify edge is deliberately different: comma-format arrays only emit
scalar ``JsonValue`` entries. Nested arrays or objects inside a comma-format
array are not coerced to JavaScript's ``[object Object]`` string form.

One parser internals edge is also adapted: upstream uses hidden
``side-channel`` metadata to keep non-throwing overflow objects marked while
duplicate arrays are merged. The C++ port uses plain ``JsonValue`` trees, so
set ``throwOnLimitExceeded`` when array overflow must fail closed exactly at
the request edge.

The libgen catch-up audit is recorded in ``docs/divergences.md``. Parser
``arrayLimit`` boundaries, comma-array overflow, duplicate implicit-array
limit throws, and double-encoded ``decodeDotInKeys`` round-trips are
covered by compatibility tests.
