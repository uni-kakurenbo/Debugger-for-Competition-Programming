#pragma once


#include <cassert>
#include <cstdint>
#include <iostream>


#include "macro/expand_tails.hpp"

#include "snippet/aliases.hpp"

#include "internal/dev_env.hpp"
#include "internal/types.hpp"

#include "numeric/internal/modint_interface.hpp"
#include "numeric/internal/primality_test.hpp"
#include "numeric/internal/primitive_root.hpp"
#include "numeric/barrett_reduction.hpp"

#include "template/debug.hpp"


namespace lib {


namespace internal {


template<std::unsigned_integral Value, std::unsigned_integral Large, Value Mod>
    requires has_double_digits_of<Large, Value> && (Mod > 0)
struct static_modint_impl : modint_interface<static_modint_impl<Value, Large, Mod>, Value> {
    using signed_value_type = std::make_signed_t<Value>;
    using unsigned_value_type = Value;
    using signed_large_type = std::make_signed_t<Large>;
    using unsigned_large_type = Large;

    static constexpr int digits = std::numeric_limits<unsigned_value_type>::digits;
    static constexpr unsigned_value_type max() noexcept { return std::numeric_limits<unsigned_value_type>::max(); }


  private:
    using mint = static_modint_impl;
    using base = modint_interface<static_modint_impl<Value, Large, Mod>, Value>;

    unsigned_value_type _val = 0;


  public:
    static constexpr bool is_prime = lib::internal::is_prime<mint>(Mod);
    static constexpr unsigned_value_type primitive_root = lib::internal::primitive_root<mint>(Mod);

    static constexpr mint zero = 0;
    static constexpr mint one = 1;

    static constexpr unsigned_value_type mod() noexcept(NO_EXCEPT) { return Mod; }

    static constexpr inline mint raw(const unsigned_value_type v) noexcept(NO_EXCEPT) {
        mint res;
        res._val = v;
        return res;
    }


    constexpr static_modint_impl() = default;

    template<std::integral T>
    constexpr static_modint_impl(T v) noexcept(NO_EXCEPT) {
        using common_type = std::common_type_t<T, unsigned_value_type>;
        const common_type m = static_cast<common_type>(Mod);

        v %= m;

        if constexpr(std::is_signed_v<T>) {
            if(v < 0) v += m;
        }

        this->_val = v;
    }


    constexpr inline unsigned_value_type val() const noexcept(NO_EXCEPT) { return this->_val; }


    constexpr inline mint& operator+=(const mint& rhs) noexcept(NO_EXCEPT) {
        if(this->_val >= Mod - rhs._val) this->_val -= Mod;
        this->_val += rhs._val;
        return *this;
    }

    constexpr inline mint& operator-=(const mint& rhs) noexcept(NO_EXCEPT) {
        if(this->_val < rhs._val) this->_val += Mod;
        this->_val -= rhs._val;
        return *this;
    }

    constexpr inline mint& operator*=(const mint& rhs) noexcept(NO_EXCEPT) {
        this->_val =
            static_cast<unsigned_value_type>(
                static_cast<unsigned_large_type>(this->_val) * rhs._val % Mod
            );
        return *this;
    }

    constexpr mint inv() const noexcept(NO_EXCEPT) {
        if constexpr(mint::is_prime) {
            assert(this->_val);
            return this->pow(Mod - 2);
        } else
        {
            return this->base::inv();
        }
    }

    friend constexpr bool operator==(const mint& lhs, const mint& rhs) noexcept(NO_EXCEPT) { return lhs._val == rhs._val; }
};


template<std::unsigned_integral Value, std::unsigned_integral Large, i64 Id>
    requires has_double_digits_of<Large, Value>
struct montgomery_modint_impl : modint_interface<montgomery_modint_impl<Value, Large, Id>, Value> {
    using signed_value_type = std::make_signed_t<Value>;
    using unsigned_value_type = Value;
    using signed_large_type = std::make_signed_t<Large>;
    using unsigned_large_type = Large;

    static constexpr int digits = std::numeric_limits<unsigned_value_type>::digits - 2;
    static constexpr unsigned_value_type max() noexcept { return (unsigned_value_type{ 1 } << mint::digits) - 1; }

  private:
    using mint = montgomery_modint_impl;

    unsigned_value_type _val = 0;

    static inline unsigned_value_type _mod;
    static inline unsigned_value_type _np;
    static inline unsigned_value_type _r2;

    constexpr static unsigned_value_type _mni() noexcept(NO_EXCEPT) {
        unsigned_value_type res = mint::_mod;
        while(mint::_mod * res != 1) res *= unsigned_value_type{ 2 } - mint::_mod * res;
        return res;
    }

    constexpr static unsigned_value_type _reduce(const unsigned_large_type &v) noexcept(NO_EXCEPT) {
        return
            static_cast<unsigned_value_type>(
                (
                    v + static_cast<unsigned_large_type>(static_cast<unsigned_value_type>(v) * mint::_np) * _mod
                ) >> std::numeric_limits<unsigned_value_type>::digits
            );
    }

    inline unsigned_value_type _normalized() const noexcept(NO_EXCEPT) {
        if(this->_val < mint::_mod) return this->_val;
        return this->_val - mint::_mod;
    }

  public:
    static inline mint zero;
    static inline mint one;

    static constexpr unsigned_value_type mod() noexcept(NO_EXCEPT) { return mint::_mod; }

