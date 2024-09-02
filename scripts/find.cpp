#include <filesystem>
#include <string>
#include <iostream>

#include "index/index.h"

using namespace std;

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

int main(int argc, char **argv)
{
    std::filesystem::path input = argv[1];
    bool all = (std::atoi(argv[2]) == 1);

    double time;

    Index index = Index();
    index.load(input);

    int res = -1;
    int max = 0;

    for (int i = 3; i < argc; i++)
    {
        time = index.find(argv[i]);
        if (all){
            cout << '>' << argv[i] << '\t' << time << "us" << '\t' << index.occurences.size() << std::endl;
            for (size_t i = 0; i < index.occurences.size(); i++){
                std::cout << '[' << index.occurences[i].index << ',' << index.occurences[i].index + index.occurences[i].length << "]{" << index.occurences[i].first_occ << ',' << index.occurences[i].last_occ << "}\t";
            }
            std::cout << std::endl;
        }else{
            for (size_t i = 0; i < index.occurences.size(); i++)
            {
                if ((index.occurences[i].length > max) && (index.occurences[i].first_occ == index.occurences[i].last_occ)){
                    max = index.occurences[i].length;
                    res = index.occurences[i].first_occ;
                }
            }
            std::cout << res << endl;
        }
    }

    return 0;
}