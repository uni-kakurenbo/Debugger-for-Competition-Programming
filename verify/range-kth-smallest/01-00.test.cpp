/*
 * @uni_kakurenbo
 * https://github.com/uni-kakurenbo/competitive-programming-workspace
 *
 * CC0 1.0  http://creativecommons.org/publicdomain/zero/1.0/deed.ja
 */
/* #language C++ 20 GCC */

#define PROBLEM "https://atcoder.jp/contests/abc239/tasks/abc239_e"

#include <iostream>
#include "snippet/aliases.hpp"
#include "snippet/fast_io.hpp"
#include "snippet/iterations.hpp"
#include "adapter/io.hpp"
#include "data_structure/wavelet_matrix.hpp"
#include "graph.hpp"

signed main() {

    int n, q; std::cin >> n >> q;
    lib::valarray<int> x(n); input >> x;

    lib::graph<int> G(n); G.read_bidirectionally(n - 1);

    lib::vector<int> vs;
    lib::valarray<int> in(n), out(n);
    auto dfs = [&](auto&& dfs, int v, int p) -> void {
        in[v] = vs.size();
        vs.push_back(v);
        ITR(e, G[v]) if(e.to != p) dfs(dfs, e.to, v);
        vs.push_back(v);
        out[v] = vs.size();
    };
    dfs(dfs, 0, -1);

    lib::valarray<int> y(2 * n);
    REP(i, 2 * n) y[i] = x[vs[i]];

    lib::wavelet_matrix<lib::u32>::compressed data(y);

    REP(q) {
        int v, k; std::cin >> v >> k; --v, --k;
        print(data(in[v], out[v]).kth_largest(2 * k));
    }
}
