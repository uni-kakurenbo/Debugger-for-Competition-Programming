#pragma once


#include <vector>


namespace lib {


template<class T> std::vector<T> divisors(const T k) {
    std::vector<T> res;

    for(T i=1; i*i<=k; ++i) {
        if(k%i == 0) {
            vector.emplace_back(i);
            if(i*i < k) vector.emplace_back(k/i);
        }
    }

    return res;
}


} // namespace lib
