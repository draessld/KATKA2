
#include <iostream>
#include <bits/stdc++.h>

#include <boost/program_options.hpp>
#include <sdsl/bit_vectors.hpp>

namespace po = boost::program_options;

const std::string usageInfoString = "";
const std::string desc = "";

typedef struct Config
{
    uint32_t w;
    bool printable = false;
    bool silent = false;
    std::filesystem::path output_path;
    std::filesystem::path input_path;
    std::string suffix = ".dgt";
} Config;

Config cfg;

int handle_parameters(int argc, char **argv)
{
    po::options_description desc("Allowed options");

    desc.add_options()("help", "produce help message")
    // ("help-verbose", "display verbose help message")
    ("printable,p", "to see hashed values")
    // ("silent,s", "silent mode")
    ("window_size,w", po::value<uint32_t>(&cfg.w), "lookahead window size")
    // ("output-file,o", po::value<std::filesystem::path>(&cfg.output_path), "output file")
    ("input-file,i", po::value<std::filesystem::path>(&cfg.input_path), "input file");

    po::positional_options_description posOptions;
    posOptions.add("input-file", 1);
    posOptions.add("window_size", 1);

    po::variables_map vm;
    try
    {
        po::store(po::command_line_parser(argc, argv).options(desc).positional(posOptions).run(), vm);
        if (vm.count("help"))
        {
            std::cout << "Usage: " << argv[0] << " " << usageInfoString << std::endl
                      << std::endl;
            std::cout << desc << std::endl;

            return 1;
        }
        if (vm.count("printable"))
        {
            cfg.printable = true;
        }
        // if (vm.count("silent"))
        // {
        //     cfg.silent = true;
        // }
        if (vm.count("help-verbose"))
        {
            std::cout << desc << std::endl;
            std::cout << "Usage: " << argv[0] << " " << usageInfoString << std::endl
                      << std::endl;

            return 1;
        }
        if (vm.count("input-file") == 0)
        {
            std::cout << "Usage: " << argv[0] << " " << usageInfoString << std::endl
                      << std::endl;
            std::cout << desc << std::endl;

            return -1;
        }
        po::notify(vm);
    }
    catch (const po::error &e)
    {
        std::cerr << "Usage: " << argv[0] << " " << usageInfoString << std::endl
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
    for (size_t i = 0; i < text.size() - w; i++)
    {
        if (text[i + w] == '$')
        {
            i += w;
            continue;
        }
        int minVal = text[i];
        int minPos = i;
        for (size_t k = i + 1; k < (i + w); k++)
        {
            if (text[k] < minVal)
            {
                minVal = text[k];
                minPos = k;
            }
        }
        B[minPos] = 1;
    }

    size_t j = 0;
    for (size_t i = 0; i < B.size(); i++)
    {
        if (B[i] || text[j] == '$' || text[j] == '#')
            j++;
        else
            text.erase(j, 1);
    }

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

    //  read
    std::ifstream in(cfg.input_path, std::ios::binary);

    if (!in)
    {
        std::cout << "ERROR: File " << cfg.input_path << " does not exist. Exit." << std::endl;
        return 1;
    }

    // Read the file character by character
    char c;
    int i = 0;
    char buffer[3];
    int skip = 2;

    std::string digest;

    while (in.get(c))
    {
        buffer[i % 3] = c;
        if (c == '$' || c == '#')
        {
            digest.push_back('$');
            skip = 3;
        }
        i++;
        if ((skip--) <= 0)
        {
            // std::cout << buffer[i%3] << buffer[(i+1)%3] << buffer[(i+2)%3] <<':'<< triple2int(buffer[i%3],buffer[(i+1)%3], buffer[(i+2)%3])+37 << std::endl;;
            if (cfg.printable)
            {
                digest.push_back(triple2int(buffer[i % 3], buffer[(i + 1) % 3], buffer[(i + 2) % 3]) + 37);
            }
            else
            {
                digest.push_back(hash_function(triple2int(buffer[i % 3], buffer[(i + 1) % 3], buffer[(i + 2) % 3])));
            }
        }
    }

    in.close();

    minimizer_digest(digest, cfg.w);
    // if (!cfg.silent)
    std::cout << digest << std::endl;

    // std::ofstream out(cfg.output_path, std::ios::binary);

    // if (!out.is_open())
    // {
    //     std::cout << "ERROR: File " << cfg.output_path << " cannot be openned. Exit." << std::endl;
    //     return 1;
    // }

    // out << digest;
    // out.close();

    return 0;
}
