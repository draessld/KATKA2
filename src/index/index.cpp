#include "index.h"

Index::Index(std::filesystem::path index_folder)
{
    config.base_folder = index_folder;
}

Index::Index(bool rebuild, std::filesystem::path index_folder) : rebuild(rebuild)
{
    config.base_folder = index_folder;
}

Index::~Index() {}

double Index::build(std::filesystem::path text_file)
{

    config.base_folder.append(text_file.filename().replace_extension().c_str());
    std::cout << config.base_folder << std::endl;

    std::cout << "-=-=-=-=-=-   Building index   ...   " << std::endl;


    auto base = std::chrono::high_resolution_clock::now();
    auto startTime = base;
    // TODO  replace BWT/SA/LCP for bigBWT implementation on https://gitlab.com/manzai/Big-BWT/

    //  Build SA & fm-index
    if (rebuild || !sdsl::load_from_file(fm_index, config.base_folder.replace_extension(config.index_suffix)))
    {
        std::ifstream in(text_file);
        if (!in)
        {
            std::cout << "ERROR: File " << text_file << " does not exist. Exit." << std::endl;
        }
        std::cout << "No fmindex on " << config.base_folder.replace_extension(config.index_suffix) << " located. Building now ...   ";
        sdsl::construct(fm_index, text_file, 1);
        sdsl::store_to_file(fm_index, config.base_folder.replace_extension(config.index_suffix));
        std::cout << " ==> DONE in " << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - startTime).count() << "um" << std::endl;
    }

    startTime = std::chrono::high_resolution_clock::now();

    text_size = fm_index.size();

    // Constructing the LCP array
    if (rebuild || !sdsl::load_from_file(lcp, config.base_folder.replace_extension(config.lcp_suffix)))
    {
        std::ifstream in(text_file);
        if (!in)
        {
            std::cout << "ERROR: File " << text_file << " does not exist. Exit." << std::endl;
        }
        std::cout << "No LCP on" << config.base_folder.replace_extension(config.lcp_suffix) << " located. Building LCP now." << std::endl;
        sdsl::construct(lcp, text_file, 1);
        sdsl::store_to_file(lcp, config.base_folder.replace_extension(config.lcp_suffix));
        std::cout << " ==> DONE in " << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - startTime).count() << "um" << std::endl;
    }
    startTime = std::chrono::high_resolution_clock::now();

    //  build RMaxQ SA support
    if (rebuild || !sdsl::load_from_file(rmq_sa_max, config.base_folder.replace_extension(config.rmq_sa_suffix_max)))
    {
        std::ifstream in(text_file);
        if (!in)
        {
            std::cout << "ERROR: File " << text_file << " does not exist. Exit." << std::endl;
        }
        std::cout << "No RMQ for SA on " << config.base_folder.replace_extension(config.rmq_sa_suffix_max) << " located. Building now ... ";
        sdsl::construct(tmp_csa, text_file, 1);
        sdsl::util::assign(rmq_sa_max, sdsl::rmq_succinct_sct<false>(&tmp_csa));
        sdsl::store_to_file(rmq_sa_max, config.base_folder.replace_extension(config.rmq_sa_suffix_max));
        std::cout << " ==> DONE in " << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - startTime).count() << "um" << std::endl;
    }
    startTime = std::chrono::high_resolution_clock::now();

    //  build RMinQ SA support
    if (rebuild || !sdsl::load_from_file(rmq_sa_min, config.base_folder.replace_extension(config.rmq_sa_suffix_min)))
    {
        std::ifstream in(text_file);
        if (!in)
        {
            std::cout << "ERROR: File " << text_file << " does not exist. Exit." << std::endl;
        }
        std::cout << "No RMQ for SA on " << config.base_folder.replace_extension(config.rmq_sa_suffix_min) << " located. Building now ... ";
        sdsl::construct(tmp_csa, text_file, 1);
        sdsl::util::assign(rmq_sa_min, sdsl::rmq_succinct_sct<true>(&tmp_csa));
        sdsl::store_to_file(rmq_sa_min, config.base_folder.replace_extension(config.rmq_sa_suffix_min));
        std::cout << " ==> DONE in " << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - startTime).count() << "um" << std::endl;
    }
    startTime = std::chrono::high_resolution_clock::now();

    //  build RMinQ LCP support
    if (rebuild || !sdsl::load_from_file(rmq_lcp_min, config.base_folder.replace_extension(config.rmq_lcp_suffix_min)))
    {
        std::ifstream in(text_file);
        if (!in)
        {
            std::cout << "ERROR: File " << text_file << " does not exist. Exit." << std::endl;
        }
        std::cout << "No RMQ for LCP  on " << config.base_folder.replace_extension(config.rmq_lcp_suffix_min) << " located. Building now ...   ";
        sdsl::construct(tmp_lcp, text_file, 1);
        sdsl::util::assign(rmq_lcp_min, sdsl::rmq_succinct_sct<true>(&tmp_lcp));
        sdsl::store_to_file(rmq_lcp_min, config.base_folder.replace_extension(config.rmq_lcp_suffix_min));
        std::cout << " ==> DONE in " << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - startTime).count() << "um" << std::endl;
    }
    startTime = std::chrono::high_resolution_clock::now();

    //  build rank support for $ character
    if (rebuild || !sdsl::load_from_file(B, config.base_folder.replace_extension(config.rank_support_suffix)))
    {
        std::ifstream in(text_file);
        if (!in)
        {
            std::cout << "ERROR: File " << text_file << " does not exist. Exit." << std::endl;
        }
        std::cout << "No rank support  on " << config.base_folder.replace_extension(config.rank_support_suffix) << " located. Building now ...  ";
        size_t i = 0;
        B.resize(text_size);
        char c;
        while (in.get(c))
        {
            B[i] = 0;
            if (c == '$')
                B[i++] = 1;
            else
                i++;
        }
        sdsl::store_to_file(B, config.base_folder.replace_extension(config.rank_support_suffix));
        std::cout << " ==> DONE in " << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - startTime).count() << "um" << std::endl;
    }
    sdsl::util::assign(rankB, &B);

    double index_size = size_in_mega_bytes(fm_index) + size_in_mega_bytes(rmq_lcp_min) + size_in_mega_bytes(rmq_sa_max) + size_in_mega_bytes(rmq_sa_min) + size_in_mega_bytes(lcp) + size_in_mega_bytes(rankB);
    std::cout << "-=-=-=-=-=-   Building index - DONE, size:  " << index_size << " MiB." << std::endl;
    return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - base).count();
}

