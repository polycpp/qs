Known differences from npm qs
=============================

polycpp qs follows the supported npm qs parse/stringify behavior, but the
C++ API is intentionally typed. These surfaces differ from JavaScript qs:

- Date, Buffer, and other polycpp objects with ``toString()`` are adapted
  explicitly with ``qs::toQsValue(...)`` before stringification because
  :cpp:class:`polycpp::JsonValue` cannot carry arbitrary runtime objects.
- JavaScript-only value types such as Symbol, BigInt, functions, and
  ``undefined`` are not represented by :cpp:class:`polycpp::JsonValue`.
- Runtime object-shape options such as ``plainObjects``,
  ``allowPrototypes``, and ``strictMerge``. ``__proto__`` keys are still
  dropped defensively.
- Callback hooks are typed C++ callbacks. Custom parse ``decoder`` returns a
  ``JsonValue`` for values or a string/null-like value for keys; stringify
  ``filter`` uses ``std::optional<JsonValue>`` so ``std::nullopt`` means
  omit this value.

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
