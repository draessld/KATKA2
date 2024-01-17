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

    //  build compressed suffix tree
    if (rebuild || !sdsl::load_from_file(cst, config.base_folder.replace_extension(config.cst_suffix)))
    {
        std::ifstream in(text_file);
        if (!in)
        {
            std::cout << "ERROR: File " << text_file << " does not exist. Exit." << std::endl;
        }
        std::cout << "No compact suffix tree on " << config.base_folder.replace_extension(config.cst_suffix) << " located. Building now ...   ";
        sdsl::construct(cst, text_file, 1);
        sdsl::store_to_file(cst, config.base_folder.replace_extension(config.cst_suffix));
        std::cout << " ==> DONE in " << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - startTime).count() << "um" << std::endl;
    }
    startTime = std::chrono::high_resolution_clock::now();

    text_size = cst.size();

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

    double index_size = size_in_mega_bytes(cst) + size_in_mega_bytes(rankB);
    std::cout << "-=-=-=-=-=-   Building index - DONE, size:  " << index_size << " MiB." << std::endl;
    return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - base).count();
}

int Index::update_range(char c, size_t &length, Index::size_type &olb, Index::size_type &orb, Index::size_type &lb, Index::size_type &rb)
{

    return 0;
}

template<class t_cst>
void output_node(const typename t_cst::node_type& v, const t_cst& cst)
{
    std::cout << cst.depth(v) << "-[" << cst.lb(v) << ","
         << cst.rb(v) << "] ";
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

    size_t length = 1;
    auto o = r;

    for (int i = pattern.size() - 1; i >= 0; i--) //  at most 2*pattern.size() iterations
    {
        //  propagate into tree
        r = o;
        o = cst.wl(r,pattern[i]);
        if (cst.depth(o)==0){
            //  character was not found - returns root

            //  report MEM
            if (length > 1){
                // std::cout << "MEM found " << extract(cst,r).substr(0,length-1) << "with original position in the text: "  << cst.sn(cst.leftmost_leaf(r)) << "," << cst.sn(cst.rightmost_leaf(r)) << std::endl;
                occurences.emplace_back(i + 1, length - 1, rankB(cst.sn(cst.leftmost_leaf(r))), rankB(cst.sn(cst.rightmost_leaf(r)))); // Save MEM position in pattern, length, leftmost occ genome number, rightmost occ genome number
            }
            if (cst.depth(r)!=0){
                r = cst.parent(r);
                length = cst.depth(r);
                i++;
            }else
                length--;
        }
        if (i == 0){
            //  end of the pattern, report MEM
            if (length != 0){
                // std::cout << "MEM found " << extract(cst,r).substr(0,length) << std::endl;
                occurences.emplace_back(i, length, rankB(cst.sn(cst.leftmost_leaf(r))), rankB(cst.sn(cst.rightmost_leaf(r)))); // Save MEM position in pattern, length, leftmost occ genome number, rightmost occ genome number
            }
            break;
        }
        length++;
        // std::cout <<std::endl;

    }
    
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - base).count();
}