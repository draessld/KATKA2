#include <filesystem>
#include <string>
#include <iostream>

#include "utils/Informations_signs.h"
#include "utils/Config.h"
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
        ("rebuild,c", "rebuild data structures even they already exist")
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

// std::string fastaToConcatenatedFile(std::filesystem::path& fastaFile) {
//     //  check if txt does not exists
//     std::string ext = fastaFile.extension();
//     if (std::filesystem::exists(fastaFile.replace_extension(".txt"))){
//         std::cout << fastaFile << " has been already created and found on the required location" << std::endl;
//         return fastaFile;
//     }
//     fastaFile.replace_extension(ext);

//     std::ifstream inputFasta(fastaFile);
//     std::string line;
//     std::string concatenatedSequence;

// while (std::getline(inputFasta, line)) {
//         if (line[0] != '>')
//             concatenatedSequence += line;
//         if (line[0] == '>' && !concatenatedSequence.empty())
//             concatenatedSequence += '$';            
//     }

//     // Generate a unique file name
//     std::string outputFileName = fastaFile.replace_extension(".txt");

//     // Save the concatenated sequence to the file
//     std::ofstream outputFile(outputFileName);
//     outputFile << concatenatedSequence;
//     outputFile.close();

//     return outputFileName;
// }

void run()
{
    // Measurements

    //  BUILD
    // if(cfg.input_path.extension() == ".fa" || cfg.input_path.extension() == ".fasta"){ // TODO - better recognition or an argument setup
    //     std::cout << "fasta to string" << std::endl;
    //     cfg.input_path = fastaToConcatenatedFile(cfg.input_path);
    // }
    Index index = Index(cfg.rebuild,cfg.input_path); //  load or build index
    double build_result = index.build();

    std::cout << "Index size: " << build_result << "MB" << std::endl;
    // std::cout << "Build time: " << build_result << "s" << std::endl;
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