#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mtwister.h"

int main(int argc, char **argv) {
	int ell, p1, p2;

	ell = atoi(argv[1]);
	p1 = atoi(argv[2]);
	p2 = atoi(argv[3]);

	MTRand x = seedRand(1337);

	char *tree[63];

	tree[0] = (char *) malloc(ell + 2);

	for (int j = 0;  j < ell; j++) {
		switch ((int) (genRand(&x) * 4)) {
			case 0: tree[0][j] = 'A';
				break;
			case 1: tree[0][j] = 'C';
				break;
			case 2: tree[0][j] = 'G';
				break;
			case 3: tree[0][j] = 'T';
				break;
		}
	}
	tree[0][ell] = '$';
	tree[0][ell + 1] = '\0';

	for (int i = 1; i < 63; i++) {
		tree[i] = (char *) malloc(ell + 2);
		memcpy(tree[i], tree[(int) ((i - 1) / 2)], ell + 2);

		for (int j = 0; j < ell; j++) {
			if ((int) (genRand(&x) * 100) < p1) {
				switch ((int) (genRand(&x) * 4)) {
					case 0: tree[i][j] = 'A';
						break;
					case 1: tree[i][j] = 'C';
						break;
					case 2: tree[i][j] = 'G';
						break;
					case 3: tree[i][j] = 'T';
						break;
				}
			}
		}
	}

	for (int i = 31; i < 63; i++) {
		printf("%s\n", tree[i]);
	}
	

	char *read = (char *) malloc(ell + 1);
	int r = (int) (genRand(&x) * 32);

	memcpy(read, tree[r + 31], ell);
	read[ell] = '\0';

	for (int j = 0; j < ell; j++) {
		if ((int) (genRand(&x) * 100) < p2) {
			switch ((int) (genRand(&x) * 4)) {
				case 0: read[j] = 'A';
					break;
				case 1: read[j] = 'C';
					break;
				case 2: read[j] = 'G';
					break;
				case 3: read[j] = 'T';
					break;
			}
		}
	}

	printf("\n\n%s\n", read);
	printf("\n\n%d\n", r);

	return r;
}