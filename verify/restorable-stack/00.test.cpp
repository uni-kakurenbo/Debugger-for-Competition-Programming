/*
 * @uni_kakurenbo
 * https://github.com/uni-kakurenbo/competitive-programming-workspace
 *
 * CC0 1.0  http://creativecommons.org/publicdomain/zero/1.0/deed.ja
 */
/* #language C++ 20 GCC */

#define PROBLEM "https://atcoder.jp/contests/abc273/tasks/abc273_e"


#include <iostream>
#include "snippet/aliases.hpp"
#include "snippet/fast_io.hpp"
#include "snippet/iterations.hpp"
#include "adapter/io.hpp"
#include "adapter/string.hpp"
#include "data_structure/restorable_stack.hpp"

signed main() {
    int q; input >> q;

    lib::restorable_stack<int> data;

    REP(i, q) {
        lib::string t; input >> t;
        if(t == "ADD") {
            int x; input >> x;
            data.push(x);
        }
        if(t == "DELETE") {
            if(!data.empty()) data.pop();
        }
        if(t == "SAVE") {
            int x; input >> x;
            data.save(x);
        }
        if(t == "LOAD") {
            int x; input >> x;
            data.load_or_clear(x);
        }

        print << data.top_or(-1);
        if(i < q - 1) print << " ";
        else print();
    }
}
