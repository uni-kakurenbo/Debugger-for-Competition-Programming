#pragma once


#include <cassert>
#include <algorithm>
#include <vector>
#include <type_traits>
#include <concepts>
#include <ranges>
#include <bit>


#include "snippet/iterations.hpp"

#include "internal/dev_env.hpp"
#include "internal/types.hpp"
#include "internal/iterator.hpp"
#include "internal/point_reference.hpp"
#include "internal/range_reference.hpp"
#include "internal/unconstructible.hpp"

#include "algebraic/internal/concepts.hpp"
#include "action/base.hpp"

#include "debugger/debug.hpp"

namespace lib {

namespace internal {

namespace dynamic_segment_tree_impl {


// Thanks to: https://lorent-kyopro.hatenablog.com/entry/2021/03/12/025644
template<algebraic::internal::monoid Monoid, class Allocator = std::allocator<Monoid>>
struct core {
    using size_type = internal::size_t;
    using operand = Monoid;

    using allocator_type = Allocator;

    struct node_type;
    using node_pointer = std::add_pointer_t<node_type>;

  private:
    using allocator_traits = std::allocator_traits<allocator_type>;

    using node_allocator_type = allocator_traits::template rebind_alloc<node_type>;
    using node_allocator_traits = std::allocator_traits<node_allocator_type>;

    [[no_unique_address]] node_allocator_type _allocator;

  public:
    struct node_type {
        size_type index;
        operand val, acc;

        node_pointer left = node_type::nil, right = node_type::nil;

        node_type() noexcept = default;

        node_type(const size_type _index, const operand& _val) noexcept(NO_EXCEPT)
          : index(_index), val(_val), acc(_val)
        {}

        static inline node_pointer nil = nullptr;
    };

  protected:
    inline void pull(const node_pointer tree) const noexcept(NO_EXCEPT) {
        tree->acc = tree->left->acc + tree->val + tree->right->acc;
    }


    node_pointer create_node(const size_type pos, const operand& val) noexcept(NO_EXCEPT) {
        node_pointer node = node_allocator_traits::allocate(this->_allocator, 1);
        node_allocator_traits::construct(this->_allocator, node, pos, val);

        return node;
    }

    void delete_tree(node_pointer tree) noexcept(NO_EXCEPT) {
        if(tree == node_type::nil) return;

        this->delete_tree(tree->left);
        this->delete_tree(tree->right);

        node_allocator_traits::deallocate(this->_allocator, tree, 1);
    }

    static inline unsigned _instance_count = 0;

  public:
    template<std::random_access_iterator I>
    node_pointer build(const size_type lower, const size_type upper, const size_type l, const size_type r, I first) noexcept(NO_EXCEPT) {
        const size_type middle = (lower + upper) >> 1;
        const auto itr = std::ranges::next(first, middle);

        if(middle < l || r <= middle) return node_type::nil;

        node_pointer node = this->create_node(middle, *itr);

        node->left = this->build(lower, middle, l, middle, first);
        node->right = this->build(middle, upper, middle + 1, r, first);

        this->pull(node);

        return node;
    }

    template<std::random_access_iterator I, std::sentinel_for<I> S>
    node_pointer build(I first, S last) noexcept(NO_EXCEPT) {
        const size_type size = std::ranges::distance(first, last);
        return this->build(0, size, 0, size, first);
    }


    void set(node_pointer& tree, const size_type lower, const size_type upper, size_type pos, operand val) noexcept(NO_EXCEPT) {
        if(tree == node_type::nil) {
            tree = this->create_node(pos, val);
            return;
        }

        if(tree->index == pos) {
            tree->val = val;
            this->pull(tree);
            return;
        }

        const size_type middle = (lower + upper) >> 1;

        if(pos < middle) {
            if(tree->index < pos) std::swap(tree->index, pos), std::swap(tree->val, val);
            this->set(tree->left, lower, middle, pos, val);
        }
        else {
            if(pos < tree->index) std::swap(tree->index, pos), std::swap(tree->val, val);
            this->set(tree->right, middle, upper, pos, val);
        }

        this->pull(tree);
    }


