Parse and stringify
===================

The two top-level entry points. Both operate on
:cpp:class:`polycpp::JsonValue` trees.

Parsing
-------

``qs::parse`` always returns a top-level
``polycpp::JsonObject``, including for empty input. Nested
bracket or dot notation becomes nested objects and arrays, but decoded
wire values remain strings unless an option changes that behavior; for
example, a bare key becomes ``""`` by default and ``null`` with
``strictNullHandling``.

Stringifying
------------

``qs::stringify`` walks a top-level object. The object's members may be
strings, numbers, booleans, nulls, arrays, or nested objects, but a
non-object root returns an empty string. Date, Buffer, and other
polycpp-style objects should be converted with ``qs::toQsValue(...)``
before they are placed in the ``JsonValue`` tree.

The serializer detects cycles in nested objects and throws
:cpp:class:`polycpp::RangeError` rather than recursing forever.

.. doxygenfunction:: polycpp::qs::parse
.. doxygenfunction:: polycpp::qs::stringify
