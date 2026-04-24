Choose an array format for stringify
====================================

**When to reach for this:** you're serialising a ``JsonArray`` and
need to match a consumer that expects a specific shape — Rails, PHP,
Go's ``net/url``, Elasticsearch, a browser ``<form>``.

Set :cpp:member:`polycpp::qs::StringifyOptions::arrayFormat` to one
of four values:

.. code-block:: cpp

   #include <polycpp/qs/qs.hpp>

   using namespace polycpp;
   JsonValue v = JsonObject{{"a", JsonArray{"b", "c"}}};

   qs::StringifyOptions opts;
   opts.encode = false;  // for readability below

   opts.arrayFormat = qs::ArrayFormat::indices;   // default, npm's default
   qs::stringify(v, opts);                        // "a[0]=b&a[1]=c"

   opts.arrayFormat = qs::ArrayFormat::brackets;  // Rails / Express forms
   qs::stringify(v, opts);                        // "a[]=b&a[]=c"

   opts.arrayFormat = qs::ArrayFormat::repeat;    // Go, Java Spring
   qs::stringify(v, opts);                        // "a=b&a=c"

   opts.arrayFormat = qs::ArrayFormat::comma;     // Elasticsearch
   qs::stringify(v, opts);                        // "a=b,c"

Match the consumer, not your taste. ``indices`` is the safest default
because it survives round-tripping through
:cpp:func:`polycpp::qs::parse` at any ``depth``; ``comma`` is the
most ambiguous because it conflicts with values that contain commas.

For a single-element array under
:cpp:enumerator:`polycpp::qs::ArrayFormat::comma`, set
:cpp:member:`polycpp::qs::StringifyOptions::commaRoundTrip` to
``true`` to emit ``a[]=b`` instead of ``a=b`` — otherwise the
decoder can't tell an array from a scalar.
