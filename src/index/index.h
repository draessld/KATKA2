#ifndef INDEX_H
#define INDEX_H

#include <bits/stdc++.h>
#include <chrono>
#include <algorithm>

#include <sdsl/suffix_arrays.hpp>
#include <sdsl/rmq_support.hpp>
#include <sdsl/lcp.hpp>
#include <sdsl/int_vector.hpp>
#include <sdsl/csa_wt.hpp>

struct Indexcfg{
    bool rebuild = false;
    std::filesystem::path out_path;
    std::string index_suffix = ".fmi";
    std::string lcp_suffix = ".lcp";
    std::string rmq_sa_suffix_min = ".rmqsamin";
    std::string rmq_sa_suffix_max = ".rmqsamax";
    std::string rmq_lcp_suffix_min = ".rmqlcpmin";
    std::string rank_support_suffix = ".ranksupp";
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
    sdsl::lcp_bitcompressed<> tmp_lcp;
    sdsl::csa_wt<sdsl::wt_huff<>, 1, 1 << 20> tmp_csa;
    typedef sdsl::csa_wt<sdsl::wt_huff<sdsl::rrr_vector<127>>, 512, 1024> fm_index_type;
    bool rebuild = false;

public:
    typedef fm_index_type::size_type size_type;
    int update_range(char c,size_t &length, size_type &olb, size_type &orb, size_type &lb, size_type &rb);

    Indexcfg config;
    size_t text_size;
    sdsl::rank_support_v<> rankB;
    sdsl::lcp_bitcompressed<> lcp;
    sdsl::bit_vector B;
    fm_index_type fm_index;
    sdsl::rmq_succinct_sct<true> rmq_sa_min;
    sdsl::rmq_succinct_sct<false> rmq_sa_max;
    sdsl::rmq_succinct_sct<true> rmq_lcp_min;

    double index_size;
    std::vector<mem_occ> occurences;

    Index(std::filesystem::path output_path);
    Index(bool rebuild,std::filesystem::path output_path);
    ~Index();

    // void print_MEMs(std::vector<mem_occ> occurences);
    double build(std::filesystem::path text_file); //  construct fm index supporting mem searching and rmq structure over SA
    double locate(std::string pattern);            //  locate all MeMs
};
#endif //  INDEX_H
