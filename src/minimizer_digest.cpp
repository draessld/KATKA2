
#include <iostream>
#include <bits/stdc++.h>

#include <boost/program_options.hpp>
#include <sdsl/bit_vectors.hpp>

namespace po = boost::program_options;

const std::string usage = "-w<window> [-k<m-mer length, default 3>] [-H<hash>] (-i<file> | -s<sequence>)";

typedef struct Config
{
    int w = 0;                 // minimizer window: number of consecutive m-mers looked at
    int k = 3;                 // m-mer length (paper uses 3). Named 'k' for CLI back-compat.
    std::string hash_name = "djb2"; // hash used only to SELECT minimizers
    std::filesystem::path input_path;
    std::string suffix = ".dgt";
    std::string pattern;

} Config;

Config cfg;

int handle_parameters(int argc, char **argv)
{
    po::options_description desc("Allowed options");

    desc.add_options()("help", "produce help message")("window_size,w", po::value<int>(&cfg.w), "minimizer window size (#consecutive m-mers)")("k_mer_size,k", po::value<int>(&cfg.k), "m-mer length (default 3)")("hash,H", po::value<std::string>(&cfg.hash_name), "selection hash: djb2|sdbm|loselose|mult (default djb2)")("input-sequence,s", po::value<std::string>(&cfg.pattern), "simple sequence input")("input-file,i", po::value<std::filesystem::path>(&cfg.input_path), "input file");

    po::positional_options_description posOptions;
    posOptions.add("window_size", 1);
    posOptions.add("k_mer_size", 1);

    po::variables_map vm;
    try
    {
        po::store(po::command_line_parser(argc, argv).options(desc).positional(posOptions).run(), vm);
        if (vm.count("help"))
        {
            std::cout << "Usage: " << argv[0] << " " << usage << std::endl
                      << std::endl;
            std::cout << desc << std::endl;

            return 1;
        }
        po::notify(vm);
        if (vm.count("input-file") == 0 && vm.count("input-sequence") == 0)
        {
            std::cout << "Usage: " << argv[0] << " " << usage << std::endl
                      << std::endl;
            std::cout << desc << std::endl;

            return -1;
        }
    }
    catch (const po::error &e)
    {
        std::cerr << "Usage: " << argv[0] << " " << usage << std::endl
                  << std::endl;
        std::cerr << desc << std::endl;

        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}

// ==========================================================================
//  m-mer identity encoding
//  Maps the m-mer text[pos..pos+m) to a unique integer in [0, 4^m).
//  This is what the DIGEST characters are built from, so it MUST be
//  injective: equal m-mers -> equal value -> equal digest bytes, which is
//  what preserves MEMs through the digest. (T -> 0, A -> 1, C -> 2, G -> 3.)
// ==========================================================================
uint64_t mmer2int(const std::string &s, size_t pos, size_t m)
{
    uint64_t val = 0;
    uint64_t radix = 1;
    for (size_t i = 0; i < m; i++)
    {
        switch (s[pos + i])
        {
        case 'A':
            val += 1 * radix;
            break;
        case 'C':
            val += 2 * radix;
            break;
        case 'G':
            val += 3 * radix;
            break;
        case 'T':
            // T -> 0
            break;
        }
        radix *= 4;
    }
    return val;
}

// ==========================================================================
//  k-mer hash functions (used ONLY to select minimizers, never as identity)
//  Each hashes the m-mer text[pos..pos+len). Any well-spread hash works;
//  a better spread means fewer ties and more evenly placed minimizers.
//  Sources: http://www.cse.yorku.ca/~oz/hash.html
// ==========================================================================
uint64_t djb2(const std::string &s, size_t pos, size_t len)
{
    uint64_t hash = 5381;
    for (size_t i = 0; i < len; i++)
        hash = ((hash << 5) + hash) + (unsigned char)s[pos + i]; // hash * 33 + c
    return hash;
}

uint64_t sdbm(const std::string &s, size_t pos, size_t len)
{
    uint64_t hash = 0;
    for (size_t i = 0; i < len; i++)
        hash = (unsigned char)s[pos + i] + (hash << 6) + (hash << 16) - hash;
    return hash;
}

uint64_t loselose(const std::string &s, size_t pos, size_t len)
{
    uint64_t hash = 0;
    for (size_t i = 0; i < len; i++)
        hash += (unsigned char)s[pos + i];
    return hash;
}

// Original KATKA2 hash: multiplicative over the m-mer identity. Small range
// (mod 8863) so it ties often; kept for comparison with the paper.
uint64_t mult(const std::string &s, size_t pos, size_t len)
{
    return (2544ULL * mmer2int(s, pos, len) + 3937) % 8863;
}

typedef uint64_t (*hash_fn)(const std::string &, size_t, size_t);

hash_fn select_hash(const std::string &name)
{
    if (name == "djb2")
        return djb2;
    if (name == "sdbm")
        return sdbm;
    if (name == "loselose")
        return loselose;
    if (name == "mult")
        return mult;
    std::cerr << "Unknown hash '" << name << "', falling back to djb2." << std::endl;
    return djb2;
}

// ==========================================================================
//  Minimizer digest
//    m    m-mer length
//    w    window size (number of consecutive m-mers)
//    hash selection hash
//
//  For every window of w consecutive m-mers, the m-mer with the smallest
//  hash is marked (the minimizer). The digest is the concatenation of the
//  injective encodings of the marked m-mers, with '$'/'#' separators copied
//  through. Each m-mer is emitted as ceil(2m/7) bytes, 7 identity bits each,
//  offset by +37 so digest bytes never collide with '$' (36) or '#' (35).
// ==========================================================================
int minimizer_digest(std::string &text, unsigned m, unsigned w, hash_fn hash)
{
    const size_t n = text.size();
    if (n < m)
    {
        // Nothing to digest: too short to hold a single m-mer.
        text.clear();
        return 0;
    }

    // How many 7-bit bytes are needed to encode an m-mer identity (< 4^m).
    const unsigned id_bits = 2 * m;                 // 4^m == 2^(2m)
    const unsigned bytes = (id_bits + 6) / 7;       // ceil(id_bits / 7)

    // 1) Hash every m-mer. m-mers straddling a separator are invalid and get
    //    the maximum value so they are never chosen as a minimizer.
    std::vector<uint64_t> H(n, std::numeric_limits<uint64_t>::max());
    for (size_t i = 0; i + m <= n; i++)
    {
        bool has_sep = false;
        for (size_t j = i; j < i + m; j++)
            if (text[j] == '$' || text[j] == '#')
            {
                has_sep = true;
                break;
            }
        if (!has_sep)
            H[i] = hash(text, i, m);
    }

    // 2) Slide a window of w consecutive m-mers, mark the minimum-hash one.
    sdsl::bit_vector B(n, 0);
    if (n >= m + w - 1)
    {
        for (size_t i = 0; i + m + w - 1 <= n; i++)
        {
            uint64_t minVal = std::numeric_limits<uint64_t>::max();
            size_t minPos = i;
            bool any = false;
            for (size_t k = 0; k < w; k++)
            {
                if (H[i + k] < minVal)
                {
                    minVal = H[i + k];
                    minPos = i + k;
                    any = true;
                }
            }
            if (any)
                B[minPos] = 1;
        }
    }

    // 3) Emit the digest: separators pass through, marked m-mers are encoded.
    std::string digest;
    for (size_t i = 0; i < n; i++)
    {
        size_t end = i + m - 1; // last position of the m-mer starting at i
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
                unsigned shift = 7 * (bytes - 1 - b); // most-significant group first
                uint8_t group = (val >> shift) & 0x7F;
                digest.push_back((char)(group + 37));
            }
        }
    }

    text = digest;
    return 0;
}

