# MEM finding tool for metagenomic classification

##  Brief description

Software for finding the **leftmost** and the **rightmost** positions of every MEM of the pattern. 
Implementation takes **input** as simple string or concatenation on strings divided with $.
```
$GATTACAT$AGATACAT$GATACAT$GATTAGAT$GATTAGATA
```

and gives **output** like
```
>pattern1  time_in_ms    #Mems
[MEM1start_position,MEM1end_position]{leftmost_genome_index,rightmost_genome_index}       [MEM2start_position,MEM2end_position]{leftmost_genome_index,rightmost_genome_index}       ...
>pattern2  time_in_ms    #Mems
[MEM1start_position,MEM1end_position]{leftmost_genome_index,rightmost_genome_index}       [MEM2start_position,MEM2end_position]{leftmost_genome_index,rightmost_genome_index}       ...
...
```

##  Compile
You need the SDSL library installed on your system (https://github.com/xxsds/sdsl-lite).

Project uses cmake to generate the Makefile. Create a build folder in the main folder:
```
$   mkdir build
$   cd build; cmake ..
$   make
```
##  Run
After compiling, run

```
$   index-build ../src/tests/test.txt
```

input should be in format either *fasta* or *txt*.
This command will create the required data structures (FMindex,RMQs,LCPs) of the text file "input.fa" and store them using filename as prefix. Use option -o to specify a different output folder for the index files.

patterns_files should contains every pattern on a new line. For searching single pattern can be used option -p\<pattern\>.
Run
```
$   index-locate ../src/tests/test/test -P../src/tests/test.pattern
$   index-locate ../src/tests/test/test -pACATA
```

##  Additional scripts
### Kernel
kernelization - losses method that keeps only first occurence of every k-mer

```
./kernelize -k3 ../src/tests/test.txt
```

### Minimizer digest

```
./minimizer_digest -w3 ../src/tests/test.txt
```

##   Experiments
-   Experiments are settled in folder experiments/
-   there are few script that can help you with running bigger number of experiments
### generate_dataset

```
Usage: generate_datasets.sh <length_of_genome> <p1> <p2> <output_folder>
```
for given arguments generate two files *dataset.\<length_of_genome\>.\<p1\>.\<p2\>.txt* and *dataset.\<length_of_genome\>.\<p1\>.\<p2\>.pattern* inside folder *\<output_folder\>*, that contains similar sequences of length *length_of_genome* based on probability *p1* and *p2*

### run_classic
```
Usage: run_classic.sh <folder>
```
for all *.txt* files in *folder* build index and locate MEM for corresponding pattern saved in same named file with extension *.pattern*  

### run_kernel
```
Usage: run_kernel.sh <folder> <k>
```
for all *.txt* files in *folder* create string kernel based on argument *k*, save kernel with extension *k.krl*, build index over kernel and locate MEM for corresponding pattern saved in same named file with extension *.pattern*  


### run_minimizerDigestKernel
```
Usage: run_minimizerDigestKernel.sh <folder> <w> <k>
```

for all *.txt* files in *folder* create minimizer digest of the text based on argument *w*, save it with extension *w.dgt*, create string kernel from minimizer digest based on argument *k*, save kernel with extension *w.dgt.k.krl*, build index over minimizer digest kernel and locate MEM for corresponding pattern saved in same named file with extension *.pattern*  

### run_all - TODO
generates set of dataset as follow:
-   length of genomes