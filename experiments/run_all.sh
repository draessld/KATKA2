#!/bin/bash

# Define arguments
ks=(4 8 12 16 32)
ws=(3 4 5 6 8 10 12)

datasets_folder="$1"
mkdir -p $datasets_folder

source ./run_classic.sh $datasets_folder 

for k in "${ks[@]}"; do
    source ./run_kernel.sh $datasets_folder $k
    for w in "${ws[@]}"; do
        source ./run_minimizerDigestKernel.sh $datasets_folder $w $k
    done
done

for w in "${ws[@]}"; do
    source ./run_minimizerDigest.sh $datasets_folder $w
done
