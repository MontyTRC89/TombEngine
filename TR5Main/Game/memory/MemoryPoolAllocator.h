#pragma once
#include <limits>
#include <iostream>
#include "qmalloc.h"
namespace T5M::Memory {
    /* The following code example is taken from the book
 * "The C++ Standard Library - A Tutorial and Reference"
 * by Nicolai M. Josuttis, Addison-Wesley, 1999
 *
 * (C) Copyright Nicolai M. Josuttis 1999.
 * Permission to copy, use, modify, sell and distribute this software
 * is granted provided this copyright notice appears in all copies.
 * This software is provided "as is" without express or implied
 * warranty, and with no claim as to its suitability for any purpose.
 */
    template <class T>
    class MemoryPoolAllocator {
        
    public:
        MemoryPool& pool;
        // type definitions
        using value_type = T;
        using pointer = T*;
        using const_pointer = const T*;
        using reference = T&;
        using const_reference = const T&;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        // rebind allocator to type U
        template <class U>
        struct rebind {
            using other = MemoryPoolAllocator<U>;
        };

        // return address of values
        pointer address(reference value) const {
            return &value;
        }
        const_pointer address(const_reference value) const {
            return &value;
        }

        /* constructors and destructor
            * - nothing to do because the allocator has no state
            */
        MemoryPoolAllocator(MemoryPool& pool) : pool(pool) {}
        MemoryPoolAllocator(const MemoryPoolAllocator& arg) : pool(arg.pool) {};
        template <class U>
        MemoryPoolAllocator(const MemoryPoolAllocator<U>& arg) : pool(arg.pool) {};
        ~MemoryPoolAllocator() = default;

        // return maximum number of elements that can be allocated
        size_type max_size() const {
#pragma push_macro("max")
#undef max
            return std::numeric_limits<std::size_t>::max() / sizeof(T);
#pragma pop_macro("max")
        }

        // allocate but don't initialize num elements of type T
        pointer allocate(size_type num, const void* = 0) {
            pointer ret = (pointer)(pool.malloc<T>(num));
            return ret;
        }

        // initialize elements of allocated storage p with value value
        void construct(pointer p, const T& value) {
            // initialize memory with placement new
            new(p)T(value);
        }

        // destroy elements of initialized storage p
        void destroy(pointer p) {
            // destroy objects by calling their destructor
            p->~T();
        }

        // deallocate storage p of deleted elements
        void deallocate(pointer p, size_type num) {
           pool.free(p);
        }
    };
}