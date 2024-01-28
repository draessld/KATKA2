#!/bin/bash

Check if the user provided a folder as input
if [ $# -eq 0 ]; then
    echo "Usage: $0 <folder>"
    exit 1
fi

datasets_folder="$1"
mkdir -p $datasets_folder

ell=(100 250 500)
p1=(10 25 30)
p2=(10 25 30)

# Iterate through the arrays using a for loop and index
for ((i=0; i<3; i++)); do
    e=${ell[$i]}
    a=${p1[$i]}
    b=${p2[$i]}

    source ./generate_datasets.sh $e $a $b $datasets_folder 
done