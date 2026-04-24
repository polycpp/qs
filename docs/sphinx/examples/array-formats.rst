Compare every ArrayFormat
=========================

Serialises the same ``JsonArray`` under every value of
:cpp:enum:`polycpp::qs::ArrayFormat` so you can pick the shape that
matches your consumer. Mirrors the table in
:doc:`../guides/pick-array-format`.

.. literalinclude:: ../../../examples/array_formats.cpp
   :language: cpp
   :caption: examples/array_formats.cpp

Sample run:

.. code-block:: bash

   ./build/examples/array_formats

Expected output:

.. code-block:: text

   indices    tag[0]=c&tag[1]=cpp&tag[2]=c#
   brackets   tag[]=c&tag[]=cpp&tag[]=c#
   repeat     tag=c&tag=cpp&tag=c#
   comma      tag=c,cpp,c#

   comma 1elt      tag=c
   comma 1elt rt   tag[]=c

The last two lines show the
:cpp:member:`polycpp::qs::StringifyOptions::commaRoundTrip` effect:
with a single-element array, the plain comma format collapses to
``tag=c`` which looks like a scalar to the next parser.  Turning
``commaRoundTrip`` on emits ``tag[]=c`` so the round-trip preserves
the array shape.
