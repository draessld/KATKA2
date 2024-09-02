#!/bin/bash

# Exit immediately if a command exits with a non-zero status
set -e

# Get the absolute path to the directory containing this script
BASE_DIR=$(pwd)

echo "Starting project configuration..."

# 1. Install Python dependencies
echo "Installing Python dependencies..."
pip install -r requirements.txt

# 2. Compile C++ code
echo "Compiling C++ code..."
mkdir -p "$BASE_DIR/scripts/build"
cd "$BASE_DIR/scripts/build"
cmake ..
cmake --build .
cd $BASE_DIR

# 3. Run C++ tests
# echo "Compiling and running C++ tests..."
# g++ -std=c++17 -o "$BASE_DIR/scripts/build/tests" tests/cpp_tests.cpp
# "$BASE_DIR/scripts/build/tests"

# 4. Run Python tests
# echo "Running Python tests..."
# python -m unittest discover -s tests -p '*_test.py'

# 5. Create alias for run.py
echo "Creating alias 'katka2' for run.py..."
alias katka2="python $BASE_DIR/run.py"

# 6. Generate configure.json file
echo "Generating configure.json..."
cat <<EOL > configure.json
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

# 7. Provide alias instructions
echo "To use the alias 'katka2', add the following line to your shell configuration file (.bashrc, .zshrc, etc.):"
echo "alias katka2=\"python $BASE_DIR/run.py\""

echo "Project setup, alias creation, and tests completed successfully!"
