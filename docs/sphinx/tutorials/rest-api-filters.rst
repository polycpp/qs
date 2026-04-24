Parse a list-endpoint filter query
==================================

**You'll build:** the request-side of a paginated list endpoint that
accepts filters, sort keys, and a page number in its query string —
the shape that ``GET /articles?filter[status]=published&sort=-date&page[number]=2``
takes in a JSON:API or Elasticsearch-style URL.

**You'll use:** :cpp:func:`polycpp::qs::parse`,
:cpp:struct:`polycpp::qs::ParseOptions`, and the
:cpp:class:`polycpp::JsonValue` helpers (``asObject``, ``asString``,
``asArray``).

**Prerequisites:** installed and linking ``polycpp::qs``. See
:doc:`../getting-started/installation` if not.

Step 1 — tighten the parse options
----------------------------------

The defaults are generous. For an API surface you want to fail loudly
on malformed input, not silently truncate it:

.. code-block:: cpp

   #include <polycpp/qs/qs.hpp>

   using namespace polycpp;

   qs::ParseOptions apiOpts{
       .ignoreQueryPrefix    = true,   // tolerate a leading '?'
       .depth                = 4,      // shallow enough to reject attacks
       .strictDepth          = true,   // throw past depth
       .parameterLimit       = 256,    // cap at a reasonable working set
       .throwOnLimitExceeded = true,   // … and throw if exceeded
   };

Tight depth and parameter caps matter because attackers craft
quadratic parse strings (``a[a][a][a]...=b``) to chew CPU. Both
``strictDepth`` and ``throwOnLimitExceeded`` are opt-in because qs
v6 matches the permissive npm default by omission.

Step 2 — parse and pull out the parts you expect
------------------------------------------------

At the edge you always know the shape you want. Treat the result like
a JSON document — walk the keys explicitly instead of iterating:

.. code-block:: cpp

   struct ListQuery {
       std::string                        status;
       std::vector<std::string>           tags;
       std::string                        sort;
       int                                pageNumber = 1;
       int                                pageSize   = 25;
   };

   ListQuery decodeListQuery(std::string_view rawQs) {
       auto parsed = qs::parse(std::string(rawQs), apiOpts);

       ListQuery out;
       if (parsed.isObject()) {
           if (parsed["filter"].isObject()) {
               if (parsed["filter"]["status"].isString())
                   out.status = parsed["filter"]["status"].asString();
               if (parsed["filter"]["tags"].isArray()) {
                   for (const auto& t : parsed["filter"]["tags"].asArray())
                       out.tags.push_back(t.asString());
               }
           }
           if (parsed["sort"].isString())
               out.sort = parsed["sort"].asString();
           if (parsed["page"].isObject()) {
               if (parsed["page"]["number"].isString())
                   out.pageNumber = std::stoi(parsed["page"]["number"].asString());
               if (parsed["page"]["size"].isString())
                   out.pageSize   = std::stoi(parsed["page"]["size"].asString());
           }
       }
       return out;
   }

Remember that ``qs::parse`` always produces string leaves — it doesn't
know that ``page[number]=2`` was meant to be an integer. Coerce at
the boundary, not deeper into your domain.

Step 3 — normalise the sort key
-------------------------------

APIs typically use a ``-`` prefix for descending sort. That's not
qs's concern, but once you have the string a tiny helper pays for
itself:

.. code-block:: cpp

   struct SortSpec {
       std::string field;
       bool        descending;
   };

   SortSpec parseSort(std::string_view raw) {
       if (raw.empty())          return {"id", false};
       if (raw.front() == '-')   return {std::string(raw.substr(1)), true};
       return {std::string(raw), false};
   }

Step 4 — wire it all together
-----------------------------

A single call site that turns an HTTP query string into a strongly
typed request object:

.. code-block:: cpp

   try {
       auto q    = decodeListQuery(request.query());
       auto sort = parseSort(q.sort);
       db.list(q.status, q.tags, sort.field, sort.descending,
               q.pageNumber, q.pageSize);
   } catch (const polycpp::RangeError& e) {
       // strictDepth / throwOnLimitExceeded landed here.
       response.writeHead(400);
       response.end("query too complex");
   }

What you learned
----------------

- ``strictDepth`` and ``throwOnLimitExceeded`` turn qs from "best
  effort" into a strict parser with bounded work.
- Values are always strings — coerce types at the API boundary.
- Bracket notation (``filter[status]``, ``page[number]``) nests
  naturally into ``JsonObject`` children; you walk them the same way
  you'd walk any JSON document.

Next: :doc:`express-style-forms` — driving the stringify side to
round-trip an HTML form post.
