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
    string pattern = argv[3];

    double time;

    Index index = Index();
    index.load(input);

    int res = -1;
    int max = 0;
    string str;
    try
    {
        ifstream inf(pattern, ios::binary);
        if (!inf)
        {
            cout << "no pattern" << endl;
            time = index.find(pattern);
            if (all)
            {
                cout << '>' << pattern << '\t' << time << "us" << '\t' << index.occurences.size() << std::endl;
                for (size_t i = 0; i < index.occurences.size(); i++)
                {
                    std::cout << '[' << index.occurences[i].index << ',' << index.occurences[i].index + index.occurences[i].length << "]{" << index.occurences[i].first_occ << ',' << index.occurences[i].last_occ << "}\t";
                }
                std::cout << std::endl;
            }
            else
            {
                for (size_t i = 0; i < index.occurences.size(); i++)
                {
                    if ((index.occurences[i].length > max) && (index.occurences[i].first_occ == index.occurences[i].last_occ))
                    {
                        max = index.occurences[i].length;
                        res = index.occurences[i].first_occ;
                    }
                }
                std::cout << res << endl;
            }
        }
        else
        {
            // cout << "file" << endl;
            // inf.seekg(0, std::ios::end);
            // size_t size = inf.tellg();
            // std::string buffer(size, ' ');
            // inf.seekg(0);
            // inf.read(&buffer[0], size);
            while (getline(inf, str))
            {
                // cout << str << endl;
                time = index.find(str);
                if (all)
                {
                    cout << '>' << str << '\t' << time << "us" << '\t' << index.occurences.size() << std::endl;
                    for (size_t i = 0; i < index.occurences.size(); i++)
                    {
                        // if ((index.occurences[i].length > max) && (index.occurences[i].first_occ == index.occurences[i].last_occ))
                        // {
                        //     max = index.occurences[i].length;
                        //     res = index.occurences[i].first_occ;
                        // }
                        std::cout << '[' << index.occurences[i].index << ',' << index.occurences[i].index + index.occurences[i].length << "]{" << index.occurences[i].first_occ << ',' << index.occurences[i].last_occ << "}\t";
                    }

                    // std::cout << res << endl;
                    std::cout << std::endl;
                }
                else
                {
                    for (size_t i = 0; i < index.occurences.size(); i++)
                    {
                        if ((index.occurences[i].length > max) && (index.occurences[i].first_occ == index.occurences[i].last_occ))
                        {
                            max = index.occurences[i].length;
                            res = index.occurences[i].first_occ;
                        }
                    }
                    std::cout << res << endl;
                }
            }

            inf.close();
        }
    }
    catch (const exception &e)
    {
        cerr << e.what() << '\n';
    }

    // for (int i = 3; i < argc; i++)
    // {
    //     time = index.find(argv[i]);
    //     if (all)
    //     {
    //         cout << '>' << argv[i] << '\t' << time << "us" << '\t' << index.occurences.size() << std::endl;
    //         for (size_t i = 0; i < index.occurences.size(); i++)
    //         {
    //             std::cout << '[' << index.occurences[i].index << ',' << index.occurences[i].index + index.occurences[i].length << "]{" << index.occurences[i].first_occ << ',' << index.occurences[i].last_occ << "}\t";
    //         }
    //         std::cout << std::endl;
    //     }
    //     else
    //     {
    //         for (size_t i = 0; i < index.occurences.size(); i++)
    //         {
    //             if ((index.occurences[i].length > max) && (index.occurences[i].first_occ == index.occurences[i].last_occ))
    //             {
    //                 max = index.occurences[i].length;
    //                 res = index.occurences[i].first_occ;
    //             }
    //         }
    //         std::cout << res << endl;
    //     }
    // }

    return 0;
}