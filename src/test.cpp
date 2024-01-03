#include <iostream>

#include <algorithm>
#include <sdsl/suffix_arrays.hpp>
#include <sdsl/rmq_support.hpp>
#include <sdsl/lcp.hpp>
#include <sdsl/int_vector.hpp>

using namespace std;
using namespace sdsl;

struct interval {
    size_t lb, rb; // left bound, right bound (both inclusive)
    size_t min_val, min_idx; // minimal value, index of the minimum

    interval(size_t lb, size_t rb, size_t min_val, size_t min_idx):
        lb(lb), rb(rb), min_val(min_val), min_idx(min_idx) {};

    interval(const interval& i):
        lb(i.lb), rb(i.rb), min_val(i.min_val), min_idx(i.min_idx) {};

    bool operator>(const interval& i)const {
        if (min_val != i.min_val) {
            return min_val > i.min_val;
        }
        if (min_idx != i.min_idx) {
            return min_idx > i.min_idx;
        }
        if (lb != i.lb) {
            return lb > i.lb;
        }
        return rb > i.rb;
    }
};

int main(int argc, char const *argv[])
{
    string text = "GAAT$GATAT";
    typedef csa_wt<wt_huff<rrr_vector<127> >, 512, 1024> fm_index_type;
    typedef fm_index_type::size_type size_type;

    fm_index_type fm_index;
    bit_vector B;
    lcp_wt<> lcp;
    rmq_succinct_sct<> rmq;
    csa_wt<wt_huff<>, 1, 1 << 20> tmp_csa;

    construct_im(fm_index, text,1);
    construct_im(lcp, text, 1);

    // construct_im(tmp_csa, text, 1);
    util::assign(rmq, rmq_succinct_sct<>(&fm_index));

    for (size_t i = 0; i < text.size(); i++)
    {
        cout << i<<':'<<fm_index.text[i] << endl;
    }
    
    cout << fm_index.text << endl;
    cout << fm_index << endl;
    cout << lcp << endl;

    string prompt = "\e[0;32m>\e[0m ";
    
    size_t max_locations = 5;
    size_t post_context = 2;
    size_t pre_context = 2;

    string query = "TA";
    size_type m  = query.size();
    size_type lb = 0, rb=0;
    size_t occs = backward_search(fm_index,0,2,'T',lb,rb);
    // size_t occs = backward_search(fm_index, 0, fm_index.size()-1, query.begin(), query.end(), lb, rb);
    cout << "# of occurrences: " << occs << endl;
    // if (occs > 0) {
    //         cout << "Location and context of first occurrences: " << endl;
    //         size_t min_idx = rmq(lb, rb);
    //         size_t min_val = fm_index[min_idx];
    //         priority_queue<interval, vector<interval>, greater<interval> > pq; // min-heap
    //         pq.push(interval(lb, rb, min_val, min_idx));
    //         for (size_t i = 0, pre_extract = pre_context, post_extract = post_context; i < min(occs, max_locations); ++i) {
    //             interval r = pq.top(); pq.pop();
    //             size_t location = r.min_val;
    //             if (r.min_idx > r.lb) {
    //                 min_idx = rmq(r.lb, r.min_idx-1);
    //                 pq.push(interval(r.lb, r.min_idx-1, fm_index[min_idx], min_idx));
    //             }
    //             if (r.min_idx < r.rb) {
    //                 min_idx = rmq(r.min_idx+1, r.rb);
    //                 pq.push(interval(r.min_idx+1, r.rb, fm_index[min_idx], min_idx));
    //             }
    //             cout << setw(8) << location << ": ";
    //             if (pre_extract > location) {
    //                 pre_extract = location;
    //             }
    //             if (location+m+ post_extract > fm_index.size()) {
    //                 post_extract = fm_index.size()-location-m;
    //             }
    //             auto s   = extract(fm_index, location-pre_extract, location+m+ post_extract-1);
    //             string pre = s.substr(0, pre_extract);
    //             s = s.substr(pre_extract);
    //             if (pre.find_last_of('\n') != string::npos) {
    //                 pre = pre.substr(pre.find_last_of('\n')+1);
    //             }
    //             cout << pre;
    //             cout << "\e[1;31m";
    //             cout << s.substr(0, m);
    //             cout << "\e[0m";
    //             string context = s.substr(m);
    //             cout << context.substr(0, context.find_first_of('\n')) << endl;
    //         }
    //     }
    return 0;
}