    operand get(const node_pointer tree, const size_type lower, const size_type upper, const size_type pos) const noexcept(NO_EXCEPT) {
        if(tree == node_type::nil) return {};
        if(tree->index == pos) return tree->val;

        const size_type middle = (lower + upper) >> 1;

        if(pos < middle) return this->get(tree->left, lower, middle, pos);
        else return this->get(tree->right, middle, upper, pos);
    }


    operand fold(const node_pointer tree, const size_type lower, const size_type upper, const size_type l, const size_type r) const noexcept(NO_EXCEPT) {
        if(tree == node_type::nil || upper <= l || r <= lower) return {};
        if(l <= lower && upper <= r) return tree->acc;

        const size_type middle = (lower + upper) >> 1;

        operand val = this->fold(tree->left, lower, middle, l, r);
        if(l <= tree->index && tree->index < r) val = val + tree->val;

        return val + this->fold(tree->right, middle, upper, l, r);
    }


    void clear(const node_pointer tree, const size_type lower, const size_type upper, const size_type l, const size_type r) const noexcept(NO_EXCEPT) {
        if(tree == node_type::nil || upper <= l || r <= lower) return;

        if(l <= lower && upper <= r) {
            this->delete_tree(tree);
            tree = node_type::nil;
            return;
        }

        const size_type middle = (lower + upper) >> 1;

        this->clear(tree->left, lower, middle, l, r);
        this->clear(tree->right, middle, upper, l, r);

        this->pull(tree);
    }


    template<class F>
    size_type max_right(const node_pointer tree, const size_type lower, const size_type upper, const size_type l, F f, operand& acc) const {
        if(tree == node_type::nil || upper <= l) return -1;

        if(f(acc + tree->acc)) {
            acc = acc + tree->acc;
            return -1;
        }

        const size_type middle = (lower + upper) >> 1;
        const size_type res = this->max_right(tree->left, lower, middle, l, f, acc);

        if(res != -1) return res;

        if(l <= tree->index && !f(acc = acc + tree->val)) return tree->index;

        return this->max_right(tree->right, middle, upper, l, f, acc);
    }

    template<class F>
    size_type min_left(const node_pointer tree, const size_type lower, const size_type upper, const size_type r, F f, operand& acc) const {
        if(tree == node_type::nil || r <= lower) return 0;

        if(f(tree->acc + acc)) {
            acc = tree->acc + acc;
            return 0;
        }

        const size_type middle = (lower + upper) >> 1;
        const size_type res = this->min_left(tree->right, middle, upper, r, f, acc);

        if(res != 0) return res;

        if(tree->index < r && !f(acc = tree->val + acc)) {
            return tree->index + 1;
        }

        return this->min_left(tree->left, lower, middle, r, f, acc);
    }

  public:
    explicit core(const allocator_type& alloc = {}) noexcept(NO_EXCEPT) : _allocator(alloc) {
        if(core::_instance_count++ == 0) {
            node_type::nil = new node_type{};
        }
    }

    ~core() {
        if(--core::_instance_count == 0) {
            delete node_type::nil;
        }
    }


    core(const core&, const allocator_type& allocator) noexcept(NO_EXCEPT) : _allocator(allocator) {}
    core(core&&, const allocator_type& allocator) noexcept(NO_EXCEPT) : _allocator(allocator) {}


    debugger::debug_t dump_rich(const node_pointer tree, const std::string prefix = "   ", const int dir = 0) const {
        if(!tree || tree == node_type::nil) return prefix + "\n";

        const auto left = this->dump_rich(tree->left, prefix + (dir == 1 ? "| " : "  "), -1);
        const auto here = prefix + "--+ " + debugger::dump(tree->index) + " : " + debugger::dump(tree->val) + "\n";

        const auto right = this->dump_rich(tree->right, prefix + (dir == -1 ? "| " : "  "), 1);

        return left + here + right;
    }

    debugger::debug_t _debug(const node_pointer tree) const {
        if(!tree || tree == node_type::nil) return "";

        return
            "(" +
            this->_debug(tree->left) + " " +
            debugger::dump(tree->val) +
            this->_debug(tree->right) +
            ")";
    }
};


} // namespace dynamic_segment_tree_impl

} // namespace internal


template<class, class = std::allocator<void>>
struct dynamic_segment_tree : internal::unconstructible {};


template<algebraic::internal::monoid Monoid, class Allocator>
struct dynamic_segment_tree<Monoid, Allocator> : private internal::dynamic_segment_tree_impl::core<Monoid, Allocator> {
  private:
    using core = typename internal::dynamic_segment_tree_impl::core<Monoid, Allocator>;

