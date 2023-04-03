#include <iostream>

#include "snippet/aliases.hpp"
#include "snippet/iterations.hpp"

// #include "template.hpp"

#include "data_structure/lazy_segment_tree.hpp"
#include "data_structure/range_action/range_add_range_min.hpp"

signed main() {
    int n, q; std::cin >> n >> q;

    lib::lazy_segment_tree<lib::actions::range_add_range_min<ll>> data(n, 0);
    // debug(data);

    REP(q) {
        int t; std::cin >> t;
        if(t == 0) {
            int l, r, x; std::cin >> l >> r >> x; ++r;
            data(l, r) += x;
        }
        if(t == 1) {
            int l, r; std::cin >> l >> r; ++r;
            std::cout << data(l, r).min() << "\n";
        }
        // debug(data);
    }
}
