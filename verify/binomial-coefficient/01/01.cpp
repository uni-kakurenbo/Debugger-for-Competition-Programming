/*
 * @uni_kakurenbo
 * https://github.com/uni-kakurenbo/competitive-programming-workspace
 *
 * CC0 1.0  http://creativecommons.org/publicdomain/zero/1.0/deed.ja
 */
/* #language C++ 20 GCC */
/* #region template */
#include <bits/stdc++.h>

#include "template.hpp"
/* #endregion */

void solve();

signed main() {
    debug(__cplusplus);
    int $ = 1;
    // std::cin >> $;
    for(int _ = 0; _ < $; ++_) {
        DEBUG("Case: #" + std::to_string(_));
        solve();
    }
    return 0;
}

#include "numeric/binomial_coefficient.hpp"

void solve() {
    int t; cin >> t;
    i32 m; cin >> m;

    lib::binomial_coefficient_prime_power_mod<i32> binom(m, 1, 10'000'000);

    REP(t) {
        i32 n, k; cin >> n >> k;
        print(binom.comb(n, k));
    }
}
