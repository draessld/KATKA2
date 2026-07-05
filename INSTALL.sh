#!/bin/bash
#
# KATKA2 installer: build the C++ tools and generate configure.json for run.py.

set -e

# Absolute path to the directory containing this script.
BASE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$BASE_DIR"

echo "Starting project configuration in $BASE_DIR ..."

# 1. Install Python dependencies (non-fatal: only needed for download/configure).
echo "Installing Python dependencies..."
if ! pip install -r requirements.txt; then
    echo "WARNING: 'pip install -r requirements.txt' failed."
    echo "         The core pipeline (minimize/kernelize/build/find) still works;"
    echo "         Biopython/requests/tqdm are only needed for 'download'/'configure'."
fi

# 2. Compile the C++ tools (Release / -O3).
echo "Compiling C++ code..."
mkdir -p "$BASE_DIR/scripts/build"
cd "$BASE_DIR/scripts/build"
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . -j
cd "$BASE_DIR"

# 3. Generate configure.json (paths run.py reads via get_exe()).
echo "Generating configure.json..."
cat <<EOL > "$BASE_DIR/configure.json"
{
    "output_dir": "$BASE_DIR/output",
    "build_dir": "$BASE_DIR/scripts/build",
    "cpp_exe":{
        "build_exe": "$BASE_DIR/scripts/build/build",
        "find_exe": "$BASE_DIR/scripts/build/find",
        "kernelize_exe": "$BASE_DIR/scripts/build/kernelize",
        "minimize_exe": "$BASE_DIR/scripts/build/minimize"
    }
}
EOL

# 4. Offer a convenience alias (added to ~/.bash_aliases; guarded, never fatal).
ALIAS_NAME="katka2"
ALIAS_COMMAND="python3 $BASE_DIR/run.py"
BASH_ALIASES_FILE="$HOME/.bash_aliases"
touch "$BASH_ALIASES_FILE"
if grep -q "alias $ALIAS_NAME=" "$BASH_ALIASES_FILE" 2>/dev/null; then
    sed -i "s|alias $ALIAS_NAME=.*|alias $ALIAS_NAME='$ALIAS_COMMAND'|" "$BASH_ALIASES_FILE"
    echo "Alias '$ALIAS_NAME' updated in $BASH_ALIASES_FILE"
else
    echo "alias $ALIAS_NAME='$ALIAS_COMMAND'" >> "$BASH_ALIASES_FILE"
    echo "Alias '$ALIAS_NAME' added to $BASH_ALIASES_FILE"
fi

echo
echo "Done. Start a new shell (or run 'source ~/.bash_aliases') to use:"
echo "    katka2 minimize -m 3 -w 10 <file>"
echo "or call it directly:  python3 $BASE_DIR/run.py --help"
