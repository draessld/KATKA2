#!/bin/bash

#   DESC
#   generate required datasets with given

execute_generate="../build/generate_dataset"

# Check if the user provided a folder as input
if [ $# -lt 4 ]; then
    echo "Usage: $0 <length_of_genome> <p1> <p2> <dataset_folder>"
    exit 1
fi

ell="$1"
p1="$2"
p2="$3"
folder="$4"
basename="dataset_${ell}_${p1}_${p2}"

mkdir -p $folder

# Check if the specified executes exists
if [ ! -f "$execute_generate" ]; then
    echo "Error: Execute '$execute_generate' not found."
    exit 1
fi

#  generate dataset
echo "Generating dataset with parameter $ell $p1 $p2"

echo "Running program output"
# $execute_generate $ell $p1 $2 | awk -v base_name=$folder$basename -v RS= '{ print > (base_name NR ".txt") }'
output=$($execute_generate $ell $p1 $p2)
r=$(echo "$output" | tail -n 1)
output=$(echo "$output" | head -n -1 | awk -v base_name=$folder$basename -v RS= '{ print > (base_name NR ".txt") }')
echo $r
#   rename
mv "$folder${basename}1.txt" "$folder${basename}_$r.txt"
mv "$folder${basename}2.txt" "$folder${basename}_$r.pattern"
echo "Generating dataset with parameter $ell $p1 $p2 => DONE"

echo result saved on $folder${basename}.*
