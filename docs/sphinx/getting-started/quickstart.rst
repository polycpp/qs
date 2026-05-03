Quickstart
==========

This page walks through a minimal qs program end-to-end. Copy the
snippet, run it, then jump to :doc:`../tutorials/index` for
task-oriented walkthroughs or :doc:`../api/index` for the full
reference.

We'll parse a small but representative query string into a
:cpp:class:`polycpp::JsonValue` tree, pluck a couple of nested values
out, and stringify a hand-built object back to a URL-safe form.

Full example
------------

.. code-block:: cpp

   #include <cstdint>
   #include <iostream>
   #include <polycpp/core/json.hpp>
   #include <polycpp/qs/qs.hpp>

   using namespace polycpp;

   int main() {
       // 1. Parse bracket-nested + array-indexed input.
       auto parsed = qs::parse(
           "user[name]=ada&user[email]=ada%40acme.io&tags[0]=c&tags[1]=cpp");

       std::cout << "name: "  << parsed["user"]["name"].asString()  << '\n';
       std::cout << "email: " << parsed["user"]["email"].asString() << '\n';
       std::cout << "tags:  " << parsed["tags"][0].asString()
                 << ", "     << parsed["tags"][1].asString()       << '\n';

       // 2. Build an object and stringify with bracket-array format.
       JsonValue obj = JsonObject{
           {"q",    std::string{"polycpp"}},
           {"page", int64_t{2}},
           {"tag",  JsonArray{"c", "cpp"}},
       };
       qs::StringifyOptions opts;
       opts.arrayFormat = qs::ArrayFormat::brackets;
       std::cout << "out: " << qs::stringify(obj, opts) << '\n';

       // 3. Opt into dot notation for a human-friendly form.
       qs::StringifyOptions dotted;
       dotted.allowDots = true;
       dotted.encode    = false;   // turn percent-encoding off to read it
       std::cout << "dotted: "
                 << qs::stringify(JsonValue(JsonObject{
                        {"a", JsonObject{{"b", std::string{"c"}}}}}),
                    dotted) << '\n';
   }

Save the snippet as ``main.cpp`` in a project that uses the
``FetchContent`` wiring from :doc:`installation`, then build and run it:

.. code-block:: bash

   cmake -B build -G Ninja
   cmake --build build
   ./build/my_app

Expected output:

.. code-block:: text

   name: ada
   email: ada@acme.io
   tags:  c, cpp
   out: q=polycpp&page=2&tag%5B%5D=c&tag%5B%5D=cpp
   dotted: a.b=c

What just happened
------------------

1. :cpp:func:`polycpp::qs::parse` understands bracket notation
   natively: ``user[name]=ada`` nests into a ``JsonObject`` named
   ``user`` with a ``name`` key. Numeric indices
   (``tags[0]=c&tags[1]=cpp``) produce a ``JsonArray`` instead. The
   result is always a top-level ``JsonObject`` — empty input gives an
   empty object, not a null.

2. :cpp:func:`polycpp::qs::stringify` walks a top-level ``JsonObject``
   root and emits ``key=value`` pairs joined by the configured delimiter.
   The values inside that object may be strings, numbers, booleans, nulls,
   arrays, or nested objects. The
   :cpp:member:`polycpp::qs::StringifyOptions::arrayFormat` field
   picks the flavor; the default
   :cpp:enumerator:`polycpp::qs::ArrayFormat::indices` writes
   ``tag[0]=c&tag[1]=cpp``, whereas
   :cpp:enumerator:`polycpp::qs::ArrayFormat::brackets` writes
   ``tag[]=c&tag[]=cpp``.

3. Brackets are percent-encoded by default (``%5B``/``%5D``) because
   they're reserved characters in a URL path. When you're piping the
   output into somewhere that treats them literally, such as a log line or
   a debug print, set
   :cpp:member:`polycpp::qs::StringifyOptions::encode` to ``false``
   to see the raw form. Dot notation is the other readability win
   (:cpp:member:`polycpp::qs::StringifyOptions::allowDots`).

Next steps
----------

- :doc:`../tutorials/index` — step-by-step walkthroughs of common tasks.
- :doc:`../guides/index` — short how-tos for specific problems.
- :doc:`../api/index` — every public type, function, and option.
- :doc:`../examples/index` — runnable programs you can drop into a sandbox.
