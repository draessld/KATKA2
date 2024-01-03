/*
 * TODO in future 
 *  number of genomes as arugment, now constant of 32 genomes (tree of size 2^5) 
 *  universal tree, not only full binary
 *  variable genome size, now given as argument
*/

#include <iostream>
#include <vector>
#include <cmath>

#define SEED 1337
#define ALPHABET "ACGT"

using namespace std;

std::string usage =
"create_tree [options] <genome_size> <mutation_probability> <#_of_genomes>";

void get_genome(int size, string& to_fill){
    for (int j = 0;  j < size; j++) {
        to_fill.push_back(ALPHABET[rand() %4]); 
	}
	to_fill.push_back('$');
}

void mutate(int size, string& from, string& to, const int prob){
    const int treshold = prob*RAND_MAX;
    bool mutate = false;
    for (int i = 0;  i < size; i++) {
        mutate = (rand() < treshold);
        if(mutate)
            to.push_back(ALPHABET[rand() %4]); //  TODO there should be avoided to mutate on the same character
        else
            to.push_back(from[i]);
	}
}


// void print_binary_tree(vector<string> tree, const int genome_size, int depth){
//     //TODO
// }

int main(int argc, char const *argv[])
{
    if (argc<4){
        std::cout << usage << std::endl;
        return 0;
    }

	const int genome_size = atoi(argv[1]);	//	genome size
	const int prob = atoi(argv[2]);	// probability - integer transfered into double using 
    const int genomes = atoi(argv[3]);
    const int tree_depth = log2(genomes)+1;

    const uint32_t total_number_of_genomes = 1<<tree_depth;    //  number of genomes in the tree (in)
    // const int N = total_number_of_genomes * genome_size;   //  total size of the tree (number of genomes * length of genome)  

    // cout << genome_size << ", " << prob << ", " << total_number_of_genomes << ", " << N << endl;

    srand(SEED);

    //  alocate space for the tree
    vector<string> tree(total_number_of_genomes, "");

    get_genome(genome_size, tree[0]);

    for (size_t i = 1; i < total_number_of_genomes; i++)
    {
        mutate(genome_size, tree[(int) ((i - 1) / 2)], tree[i], prob);
	    tree[i].push_back('$');
    }
    
    //  print concatenated leaves
    for (size_t i = (total_number_of_genomes>>1); i < total_number_of_genomes; i++) {
        cout << tree[i];
	}

    tree.clear();

    return 0;
}
