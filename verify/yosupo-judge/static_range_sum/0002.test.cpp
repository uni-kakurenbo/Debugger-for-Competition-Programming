/*
 * @uni_kakurenbo
 * https://github.com/uni-kakurenbo/competitive-programming-workspace
 *
 * CC0 1.0  http://creativecommons.org/publicdomain/zero/1.0/deed.ja
 */
/* #language C++ GCC */

#define PROBLEM "https://judge.yosupo.jp/problem/static_range_sum"

#include <iostream>
#include "snippet/aliases.hpp"
#include "snippet/fast_io.hpp"
#include "snippet/iterations.hpp"
#include "adaptor/io.hpp"
#include "adaptor/valarray.hpp"
#include "data_structure/fenwick_tree.hpp"
#include "algebraic/addition.hpp"

signed main() {
    uni::i32 n, q; input >> n >> q;
    uni::valarray<uni::i64> a(n); input >> a;
    uni::fenwick_tree<uni::algebraic::addition<uni::i64>> sum(a);

    REP(q) {
        uni::i32 l, r; input >> l >> r;
        print(sum(l, r).fold());
    }
}
