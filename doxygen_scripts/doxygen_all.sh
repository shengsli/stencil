#!/bin/bash
doxygen doxygen_all.config
rm -r ~/dev/public_html/ed/stencil/doxygen_all/
mv doxygen_all/ ~/dev/public_html/ed/stencil/
