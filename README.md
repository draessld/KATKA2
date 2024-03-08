# MEM finding tool for taxonomic classification

##  Brief description
Software for finding the **leftmost** and the **rightmost** positions of each MEMs of the read
with respect to the text described in *Draesslerova, D., Ahmed, O., Gagie, T., Holub, J., Langmead, B., Manzini, G., & Navarro, G. Taxonomic classification with maximal exact matches in KATKA kernels and minimizer digests*


Implementation takes **input** as a simple string or a concatenation of strings divided with $ character.
```
GATTACAT$AGATACAT$GATACAT$GATTAGAT$GATTAGATA
```

Gives an **output** like
```
>pattern  time_in_ms    #Mems
[start_position,end_position]{leftmost_genome,rightmost_genome}       [start_position,end_position]{leftmost_genome,rightmost_genome}       ...
>pattern  time_in_ms    #Mems
[start_position,end_position]{leftmost_genome,rightmost_genome}       [start_position,end_position]{leftmost_genome,rightmost_genome}       ...
...
```

### Compile
You need the boost library and SDSL installed on your system. Please use version [this](https://github.com/vgteam/sdsl-lite) version of SDSL. 

Project uses cmake to generate the Makefile. Create a build folder in the main folder:
```
$   mkdir build
$   cd build; cmake ..
$   make
```
###  Run
To build index, run

```
$   index-build ../src/tests/test.txt
```

This command will create the required data structures of the text file and store them using filename as prefix.
patterns_files should contains every pattern on a new line. For searching simple pattern can be used option -p\<pattern\>.
To locate all MEMs in relation to pattern
```
$   ./index-locate ../src/tests/test/test -P../src/tests/test.pattern
$   ./index-locate ../src/tests/test/test -p<pattern>
```
results are printed on the standard output

###  Creating KATKA kernels
To create KATKA kernel with parameter *k*, run
```
$   ./kernelize -i../src/tests/test.txt -k<k>
``` 
to kernelize full text in the file, or 

```
$   ./kernelize -s<pattern> -k<k>
``` 
to kernelize simple pattern

###  Creating minimizer digests
To create minimizer digests with parameter *w*, run either
```
$   ./minimizer_digest -i../src/tests/test.txt -w<w>
``` 
to digest full text in the file, or 

```
$   ./minimizer_digest -s<pattern> -w<w>
``` 
to digest simple pattern

**Note:** So far, parameter k for minimizers is static k=3

##  Experiments
Experiments described in the article were run over the SILVA dataset, available [here](https://www.arb-silva.de/download/arb-files/). [1000](genera_from_1100_to_2100.txt) samples were taken in range 1100-2100 from total 9118 reference files. Main result of the project is described within the following graph. 
![graph](1000genomes_simulated_error_rate_1p_w_lines.png?raw=true "Title")