int Index::update_range(char c, size_t &length, Index::size_type &olb, Index::size_type &orb, Index::size_type &lb, Index::size_type &rb)
{
    int minllcp;
    int minrlcp;
    int tmp;

    int number_of_c = fm_index.bwt.rank(olb, c);
    int previous_c = 0;
    int next_c = fm_index.size() - 1;

    if (number_of_c != 0)
        fm_index.bwt.select(number_of_c, c);
    next_c = fm_index.bwt.select(number_of_c + 1, c);

    if (previous_c <= 0) //  previous_c doesnt exists
        minllcp = 0;
    else
        minllcp = rmq_lcp_min(previous_c, olb);

    if (next_c >= (int)lcp.size())
    { //  next_c doesnt exists
        minrlcp = 0;
    }
    else
        minrlcp = rmq_lcp_min(orb, next_c);

    length = std::max(lcp[minllcp], lcp[minrlcp]); //  maximal prefix length

    if (length == 0)
    {
        lb = 0;
        rb = fm_index.size() - 1;
        if (olb == lb && orb == rb)
        {
            return 1;
        }
        return 0;
    }

    //  find PSV in O(n) - TODO index LCP to get better, or can be done somehow using existing structures?
    lb = lcp[olb];
    for (tmp = olb; tmp >= 0; tmp--)
    {
        if (lcp[tmp] < length)
        {
            lb = tmp;
            break;
        }
    }

    //  find NSV in O(n) - TODO index LCP to get better, or can be done somehow using existing structures?
    rb = lcp[orb];
    for (tmp = orb; tmp < (int)lcp.size(); tmp++)
    {
        if (lcp[tmp] < length)
        {
            rb = tmp;
            break;
        }
        if (tmp == (int)lcp.size() - 1)
        {
            rb = lcp.size() - 1;
            break;
        }
    }

    if (olb == lb && orb == rb)
    {
        return 1;
    }
    return 0;
}

double Index::locate(std::string pattern)
{
    auto base = std::chrono::high_resolution_clock::now();

    size_type lb = 0;
    size_type rb = fm_index.size() - 1;
    size_type olb, orb;

    // std::cout << B << std::endl;
    // std::cout << rankB(15) << std::endl;

    size_t length = 1;
    bool stop = false;

    for (int i = pattern.size() - 1; i >= 0; i--) //  at most 2*pattern.size() iterations
    {
        olb = lb;
        orb = rb;
        sdsl::backward_search(fm_index, olb, orb, pattern[i], lb, rb);
        if (lb > rb)
        {
            if (length > 1)                                                                                                               // do not stor zero-length MEMs
                occurences.emplace_back(i + 1, length - 1, rankB(fm_index[rmq_sa_min(olb, orb)]), rankB(fm_index[rmq_sa_max(olb, orb)])); // Save MEM position in pattern, length, leftmost occ genome number, rightmost occ genome number
            // std::cout << i+1 <<'\t' << length-1 << '\t' <<rankB(fm_index[rmq_sa_min(olb, orb)]) << '\t' << rankB(fm_index[rmq_sa_max(olb, orb)]) << std::endl;
            stop = update_range(pattern[i], length, olb, orb, lb, rb); // get wider range with searched character
            if (stop)                                                  //  check if range was changed or stop
                break;
            i++; //  for searching same character one more time with wider range
        }
        if (i == 0)
        {
            if (length != 0)                                                                                                  // do not store zero-length MEMs
                                                                                                                              // std::cout << i <<'\t' << length << '\t' <<rankB(fm_index[rmq_sa_min(lb, rb)]) << '\t' << rankB(fm_index[rmq_sa_max(lb, rb)]) << std::endl;
                occurences.emplace_back(i, length, rankB(fm_index[rmq_sa_min(lb, rb)]), rankB(fm_index[rmq_sa_max(lb, rb)])); // Save MEM position in pattern, length, leftmost occ genome number, rightmost occ genome number
            break;
        }
        length++;
    }
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - base).count();
}