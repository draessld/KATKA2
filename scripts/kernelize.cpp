#include <iostream>
#include <bits/stdc++.h>

#include <sdsl/suffix_arrays.hpp>
#include <sdsl/lcp.hpp>

using namespace std;

int string_kernel(string &text, unsigned k)
{
    // create SA, LCP and B
    sdsl::csa_bitcompressed<> sa;
    sdsl::lcp_bitcompressed<> lcp;
    sdsl::construct_im(sa, text, 1);
    sdsl::construct_im(lcp, text, 1);
    int n = text.size();

    sdsl::bit_vector B(n, 0);

    uint32_t min = n;
    int max = -1;
    bool flag;

    for (int i = n - 1; i >= 0; i--)
    {
        if (sa[i + 1] < min)
            min = sa[i + 1];

        if ((int)sa[i + 1] > max)
            max = sa[i + 1];

        if (lcp[i + 1] < k)
        {
            flag = true;
            for (int j = min; j < (int)(min + k) && j < n; j++)
            {
                if (text[j] == '$' || text[j] == '#')
                    flag = false;
            }

            if (flag)
            {
                for (int j = min; j < (int)(min + k) && j < n; j++)
                    B[j] = 1;

                for (int j = max; j < (int)(max + k) && j < n; j++)
                    B[j] = 1;
            }

            min = n;
            max = -1;
        }
    }
    string kernel;
    bool gap = false;

    for (size_t i = 0; i < B.size(); i++)
    {
        if (!B[i] && text[i] != '$' && text[i] !='#'){
                if(!gap){
                    gap = true;
                    kernel.push_back('#');
                }
        }else{
            if(kernel.back() == '#' && text[i] == '$'){
                kernel.back() = text[i];
                gap = true;
            }else if(text[i] == '$' || text[i] == '#'){
                kernel.push_back(text[i]);
                gap = true;
            }else{
                kernel.push_back(text[i]);
                gap = false;
            }
        }
    }

    text = kernel;

    return 0;
}

int main(int argc, char **argv)
{
    int k = atoi(argv[1]);
    string input = argv[2];

    string_kernel(input, k);
        cout << input;

    return 0;
}