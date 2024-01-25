#ifndef INDEX_H
#define INDEX_H

#include <bits/stdc++.h>
#include <chrono>
#include <algorithm>

#include <sdsl/suffix_trees.hpp>
#include <sdsl/rmq_support.hpp>

struct Indexcfg{
    bool rebuild = false;
    std::filesystem::path text_file;
    std::filesystem::path base_path; 
};

struct mem_occ{
    int index;
    int length;
    
    int first_occ;
    int last_occ;
    mem_occ(int index,int length,int first_occ,int last_occ) :index(index), length(length), first_occ(first_occ),last_occ(last_occ){}
};

class Index
{
private:
    typedef sdsl::cst_sada<sdsl::csa_wt<>, sdsl::lcp_support_sada<>> cst_type;
    bool rebuild = false;

public:
    typedef cst_type::size_type size_type;

    Indexcfg config;
    size_t text_size;
    sdsl::rank_support_v<> rankB;
    sdsl::bit_vector B;
    cst_type cst;

    sdsl::rmq_succinct_sada<true> rmq_sa_min;
    sdsl::rmq_succinct_sada<false> rmq_sa_max;

    double index_size;
    std::vector<mem_occ> occurences;

    Index(std::filesystem::path output_path);
    Index(bool rebuild,std::filesystem::path output_path);
    ~Index();

    double build(); //  construct fm index supporting mem searching and rmq structure over SA
    double locate(std::string pattern);            //  locate all MeMs
};
#endif //  INDEX_H
