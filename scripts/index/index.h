#ifndef INDEX_H
#define INDEX_H

#include <bits/stdc++.h>
#include <chrono>
#include <algorithm>

#include <sdsl/suffix_trees.hpp>
#include <sdsl/rmq_support.hpp>

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
    // typedef sdsl::cst_sada<sdsl::csa_wt<>, sdsl::lcp_support_sada<>> cst_type;
    typedef sdsl::cst_sct3<sdsl::csa_wt<sdsl::wt_huff<sdsl::bit_vector_il<>>, 1024, 1<<20>, sdsl::lcp_support_sada<>> cst_type;
    bool rebuild = false;
    sdsl::csa_bitcompressed<> tmp_csa;
    // std::filesystem::path input;
    // std::filesystem::path base_path; 

public:
    typedef cst_type::size_type size_type;

    size_t text_size;
    sdsl::rank_support_v<> rankB;
    sdsl::bit_vector B;
    cst_type cst;

    sdsl::rmq_succinct_sct<true> rmq_sa_min;
    sdsl::rmq_succinct_sct<false> rmq_sa_max;
    std::vector<mem_occ> occurences;

    Index();
    ~Index();

    void build(std::filesystem::path input,std::filesystem::path output); //  construct fm index supporting mem searching and rmq structure over SA
    void load(std::filesystem::path input); //  construct fm index supporting mem searching and rmq structure over SA
    double find(std::string pattern);            //  locate all MeMs
};
#endif //  INDEX_H