    static constexpr void set_mod(const unsigned_value_type m) noexcept(NO_EXCEPT) {
        assert((m & 1) == 1);

        if(mint::mod() == m) return;

        assert(m <= mint::max());

        mint::_mod = m;
        mint::_r2 = static_cast<unsigned_value_type>(-static_cast<unsigned_large_type>(m) % m);
        mint::_np = -mint::_mni();
        mint::one._val = mint::_reduce(mint::_r2);
    }

    static constexpr inline mint raw(const unsigned_large_type v) noexcept(NO_EXCEPT)
    {
        mint res;
        res._val = mint::_reduce(v * mint::_r2);
        return res;
    };


    constexpr montgomery_modint_impl() = default;

    template<std::integral T>
    constexpr montgomery_modint_impl(T v) noexcept(NO_EXCEPT) {
        using common_type = std::common_type_t<T, unsigned_value_type>;

        const common_type m2 = static_cast<common_type>(mint::_mod << 1);

        if constexpr(std::is_signed_v<T>) {
            if(
                (v > 0 && static_cast<common_type>(v) >= m2)
            ) {
                v %= m2;
            }
            if(v < 0) {
                if(static_cast<common_type>(-v) >= m2) v %= m2;
                v += m2;
            }
        }
        else {
            if(static_cast<common_type>(v) >= m2) v %= m2;
        }

        this->_val = mint::_reduce(static_cast<unsigned_large_type>(v) * mint::_r2);
    }


    constexpr inline unsigned_value_type val() const noexcept(NO_EXCEPT) {
        unsigned_value_type res = mint::_reduce(this->_val);
        return res >= mint::_mod ? res - mint::_mod : res;
    }


    constexpr inline mint& operator+=(const mint& rhs) noexcept(NO_EXCEPT) {
        if(static_cast<signed_value_type>(this->_val += rhs._val - (mint::_mod << 1)) < 0) this->_val += mint::_mod << 1;
        return *this;
    }

    constexpr inline mint& operator-=(const mint& rhs) noexcept(NO_EXCEPT) {
        if(static_cast<signed_value_type>(this->_val -= rhs._val) < 0) this->_val += mint::_mod << 1;
        return *this;
    }

    constexpr inline mint& operator*=(const mint& rhs) noexcept(NO_EXCEPT) {
        this->_val = mint::_reduce(static_cast<unsigned_large_type>(this->_val) * rhs._val);
        return *this;
    }

    friend inline constexpr bool operator==(const mint& lhs, const mint& rhs) noexcept(NO_EXCEPT) { return lhs._normalized() == rhs._normalized(); }
};


template<std::unsigned_integral Value, std::unsigned_integral Large, i64 Id>
    requires has_double_digits_of<Large, Value>
struct barrett_modint_impl : modint_interface<barrett_modint_impl<Value, Large, Id>, Value> {
    using signed_value_type = std::make_signed_t<Value>;
    using unsigned_value_type = Value;
    using signed_large_type = std::make_signed_t<Large>;
    using unsigned_large_type = Large;

  private:
    using barrett = barrett_context<unsigned_value_type, unsigned_large_type>;
    using mint = barrett_modint_impl;

    unsigned_value_type _val = 0;

    static inline barrett _barrett;

  public:
    static constexpr int digits = barrett::digits;
    static constexpr unsigned_value_type max() noexcept { return barrett::max(); }

    static inline mint zero;
    static inline mint one = 1;

    static constexpr unsigned_value_type mod() noexcept(NO_EXCEPT) { return mint::_barrett.mod(); }

    static constexpr void set_mod(const unsigned_value_type m) noexcept(NO_EXCEPT) {
        if(mint::mod() == m) return;
        mint::_barrett = mint::barrett(m);
    }

    static constexpr inline mint raw(const unsigned_value_type v) noexcept(NO_EXCEPT)
    {
        mint res;
        res._val = v;
        return res;
    };

    constexpr barrett_modint_impl() = default;

    template<std::integral T>
    constexpr barrett_modint_impl(T v) noexcept(NO_EXCEPT) {
        using common_type = std::common_type_t<T, unsigned_value_type>;
        const common_type m = static_cast<common_type>(mint::mod());

        if(v > 0) {
            if constexpr(std::numeric_limits<common_type>::digits <= 64) {
                v = mint::_barrett.remainder(v);
            }
            else {
                v %= m;
            }
        }

        if constexpr(std::is_signed_v<T>) {
            if(v < 0) {
                if(static_cast<common_type>(-v) >= m) v %= m;
                v += m;
            }
        }

        this->_val = static_cast<unsigned_value_type>(v);
    }


    constexpr inline unsigned_value_type val() const noexcept(NO_EXCEPT) { return this->_val; }


    constexpr inline mint& operator+=(const mint& rhs) noexcept(NO_EXCEPT) {
        if((this->_val += rhs._val) >= mint::mod()) this->_val -= mint::mod();
        return *this;
    }

    constexpr inline mint& operator-=(const mint& rhs) noexcept(NO_EXCEPT) {
        if((this->_val += mint::mod() - rhs._val) >= mint::mod()) this->_val -= mint::mod();
        return *this;
    }

    constexpr inline mint& operator*=(const mint& rhs) noexcept(NO_EXCEPT) {
        this->_val = mint::_barrett.multiply(this->_val, rhs._val);
        return *this;
    }

    friend inline constexpr bool operator==(const mint& lhs, const mint& rhs) noexcept(NO_EXCEPT) { return lhs._val == rhs._val; }
};


} // namespace internal


} // namespace lib


constexpr lib::modint998244353 operator""_mod998244353(unsigned long long x) { return x; }
constexpr lib::modint1000000007 operator""_mod1000000007(unsigned long long x) { return x; }
