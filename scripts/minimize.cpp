
#include <iostream>
#include <bits/stdc++.h>

#include <sdsl/bit_vectors.hpp>

using namespace std;

// Alphabet size (A/C/G/T). N and separators are handled separately.
static const size_t sigma = 4;

// ==========================================================================
//  m-mer identity encoding
//  Maps the m-mer text[pos..pos+m) to a unique integer in [0, 4^m).
//  MUST be injective: equal m-mers -> equal value -> equal digest bytes,
//  which is what preserves MEMs through the digest. (T -> 0, A -> 1, C -> 2,
//  G -> 3.)
// ==========================================================================
uint64_t mmer2int(const string &s, size_t pos, size_t m)
{
    uint64_t val = 0;
    uint64_t radix = 1;
    for (size_t i = 0; i < m; i++)
    {
        switch (s[pos + i])
        {
        case 'A': val += 1 * radix; break;
        case 'C': val += 2 * radix; break;
        case 'G': val += 3 * radix; break;
        case 'T': /* 0 */ break;
        }
        radix *= sigma;
    }
    return val;
}

// ==========================================================================
//  k-mer hash functions (used ONLY to select minimizers, never as identity)
//  Sources: http://www.cse.yorku.ca/~oz/hash.html
// ==========================================================================
uint64_t djb2(const string &s, size_t pos, size_t len)
{
    uint64_t h = 5381;
    for (size_t i = 0; i < len; i++)
        h = ((h << 5) + h) + (unsigned char)s[pos + i];
    return h;
}
uint64_t sdbm(const string &s, size_t pos, size_t len)
{
    uint64_t h = 0;
    for (size_t i = 0; i < len; i++)
        h = (unsigned char)s[pos + i] + (h << 6) + (h << 16) - h;
    return h;
}
uint64_t loselose(const string &s, size_t pos, size_t len)
{
    uint64_t h = 0;
    for (size_t i = 0; i < len; i++)
        h += (unsigned char)s[pos + i];
    return h;
}
uint64_t mult(const string &s, size_t pos, size_t len)
{
    return (2544ULL * mmer2int(s, pos, len) + 3937) % 8863;
}

typedef uint64_t (*hash_fn)(const string &, size_t, size_t);

hash_fn select_hash(const string &name)
{
    if (name == "sdbm") return sdbm;
    if (name == "loselose") return loselose;
    if (name == "mult") return mult;
    return djb2; // default
}

// ==========================================================================
//  Minimizer digest. For every window of w consecutive m-mers, the m-mer with
//  the smallest hash is marked. The digest concatenates the injective
//  encodings of the marked m-mers (ceil(2m/7) bytes, 7 identity bits each,
//  offset by +37 so digest bytes never collide with '$' (36) or '#' (35)),
//  copying '$'/'#' separators through.
// ==========================================================================
int minimizer_digest(const string &text, size_t m, size_t w, hash_fn hash)
{
    const size_t n = text.size();
    if (n < m)
    {
        cout << endl;
        return 0;
    }

    const unsigned id_bits = 2 * m;           // 4^m == 2^(2m)
    const unsigned bytes = (id_bits + 6) / 7; // ceil(id_bits / 7)

    // 1) Hash every m-mer; m-mers straddling a separator are never chosen.
    vector<uint64_t> H(n, numeric_limits<uint64_t>::max());
    for (size_t i = 0; i + m <= n; i++)
    {
        bool has_sep = false;
        for (size_t j = i; j < i + m; j++)
            if (text[j] == '$' || text[j] == '#') { has_sep = true; break; }
        if (!has_sep)
            H[i] = hash(text, i, m);
    }

    // 2) Slide a window of w consecutive m-mers, mark the minimum-hash one.
    sdsl::bit_vector B(n, 0);
    if (n >= m + w - 1)
    {
        for (size_t i = 0; i + m + w - 1 <= n; i++)
        {
            uint64_t minVal = numeric_limits<uint64_t>::max();
            size_t minPos = i;
            bool any = false;
            for (size_t k = 0; k < w; k++)
                if (H[i + k] < minVal) { minVal = H[i + k]; minPos = i + k; any = true; }
            if (any)
                B[minPos] = 1;
        }
    }

    // 3) Emit the digest.
    string digest;
    for (size_t i = 0; i < n; i++)
    {
        size_t end = i + m - 1;
        if (end < n && (text[end] == '$' || text[end] == '#'))
        {
            digest.push_back(text[end]);
            continue;
        }
        if (i + m <= n && B[i])
        {
            uint64_t val = mmer2int(text, i, m);
            for (unsigned b = 0; b < bytes; b++)
            {
                unsigned shift = 7 * (bytes - 1 - b);
                uint8_t group = (val >> shift) & 0x7F;
                digest.push_back((char)(group + 37));
            }
        }
    }

    cout << digest << endl;
    return 0;
}

int main(int argc, char **argv)
{
    if (argc < 4)
    {
        cerr << "Usage: " << argv[0] << " <m> <w> <file|sequence> [hash]" << endl;
        cerr << "  hash: djb2 (default) | sdbm | loselose | mult" << endl;
        return -1;
    }

    int m = atoi(argv[1]);
    int w = atoi(argv[2]);
    string input = argv[3];
    hash_fn hash = select_hash(argc >= 5 ? string(argv[4]) : string("djb2"));

    if (m <= 0 || w <= 0)
    {
        cerr << "parameters have to be non-zero positive values" << endl;
        return -1;
    }

    try
    {
        ifstream inf(input, ios::binary);
        if (!inf)
        {
            minimizer_digest(input, m, w, hash);
        }
        else
        {
            inf.seekg(0, std::ios::end);
            size_t size = inf.tellg();
            std::string buffer(size, ' ');
            inf.seekg(0);
            inf.read(&buffer[0], size);
            minimizer_digest(buffer, m, w, hash);
            inf.close();
        }
    }
    catch (const exception &e)
    {
        cerr << e.what() << '\n';
    }

    return 0;
}
