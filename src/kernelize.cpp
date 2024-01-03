
#include <iostream>
#include <bits/stdc++.h>

#include <sdsl/suffix_arrays.hpp>
#include <sdsl/lcp.hpp>

#include <boost/program_options.hpp>

namespace po = boost::program_options;

const std::string usageInfoString = "";
const std::string desc = "";

typedef struct Config
{
    uint32_t k;
    bool silent = false;
    std::filesystem::path output_path;
    std::filesystem::path input_path;
    std::string suffix = ".krl";
} Config;

Config cfg;

int handle_parameters(int argc, char **argv)
{
    po::options_description desc("Allowed options");

    desc.add_options()("help", "produce help message")("help-verbose", "display verbose help message")("silent,s", "silent mode")("kmer_length,k", po::value<uint32_t>(&cfg.k), "kmer")("output-file,o", po::value<std::filesystem::path>(&cfg.output_path), "output file")("input-file,i", po::value<std::filesystem::path>(&cfg.input_path), "input file");

    po::positional_options_description posOptions;
    posOptions.add("input-file", 1);
    posOptions.add("kmer_length", 1);

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
        if (vm.count("help-verbose"))
        {
            std::cout << desc << std::endl;
            std::cout << "Usage: " << argv[0] << " " << usageInfoString << std::endl
                      << std::endl;

            return 1;
        }
        if (vm.count("silent"))
        {
            cfg.silent = true;
        }
        if (vm.count("input-file") == 0)
        {
            std::cout << "Usage: " << argv[0] << " " << usageInfoString << std::endl
                      << std::endl;
            std::cout << desc << std::endl;

            return -1;
        }
        po::notify(vm);

        if (vm.count("basefolder") == 0)
        {
            std::ostringstream oss;
            oss << cfg.input_path.parent_path().c_str() << "/" << cfg.input_path.filename().replace_extension("").c_str() << "/k" << cfg.k << cfg.input_path.filename().replace_extension(cfg.suffix).c_str();
            cfg.output_path = oss.str();
            if (!std::filesystem::exists(cfg.output_path.parent_path()))
            {
                std::filesystem::create_directories(cfg.output_path.parent_path());
            }
        }
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

int string_kernel(std::string &text, char c, unsigned k)
{
    // create SA, LCP and B

    sdsl::csa_bitcompressed<> sa;
    sdsl::lcp_bitcompressed<> lcp;
    sdsl::construct_im(sa, text, 1);
    sdsl::construct_im(lcp, text, 1);
    int n = text.size();

    sdsl::bit_vector B(n, 0);

    uint32_t min = n;
    int max = 0;
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
            for (int j = min; j < (min + k) && j < n; j++)
            {
                if (text[j] == '$' || text[j] == '#')
                    flag = false;
            }

            if (flag)
            {
                for (int j = min; j < min + k && j < n; j++)
                    B[j] = 1;

                for (int j = max; j < max + k && j < n; j++)
                    B[j] = 1;
            }

            min = n;
            max = -1;
        }
    }

    for (size_t i = 0; i < B.size(); i++)
{
        if (B[i] || text[i] == '$'){
        }else{
            text[i] = '-';
        }

}

    // flag = false;
    // size_t j = 0;
    // for (size_t i = 0; i < B.size(); i++)
    // {
    //     if (B[i] || text[j] == '$')
    //     {
    //         if (text[j] == '$')
    //         {
    //             if (flag)
    //             {
    //                 text.erase(--j, 1);
    //                 // text[--j] = '-';
    //             }
    //             flag = true;
    //         }
    //         else
    //             flag = false;
    //         j++;
    //     }
    //     else
    //     {
    //         if (!flag)
    //         {
    //             text[j++] = c;
    //             flag = true;
    //         }
    //         else
    //         {
    //             // text.erase(j, 1);
    //             text[j] = '-';
    //         }
    //     }
    // }

    // std::cout << text << std::endl;

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
    std::string kernel;

    while (in.get(c))
        kernel.push_back(c);

    in.close();

    string_kernel(kernel, '#', cfg.k);
    if (!cfg.silent)
        std::cout << kernel << std::endl;

    std::ofstream out(cfg.output_path, std::ios::binary);

    if (!out.is_open())
    {
        std::cout << "ERROR: File " << cfg.output_path << " cannot be openned. Exit." << std::endl;
        return 1;
    }

    out << kernel;
    out.close();

    return 0;
}
