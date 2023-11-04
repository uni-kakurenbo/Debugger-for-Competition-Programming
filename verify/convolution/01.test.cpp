/*
 * @uni_kakurenbo
 * https://github.com/uni-kakurenbo/competitive-programming-workspace
 *
 * CC0 1.0  http://creativecommons.org/publicdomain/zero/1.0/deed.ja
 */
/* #language C++ GCC */

#define PROBLEM "https://judge.yosupo.jp/problem/convolution_mod"

#include "snippet/aliases.hpp"
#include "snippet/fast_io.hpp"
#include "adapter/io.hpp"
#include "numeric/modint.hpp"
#include "convolution/sum.hpp"

using mint = lib::modint998244353;

signed main() {
    int n, m; std::cin >> n >> m;
    std::vector<mint> a(n), b(m); input >> a >> b;
    print(lib::convolution(a, b));
}
