Control how duplicate keys are merged
=====================================

**When to reach for this:** the caller is sending the same key twice
(``a=1&a=2``) and you need to decide whether to keep both, the first,
or the last.

Set :cpp:member:`polycpp::qs::ParseOptions::duplicates` to the
strategy you want:

.. code-block:: cpp

   #include <polycpp/qs/qs.hpp>

   using namespace polycpp;

   qs::ParseOptions opts;

   opts.duplicates = qs::Duplicates::combine;  // default
   auto both = qs::parse("a=1&a=2", opts);
   // both["a"] is a JsonArray: ["1", "2"]

   opts.duplicates = qs::Duplicates::first;
   auto first = qs::parse("a=1&a=2", opts);
   // first["a"].asString() == "1"

   opts.duplicates = qs::Duplicates::last;
   auto last = qs::parse("a=1&a=2", opts);
   // last["a"].asString() == "2"

Picking "last" is a common compromise — it matches PHP's ``$_GET``
and lets callers override an earlier value — but it silently drops
data. When an API takes tag lists or multi-select filters,
``combine`` is almost always what you want; reach for it first.

The flag applies to top-level keys. Arrays built through
:cpp:member:`polycpp::qs::ParseOptions::parseArrays` (``a[0]=1&a[1]=2``)
are not subject to duplicate collapsing — the indices make them
distinct.
