#!/bin/bash
doxygen doxygen_stencil.config
rm -r ~/dev/public_html/ed/stencil/doxygen_stencil/
mv doxygen_stencil/ ~/dev/public_html/ed/stencil/

