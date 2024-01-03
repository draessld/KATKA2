#!/bin/bash

base_folder="./basic"
program_path="/home/draesdom/Documents/projects/minimizer-digestKATKAkernel/src/tests/artificial/generate_tree"

genome_sizes=( 25 50 100 500)
probs=(1 10 25)
number_of_genomes=(32 64 128 256)

mkdir -p $base_folder

for genome_size in "${genome_sizes[@]}" 
do
    for prob in "${probs[@]}" 
    do
        for number in "${number_of_genomes[@]}" 
        do
            echo "generating file with $number genomes of size: $genome_size and probability of mutation: $prob into $base_folder/genomes_basic_${genome_size}_${prob}.txt"
            $program_path $genome_size $prob $number>$base_folder/genomes_basic_${genome_size}_${prob}_${number}.txt
        done
    done
done