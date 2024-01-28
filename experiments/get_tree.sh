#!/bin/bash

reference_path="/home/draesdom/Documents/projects/KATKA2/experiments/silva/silva_genera_paper_data/silva_reference_files"

# Use find to locate all .txt files in the specified folder and its subdirectories
fasta_files=$(find "$reference_path" -type f -name "*.fa")
output_file="/home/draesdom/Documents/projects/KATKA2/experiments/silva/silva_genera_paper_data/silva_reference_files/all.fa"
echo "" > $output_file

# Check if any .txt files were found
if [ -z "$fasta_files" ]; then
    echo "No .txt files found in '$reference_path' or its subdirectories."
    exit 0
fi

counter=0

# Iterate through all .txt files in the directory
for file in $fasta_files; do
    # echo $file
    # if [ "$counter" -lt 5 ]; then
        # echo "Processing file: $file"
        cat $file | sed -n '2~2p' | paste -s -d'#'>> "$output_file"
        echo "$">>$output_file
        # Add your processing logic here

        # Increment the counter
        # ((counter++))
    # else
        # break  # Exit the loop after processing 10 files
    # fi
done