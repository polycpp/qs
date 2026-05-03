Known differences from npm qs
=============================

polycpp qs follows the supported npm qs parse/stringify behavior, but the
C++ API is intentionally typed. The differences below are the expected public
contract, not missing options.

Value model
-----------

The parser and serializer operate on :cpp:class:`polycpp::JsonValue`. That
means JavaScript-only value types such as Symbol, BigInt, functions, and
``undefined`` are not represented. Date, Buffer, and other polycpp objects
with ``toString()`` are adapted explicitly with ``qs::toQsValue(...)`` before
stringification because ``JsonValue`` cannot carry arbitrary runtime objects.

Runtime-only JavaScript options
-------------------------------

Options that exist only to control JavaScript object shape are not exposed:
``plainObjects``, ``allowPrototypes``, and ``strictMerge``. C++ ``JsonObject``
has no prototype chain, and ``__proto__`` keys are still dropped defensively.

Typed callbacks
---------------

Callback hooks are typed C++ callbacks. Custom parse ``decoder`` returns a
``JsonValue``; for decoded keys, ``nullptr`` means omit the key-value pair.
Stringify ``filter`` uses ``std::optional<JsonValue>``, so ``std::nullopt``
means omit this value.

Behavior edges
--------------

Comma-format arrays only emit scalar ``JsonValue`` entries. Nested arrays or
objects inside a comma-format array are not coerced to JavaScript's
``[object Object]`` string form.

Upstream uses hidden ``side-channel`` metadata to keep non-throwing overflow
objects marked while duplicate arrays are merged. The C++ port uses plain
``JsonValue`` trees, so set ``throwOnLimitExceeded`` when array overflow must
fail closed exactly at the request edge.

The libgen catch-up audit is recorded in ``docs/divergences.md``. Parser
``arrayLimit`` boundaries, comma-array overflow, duplicate implicit-array
limit throws, and double-encoded ``decodeDotInKeys`` round-trips are
covered by compatibility tests.
