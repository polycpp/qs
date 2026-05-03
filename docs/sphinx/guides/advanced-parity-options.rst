Use advanced qs parity options
==============================

Charset sentinels
-----------------

Some HTML forms include an upstream qs ``utf8=`` sentinel. Enable
``charsetSentinel`` when the input may use that convention:

.. code-block:: cpp

   namespace qs = polycpp::qs;

   qs::ParseOptions p;
   p.charset = "iso-8859-1";
   p.charsetSentinel = true;
   auto parsed = qs::parse("utf8=%E2%9C%93&name=%C3%B8", p);

For ISO-8859-1 numeric HTML entities, also enable
``interpretNumericEntities``:

.. code-block:: cpp

   qs::ParseOptions p;
   p.charset = "iso-8859-1";
   p.interpretNumericEntities = true;
   auto parsed = qs::parse("face=%26%239786%3B", p);

Custom decoding and encoding
----------------------------

Callbacks are typed. A decoder receives the raw component plus a context
describing the effective charset and whether the component is a key or value.
Use ``defaultDecode`` when you want upstream-compatible percent decoding
before your own transformation.

.. code-block:: cpp

   qs::ParseOptions p;
   p.decoder = [](const std::string& raw, const qs::DecodeContext& ctx) {
       polycpp::JsonValue decoded = qs::defaultDecode(raw, ctx);
       if (ctx.kind == qs::ComponentKind::value) {
           return polycpp::JsonValue("redacted");
       }
       return decoded;
   };

Stringify has matching ``encoder``, ``formatter``, ``filter``, and ``sort``
hooks on ``StringifyOptions``.

Date and Buffer values
----------------------

``JsonValue`` cannot store arbitrary C++ objects. Convert Date, Buffer, or
other polycpp-style ``toString()`` objects explicitly before stringifying:

.. code-block:: cpp

   polycpp::Date date(6.0);
   qs::StringifyOptions s;
   s.serializeDate = [](const polycpp::Date& d) -> polycpp::JsonValue {
       return d.getTime() * 7;
   };

   polycpp::JsonValue doc = polycpp::JsonObject{
       {"when", qs::toQsValue(date, s)}
   };
   qs::stringify(doc, s); // "when=42"
