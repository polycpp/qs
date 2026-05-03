Options
=======

Aggregate option structs consumed by the two top-level functions.
Every field has a sensible default matching the npm qs package; use
designated initialisers to override just what you need.

Common parse choices
--------------------

``ignoreQueryPrefix``
    Accept either ``"?a=b"`` or ``"a=b"`` from request URLs.

``allowDots`` and ``decodeDotInKeys``
    Treat ``a.b=c`` as nested keys, and optionally preserve literal dots in
    key names that were double-encoded by another qs implementation.

``duplicates``
    Decide whether repeated keys combine into arrays, keep the first value,
    or keep the last value.

``comma``
    Split literal comma-separated values such as ``tag=c,cpp`` into arrays.

``parameterLimit``, ``depth``, ``arrayLimit``, ``strictDepth``, and
``throwOnLimitExceeded``
    Bound parser work for request inputs. Enable the throwing options when
    the caller should reject abusive inputs instead of accepting a truncated
    or object-shaped fallback.

``charset``, ``charsetSentinel``, and ``interpretNumericEntities``
    Handle legacy ISO-8859-1 form submissions and the upstream qs ``utf8=``
    sentinel convention.

``decoder``
    Override percent decoding for keys and values. Custom decoders receive
    a :cpp:struct:`polycpp::qs::DecodeContext` and can call
    :cpp:func:`polycpp::qs::defaultDecode` for the built-in behavior.

Common stringify choices
------------------------

``arrayFormat`` and ``indices``
    Choose whether arrays emit as ``a[0]=b``, ``a[]=b``, ``a=b&a=c``, or
    ``a=b,c``. ``indices`` is kept for deprecated qs compatibility; prefer
    ``arrayFormat`` in new code.

``allowDots`` and ``encodeDotInKeys``
    Emit nested object paths with dots, and optionally encode literal dots
    in key names so a later ``decodeDotInKeys`` parse can restore them.

``format``, ``encode``, and ``encodeValuesOnly``
    Pick RFC3986 query encoding, RFC1738 form encoding with ``+`` spaces, or
    partial/no encoding when another layer already handles escaping.

``skipNulls``, ``strictNullHandling``, and ``allowEmptyArrays``
    Control whether nulls and empty arrays are omitted, emitted as bare keys,
    or emitted with explicit empty markers.

``sort`` and ``filterKeys``
    Produce deterministic output or select a stable key order.

``encoder``, ``formatter``, ``filter``, and ``serializeDate``
    Customize traversal and encoding with typed callbacks. See
    :doc:`../guides/advanced-parity-options` for examples, including
    Date/Buffer adapters.

ParseOptions
------------

Passed to :cpp:func:`polycpp::qs::parse`.

.. doxygenstruct:: polycpp::qs::ParseOptions
   :members:
   :undoc-members:

StringifyOptions
----------------

Passed to :cpp:func:`polycpp::qs::stringify`.

.. doxygenstruct:: polycpp::qs::StringifyOptions
   :members:
   :undoc-members:
