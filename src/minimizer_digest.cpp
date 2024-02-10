
#include <iostream>
#include <bits/stdc++.h>

#include <boost/program_options.hpp>
#include <sdsl/bit_vectors.hpp>

namespace po = boost::program_options;

const std::string usage = "";

typedef struct Config
{
    int w;
    std::filesystem::path input_path;
    std::string suffix = ".dgt";
    std::string pattern;

} Config;

Config cfg;

int handle_parameters(int argc, char **argv)
{
    po::options_description desc("Allowed options");

    desc.add_options()("help", "produce help message")("window_size,w", po::value<int>(&cfg.w), "lookahead window size")("input-sequence,s", po::value<std::string>(&cfg.pattern), "simple sequence input")("input-file,i", po::value<std::filesystem::path>(&cfg.input_path), "input file");

    po::positional_options_description posOptions;
    posOptions.add("window_size", 1);

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

int hash_function(int x)
{
    return ((2544 * x + 3937) % 8863);
}

int triple2int(char a, char b, char c)
{
    int sum = 0;

    switch (a)
    {
    case 'C':
        sum = 1;
        break;
    case 'G':
        sum = 2;
        break;
    case 'T':
        sum = 3;
        break;
    }

    switch (b)
    {
    case 'C':
        sum += 4;
        break;
    case 'G':
        sum += 8;
        break;
    case 'T':
        sum += 12;
        break;
    }

    switch (c)
    {
    case 'C':
        sum += 16;
        break;
    case 'G':
        sum += 32;
        break;
    case 'T':
        sum += 48;
        break;
    }

    return (sum);
}

int minimizer_digest(std::string &text, unsigned w)
{
    sdsl::bit_vector B(text.size(), 0);
    int *H = new int[text.size()];

    for (size_t i = 0; i <= text.size() - 3; i++)
    {
        H[i] = hash_function(triple2int(text[i], text[i + 1], text[i + 2]));
    }

    if (text.size() >= (w + 2))
    {
        for (size_t i = 0; i <= (text.size() - 2 - w); i++)
        {
            if (text[i + 1 + w] == '$' || text[i + w + 1] == '#')
            {
                i += (1 + w);
                continue;
            }

            int minVal = H[i];
            int minPos = i;
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

    std::string digest;

    for (size_t i = 0; i < B.size(); i++)
    {
        if (text[i + 2] == '$' || text[i + 2] == '#')
        {
            digest.push_back(text[i + 2]);
            continue;
        }
        if (B[i])
        {
            digest.push_back((char)(triple2int(text[i], text[i + 1], text[i + 2]) + 37));
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
            if (!in)
            {
                std::cout << "ERROR: File " << cfg.input_path << " does not exist. Exit." << std::endl;
                return 1;
            }
        }
        in.close();
        minimizer_digest(digest, cfg.w);
        std::cout << digest << std::endl;
    }

    if (!cfg.pattern.empty())
    {
        minimizer_digest(cfg.pattern, cfg.w);
        std::cout << cfg.pattern << std::endl;
    }

    return 0;
}
