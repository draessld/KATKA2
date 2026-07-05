# MEM finding tool for taxonomic classification

##  Brief description
Software for finding the **leftmost** and the **rightmost** positions of each MEM of a read
with respect to the text, described in *Draesslerova, D., Ahmed, O., Gagie, T., Holub, J., Langmead, B., Manzini, G., & Navarro, G. Taxonomic classification with maximal exact matches in KATKA kernels and minimizer digests*.

The text is a concatenation of reference genomes divided with the `$` character:
```
GATTACAT$AGATACAT$GATACAT$GATTAGAT$GATTAGATA$
```

For each pattern the tool reports its MEMs and, per MEM, the leftmost/rightmost genome it occurs in:
```
>pattern  time_us   #MEMs
[start,end]{leftmost_genome,rightmost_genome}   [start,end]{leftmost_genome,rightmost_genome}   ...
```

## Install

Requirements:
- **SDSL** — use [this fork](https://github.com/vgteam/sdsl-lite). The build expects SDSL
  headers in `~/include` and libraries in `~/lib` (see `scripts/CMakeLists.txt`; adjust those
  two lines if installed elsewhere).
- **Boost** (`program_options`) and a C++17 compiler + CMake.
- **Python 3** for the `katka2` driver. `Biopython`, `requests` and `tqdm`
  (`requirements.txt`) are only needed for the `download` / `configure` commands; the core
  pipeline runs without them.

```
$   bash INSTALL.sh
```
This builds the C++ tools into `scripts/build/`, writes `configure.json`, and adds a
`katka2` alias for `python3 run.py`. Start a new shell (or `source ~/.bash_aliases`) to use
the alias; otherwise call `python3 run.py <command>` directly.

## Usage

Everything is driven through `run.py` (aliased `katka2`). `-m`, `-w`, `-k` accept either a
single value or a range `start-stop-step`.

### Minimizer digest
Replaces the text with the sequence of its minimizers. Each window of *w* consecutive
*m*-mers contributes its minimum-hash *m*-mer, written under an injective encoding so MEMs
are preserved (*m* defaults to the paper's 3).
```
$   katka2 minimize -m 3 -w 10 <file-or-sequence> [--line_parse]
```
Output is written next to the input as `<file>_m<m>_w<w>`.

### KATKA kernel
Keeps only the characters needed to preserve MEMs of length ≥ *k*; gaps collapse to `#`.
```
$   katka2 kernelize -k 31 <file-or-sequence> [--line_parse]
```
Output is written next to the input as `<file>_k<k>`.

### Build an index
Optionally minimize (`-m -w`) and/or kernelize (`-k`) the inputs first, then build the index.
```
$   katka2 build [-m 3 -w 10] [-k 31] <file>
```
The index is written to `<transformed-file>.index/`.

### Find MEMs
```
$   katka2 find_mems <index_dir> -p <pattern-file-or-sequence>    # all MEMs -> <index>/out.out
$   katka2 find_leaf <index_dir> <pattern-file-or-sequence>       # best single-genome (leaf) match
```

### Data preparation (needs Biopython)
```
$   katka2 configure <tree.nwk> <fasta_dir>   # concatenate leaves' FASTA into one text
$   katka2 download  <tree.nwk> <ncbi_db>     # fetch leaf sequences from NCBI
$   katka2 list                               # list built indices
$   katka2 clean                              # clear the workspace
```

##  Experiments
Experiments described in the article were run over the SILVA dataset, available
[here](https://www.arb-silva.de/download/arb-files/). [1000](genera_from_1100_to_2100.txt)
samples were taken in range 1100-2100 from the total 9118 reference files. The main result is
shown below.
![graph](1000genomes_simulated_error_rate_1p_w_lines.png?raw=true "Title")
