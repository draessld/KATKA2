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
You need the boost library (`program_options`) and SDSL installed on your system. Please use [this](https://github.com/vgteam/sdsl-lite) version of SDSL.

The build expects the SDSL headers in `~/include` and the SDSL libraries in `~/lib` (see `CMakeLists.txt`). Adjust those two lines if you installed SDSL elsewhere.

Project uses cmake to generate the Makefile. Create a build folder in the main folder:
```
$   mkdir build
$   cd build; cmake ..
$   make
```
This produces four executables: `index-build`, `index-locate`, `kernelize` and `minimizer_digest`.

Alternatively, run `./INSTALL.sh` to build the tools and add them as shell aliases in
`~/.bash_aliases` (no Python required), so you can call `index-build`, `index-locate`,
`kernelize` and `minimizer_digest` from anywhere.
###  Run
To build index, run

```
$   index-build ../src/tests/test.txt
```

This command creates the required data structures next to the input as `<file>.index/`.
The pattern file should contain one pattern per line. For a single pattern use option -p\<pattern\>.
To locate all MEMs, pass the **same original text file** to `index-locate` (it re-derives the
index location automatically — do not pass the `.index/` path):
```
$   ./index-locate ../src/tests/test.txt -P../src/tests/test.pattern
$   ./index-locate ../src/tests/test.txt -p<pattern>
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
A minimizer digest replaces the text with the sequence of its minimizers. Each window of *w* consecutive *m*-mers contributes its minimum-hash *m*-mer; that *m*-mer is written to the digest under an injective encoding, so maximal exact matches are preserved. Parameters: *w* (window), *m* (the *m*-mer length, option `-k`, default 3), and the selection hash (option `-H`, one of `djb2` | `sdbm` | `loselose` | `mult`, default `djb2`).

To digest the whole file:
```
$   ./minimizer_digest -i../src/tests/test.txt -w<w> -k<m> -H<hash>
``` 
or a single sequence:
```
$   ./minimizer_digest -s<sequence> -w<w> -k<m> -H<hash>
``` 
`-k` and `-H` are optional (default `-k3 -Hdjb2`). Example: `./minimizer_digest -i../src/tests/test.txt -w10 -k3`.

##  Experiments
Experiments described in the article were run over the SILVA dataset, available [here](https://www.arb-silva.de/download/arb-files/). [1000](genera_from_1100_to_2100.txt) samples were taken in range 1100-2100 from total 9118 reference files. Main result of the project is described within the following graph. 
![graph](1000genomes_simulated_error_rate_1p_w_lines.png?raw=true "Title")