  public:
    using value_type = Monoid;
    using size_type = typename core::size_type;

    using allocator_type = Allocator;

  private:
    inline size_type _positivize_index(const size_type p) const noexcept(NO_EXCEPT) {
        return p < 0 ? this->_n + p : p;
    }


    using node_type = typename core::node_type;
    using node_pointer = typename core::node_pointer;

    size_type _n = 0;
    node_pointer _root = node_type::nil;

  public:
    dynamic_segment_tree(const allocator_type allocator = {}) noexcept(NO_EXCEPT) : core(allocator) {};

    ~dynamic_segment_tree() noexcept(NO_EXCEPT) { this->delete_tree(this->_root); }


    explicit dynamic_segment_tree(const size_type n, const allocator_type allocator = {}) noexcept(NO_EXCEPT)
      : core(allocator)
    {
        this->_n = n;
    }

    template<std::convertible_to<value_type> T>
    dynamic_segment_tree(const std::initializer_list<T>& init_list, const allocator_type allocator = {}) noexcept(NO_EXCEPT)
      : dynamic_segment_tree(init_list, allocator)
    {}

    template<std::input_iterator I, std::sized_sentinel_for<I> S>
    dynamic_segment_tree(I first, S last, const allocator_type allocator = {}) noexcept(NO_EXCEPT) : dynamic_segment_tree(allocator) {
        this->assign(first, last);
    }

    template<std::ranges::input_range R>
    explicit dynamic_segment_tree(R&& range, const allocator_type allocator = {}) noexcept(NO_EXCEPT)
      : dynamic_segment_tree(ALL(range), allocator)
    {}


    inline size_type size() const noexcept(NO_EXCEPT) { return this->_n; }


    inline auto& clear() const noexcept(NO_EXCEPT) {
        this->delete_tree(this->_root);
        this->_root = node_type::nil;
        return *this;
    }


    template<std::convertible_to<value_type> T>
    inline auto& assign(const std::initializer_list<T>& init_list) noexcept(NO_EXCEPT) { return this->assign(init_list); }

    template<std::input_iterator I, std::sized_sentinel_for<I> S>
    inline auto& assign(I first, S last) noexcept(NO_EXCEPT) {
        this->delete_tree(this->_root);
        this->_n = std::ranges::distance(first, last);
        this->_root = this->build(first, last);
        return *this;
    }

    template<std::ranges::input_range R>
    inline auto& assign(R&& range) noexcept(NO_EXCEPT) { return this->assign(ALL(range)); }


    inline auto& set(const size_type pos, value_type val) noexcept(NO_EXCEPT) {
        assert(pos < this->_n);
        this->core::set(this->_root, 0, this->_n, pos, val);
        return *this;
    }


    inline value_type get(const size_type pos) const noexcept(NO_EXCEPT) {
        assert(pos < this->_n);
        return this->core::get(this->_root, 0, this->_n, pos);
    }


    inline auto& add(const size_type pos, const value_type val) noexcept(NO_EXCEPT) {
        assert(0 <= pos && pos < this->_n);
        this->set(pos, this->get(pos) + val);
        return *this;
    }


    inline value_type fold(const size_type l, const size_type r) const noexcept(NO_EXCEPT) {
        assert(0 <= l && l <= r && r <= this->_n);
        return this->core::fold(this->_root, 0, this->_n, l, r);
    }

    inline value_type fold() const noexcept(NO_EXCEPT) { return this->_root->acc; }


    inline auto& clear(const size_type l, const size_type r) noexcept(NO_EXCEPT) {
        assert(0 <= l && l <= r && r <= this->_n);
        this->core::clear(this->_root, 0, this->_n, l, r);
        return *this;
    }


    template<bool (*f)(value_type)>
    inline size_type max_right(const size_type l) const noexcept(NO_EXCEPT) {
        return this->max_right(l, [](const value_type val) { return f(val); });
    }

