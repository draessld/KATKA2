#include "index.h"

Index::Index()  {}

Index::Index(bool rebuild)  : rebuild(rebuild){}

Index::~Index() {}

int Index::build(std::filesystem::path text_file){
    std::cout << "-=-=-=-=-=-   Building index   ...   " << std::endl;

    std::filesystem::path tmp_path = text_file;

    //  Build SA & fm-index
    tmp_path.replace_extension(config.index_suffix);
    if (rebuild || !sdsl::load_from_file(fm_index, tmp_path))
    {
        std::ifstream in(text_file);
        if (!in)
        {
            std::cout << "ERROR: File " << text_file << " does not exist. Exit." << std::endl;
        }
        std::cout << "No index " << tmp_path.filename() << " located. Building index now." << std::endl;
        sdsl::construct(fm_index, text_file, 1);
        sdsl::store_to_file(fm_index, tmp_path);
        std::cout << " ==> DONE " << std::endl;
    }

    text_size = fm_index.size();

    // Constructing the LCP array
    tmp_path.replace_extension(config.lcp_suffix);
    if (rebuild || !sdsl::load_from_file(lcp, tmp_path))
    {
        std::ifstream in(text_file);
        if (!in)
        {
            std::cout << "ERROR: File " << text_file << " does not exist. Exit." << std::endl;
        }
        std::cout << "No LCP " << tmp_path.filename() << " located. Building LCP now." << std::endl;
        sdsl::construct(lcp, text_file, 1);
        sdsl::store_to_file(lcp, tmp_path);
        std::cout << " ==> DONE " << std::endl;
    }

    //  build RMaxQ SA support
    tmp_path.replace_extension(config.rmq_sa_suffix_max);
    if (rebuild || !sdsl::load_from_file(rmq_sa_max, tmp_path))
    {
        std::ifstream in(text_file);
        if (!in)
        {
            std::cout << "ERROR: File " << text_file << " does not exist. Exit." << std::endl;
        }
        std::cout << "No RMQ for SA" << text_file.filename() << " located. Building now." << std::endl;
        sdsl::construct(tmp_csa, text_file, 1);
        sdsl::util::assign(rmq_sa_max, sdsl::rmq_succinct_sct<false>(&tmp_csa));
        sdsl::store_to_file(rmq_sa_max, tmp_path);
        std::cout << " ==> DONE " << std::endl;
    }
    
    //  build RMinQ SA support
    tmp_path.replace_extension(config.rmq_sa_suffix_min);
    if (rebuild || !sdsl::load_from_file(rmq_sa_min, tmp_path))
    {
        std::ifstream in(text_file);
        if (!in)
        {
            std::cout << "ERROR: File " << text_file << " does not exist. Exit." << std::endl;
        }
        std::cout << "No RMQ for SA" << text_file.filename() << " located. Building now." << std::endl;
        sdsl::construct(tmp_csa, text_file, 1);
        sdsl::util::assign(rmq_sa_min, sdsl::rmq_succinct_sct<true>(&tmp_csa));
        sdsl::store_to_file(rmq_sa_min, tmp_path);
        std::cout << " ==> DONE " << std::endl;
    }

    //  build RMinQ LCP support
    tmp_path.replace_extension(config.rmq_lcp_suffix_min);
    if (rebuild || !sdsl::load_from_file(rmq_lcp_min, tmp_path))
    {
        std::ifstream in(text_file);
        if (!in)
        {
            std::cout << "ERROR: File " << text_file << " does not exist. Exit." << std::endl;
        }
        std::cout << "No RMQ for LCP " << text_file.filename() << " located. Building now." << std::endl;
        sdsl::construct(tmp_lcp, text_file, 1);
        sdsl::util::assign(rmq_lcp_min, sdsl::rmq_succinct_sct<true>(&tmp_lcp));
        sdsl::store_to_file(rmq_lcp_min, tmp_path);
        std::cout << " ==> DONE in " << std::endl;
    }

    //  build rank support for $ character
    tmp_path.replace_extension(config.rank_support_suffix);
    if (rebuild || !sdsl::load_from_file(B, tmp_path))
    {
        std::ifstream in(text_file);
        if (!in)
        {
            std::cout << "ERROR: File " << text_file << " does not exist. Exit." << std::endl;
        }
        std::cout << "No rank support " << text_file.filename() << " located. Building now." << std::endl;
        size_t i = 0;
        B.resize(text_size);
        char c;
        while (in.get(c)) {
            B[i] = 0;
            if(c == '$')
                B[i++] = 1;
            else i++;
        }
        sdsl::store_to_file(B, tmp_path);
        std::cout << " ==> DONE " << std::endl;
    }
    sdsl::util::assign(rankB, &B);

    double index_size = size_in_mega_bytes(fm_index) + size_in_mega_bytes(rmq_lcp_min)+ size_in_mega_bytes(rmq_sa_max)+ size_in_mega_bytes(rmq_sa_min)+ size_in_mega_bytes(lcp) + size_in_mega_bytes(rankB);
    std::cout << "-=-=-=-=-=-   Building index - DONE, size:  " << index_size << " MiB." << std::endl;
    return 0;
}

