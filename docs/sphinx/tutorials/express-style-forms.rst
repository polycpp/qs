Round-trip an Express-style form
================================

**You'll build:** a helper pair that encodes a structured form model
into an ``application/x-www-form-urlencoded`` body and decodes it
back. The shape matches what ``express.urlencoded({ extended: true })``
accepts out of the box, so a browser ``<form>`` or a ``curl --data``
call both work against the same endpoint.

**You'll use:** :cpp:func:`polycpp::qs::stringify`,
:cpp:func:`polycpp::qs::parse`, and the
:cpp:enum:`polycpp::qs::ArrayFormat` enum.

**Prerequisites:** read :doc:`rest-api-filters` first — it covers
the permissive-vs-strict option axis that we'll lean on again here.

Step 1 — model the form as a ``JsonValue``
------------------------------------------

Forms in the real world have nested objects, check-box arrays, and
empty fields. Build the model without thinking about the wire format:

.. code-block:: cpp

   #include <polycpp/core/json.hpp>
   #include <polycpp/qs/qs.hpp>

   using namespace polycpp;

   JsonValue buildForm() {
       return JsonObject{
           {"name",    std::string{"Ada Lovelace"}},
           {"email",   std::string{"ada@acme.io"}},
           {"address", JsonObject{
               {"city",    std::string{"London"}},
               {"country", std::string{"UK"}},
           }},
           {"interests", JsonArray{"math", "engines", "poetry"}},
           {"newsletter", true},
       };
   }

``qs::stringify`` serialises any ``JsonValue`` — string, number,
bool, object, or array — so the same helper works for form posts,
query strings, and URL fragments.

Step 2 — encode with form-friendly options
------------------------------------------

The defaults work, but two settings make the output nicer to read in
server logs and easier to match from a plain HTML form:

.. code-block:: cpp

   qs::StringifyOptions formOpts;
   formOpts.arrayFormat     = qs::ArrayFormat::brackets;  // foo[]=a&foo[]=b
   formOpts.encodeValuesOnly = true;                       // keys stay plain
   formOpts.format          = qs::Format::RFC1738;        // spaces as '+'

   std::string body = qs::stringify(buildForm(), formOpts);
   // name=Ada+Lovelace&email=ada%40acme.io&address[city]=London
   //   &address[country]=UK&interests[]=math&interests[]=engines
   //   &interests[]=poetry&newsletter=true

RFC 1738 turns spaces into ``+``, which matches ``<form
enctype="application/x-www-form-urlencoded">`` exactly.
``encodeValuesOnly = true`` keeps the keys human-readable — worth
the tiny correctness risk if, and only if, your key names are
ASCII-only.

Step 3 — decode back on the server
----------------------------------

The :cpp:member:`polycpp::qs::ParseOptions::comma` flag is handy for
APIs that accept comma-joined lists, but for form posts the default
parser is already right:

.. code-block:: cpp

   qs::ParseOptions formParse;
   formParse.parameterLimit       = 200;
   formParse.throwOnLimitExceeded = true;

   auto form = qs::parse(body, formParse);
   std::string name = form["name"].asString();
   // form["address"]["city"].asString() == "London"
   // form["interests"].asArray().size() == 3

``true`` and ``false`` come back as the strings ``"true"`` /
``"false"`` — everything going over the wire is a string.  The
``newsletter`` boolean from step 1 becomes
``form["newsletter"].asString() == "true"``; coerce it back in your
own layer.

Step 4 — guard against empty input
----------------------------------

Empty or missing bodies are normal. ``qs::parse("")`` returns an
empty object, never throws, so you can always index safely:

.. code-block:: cpp

   auto form = qs::parse(body);
   if (form.isObject() && !form.asObject().empty()) {
       // dispatch
   }

Avoid checking against a magic sentinel like ``null`` — the parser
never returns one unless you explicitly enabled
:cpp:member:`polycpp::qs::ParseOptions::strictNullHandling`.

What you learned
----------------

- ``ArrayFormat::brackets`` gives you the form-post convention
  ``foo[]=a&foo[]=b`` in one line.
- ``RFC1738`` vs ``RFC3986`` — pick the first for HTML forms,
  the second for anything else.
- Every decoded value is a string; coerce to richer types yourself.
- Empty bodies produce empty objects, not nulls, so no guard clause
  is needed just to index safely.
