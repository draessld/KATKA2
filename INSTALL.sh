#!/bin/bash
#
# KATKA2 installer: build the C++ tools and add them as shell aliases.
# No Python required.

set -e

# Absolute path to the directory containing this script.
BASE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$BASE_DIR"

# 1. Build (Release / -O3). Needs a C++17 compiler, Boost (program_options) and
#    SDSL (vgteam fork) with headers in ~/include and libs in ~/lib.
echo "Building KATKA2 tools..."
mkdir -p build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . -j
cd "$BASE_DIR"

# 2. Add the four tools as aliases in ~/.bash_aliases (idempotent, never fatal).
TOOLS="index-build index-locate kernelize minimizer_digest"
BASH_ALIASES_FILE="$HOME/.bash_aliases"
touch "$BASH_ALIASES_FILE"
for tool in $TOOLS; do
    line="alias $tool='$BASE_DIR/build/$tool'"
    if grep -q "alias $tool=" "$BASH_ALIASES_FILE" 2>/dev/null; then
        sed -i "s|alias $tool=.*|$line|" "$BASH_ALIASES_FILE"
    else
        echo "$line" >> "$BASH_ALIASES_FILE"
    fi
    echo "  aliased $tool -> $BASE_DIR/build/$tool"
done

echo
echo "Done. Start a new shell (or run 'source ~/.bash_aliases') to use:"
echo "    index-build <text.txt>"
echo "    index-locate <text.txt> -P<patterns>   |  -p<pattern>"
echo "    kernelize -i<text.txt> -k<k>"
echo "    minimizer_digest -i<text.txt> -w<w> -k<m> -H<hash>"