int Index::update_range(char c, size_t &length, Index::size_type &olb, Index::size_type &orb, Index::size_type &lb, Index::size_type &rb){
    // std::cout << "current range "<<c<<'(' << olb  << '/'<< orb << ')' << "length: " << length-1<< std::endl;
    // std::cout << "number of " << c << " is " << fm_index.bwt.rank(olb,c) << std::endl;
    int previous_c = 0;
    int next_c = fm_index.size()-1;
    int minllcp;
    int minrlcp;

    int number_of_c = fm_index.bwt.rank(olb,c);
    if (number_of_c!=0)
    previous_c = fm_index.bwt.select(number_of_c,c); 
    next_c = fm_index.bwt.select(number_of_c+1,c);

    if (previous_c <= 0) //  previous_c doesnt exists
        // previous_c = 0;
        minllcp =  0;
    else
        minllcp = rmq_lcp_min(previous_c,olb);


    if (next_c >= (int)lcp.size()){  //  next_c doesnt exists
        // next_c = lcp.size()-1;  // next_c is out of range
        minrlcp = 0;
    }else
        minrlcp = rmq_lcp_min(orb,next_c);

    // std::cout << "previous/olb "<<c<<'(' << previous_c  << '/'<< olb << ')' << "minlcp: " << minllcp<< std::endl;
    // std::cout << "orb/next "<<c<<'(' << orb  << '/'<< next_c << ')' << "minlcp: " << minrlcp<< std::endl;

    std::cout << lcp[minllcp] << ','  << lcp[minrlcp] << ','<<std::max(lcp[minllcp],lcp[minrlcp]) << std::endl;
    int tmp;
    length = std::max(lcp[minllcp],lcp[minrlcp]);

    if (length == 0)
    {
        lb  = 0;
        rb = fm_index.size()-1;
        // std::cout << "New range setting: (" << lb  << ','<< rb << ')' << "lcp: " << length << std::endl;
        if (olb == lb && orb == rb){
            return 1;
        }
        return 0 ;
    }
    

    //  find PSV in O(n) - TODO index LCP to get better?
    //  PSV
    lb = lcp[olb];
    for (tmp = olb; tmp >=0; tmp--){
        if(lcp[tmp] < length){
            lb = tmp;
            break;
        }
    }

    //  find NSV in O(n) - TODO index LCP to get better?
    //  NSV
    rb = lcp[orb];
    for (tmp = orb; tmp < (int)lcp.size(); tmp++){
        if(lcp[tmp] < length){
            rb = tmp;
            break;
        }
        if (tmp == (int)lcp.size()-1){
            rb = lcp.size()-1;
            break;
        }
    }
    
    // std::cout << "New range setting: (" << lb  << ','<< rb << ')' << "lcp: " << length << std::endl;
    if (olb == lb && orb == rb)
    {
        return 1;
    }
    return 0;
    
}

int Index::locate(std::string pattern)
{
    // std::cout << std::endl;
    // std::cout << "Locating pattern: " << pattern << std::endl;
    size_type lb = 0;
    size_type rb = fm_index.size()-1; 
    size_type olb,orb; 
    size_t length = 1;
    bool stop = false;
    for (int i = pattern.size() - 1; i >= 0 ; i--)
    {
        olb = lb; orb = rb;
        // std::cout << olb <<  "\t"<< orb <<"\t"<< i << std::endl;
        sdsl::backward_search(fm_index,olb,orb,pattern[i],lb,rb); 
        // std::cout << "subpttr: " <<  pattern.substr(i,length) << " was found on " << lb << ","<<rb << std::endl;
        if (lb > rb){
            // std::cout << "MEM2 has been found: " <<  pattern.substr(i+1,length-1) << " in genomes " << rankB(fm_index[rmq_sa_min(olb, orb)]) << ',' << rankB(fm_index[rmq_sa_max(olb, orb)]) << std::endl;
            if(length > 1)
                occurences.emplace_back(i+1,length-1,rankB(fm_index[rmq_sa_min(olb, orb)]),rankB(fm_index[rmq_sa_max(olb, orb)]));
            // std::cout << i+1 <<'\t' << length-1 << '\t' <<rankB(fm_index[rmq_sa_min(olb, orb)]) << '\t' << rankB(fm_index[rmq_sa_max(olb, orb)]) << std::endl;
            stop = update_range(pattern[i],length,olb,orb,lb,rb);
            if (stop)
                break;
            i++;
        }
        if(i==0){
            // std::cout << "MEM1 has been found: " <<  pattern.substr(i,length) << " in genomes " << rankB(fm_index[rmq_sa_min(lb, rb)]) << ',' << rankB(fm_index[rmq_sa_max(lb, rb)]) << std::endl;
            if(length != 0)
            // std::cout << i <<'\t' << length << '\t' <<rankB(fm_index[rmq_sa_min(lb, rb)]) << '\t' << rankB(fm_index[rmq_sa_max(lb, rb)]) << std::endl;
                occurences.emplace_back(i,length,rankB(fm_index[rmq_sa_min(lb, rb)]),rankB(fm_index[rmq_sa_max(lb, rb)]));
            break;
        }
        length++;
    }
    return 0;
}