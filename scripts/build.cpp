#include <filesystem>
#include <string>
#include <iostream>

#include "index/index.h"

using namespace std;

int main(int argc, char **argv)
{
    std::filesystem::path input = argv[1];
    std::filesystem::path output = argv[2];
    

    // std::cout << "Building: index from " << input << " saving on " << output << std::endl;

    Index index = Index();
    index.build(input,output);

    // std::cout << "Index size: " << build_result << "MB" << std::endl;
    // std::cout << "Build time: " << build_result << "μs" << std::endl;

    return 0;
}
