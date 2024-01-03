#!/bin/bash

# Specify the folder and extension
folder="/home/draesdom/Documents/projects/minimizer-digestKATKAkernel/src/tests/artificial/basic"
extension=".txt"

kernelize_program="/home/draesdom/Documents/projects/minimizer-digestKATKAkernel/build/kernelize"
mindigest_program="/home/draesdom/Documents/projects/minimizer-digestKATKAkernel/build/minimizer_digest"

# Use find to locate files with the specified extension in the given folder
files=($(find "$folder" -type f -name "*$extension" -print))

for file in "${files[@]}"; do
    echo "$file"
    $kernelize_program $file -k5 -s
    $kernelize_program $file -k10 -s
    $mindigest_program $file -w10 -s
    $mindigest_program $file -w15 -s
done
