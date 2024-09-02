#include <iostream>
#include <bits/stdc++.h>

#include <sdsl/suffix_arrays.hpp>
#include <sdsl/lcp.hpp>

int string_kernel(std::string &text, unsigned k)
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

    std::string kernel;
    bool gap = false;

    for (size_t i = 0; i < B.size(); i++)
    {
        if (!B[i] && text[i] != '$' && text[i] !='#'){
                gap = true;
                kernel.push_back('#');
        }else{
            if(kernel.back() == '#' && text[i] == '$'){
                kernel.back() = text[i];
                    kernel.push_back('\n');            
                gap = true;
            }else if(text[i] == '$' || text[i] == '#'){
                kernel.push_back(text[i]);
                    kernel.push_back('\n');
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
    std::filesystem::path input_path = argv[2];

    std::ifstream in(input_path, std::ios::binary);

    if (!in)
    {
        std::cout << "ERROR: File " << input_path << " does not exist. Exit." << std::endl;
        return 1;
    }

    // Read the file character by character
    std::string kernel;
    // std::string line;

    // while (std::getline(in, line)) {
    //     kernel += line;
    // }

        // Read the file character by character
    char ch;
    while (in.get(ch)) {
        // Skip carriage return characters
        if (ch == '\r' || ch == '\n') {
            continue;
        }

        // Concatenate non-carriage return characters
        kernel += ch;
    }

    // while (in.get(c))
    //     kernel.push_back(c);

    in.close();

    string_kernel(kernel, k);
    // if (!silent)
        std::cout << kernel << std::endl;

    // std::ofstream out(output_path, std::ios::binary);

    // if (!out.is_open())
    // {
    //     std::cout << "ERROR: File " << output_path << " cannot be openned. Exit." << std::endl;
    //     return 1;
    // }

    // out << kernel;
    // out.close();

    return 0;
}