int main(int argc, char **argv)
{
    /*  Handle input parameters */
    int parameter_handle_result = handle_parameters(argc, argv);
    if (parameter_handle_result == -1)
    {
        std::cout << "Error while reading parameters\n"
                  << std::endl;
        return EXIT_FAILURE;
    }
    else if (parameter_handle_result == 1)
    {
        return EXIT_SUCCESS;
    }

    if (cfg.k < 1 || cfg.w < 1)
    {
        std::cerr << "Error: both -k (m-mer length) and -w (window) must be >= 1." << std::endl;
        return EXIT_FAILURE;
    }

    hash_fn hash = select_hash(cfg.hash_name);

    if (!cfg.input_path.empty())
    {
        std::string digest;
        std::ifstream in(cfg.input_path, std::ios::binary);
        if (!in)
        {
            std::cout << "ERROR: File " << cfg.input_path << " does not exist. Exit." << std::endl;
            return 1;
        }

        char c;
        while (in.get(c))
        {
            if (c == '\t' || c == '\n')
                continue;
            digest.push_back(c);
        }
        in.close();
        minimizer_digest(digest, (unsigned)cfg.k, (unsigned)cfg.w, hash);
        std::cout << digest << std::endl;
    }

    if (!cfg.pattern.empty())
    {
        minimizer_digest(cfg.pattern, (unsigned)cfg.k, (unsigned)cfg.w, hash);
        std::cout << cfg.pattern << std::endl;
    }

    return 0;
}
