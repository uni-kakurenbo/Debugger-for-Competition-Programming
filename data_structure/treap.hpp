#pragma once


#include <memory>
#include <memory_resource>
#include <cassert>
#include <utility>
#include <type_traits>
#include <initializer_list>
#include <random>
#include <concepts>
#include <ranges>


#include "snippet/aliases.hpp"
#include "utility/functional.hpp"

#include "internal/dev_env.hpp"
#include "internal/uncopyable.hpp"
#include "internal/dummy.hpp"
#include "internal/types.hpp"
#include "internal/concepts.hpp"

#include "global/constants.hpp"

#include "random/engine.hpp"


#include "debugger/debug.hpp"


namespace lib {

namespace internal {


// Thanks to: https://github.com/xuzijian629/library2/blob/master/treap/implicit_treap.cpp
template<class Allocator, class Derived, std::integral SizeType, class ValueType, i64 Id>
struct treap_impl : private uncopyable {
    using size_type = SizeType;
    using value_type = ValueType;

    using allocator_type = Allocator;


    struct node_type;
    using node_pointer = std::add_pointer_t<node_type>;

  private:
    using derived = Derived;

    inline derived* _derived() noexcept(NO_EXCEPT) {
        return static_cast<derived*>(this);
    }
    inline const derived* _derived() const noexcept(NO_EXCEPT) {
        return static_cast<const derived*>(this);
    }


    using allocator_traits = std::allocator_traits<allocator_type>;

    using node_allocator_type = allocator_traits::template rebind_alloc<node_type>;
    using node_allocator_traits = std::allocator_traits<node_allocator_type>;

    node_allocator_type _allocator;


    static inline random_engine_32bit _rand;

    static inline unsigned _instance_count = 0;


    using priority_type = random_engine_32bit::result_type;

  protected:
    void pushup(const node_pointer tree) const noexcept(NO_EXCEPT) {
        if(tree == node_type::nil) return;
        tree->size = tree->left->size + tree->length + tree->right->size;
        this->_derived()->pushup(tree);
    }


    void rectify(const node_pointer tree) const noexcept(NO_EXCEPT) {
        if(tree->size == 0) return;

        std::vector<priority_type> priorities(tree->size);
        std::ranges::generate(priorities, this->_rand);
        std::ranges::make_heap(priorities);

        std::queue<node_pointer> queue;
        queue.push(tree);

        auto itr = std::ranges::begin(priorities);
        while(!queue.empty()) {
            node_pointer node = queue.front();
            queue.pop();

            node->priority = *(itr++);

            if(node->left != node_type::nil) queue.push(node->left);
            if(node->right != node_type::nil) queue.push(node->right);
        }
    }


    node_pointer create_node(const value_type& val, const size_type size) noexcept(NO_EXCEPT) {
        if(size == 0) return node_type::nil;

        node_pointer node = node_allocator_traits::allocate(this->_allocator, 1);
        node_allocator_traits::construct(this->_allocator, node, node_type{ val, size });

        return node;
    }

    void delete_tree(node_pointer tree) noexcept(NO_EXCEPT) {
        if(tree == node_type::nil) return;

        this->delete_tree(tree->left);
        this->delete_tree(tree->right);

        node_allocator_traits::deallocate(this->_allocator, tree, 1);
    }


    void _split(const node_pointer tree, const size_type pos, node_pointer& left, node_pointer& right) noexcept(NO_EXCEPT) {
        if(tree == node_type::nil) {
            left = right = node_type::nil;
            return;
        }

        this->_derived()->pushdown(tree);

        const size_type lower_bound = tree->left->size;
        const size_type upper_bound = tree->size - tree->right->size;

        if(pos <= lower_bound) {
            this->split(tree->left, pos, left, tree->left), right = std::move(tree);
            this->pushup(right);
        }
        else if(pos >= upper_bound) {
            this->split(tree->right, pos - upper_bound, tree->right, right), left = std::move(tree);
            this->pushup(left);
        }
        else {
            tree->length = pos - lower_bound;
            this->merge(tree->right, this->create_node(tree->data, upper_bound - pos), tree->right);

            this->split(tree->right, 0, tree->right, right), left = std::move(tree);
            this->pushup(left);
        }
    }

  public:
    treap_impl(const allocator_type& alloc = {}) noexcept(NO_EXCEPT)
      : _allocator(alloc)
    {
        if(treap_impl::_instance_count++ == 0) {
            node_type::nil = new node_type{};
        }
    }

    ~treap_impl() noexcept {
        if(--treap_impl::_instance_count == 0) {
            delete node_type::nil;
        }
    }


    struct node_type {
        priority_type priority = std::numeric_limits<priority_type>::lowest();

        node_pointer left = node_type::nil, right = node_type::nil;

        size_type length, size;
        [[no_unique_address]] value_type data;

        node_type() noexcept = default;

        node_type(const value_type& _data, const size_type _size) noexcept(NO_EXCEPT)
            : priority(treap_impl::_rand()), length(_size), size(_size), data(_data)
        {}

        static inline node_pointer nil = nullptr;
    };


    template<bool STRICT = false>
    void split(const node_pointer tree, const value_type& val, node_pointer& left, node_pointer& right) noexcept(NO_EXCEPT) {
        if(tree == node_type::nil) {
            left = right = node_type::nil;
            return;
        }

        this->_derived()->pushdown(tree);

        if(val < tree->data || (!STRICT && val == tree->data)) {
            this->split(tree->left, val, left, tree->left), right = std::move(tree);
            this->pushup(right);
        }
        else {
            this->split(tree->right, val, tree->right, right), left = std::move(tree);
            this->pushup(left);
        }
    }


    void split(const node_pointer tree, const size_type pos, node_pointer& left, node_pointer& right) noexcept(NO_EXCEPT) {
        if(pos <= 0) {
            left = this->create_node(value_type{}, -pos);
            right = std::move(tree);
        }
        else if(tree->size <= pos) {
            right = this->create_node(value_type{}, pos - tree->size);
            left = std::move(tree);
        }
        else {
            this->_split(std::move(tree), pos, left, right);
        }
    }


    void merge(node_pointer& tree, const node_pointer left, const node_pointer right) noexcept(NO_EXCEPT) {
        this->_derived()->pushdown(left);
        this->_derived()->pushdown(right);

        // debug(left->priority, right->priority);

        if(left == node_type::nil || right == node_type::nil) {
            tree = left == node_type::nil ? right : left;
        }
        else if(left->priority > right->priority) {
            this->merge(left->right, left->right, right), tree = std::move(left);
        }
        else {
            this->merge(right->left, left, right->left), tree = std::move(right);
        }

        this->pushup(tree);
    }
};


} // namespace internal


template<std::integral SizeType = i64, class Allocator = std::allocator<SizeType>, i64 Id = -1>
struct treap_context {
    template<class Derived, class ValueType = internal::dummy>
    using interface = internal::treap_impl<Allocator, Derived, SizeType, ValueType, Id>;
};


namespace pmr {


template<std::integral SizeType = i64, i64 Id = -1>
using treap_context = lib::treap_context<SizeType, std::pmr::polymorphic_allocator<SizeType>, Id>;


} // namespace pmr


} // namespace lib
