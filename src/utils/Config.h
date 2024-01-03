#ifndef CONFIG_H
#define CONFIG_H

#include <fstream>
#include <iostream>
#include <filesystem>

class Config
{
public:

    std::filesystem::path output_path;
    std::filesystem::path input_path;
    std::filesystem::path pattern_file;
    bool silent;
    unsigned  repetition;
    bool rebuild; //  rebuild even indexes exists
    bool digest;
    bool kernel;
    std::string pattern;

    unsigned w;    //  window size for kernel build
    unsigned K;    //  k-mer size
    unsigned M;   


    Config(/* args */);
    ~Config();
    int save_config_file();
    int load_config_file();
};

Config::Config(/* args */){}

Config::~Config(){}

int Config::save_config_file(){
    if(!silent)
        std::cout << "saving to:"<<output_path.replace_extension() / "config.cfg" << std::endl; 
    std::ofstream file(output_path.replace_extension() / "config.cfg");
    if (!file.is_open())
    {
        std::cerr << "Error while reading file" << std::endl;
        return -1;
    }

    file.close();
    return 0;
}

int Config::load_config_file(){
    if(!silent)
        std::cout << "loading from:"<<input_path.replace_extension() / "config.cfg" << std::endl; 
    std::ifstream file(input_path.replace_extension()/ "config.cfg");
    if (!file.is_open())
    {
        std::cerr << "Error while reading file" << std::endl;
        return -1;
    }
    
    return 0;
}

#endif // CONFIG_H
