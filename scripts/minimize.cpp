
#include <iostream>
#include <bits/stdc++.h>

#include <boost/program_options.hpp>
#include <sdsl/bit_vectors.hpp>

#define q INT_MAX //  alphabet size with # and $ and N 
#define base CHAR_MAX
size_t sigma = 4;
uint8_t bits;
uint8_t bytes;
uint8_t bits_per_byte;

using namespace std;

unsigned hash_function(unsigned x)
{
    return ((2544 * x + 3937) % 8863);
}

int mmer2int(const string &str, size_t m){
    int sum = 0;
    int add = 1;
    for(size_t i=0;i<m;i++){
        switch (str[i])
        {
        case 'A':
            sum += add;
            break;
        case 'C':
            sum += 2*add;
            break;
        case 'G':
            sum += 3*add;
            break;
        // case 'T':
        //     sum += 4*add;
        //     break;
        }
        add *= sigma;
    }
    return sum;
}

int minimizer_digest(const string &text, size_t m, size_t w)
{
    sdsl::bit_vector B(text.size(), 0);
    unsigned *H = new unsigned[text.size()];
    int h = 1;
    size_t i;

    unsigned minVal;
    unsigned minPos;

    for (size_t i = 0; i <= text.size() - m; i++)
    {
        H[i] = hash_function(mmer2int(text.substr(i,m),m));
        // cout << text.substr(i,m) << ":" << mmer2int(text.substr(i,m),m) << ',' << H[i] <<  endl;
    }

    if (text.size() >= (w + m - 1))
    {
        for (i = 0; i <= (text.size() - m + 1 - w); i++)
        {
            if (text[i + 1 + w] == '$' || text[i + w + 1] == '#')
            {
                i += (1 + w);
                continue;
            }

            minVal = H[i];
            minPos = i;
            for (size_t k = 1; k < w; k++)
            {
                if (H[i + k] < minVal)
                {
                    minVal = H[i + k];
                    minPos = i + k;
                }
            }
            B[minPos] = 1;
        }
    }

    string digest;
    uint8_t byte;

    for (i = 0; i < B.size(); i++)
    {
        if (text[i + m - 1] == '$' || text[i + m - 1] == '#')
        {
            digest.push_back(text[i + m - 1]);
            continue;
        }
        // else{
        //     digest.push_back(' ');
        // }
        if (B[i])
        {
            h = mmer2int(text.substr(i,m),m);
            // cout << h<<":";
            for (size_t i = 0; i < bytes; ++i) {
                // byte = (h >> (8 * i)) & 0xFF;
                byte = (h >> (bits - (i + 1) * bits_per_byte)) & ((1 << bits_per_byte) - 1);
                // cout << bitset<8>(byte+37) << " "<<int(byte+37)<< ',';
                // if(byte != 0)
                digest.push_back(byte+37);
            }
            // cout <<endl;
        }
        // else{
        //     digest.push_back(' ');
        // }
    }

    cout << digest <<endl;
    return 0;
}

// unsigned int bytes_required(unsigned int m) {
//     return ceil((m*log2(4)+6)/8);
// }
unsigned int bits_required(unsigned int m, unsigned int b) {// b is an alphabet size
    // cout <<"max int: "<< pow(b,m+1)-1 << endl;
    // cout <<"bits: "<< ceil(log2(pow(b,m+1)-1)) << endl;
    return ceil(log2(pow(b,m+1)-1));
}

int main(int argc, char **argv)
{
    uint m = atoi(argv[1]);
    uint w = atoi(argv[2]);
    string input = argv[3];

    if (m<=0 || w <= 0 || m>w){
        cout << "parameters has to be non-zero positive values and holds m<w";
        return -1;
    }
    //  get number of bytes required to represent all 
    // unsigned combinations = pow(5,m);

    bits = bits_required(m-1,sigma); //15625
    bytes = ceil((double)bits/7);
    bits_per_byte = bits / bytes;
    // cout << "characters per w: " << (int)bytes << endl;

    try
    {
        ifstream inf(input, ios::binary);
        if (!inf)
        {
            minimizer_digest(input,m,w);
        }else{
            inf.seekg(0, std::ios::end);
            size_t size = inf.tellg();
            std::string buffer(size, ' ');
            inf.seekg(0);
            inf.read(&buffer[0], size); 
            minimizer_digest(buffer,m,w);
            inf.close();
        }
    }
    catch(const exception& e)
    {
        cerr << e.what() << '\n';
    }
    
    return 0;
}

// int main(int argc, char **argv)
// {
//     int m = atoi(argv[1]);
//     int w = atoi(argv[2]);

//     if (m<=0 || w <= 0 || m>w){
//         cout << "parameters has to be non-zero positive values and holds m<w";
//         return -1;
//     }
//     //  get number of bytes required to represent all 
//     // unsigned combinations = pow(5,m);

//     bits = bits_required(m-1,sigma); //15625
//     bytes = ceil((double)bits/7);
//     bits_per_byte = bits / bytes;
//     // cout << "characters per w: " << (int)bytes << endl;
//     string input = argv[3];

//     minimizer_digest(input,m,w);
//     return 0;
// }