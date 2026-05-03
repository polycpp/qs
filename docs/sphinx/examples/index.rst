Examples
========

Self-contained programs exercising the main features of qs. Each
example compiles against the public API only — no private headers, no
non-exported targets.

.. toctree::
   :maxdepth: 1

   parse-roundtrip
   array-formats

Running an example
------------------

From the repository root:

.. code-block:: bash

   cmake -B build -G Ninja -DPOLYCPP_QS_BUILD_EXAMPLES=ON
   cmake --build build --target polycpp_qs_example_parse_roundtrip
   ./build/examples/parse_roundtrip 'a[b]=1&a[c]=2&tags[]=c&tags[]=cpp'

   cmake --build build --target polycpp_qs_example_array_formats
   ./build/examples/array_formats

Examples are only built when ``POLYCPP_QS_BUILD_EXAMPLES=ON`` is passed to CMake.
