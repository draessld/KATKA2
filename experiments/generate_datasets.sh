#!/bin/bash

#   DESC
#   generate required datasets with given

execute_generate="../build/generate_dataset"
output_folder="./parameter_tuning/"

mkdir -p $output_folder

# Check if the user provided a folder as input
if [ $# -lt 3 ]; then
    echo "Usage: $0 <length_of_genome> <p1> <p2>"
    exit 1
fi

# dataset="$1"
ell="$1"
p1="$2"
p2="$3"
basename="dataset.$ell.$p1.$p2"

# Check if the specified executes exists
if [ ! -f "$execute_generate" ]; then
    echo "Error: Execute '$execute_generate' not found."
    exit 1
fi

#  generate dataset
echo "Generating dataset with parameter $ell $p1 $p2"

echo "Running program output"
# $execute_generate $ell $p1 $2 | awk -v base_name=$output_folder$basename -v RS= '{ print > (base_name NR ".txt") }'
output=$($execute_generate $ell $p1 $2)
r=$(echo "$output" | tail -n 1)
output=$(echo "$output" | head -n -1 | awk -v base_name=$output_folder$basename -v RS= '{ print > (base_name NR ".txt") }')
echo $r
#   rename
mv "$output_folder${basename}1.txt" "$output_folder${basename}.g$r.txt"
mv "$output_folder${basename}2.txt" "$output_folder${basename}.g$r.pattern"
echo "Generating dataset with parameter $ell $p1 $p2 => DONE"

echo result saved on $output_folder${basename}.*
