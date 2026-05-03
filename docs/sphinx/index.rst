qs
==

**Nested query string parse and stringify**

Parse URL query strings into nested :cpp:class:`polycpp::JsonValue`
trees, and serialise them back — with the bracket notation
(``a[b][c]=d``), dot notation, comma arrays, and duplicate-key
handling that plain ``application/x-www-form-urlencoded`` alone never
covers. A C++20 port of npm qs v6 focused on the shapes every REST
API actually sees.

.. code-block:: cpp

   #include <polycpp/qs/qs.hpp>
   using namespace polycpp;

   // Bracket notation nests into a JsonObject.
   auto parsed = qs::parse("a[b][c]=42&a[b][d]=hi");
   // parsed["a"]["b"]["c"].asString() == "42"

   // Arrays by index, brackets, repeat, or comma.
   qs::StringifyOptions opts{.arrayFormat = qs::ArrayFormat::brackets};
   std::string out = qs::stringify(
       JsonValue(JsonObject{{"tag", JsonArray{"c", "cpp"}}}), opts);
   // out == "tag%5B%5D=c&tag%5B%5D=cpp"

.. grid:: 2

   .. grid-item-card:: Drop-in familiarity
      :margin: 1

      Two entry points straight from npm qs — ``parse`` and
      ``stringify`` — taking the same option structs (``allowDots``,
      ``arrayFormat``, ``duplicates``, ``strictNullHandling``) as the
      JavaScript original.

   .. grid-item-card:: C++20 native
      :margin: 1

      A compiled CMake target with a small public header, typed option
      structs, and ``std::string``/``JsonValue`` interop throughout.

   .. grid-item-card:: Tested
      :margin: 1

      92 compatibility tests adapted from npm qs behavior, covering empty keys,
      ``decodeDotInKeys`` round-trips, ISO-8859-1 charsets, and every
      ``ArrayFormat`` (indices, brackets, repeat, comma).

   .. grid-item-card:: Plays well with polycpp
      :margin: 1

      Uses the same JSON value and error types as the rest of the polycpp
      ecosystem, so downstream companions such as Express can reuse it
      directly.

Getting started
---------------

.. code-block:: bash

   # With FetchContent (recommended)
   FetchContent_Declare(
       polycpp_qs
       GIT_REPOSITORY https://github.com/polycpp/qs.git
       GIT_TAG        master
   )
   FetchContent_MakeAvailable(polycpp_qs)
   target_link_libraries(my_app PRIVATE polycpp::qs)

:doc:`Installation <getting-started/installation>` · :doc:`Quickstart <getting-started/quickstart>` · :doc:`Tutorials <tutorials/index>` · :doc:`API reference <api/index>`

.. toctree::
   :hidden:
   :caption: Getting started

   getting-started/installation
   getting-started/quickstart

.. toctree::
   :hidden:
   :caption: Tutorials

   tutorials/index

.. toctree::
   :hidden:
   :caption: How-to guides

   guides/index

.. toctree::
   :hidden:
   :caption: API reference

   api/index

.. toctree::
   :hidden:
   :caption: Examples

   examples/index
