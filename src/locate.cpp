#include <filesystem>
#include <string>
#include <iostream>

#include "index/index.h"

#include <boost/program_options.hpp>

std::string version = "1.0.0";
std::string usage = "<input_file> <pattern>";

std::string pattern;
std::filesystem::path pattern_file;
std::filesystem::path input_path;

namespace po = boost::program_options;
using namespace std;

int handle_parameters(int argc, char **argv)
{
    po::options_description desc("Allowed options");

    desc.add_options()("help", "produce help message")("version,v", "display version info")("pattern,p", po::value<std::string>(&pattern), "print occurences of every pattern")("index-file,i", po::value<std::filesystem::path>(&input_path)->required(), "index file")("pattern-file,P", po::value<std::filesystem::path>(&pattern_file), "input pattern file path (positional arg 2)");

    po::positional_options_description posOptions;
    posOptions.add("index-file", 1);

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
        if (vm.count("version"))
        {
            std::cout << version << std::endl;
            return 1;
        }
        if (vm.count("index-file") == 0)
        {
            std::cout << "Usage: " << argv[0] << " " << usage << std::endl
                      << std::endl;
            std::cout << desc << std::endl;

            return -1;
        }
        po::notify(vm);
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

int read_patterns(std::filesystem::path file_path, std::vector<std::string> &patterns)
{
    // std::cout << "-=-=-=-=-=-   Reading Patterns   ..."   <<    std::endl;

    std::ifstream in(file_path);
    unsigned number_of_patterns = 0;

    if (!in.is_open())
    {
        std::cerr << "Error opening file: " << file_path << std::endl;
        return 1; // Return an error code
    }

    std::string line;
    while (std::getline(in, line))
    {
        patterns.push_back(line);
        number_of_patterns++;
    }

    in.close();
    // std::cout << "-=-=-=-=-=-   Reading Patterns - DONE, total number:" << patterns.size()   <<    std::endl;

    return 0;
}

template <typename T>
void print_MEMs(std::vector<T> occurences, std::string pattern)
{
    for (size_t i = 0; i < occurences.size(); i++)
    {
        // std::cout << pattern.substr(occurences[i].index,occurences[i].length)<< '\t' << occurences[i].index << '\t' <<  occurences[i].length << '\t' << occurences[i].first_occ << '\t' << occurences[i].last_occ << std::endl;
        std::cout << '[' << occurences[i].index << ',' << occurences[i].index + occurences[i].length << "]{" << occurences[i].first_occ << ',' << occurences[i].last_occ << "}\t";
    }
    std::cout << std::endl;
}

void run(filesystem::path index_path)
{

    //  Load index
    Index index = Index(index_path);
    index.build();

    //  Load patterns
    vector<string> patterns;

    if (!pattern_file.empty())
        read_patterns(pattern_file, patterns);
    if (!pattern.empty())
        patterns.push_back(pattern);

    //  locate patterns
    for (size_t i = 0; i < patterns.size(); i++)
    {
        std::cout << '>' << patterns[i] << '\t' << index.locate(patterns[i]) << "us" << '\t' << index.occurences.size() << std::endl;
        print_MEMs(index.occurences, patterns[i]);
        index.occurences.clear();
    }
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

    run(input_path);

    return 0;
}