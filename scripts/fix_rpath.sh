#!/bin/bash
BIN="$1"
if ! otool -l "$BIN" | grep -A 3 LC_RPATH | grep -q '@executable_path/../Resources/lib'; then
    echo "Adding rpath..."
    install_name_tool -add_rpath @executable_path/../Resources/lib "$BIN"
else
    echo "rpath already present"
fi
