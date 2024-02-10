# MEM finding tool for metagenomic classification

##  Brief description
Software for finding the **leftmost** and the **rightmost** positions of every MEM relative to the pattern described in 


Implementation takes **input** as simple string or concatenation on strings divided with $.
```
GATTACAT$AGATACAT$GATACAT$GATTAGAT$GATTAGATA
```

and gives **output** like
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

##   References
