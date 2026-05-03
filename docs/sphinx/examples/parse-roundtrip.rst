Parse and round-trip a query string
===================================

Takes a query string on the command line, parses it into a
:cpp:class:`polycpp::JsonValue` tree, pretty-prints the tree,
stringifies it back, and confirms the round-trip is a no-op.

.. literalinclude:: ../../../examples/parse_roundtrip.cpp
   :language: cpp
   :caption: examples/parse_roundtrip.cpp

Sample run:

.. code-block:: bash

   cmake -B build -G Ninja -DPOLYCPP_QS_BUILD_EXAMPLES=ON
   cmake --build build --target polycpp_qs_example_parse_roundtrip
   ./build/examples/parse_roundtrip 'a[b]=1&a[c]=2&tags[]=c&tags[]=cpp'

Expected output:

.. code-block:: text

   -- parsed --
   {
     a: {
       b: "1"
       c: "2"
     }
     tags: [
       "c"
       "cpp"
     ]
   }
   -- stringified --
   a[b]=1&a[c]=2&tags[0]=c&tags[1]=cpp
   round-trip matches: yes

Note that ``tags[]=c`` parses the same as ``tags[0]=c`` — the
parser doesn't preserve the "no-index brackets" form, it collapses
both to the indexed
:cpp:enumerator:`polycpp::qs::ArrayFormat::indices` output on the
way back out. If that matters to you, see :doc:`array-formats`.
