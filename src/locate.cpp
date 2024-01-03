#include <filesystem>
#include <string>
#include <iostream>

#include "utils/Informations_signs.h"
#include "utils/Config.h"
#include "utils/metrics.hpp"
#include "index/index.h"

#include <boost/program_options.hpp>

namespace po = boost::program_options;
using namespace std;

Informations_signs inf;
Config cfg;

int handle_parameters(int argc, char **argv)
{
    po::options_description desc("Allowed options");

    desc.add_options()("help", "produce help message")
        ("help-verbose", "display verbose help message")
        ("version,v", "display version info")
        ("silent,s", "silent mode")
        ("clean,c", "rebuild indexes even they exists")
        // ("repetition,r", po::value<unsigned>(&cfg.repetition), "number of repetition - for experiment needs")
        ("pattern,p", po::value<std::string>(&cfg.pattern), "print occurences of every pattern")
        ("input-file,i", po::value<std::filesystem::path>(&cfg.input_path), "input file")
        ("pattern-file,P", po::value<std::filesystem::path>(&cfg.pattern_file)->required(), "input pattern file path (positional arg 2)");

    po::positional_options_description posOptions;
    posOptions.add("input-file", 1);

    po::variables_map vm;

    try
    {
        po::store(po::command_line_parser(argc, argv).options(desc).positional(posOptions).run(), vm);

        if (vm.count("silent"))
        {
            cfg.silent = true;
        }
        if (vm.count("help"))
        {
            std::cout << "Usage: " << argv[0] << " " << inf.buildUsageInfoString << std::endl
                      << std::endl;
            std::cout << desc << std::endl;

            return 1;
        }
        if (vm.count("help-verbose"))
        {
            std::cout << inf.verboseInfoString << std::endl
                      << std::endl;

            std::cout << "Usage: " << argv[0] << " " << inf.buildUsageInfoString << std::endl
                      << std::endl;

            std::cout << desc << std::endl;
            std::cout << inf.verboseparametersStringBuild << std::endl;

            return 1;
        }
        if (vm.count("version"))
        {
            std::cout << inf.versionInfo << std::endl;
            return 1;
        }
        if (vm.count("input-file") == 0)
        {
            std::cout << "Usage: " << argv[0] << " " << inf.buildUsageInfoString << std::endl
                      << std::endl;
            std::cout << desc << std::endl;

            return -1;
        }
        po::notify(vm);

    }
    catch (const po::error &e)
    {
        std::cerr << "Usage: " << argv[0] << " " << inf.buildUsageInfoString << std::endl
                  << std::endl;
        std::cerr << desc << std::endl;

        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}

void run(filesystem::path index_path){

    //  Load index
    Index index = Index();
    index.build(index_path);

    //  Load patterns
    vector<string> patterns;
    index.read_patterns(cfg.pattern_file,patterns);
    if(!cfg.pattern.empty())
        patterns.push_back(cfg.pattern);

    //  locate patterns
    for (size_t i = 0; i < patterns.size(); i++)
    {   
        index.locate(patterns[i]);
        index.print_MEMs(index.occurences,patterns[i]);
        index.occurences.clear();
    }
}

int main(int argc, char **argv){

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
    
    run(cfg.input_path); //  create classic FM-index and RMQ over SA usign sdsl library
    
    return 0;
}