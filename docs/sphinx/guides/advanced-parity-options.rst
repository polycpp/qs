Use advanced qs parity options
==============================

Most applications only need the defaults. Use this guide when matching an
existing npm qs deployment, an older HTML form, or a callback-heavy
JavaScript integration.

Charset sentinels
-----------------

Some HTML forms include an upstream qs ``utf8=`` sentinel. Enable
``charsetSentinel`` when the input may use that convention:

.. code-block:: cpp

   #include <polycpp/qs/qs.hpp>

   namespace qs = polycpp::qs;

   qs::ParseOptions p;
   p.charset = "iso-8859-1";
   p.charsetSentinel = true;
   auto parsed = qs::parse("utf8=%E2%9C%93&name=%C3%B8", p);

For ISO-8859-1 numeric HTML entities, also enable
``interpretNumericEntities``:

.. code-block:: cpp

   #include <polycpp/qs/qs.hpp>

   namespace qs = polycpp::qs;

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

   #include <polycpp/qs/qs.hpp>

   namespace qs = polycpp::qs;

   qs::ParseOptions p;
   p.decoder = [](const std::string& raw, const qs::DecodeContext& ctx) {
       polycpp::JsonValue decoded = qs::defaultDecode(raw, ctx);
       if (ctx.kind == qs::ComponentKind::value) {
           return polycpp::JsonValue("redacted");
       }
       return decoded;
   };

Returning ``nullptr`` for a key omits that key-value pair. For values,
return the :cpp:class:`polycpp::JsonValue` that should be merged into the
parsed result.

Stringify has matching ``encoder``, ``formatter``, ``filter``, and ``sort``
hooks on ``StringifyOptions``. Custom encoders receive raw string
components; call ``defaultEncode`` when you want qs percent encoding plus a
small local change.

.. code-block:: cpp

   #include <optional>
   #include <polycpp/qs/qs.hpp>

   namespace qs = polycpp::qs;

   qs::StringifyOptions s;
   s.encoder = [](const std::string& raw, const qs::EncodeContext& ctx) {
       if (ctx.kind == qs::ComponentKind::value) {
           return qs::defaultEncode("v:" + raw, ctx);
       }
       return qs::defaultEncode(raw, ctx);
   };
   s.filter = [](const std::string& prefix, const polycpp::JsonValue& value)
       -> std::optional<polycpp::JsonValue> {
       if (prefix == "token") {
           return std::nullopt;
       }
       return value;
   };

Date and Buffer values
----------------------

``JsonValue`` cannot store arbitrary C++ objects. Convert Date, Buffer, or
other polycpp-style ``toString()`` objects explicitly before stringifying:

.. code-block:: cpp

   #include <polycpp/buffer.hpp>
   #include <polycpp/core/date.hpp>
   #include <polycpp/qs/qs.hpp>

   namespace qs = polycpp::qs;

   polycpp::Date date(6.0);
   qs::StringifyOptions s;
   s.serializeDate = [](const polycpp::Date& d) -> polycpp::JsonValue {
       return d.getTime() * 7;
   };

   polycpp::JsonValue doc = polycpp::JsonObject{
       {"when", qs::toQsValue(date, s)}
   };
   qs::stringify(doc, s); // "when=42"

   auto body = polycpp::Buffer::from("test");
   polycpp::JsonValue upload = polycpp::JsonObject{
       {"body", qs::toQsValue(body)}
   };
   qs::stringify(upload); // "body=test"
