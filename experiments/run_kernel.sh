#!/bin/bash

#   DESC
#   run basic MEM search over dataset inside this folder

execute_mem_build="../build/index-build"
execute_mem_locate="../build/index-locate"
execute_string_kernel="../build/kernelize"
execute_minimizer_digest="../build/minimizer_digest"

# Check if the user provided a folder as input
if [ $# -eq 0 ]; then
    echo "Usage: $0 <folder> <k>"
    exit 1
fi

folder="$1"
k="$2"

# Check if the specified folder exists
if [ ! -d "$folder" ]; then
    echo "Error: Folder '$folder' not found."
    exit 1
fi

# Check if the specified executes exists
if [ ! -f "$execute_mem_build" ]; then
    echo "Error: Execute '$execute_mem_build' not found."
    exit 1
fi
if [ ! -f "$execute_mem_locate" ]; then
    echo "Error: Execute '$execute_mem_locate' not found."
    exit 1
fi
if [ ! -f "$execute_string_kernel" ]; then
    echo "Error: Execute '$execute_string_kernel' not found."
    exit 1
fi
if [ ! -f "$execute_minimizer_digest" ]; then
    echo "Error: Execute '$execute_minimizer_digest' not found."
    exit 1
fi

# Use find to locate all .txt files in the specified folder and its subdirectories
txt_files=$(find "$folder" -type f -name "*.txt")

# Check if any .txt files were found
if [ -z "$txt_files" ]; then
    echo "No .txt files found in '$folder' or its subdirectories."
    exit 0
fi

# Iterate through all .txt files in the directory
for file in $txt_files; do
    echo $file

    filename=$(basename "$file")
    base_filename="${filename%.*}"  # Remove the last extension

    pattern_file="$folder/$base_filename.pattern"
    kernel_file="$folder/$base_filename.$k.krl"
    output_file="$folder/$base_filename.$k.krl.mem"

    #   check if pattern file exists
    if [ -z "$pattern_file" ]; then
        echo "No .pattern file found in '$pattern_file'"
        exit 0
    fi
    
    #   create string kernel
    echo "Kernelize $file with parameter $k"
    $execute_string_kernel $file -k$k >$kernel_file
    echo "Kernelize $file with parameter $k => DONE"
    echo Index saved on $kernel_file

    #  build index
    echo "Building index"
    $execute_mem_build $kernel_file -cs
    echo "Building index => DONE"
    echo Index saved on $file.index/

    #  locate pattern
    echo "Locating pattern"
    $execute_mem_locate $kernel_file -P$pattern_file >$output_file
    echo "Locating pattern => DONE"

    echo result saved on $output_file
done
