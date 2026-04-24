Tolerate a leading ``?``
========================

**When to reach for this:** the caller pasted the whole URL suffix
(``?a=1&b=2``) into what you parse as a query string, and the raw
``?`` is leaking through as a key name.

Set :cpp:member:`polycpp::qs::ParseOptions::ignoreQueryPrefix`:

.. code-block:: cpp

   #include <polycpp/qs/qs.hpp>

   using namespace polycpp;

   qs::ParseOptions opts;
   opts.ignoreQueryPrefix = true;

   auto parsed = qs::parse("?a=1&b=2", opts);
   // parsed["a"].asString() == "1"  — no spurious "?a" key

The mirror option on stringify is
:cpp:member:`polycpp::qs::StringifyOptions::addQueryPrefix`, which
prepends ``?`` to output.  Use it when you're building a relative
URL and want the result to concatenate cleanly onto a path.

Both flags are off by default — qs only adds or tolerates the prefix
when you ask it to.
