/*
 * @uni_kakurenbo
 * https://github.com/uni-kakurenbo/competitive-programming-workspace
 *
 * CC0 1.0  http://creativecommons.org/publicdomain/zero/1.0/deed.ja
 */
/* #language C++ 20 GCC */

#define PROBLEM "https://atcoder.jp/contests/abc300/tasks/abc300_d"

#include <iostream>
#include <cmath>
#include "snippet/aliases.hpp"
#include "snippet/fast_io.hpp"
#include "numeric/arithmetic.hpp"
#include "adapter/io.hpp"
#include "numeric/prime_enumerator.hpp"

signed main() {
    lib::i64 n; std::cin >> n;

    const lib::i64 r = std::ceil(std::pow(n, 1.0 / 5));
    const lib::i64 m = lib::sqrt_ceil((n + 11) / 12);

    lib::prime_enumerator<lib::i64> p(m);

    lib::i64 ans = 0;

    for(auto a=p.begin(), end=p.end(); a!=end; ++a) {
        if(*a > r) break;
        const lib::i64 aa = *a * *a;
        for(auto c=std::next(a); c!=end; ++c) {
            const lib::i64 aacc = aa * *c * *c;
            const lib::i64 aaacc = *a * aacc;
            if(aaacc > n) break;
            if(n < (*c - 1) * aacc) {
                if(aaacc + aacc <= n) ans += p.count(n / aacc) - a.index() - 1;
            }
            else if(*a + 1 < *c - 1) ans += c - a - 1;
        }
    }

    print(ans);
}
