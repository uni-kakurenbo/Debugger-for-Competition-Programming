#include <iostream>

#include "snippet/iterations.hpp"

#include "data_structure/implicit_treap.hpp"
#include "data_structure/range_action/range_add_range_min.hpp"

signed main() {
    int n, q; std::cin >> n >> q;
    lib::implicit_treap<lib::action::range_add_range_min<int>> data(n);
    // debug(data);

    REP(q) {
        int t; std::cin >> t;
        if(t == 0) {
            int l, r, x; std::cin >> l >> r >> x; ++r;
            data.apply(l, r, x);
        }
        if(t == 1) {
            int l, r; std::cin >> l >> r; ++r;
            std::cout << data.prod(l, r) << "\n";
        }
        // debug(data);
    }
}
