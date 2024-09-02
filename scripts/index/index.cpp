#include "index.h"

Index::Index()
{
}

Index::~Index() {}

void Index::build(std::filesystem::path input, std::filesystem::path output)
{
    std::string filename = input.filename();
    if (!std::filesystem::exists(output))
        std::filesystem::create_directories(output);
    output.append(filename);

    auto base = std::chrono::high_resolution_clock::now();
    auto startTime = base;

    std::ifstream in(input, std::ios::binary);
    if (!in)
    {
        std::cout << "ERROR: File " << input << " does not exist. Exit." << std::endl;
    }

    //  build compressed suffix tree
    sdsl::construct(cst, input, 1);
    sdsl::store_to_file(cst, output.replace_extension(".cst"));
    std::cout << "CST:" << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - startTime).count() << "um" << std::endl;
    startTime = std::chrono::high_resolution_clock::now();

    //  build RMaxQ SA support
    sdsl::construct(tmp_csa, input, 1);
    sdsl::util::assign(rmq_sa_max, sdsl::rmq_succinct_sct<false>(&tmp_csa));
    sdsl::store_to_file(rmq_sa_max, output.replace_extension(".rmq_max"));

    std::cout << "RMQmax" << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - startTime).count() << "um" << std::endl;
    startTime = std::chrono::high_resolution_clock::now();

    //  build RMinQ SA support
    sdsl::construct(tmp_csa, input, 1);
    sdsl::util::assign(rmq_sa_min, sdsl::rmq_succinct_sct<true>(&tmp_csa));
    sdsl::store_to_file(rmq_sa_min, output.replace_extension(".rmq_min"));
    std::cout << "RMQmin:" << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - startTime).count() << "um" << std::endl;
    startTime = std::chrono::high_resolution_clock::now();

    text_size = cst.size();

    //  build rank support for $ character
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
    sdsl::store_to_file(B, output.replace_extension("ranksupp"));
    sdsl::util::assign(rankB, &B);
    std::cout << "RankB:" << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - startTime).count() << "um" << std::endl;
    std::cout << "build_time:" << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - base).count() << std::endl;
    std::cout << "index_size:" << size_in_mega_bytes(cst) + size_in_mega_bytes(B) + size_in_mega_bytes(rmq_sa_min) + size_in_mega_bytes(rmq_sa_max) << std::endl;
}

void Index::load(std::filesystem::path input)
{
    std::string filename = input.filename().replace_extension("");
    // std::cout << filename << std::endl; 
    input.append(filename);

    // std::cout << input << std::endl;

    sdsl::load_from_file(cst, input.replace_extension(".cst"));
    sdsl::load_from_file(rmq_sa_max, input.replace_extension(".rmq_max"));
    sdsl::load_from_file(rmq_sa_min, input.replace_extension(".rmq_min"));
    sdsl::load_from_file(B, input.replace_extension(".ranksupp"));
    sdsl::util::assign(rankB, &B);
}

template <class t_cst>
void output_node(const typename t_cst::node_type &v, const t_cst &cst)
{
    std::cout << v << ":" << cst.depth(v) << "-[" << cst.leftmost_leaf(v) << ","
              << cst.rightmost_leaf(v) << "] "
              << "-[" << cst.lb(v) << ","
              << cst.rb(v) << "] "
              << "-[" << cst.sn(cst.leftmost_leaf(v)) << ","
              << cst.sn(cst.rightmost_leaf(v)) << "] ";
    if (cst.depth(v) == 0)
        std::cout << "root\n";
    else
    {
        std::string s = extract(cst, v);
        std::cout << s << (s[s.size() - 1] == 0 ? "$" : "") << std::endl;
    }
}

double Index::find(std::string pattern)
{
    auto base = std::chrono::high_resolution_clock::now();

    auto r = cst.root();
    size_t length = 1;
    auto o = r;

    for (int i = pattern.size() - 1; i >= 0; i--) //  at most 2*pattern.size() iterations
    {
        //  propagate into tree
        r = o;
        o = cst.wl(r, pattern[i]);

        if (cst.depth(o) == 0)
        {
            //  character was not found - returns root

            //  report MEM
            if (length > 1)
            {
                occurences.emplace_back(i + 1, length - 1, rankB(cst.csa[rmq_sa_min(cst.lb(r), cst.rb(r))]), rankB(cst.csa[rmq_sa_max(cst.lb(r), cst.rb(r))])); // Save MEM position in pattern, length, leftmost occ genome number, rightmost occ genome number
            }
            if (r != o) // no change from previous = both of them must be roots
                i++;

            while (cst.depth(cst.wl(r, pattern[i - 1])) == 0)
            {
                r = cst.parent(r);
                // std::cout << "." << cst.depth(r) << (cst.depth(r)==0) << std::endl;

                if (cst.depth(r) == 0)
                    break;
            }
            length = cst.depth(r);
            o = r;
        }
        if (i == 0)
        {
            //  end of the pattern, report MEM
            if (length != 0)
            {
                occurences.emplace_back(i, length, rankB(cst.csa[rmq_sa_min(cst.lb(r), cst.rb(r))]), rankB(cst.csa[rmq_sa_max(cst.lb(r), cst.rb(r))])); // Save MEM position in pattern, length, leftmost occ genome number, rightmost occ genome number
            }
            break;
        }
        length++;
    }

    return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - base).count();
}