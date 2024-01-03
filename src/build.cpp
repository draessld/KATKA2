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
        ("rebuild,c", "rebuild indexes even they already exist")
        // ("repetition,r", po::value<unsigned>(&cfg.repetition), "number of repetition - for experiment needs")
        ("basefolder,o", po::value<std::filesystem::path>(&cfg.output_path), "use <basefolder> as prefix for all index files. Default: current folder is the specified input_file_name")
        ("input-file,i", po::value<std::filesystem::path>(&cfg.input_path), "input file");

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
        if (vm.count("rebuild"))
        {
            cfg.rebuild = true;
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

        if (vm.count("basename") == 0)
        {   
            cfg.output_path = cfg.input_path;
        }
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

void run()
{
    // Measurements
    double time_baseline;
    long mem_baseline;
    std::vector<double> times;
    std::vector<long> mems;

    mem_baseline = get_mem_usage();
    time_baseline = get_time_usage();

    //  BUILD
    Index index = Index(cfg.rebuild); //  load or build index
    index.build(cfg.input_path);

    time_baseline = get_time_usage() - time_baseline;
    mem_baseline = get_mem_usage() - mem_baseline;

    std::cout << "Build time: " << time_baseline << std::endl;
    std::cout << "Peak RAM usage: " << mem_baseline << " kB" << std::endl;
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

    run();

    return 0;
}