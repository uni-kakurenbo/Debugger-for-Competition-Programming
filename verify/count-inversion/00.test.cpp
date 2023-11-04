/*
 * @uni_kakurenbo
 * https://github.com/uni-kakurenbo/competitive-programming-workspace
 *
 * CC0 1.0  http://creativecommons.org/publicdomain/zero/1.0/deed.ja
 */
/* #language C++ 20 GCC */

#define PROBLEM "https://atcoder.jp/contests/arc101/tasks/arc101_d"

#include "snippet/aliases.hpp"
#include "snippet/fast_io.hpp"
#include "snippet/iterations.hpp"
#include "adapter/io.hpp"
#include "iterable/applied.hpp"
#include "numeric/boundary_seeker.hpp"
#include "iterable/count_inversion.hpp"

signed main() {
    lib::i64 n; std::cin >> n;
    lib::vector<lib::i64> a(n); input >> a;
    auto b = lib::sorted(a);

    auto ok = [&](int p) {
        lib::i64 v = b[p];

        lib::valarray<int> c(n), s(n+1);
        REP(i, n) c[i] = (a[i] < v ? -1 : 1);
        REP(i, n) s[i+1] = s[i] + c[i];
        s *= -1;

        lib::i64 res = lib::inversion<false>::count_with_compression(ALL(s));
        return res >= (n*(n+1)+3)/4;
    };

    lib::boundary_seeker<int> seeker(ok);
    int v = seeker.bound(0, n);

    print(b[v]);
}