    template<class F>
    inline size_type max_right(const size_type l, const F &f) const noexcept(NO_EXCEPT) {
        assert(0 <= l && l <= this->_n);
        value_type acc;
        assert(f(acc));
        const auto res = this->core::max_right(this->_root, 0, this->_n, l, f, acc);
        return res == -1 ? this->_n : res;
    }


    template<bool (*f)(value_type)>
    inline size_type min_left(const size_type r) const noexcept(NO_EXCEPT) {
        return this->min_left(r, [](const value_type val) noexcept(NO_EXCEPT) { return f(val); });
    }

    template<class F>
    inline size_type min_left(const size_type r, const F &f) const noexcept(NO_EXCEPT) {
        assert(0 <= r && r <= this->_n);
        value_type acc;
        assert(f(acc));
        return this->core::min_left(this->_root, 0, this->_n, r, f, acc);
    }


    struct point_reference : internal::point_reference<dynamic_segment_tree> {
        point_reference(dynamic_segment_tree *const super, const size_type p) noexcept(NO_EXCEPT)
          : internal::point_reference<dynamic_segment_tree>(super, super->_positivize_index(p))
        {
            assert(0 <= this->_pos && this->_pos < this->_super->_n);
        }

        operator value_type() const noexcept(NO_EXCEPT) { return this->_super->get(this->_pos); }
        value_type val() const noexcept(NO_EXCEPT) { return this->_super->get(this->_pos); }

        inline point_reference& set(const value_type& v) noexcept(NO_EXCEPT) {
            this->_super->set(this->_pos, v);
            return *this;
        }
        inline point_reference& operator=(const value_type& v) noexcept(NO_EXCEPT) {
            this->_super->set(this->_pos, v);
            return *this;
        }

        inline point_reference& add(const value_type& v) noexcept(NO_EXCEPT) {
            this->_super->add(this->_pos, v);
            return *this;
        }
        inline point_reference& operator+=(const value_type& v) noexcept(NO_EXCEPT) {
            this->_super->add(this->_pos, v);
            return *this;
        }
    };

    struct range_reference : internal::range_reference<dynamic_segment_tree> {
        range_reference(dynamic_segment_tree *const super, const size_type l, const size_type r) noexcept(NO_EXCEPT)
          : internal::range_reference<dynamic_segment_tree>(super, super->_positivize_index(l), super->_positivize_index(r))
        {
            assert(0 <= this->_begin && this->_begin <= this->_end && this->_end <= this->_super->_n);
        }

        inline value_type fold() noexcept(NO_EXCEPT) {
            return this->_super->fold(this->_begin, this->_end);
        }
    };


    inline point_reference operator[](const size_type p) noexcept(NO_EXCEPT) { return point_reference(this, p); }
    inline range_reference operator()(const size_type l, const size_type r) noexcept(NO_EXCEPT) { return range_reference(this, l, r); }


    struct iterator : internal::container_iterator_interface<value_type, const dynamic_segment_tree, iterator> {
        iterator() noexcept = default;

        iterator(const dynamic_segment_tree *const ref, const size_type p) noexcept(NO_EXCEPT)
          : internal::container_iterator_interface<value_type, const dynamic_segment_tree, iterator>(ref, p)
        {}

        inline value_type operator*() const noexcept(NO_EXCEPT) { return this->ref()->get(this->pos()); }
    };

    inline iterator begin() const noexcept(NO_EXCEPT) { return iterator(this, 0); }
    inline iterator end() const noexcept(NO_EXCEPT) { return iterator(this, this->_n); }


    using core::dump_rich;
    using core::_debug;


    debugger::debug_t dump_rich(const std::string prefix = "   ") const {
        return "\n" + this->dump_rich(this->_root, prefix);
    }

    debugger::debug_t _debug() const {
        return "[ " + this->_debug(this->_root) + " ]";
    }
};


template<actions::internal::operatable_action Action, class Allocator>
struct dynamic_segment_tree<Action, Allocator> : dynamic_segment_tree<typename Action::operand, Allocator> {
    using dynamic_segment_tree<typename Action::operand, Allocator>::dynamic_segment_tree;
};

} // namespace lib