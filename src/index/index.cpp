#include "index.h"

Index::Index(std::filesystem::path text_file)
{
    std::string filename = text_file.filename();
    std::string ext = text_file.extension().c_str();
    config.base_path = text_file.replace_extension(ext + ".index");
    std::cout << "Index destination on " << config.base_path << std::endl;
    if (!std::filesystem::exists(config.base_path))
        std::filesystem::create_directories(config.base_path);
    config.base_path.append(filename);
    config.text_file = text_file.replace_filename(filename);
}

Index::Index(bool rebuild, std::filesystem::path text_file) : rebuild(rebuild)
{

    std::string filename = text_file.filename();
    std::string ext = text_file.extension().c_str();
    config.base_path = text_file.replace_extension(ext + ".index");
    std::cout << "Index destination on " << config.base_path << std::endl;
    if (!std::filesystem::exists(config.base_path))
        std::filesystem::create_directories(config.base_path);
    config.base_path.append(filename);
    config.text_file = text_file.replace_filename(filename);
}

Index::~Index() {}

double Index::build()
{

    std::cout << "-=-=-=-=-=-   Building index   ...   " << std::endl;

    auto base = std::chrono::high_resolution_clock::now();
    auto startTime = base;

    //  build compressed suffix tree
    std::cout << "1" << std::endl; 
    if (rebuild || !sdsl::load_from_file(cst, config.base_path.replace_extension(".cst")))
    {
        std::ifstream in(config.text_file);
        if (!in)
        {
            std::cout << "ERROR: File " << config.text_file << " does not exist. Exit." << std::endl;
        }
        std::cout << "No compact suffix tree on " << config.base_path.replace_extension(".cst") << " located. Building now ...   ";
        sdsl::construct(cst, config.text_file, 1);
        sdsl::store_to_file(cst, config.base_path.replace_extension(".cst"));
        std::cout << " ==> DONE in " << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - startTime).count() << "um" << std::endl;
    }
    startTime = std::chrono::high_resolution_clock::now();

    //  build RMaxQ SA support
    if (rebuild || !sdsl::load_from_file(rmq_sa_max, config.base_path.replace_extension(".rmq_max")))
    {
        std::cout << "No RMQ for SA on " << config.base_path.replace_extension(".rmq_max") << " located. Building now ... ";
        sdsl::construct(tmp_csa, config.text_file, 1);
        sdsl::util::assign(rmq_sa_max, sdsl::rmq_succinct_sct<false>(&tmp_csa));

        // sdsl::util::assign(rmq_sa_max, sdsl::rmq_succinct_sada<false>(&cst.csa));
        sdsl::store_to_file(rmq_sa_max, config.base_path.replace_extension(".rmq_max"));
        std::cout << " ==> DONE in " << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - startTime).count() << "um" << std::endl;
    }
    startTime = std::chrono::high_resolution_clock::now();

    //  build RMinQ SA support
    if (rebuild || !sdsl::load_from_file(rmq_sa_min, config.base_path.replace_extension(".rmq_min")))
    {
        std::cout << "No RMQ for SA on " << config.base_path.replace_extension(".rmq_min") << " located. Building now ... ";
        sdsl::construct(tmp_csa, config.text_file, 1);
        sdsl::util::assign(rmq_sa_min, sdsl::rmq_succinct_sct<true>(&tmp_csa));

        // sdsl::util::assign(rmq_sa_min, sdsl::rmq_succinct_sada<true>(&cst.csa));
        sdsl::store_to_file(rmq_sa_min, config.base_path.replace_extension(".rmq_min"));
        std::cout << " ==> DONE in " << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - startTime).count() << "um" << std::endl;
    }
    startTime = std::chrono::high_resolution_clock::now();

    text_size = cst.size();

    //  build rank support for $ character
    if (rebuild || !sdsl::load_from_file(B, config.base_path.replace_extension("ranksupp")))
    {
        std::ifstream in(config.text_file);
        if (!in)
        {
            // std::cout << "ERROR: File " << config.text_file << " does not exist. Exit." << std::endl;
        }
        std::cout << "No rank support  on " << config.base_path.replace_extension("ranksupp") << " located. Building now ...  ";
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
        sdsl::store_to_file(B, config.base_path.replace_extension("ranksupp"));
        std::cout << " ==> DONE in " << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - startTime).count() << "um" << std::endl;
    }
    sdsl::util::assign(rankB, &B);

    double index_size = size_in_mega_bytes(cst) + size_in_mega_bytes(rankB) + size_in_mega_bytes(rmq_sa_min) + size_in_mega_bytes(rmq_sa_max);
    std::cout << "-=-=-=-=-=-   Building index - DONE, size:  " << index_size << " MiB." << std::endl;
    // return index_size;
    return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - base).count();
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

double Index::locate(std::string pattern)
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

    // return

    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - base).count();
}