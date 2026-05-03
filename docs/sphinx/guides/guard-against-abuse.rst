Cap parsing work to defend against abuse
========================================

**When to reach for this:** the query string comes from the internet,
and you've seen or read about the quadratic-parse DoS pattern
(``a[a][a][a]...=b&a[a][a][a]...=c&...``) that lets an attacker chew
CPU for cents per request.

qs v6 exposes three knobs. Set them up-front at your request edge:

.. code-block:: cpp

   #include <polycpp/qs/qs.hpp>

   using namespace polycpp;

   qs::ParseOptions strict;
   strict.depth                = 5;     // default is 5; keep it.
   strict.strictDepth          = true;  // throw if exceeded.
   strict.parameterLimit       = 256;   // cap distinct keys.
   strict.arrayLimit           = 100;   // cap array indices.
   strict.throwOnLimitExceeded = true;  // throw on either cap.

   try {
       auto parsed = qs::parse(request.query(), strict);
       // dispatch with parsed
   } catch (const polycpp::RangeError& e) {
       // Log and return 400.  Don't retry with looser options.
   }

The defaults are permissive on purpose — qs is used in hand-written
scripts where throwing on oversized input would be surprising. For a
network service you want the strict variant. Pick a ``depth`` based
on the deepest legitimate nesting your API accepts (two or three is
usually plenty); anything deeper than that is noise or an attack.

``arrayLimit`` is a length cap, matching upstream qs. An indexed key at
the configured value, such as ``a[100]`` with ``arrayLimit = 100``, is
already over the cap. Comma-split values use the same cap: ``a=1,2,3``
stays an array when the limit is at least three, otherwise it becomes an
overflow object or throws when ``throwOnLimitExceeded`` is enabled.
Duplicate keys that would combine into an array beyond the cap also
throw in the strict variant.

Don't try to normalise the error message from ``RangeError::what()``
into something user-friendly. Log the raw message for your ops team
and return a generic 400 to the caller.
