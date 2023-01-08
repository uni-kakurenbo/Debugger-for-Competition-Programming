#pragma once

// #include "../inner/inner_math.hpp"
// #include "../misc/rng.hpp"

#include <cassert>
#include <vector>
#include <map>
#include <utility>
#include <algorithm>

#include "numeric/modint.hpp"
#include "random/xorshift.hpp"


namespace lib {

namespace internal {

//Thanks to: https://github.com/NyaanNyaan/library/blob/master/prime/fast-factorize.hpp
namespace fast_factorize_lib {


using value_type = uint64_t;


namespace internal {


// Miller-Rabin primality test
template<typename mint> bool primality_test(const value_type n, const std::initializer_list<value_type> as) {
    if(static_cast<value_type>(mint::mod()) != n) mint::set_mod(n);

    value_type d = n-1;

    while(~d & 1) d >>= 1;

    mint e(1), rev(n - 1);
    for(value_type a : as) {
        if(n <= a) break;
        value_type t = d;
        mint y = mint(a).pow(t);
        while(t != n - 1 && y != e && y != rev) y *= y, t *= 2;
        if(y != rev && t % 2 == 0) return false;
    }

    return true;
}


} // namespace internal


bool is_prime(const value_type n) {
    if(~n & 1) return n == 2;
    if(n <= 1) return false;

    if(n < (1UL << 31)) return internal::primality_test<modint>(n, { 2, 7, 61 });
    else return internal::primality_test<modint64>(n, { 2, 325, 9375, 28178, 450775, 9780504, 1795265022 });
}

using int64 = std::int64_t;

namespace internal {


xorshift64 rnd;

// Pollard's rho algorithm
template <typename mint, typename T> T find_factor(T n) {
    if(~n & 1) return 2;
    if(is_prime(n)) return n;

    if(static_cast<value_type>(mint::mod()) != n) mint::set_mod(n);

    mint R, one = 1;

    auto f = [&](mint x) { return x * x + R; };
    auto rnd_ = [&]() { return rnd() % (n - 2) + 2; };

    while(true) {

        mint x, y, ys, q = one;
        R = rnd_(), y = rnd_();
        T g = 1;
        constexpr int m = 128;

        for(int r = 1; g == 1; r <<= 1) {
            x = y;
            for(int i = 0; i < r; ++i)
                y = f(y);
            for(int k = 0; g == 1 && k < r; k += m) {
                ys = y;
                for(int i = 0; i < m && i < r - k; ++i)
                    q *= x - (y = f(y));
                g = std::gcd(q.val(), n);
            }
        }

        if(g == n) {
            do {
                g = std::gcd((x - (ys = f(ys))).val(), n);
            } while(g == 1);
        }
        if(g != n) return g;
    }

    assert(false);
}


std::vector<int64> factorize(value_type n) {
    if(n <= 1) return {};

    value_type p;
    if(n <= (1UL << 31)) p = find_factor<modint,std::uint32_t>(n);
    else p = find_factor<modint64,std::uint64_t>(n);

    if(p == n) return { int64(p) };

    auto l = internal::factorize(p);
    auto r = internal::factorize(n / p);

    std::copy(std::begin(r), std::end(r), std::back_inserter(l));

    return l;
}


} // namespace internal


std::vector<int64> factorize(value_type n) {
    auto res = internal::factorize(n);
    std::sort(std::begin(res), std::end(res));
    return res;
}

std::map<int64, int64> factor_count(value_type n) {
    std::map<int64, int64> mp;
    for(auto &x : factorize(n)) mp[x]++;
    return mp;
}

std::vector<int64> divisors(value_type n) {
    if(n == 0) return {};

    std::vector<std::pair<int64, int64>> v;
    for(auto &p : factorize(n)) {
        if(v.empty() || v.back().first != p) {
            v.emplace_back(p, 1);
        } else {
            v.back().second++;
        }
    }

    std::vector<int64> res;
    auto f = [&](auto rc, int i, int64 x) -> void {
        if(i == (int)v.size()) {
            res.push_back(x);
            return;
        }
        for(int j = v[i].second;; --j) {
            rc(rc, i + 1, x);
            if(j == 0)
                break;
            x *= v[i].first;
        }
    };

    f(f, 0, 1);
    std::sort(std::begin(res), std::end(res));

    return res;
}


} // namespace fast_factorize_lib

} // namespace internal


using internal::fast_factorize_lib::divisors;
using internal::fast_factorize_lib::factor_count;
using internal::fast_factorize_lib::factorize;
using internal::fast_factorize_lib::is_prime;


} // namespace lib
