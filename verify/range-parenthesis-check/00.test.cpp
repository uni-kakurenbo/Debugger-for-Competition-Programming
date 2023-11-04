/*
 * @uni_kakurenbo
 * https://github.com/uni-kakurenbo/competitive-programming-workspace
 *
 * CC0 1.0  http://creativecommons.org/publicdomain/zero/1.0/deed.ja
 */
/* #language C++ 20 GCC */

#define PROBLEM "https://atcoder.jp/contests/abc223/tasks/abc223_f"

#include <iostream>
#include "snippet/aliases.hpp"
#include "snippet/fast_io.hpp"
#include "snippet/iterations.hpp"
#include "adapter/io.hpp"
#include "adapter/string.hpp"
#include "data_structure/segment_tree.hpp"

template<class T>
struct monoid : lib::algebraic::base<lib::spair<T>>, lib::algebraic::associative/*, lib::algebraic::invertible, lib::algebraic::commutative*/ {
    using base = lib::algebraic::base<lib::spair<T>>;
    using base::base;
    // monoid() : base() {}
    monoid(const T& v) : base({ v, v }) {}
    friend inline monoid operator+(const monoid& lhs, const monoid& rhs) noexcept(NO_EXCEPT) {
        return monoid({ lib::min(lhs->first, lhs->second + rhs->first), lhs->second + rhs->second });
    }
    // inline bitxor operator-() const noexcept(NO_EXCEPT) { return -this->val(); }
};

signed main() {
    int n, q; std::cin >> n >> q;
    lib::string s; std::cin >> s;

    lib::segment_tree<monoid<int>> data(n);
    REP(i, n) data[i] = s[i] == '(' ? 1 : -1;

    REP(q) {
        int t, l, r; std::cin >> t >> l >> r;
        if(t == 1) {
            --l, --r;
            auto tmp = data[l].val();
            data[l] = data[r].val(), data[r] = tmp;
        }
        if(t == 2) {
            --l;
            auto [ pm, sm ] = data(l, r).fold().val();
            print.yesno(pm >= 0 and sm == 0);
        }
    }
}
