# MEM finding tool for metagenomic classification
From the paper: TBA

##  Brief description

Software for finding the **leftmost** and the **rightmost** positions of every MEM of the pattern. 
Implementation takes **input** in .fa / .fasta format or simple string/concatenation on strings divided with $.
```
>genome1
GATTACAT
>genome2
AGATACAT
>genome3
GATACAT
>genome4
GATTAGAT
>genome5
GATTAGATA
```

```
$GATTACAT$AGATACAT$GATACAT$GATTAGAT$GATTAGATA
```

and gives **output** like
```
>pattern  time_in_us    #Mems
MEM1     index_in_pattern       length       leftmost_genome_index       rightmost_genome_index
MEM2    index_in_pattern       ...
...
```

##  Download

To clone the repository, use following option:
```
$   git clone https://github.com/draessld/MEMmc
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
$   index-build ../src/tests/test.fa
```

input should be in format either *fasta* or *txt*.
This command will create the required data structures (FMindex,RMQs,LCPs) of the text file "input.fa" and store them using filename as prefix. Use option -o to specify a different output folder for the index files.

patterns_files should contains every pattern on a new line. For searching single pattern can be used option -p\<pattern\>.
Run
```
$   index-locate ../src/tests/test.fa -P../src/tests/test.patterns 
$   index-locate ../src/tests/test.fa -pACATA
```

##  Additional scripts
### Kernel
kernelization - method that keeps only first occurence of every k-mer

```
./kernelize -k3 ../src/tests/test.txt
```

### Minimizer digest
not sure, if it works correctly.

```
./minimizer_digest -p -w3 ../src/tests/test.txt
```


##  TODO
optimization of BWT/SA/LCP construction for large files, NSV/PSV queries.