Use dot notation instead of brackets
====================================

**When to reach for this:** you want ``a.b.c=d`` to mean the same
thing as ``a[b][c]=d`` — cleaner logs, no percent-encoded brackets,
a form that's easy to hand-type.

Flip :cpp:member:`polycpp::qs::ParseOptions::allowDots` on the
parse side, and the matching
:cpp:member:`polycpp::qs::StringifyOptions::allowDots` on the
stringify side:

.. code-block:: cpp

   #include <polycpp/qs/qs.hpp>

   using namespace polycpp;

   qs::ParseOptions pOpts;
   pOpts.allowDots = true;
   auto parsed = qs::parse("a.b.c=d", pOpts);
   // parsed["a"]["b"]["c"].asString() == "d"

   qs::StringifyOptions sOpts;
   sOpts.allowDots = true;
   sOpts.encode    = false;
   std::string out = qs::stringify(JsonValue(JsonObject{
       {"a", JsonObject{{"b", JsonObject{{"c", std::string{"d"}}}}}}}),
       sOpts);
   // out == "a.b.c=d"

If your keys themselves contain literal dots — say
``first.last=ada`` as a single flat key — you need to tell the
parser that. Set both
:cpp:member:`polycpp::qs::ParseOptions::decodeDotInKeys` on parse
and
:cpp:member:`polycpp::qs::StringifyOptions::encodeDotInKeys` on
stringify; the pair round-trips ``first%2Elast`` safely.

Don't mix the two modes inside one protocol — either you're using
dots as nesting or you're using them as literal key characters.
Trying to do both confuses every consumer you hand the URL to.
