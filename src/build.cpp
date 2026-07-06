#include <filesystem>
#include <string>
#include <iostream>

#include "index/index.h"

#include <boost/program_options.hpp>

namespace po = boost::program_options;
using namespace std;

std::string version = "1.0.0";
std::string usage = "<input_file>";

std::filesystem::path input_path;
bool rebuild; //  rebuild even indexes exists

int handle_parameters(int argc, char **argv)
{
    po::options_description desc("Allowed options");

    desc.add_options()("help", "produce help message")("version,v", "display version info")("rebuild,c", "build data structures even they already exist")("input-file,i", po::value<std::filesystem::path>(&input_path), "input file");

    po::positional_options_description posOptions;
    posOptions.add("input-file", 1);

    po::variables_map vm;

    try
    {
        po::store(po::command_line_parser(argc, argv).options(desc).positional(posOptions).run(), vm);
        if (vm.count("rebuild"))
        {
            rebuild = true;
        }
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
        if (vm.count("input-file") == 0)
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

void run()
{
    // Measurements
    Index index = Index(rebuild, input_path); //  load or build index
    double build_result = index.build();

    std::cout << "Build time: " << build_result << "μs" << std::endl;
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