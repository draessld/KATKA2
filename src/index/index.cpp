#include "index.h"

Index::Index(std::filesystem::path text_file)
{
    std::string filename = text_file.filename();
    config.base_path = text_file.replace_extension(".index");
    std::cout << "Index destination on " << config.base_path << std::endl;
    if (!std::filesystem::exists(config.base_path))
        std::filesystem::create_directories(config.base_path);
    config.base_path.append(filename);
    config.text_file = text_file.replace_filename(filename);
}

Index::Index(bool rebuild, std::filesystem::path text_file) : rebuild(rebuild)
{
    
    std::string filename = text_file.filename();
    config.base_path = text_file.replace_extension(".index");
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
        sdsl::util::assign(rmq_sa_max, sdsl::rmq_succinct_sada<false>(&cst.csa));
        sdsl::store_to_file(rmq_sa_max, config.base_path.replace_extension(".rmq_max"));
        std::cout << " ==> DONE in " << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - startTime).count() << "um" << std::endl;
    }
    startTime = std::chrono::high_resolution_clock::now();

    //  build RMinQ SA support
    if (rebuild || !sdsl::load_from_file(rmq_sa_min, config.base_path.replace_extension(".rmq_min")))
    {
        std::cout << "No RMQ for SA on " << config.base_path.replace_extension(".rmq_min") << " located. Building now ... ";
        sdsl::util::assign(rmq_sa_min, sdsl::rmq_succinct_sada<true>(&cst.csa));
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
            std::cout << "ERROR: File " << config.text_file << " does not exist. Exit." << std::endl;
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

    double index_size = size_in_mega_bytes(cst) + size_in_mega_bytes(rankB);
    std::cout << "-=-=-=-=-=-   Building index - DONE, size:  " << index_size << " MiB." << std::endl;
    return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - base).count();
}

template<class t_cst>
void output_node(const typename t_cst::node_type& v, const t_cst& cst)
{
    std::cout << v << ":"<<cst.depth(v) << "-[" << cst.leftmost_leaf(v) << ","
         << cst.rightmost_leaf(v) << "] "<< "-[" << cst.lb(v) << ","
         << cst.rb(v) << "] "<< "-[" << cst.sn(cst.leftmost_leaf(v)) << ","
         << cst.sn(cst.rightmost_leaf(v)) << "] ";
    if(cst.depth(v)==0) std::cout << "root\n";
    else {
      std::string s = extract(cst,v);
      std::cout << s << (s[s.size()-1] == 0 ? "$" : "") << std::endl;
    } 
}

double Index::locate(std::string pattern)
{
    auto base = std::chrono::high_resolution_clock::now();

    auto r = cst.root();

    // csXprintf(std::cout, "%2I %2S %3s %3P %2p %3B   %:3T", cst.csa);
    
    // std::cout << cst.csa << std::endl;
    // std::cout << "max: " << rmq_sa_max(19,20) << std::endl;
    // std::cout << "min: " << rmq_sa_min(1,10) << std::endl;

    size_t length = 1;
    auto o = r;

    for (int i = pattern.size() - 1; i >= 0; i--) //  at most 2*pattern.size() iterations
    {
        //  propagate into tree
        // std::cout << pattern[i] << std::endl;
        r = o;
        o = cst.wl(r,pattern[i]);
        // output_node(o,cst);
        if (cst.depth(o)==0){
            //  character was not found - returns root

            //  report MEM
            if (length > 1){
                
                
                // std::cout << "MEM found " << pattern.substr(i+1,length-1) << std::endl;
                occurences.emplace_back(i + 1, length - 1, rankB(cst.csa[rmq_sa_min(cst.lb(r),cst.rb(r))]), rankB(cst.csa[rmq_sa_max(cst.lb(r),cst.rb(r))])); // Save MEM position in pattern, length, leftmost occ genome number, rightmost occ genome number
                // occurences.emplace_back(i + 1, length - 1, rankB(cst.lb(r)), rankB(cst.rb(r))); // Save MEM position in pattern, length, leftmost occ genome number, rightmost occ genome number
            }
            while (cst.depth(cst.wl(r,pattern[i])) == 0)
            {
                // std::cout << "  up" << std::endl;
                r = cst.parent(r);
                // std::cout << "has new node wl on character " << pattern[i] << ":"<< cst.depth(cst.wl(r,pattern[i])) << std::endl;
                // std::cout << "      ";
                // output_node(r,cst);
                if (cst.depth(r)==0){
                    // std::cout << "      loop..ending" << std::endl;
                    break;
                }
            }
            length = cst.depth(r);
            o=r;
            i++;
            // if (cst.depth(r)!=0){
            //     o = cst.parent(r);
            //     // output_node(o,cst);
            //     length = cst.depth(o);
            //     i++;
            // }else
            //     length--;
        }
        if (i == 0){
            //  end of the pattern, report MEM
            if (length != 0){
                // std::cout << "MEM found " << extract(cst,r) << std::endl;
                occurences.emplace_back(i, length, rankB(cst.csa[rmq_sa_min(cst.lb(r),cst.rb(r))]), rankB(cst.csa[rmq_sa_max(cst.lb(r),cst.rb(r))])); // Save MEM position in pattern, length, leftmost occ genome number, rightmost occ genome number
                // occurences.emplace_back(i + 1, length - 1, rankB(cst.csa[rmq_sa_min(cst.lb(r),cst.rb(r))]), rankB(cst.csa[rmq_sa_max(cst.lb(r),cst.rb(r))])); // Save MEM position in pattern, length, leftmost occ genome number, rightmost occ genome number
            }
            break;
        }
        length++;
        // std::cout <<std::endl;

    }
    
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - base).count();
}