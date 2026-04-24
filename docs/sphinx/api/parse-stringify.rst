Parse and stringify
===================

The two top-level entry points. Both operate on
:cpp:class:`polycpp::JsonValue` trees — parsing always returns an
object at the top level, stringify accepts any JSON-shaped input.

.. doxygenfunction:: polycpp::qs::parse
.. doxygenfunction:: polycpp::qs::stringify
