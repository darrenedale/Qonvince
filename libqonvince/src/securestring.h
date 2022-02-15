#pragma once

#include <memory>
#include <string>
#include <version>

namespace LibQonvince
{
    /**
     * A secure STL Allocator that ensures its allocations are wiped on deallocation.
     *
     * Implements the Allocator named requirements in terms of the normal std::allocator template,
     * except that deallocate() overwrites the memory with 0 bytes before deferring to std::allocator::deallocate().
     * The memory to deallocate is cast to a volatile byte pointer type to ensure that the compiler doesn't optimise
     * away calls to deallocate() that would otherwise be considered redundant owing to the provided pointer not
     * being used subsequently.
     *
     * @tparam T the value type for which the allocator is providing memory.
     */
    template<class T>
    struct SecureAllocator
    {
        using base_allocator_type = std::allocator<T>;
        using base_allocator_traits_type = std::allocator_traits<base_allocator_type>;

        // STL Allocator named requirements
        using value_type = typename base_allocator_type::value_type;
        using size_type = typename base_allocator_type::size_type;
        using difference_type = typename base_allocator_type::difference_type;
        using propagate_on_container_move_assignment = typename base_allocator_type::propagate_on_container_move_assignment;

        #if (201703L <= __cplusplus && 202002L >= __cplusplus)
        // introduced in c++17, deprecated in c++23
        using is_always_equal = typename base_allocator_type::is_always_equal;
        #endif

        #if (202002L > __cplusplus)
        // introduced in c++2017 or earlier, removed in c++20
        using pointer = typename base_allocator_type::pointer;
        using const_pointer = typename base_allocator_type::const_pointer;
        using reference = typename base_allocator_type::reference;
        using const_reference = typename base_allocator_type::const_reference;
        #endif

        constexpr SecureAllocator() noexcept
        : m_baseAllocator()
        {}

        constexpr SecureAllocator(const SecureAllocator & other)
        : m_baseAllocator(other.m_baseAllocator)
        {}

        template<class U>
        SecureAllocator(const SecureAllocator<U> & other)
        : m_baseAllocator(other)
        {}

        [[nodiscard]] constexpr T * allocate(std::size_t n)
        {
            return m_baseAllocator.allocate(n);
        }


#if (202002L < __cplusplus)
        // introduced in c++23
        [[nodiscard]] constexpr std::allocation_result<T*> allocate_at_least(std::size_t n)
        {
            return m_baseAllocator.allocate_at_least(n);
        }
#endif

#if (202002L > __cplusplus)
        // deprecated in c++17, removed in c++20
#if (201103L <= __cplusplus)
        // noexcept specifier added in c++11
        pointer address(reference x) const noexcept
        {
            return m_baseAllocator.address(x);
        }

        const_pointer address(const_reference x) const noexcept
        {
            return m_baseAllocator.address(x);
        }

        size_type max_size() const noexcept
        {
            return m_baseAllocator.max_size();
        }

        size_type construct(pointer ptr, const_reference val)
        {
            return m_baseAllocator.construct(ptr, val);
        }

        // templates from c++11 onwards
        template<class U, class ... Args>
        void construct(U * ptr, Args && ... args)
        {
            m_baseAllocator.construct(ptr, std::forward(args...));
        }

        template<class U>
        void destroy(U * ptr)
        {
            m_baseAllocator.destroy(ptr);
        }
#else
        pointer address(reference x) const
        {
            return m_baseAllocator.address(x);
        }

        const_pointer address(const_reference x) const
        {
            return m_baseAllocator.address(x);
        }

        size_type max_size() const throw()
        {
            return m_baseAllocator.max_size();
        }

        void construct(pointer ptr, const_reference val)
        {
            m_baseAllocator.construct(ptr, val);
        }

        void destroy(pointer ptr)
        {
            m_baseAllocator.destroy(ptr, val);
        }
#endif
#endif

        constexpr void deallocate(T * ptr, std::size_t size)
        {
            // cast to volatile ptr to array of bytes - volatile means compiler won't optimise away
            // tested with x86_64, GCC and Clang on Godbolt
            auto * bytes = reinterpret_cast<volatile char *>(ptr);
            auto byteSize= size * sizeof(T);

            // overwrite entire storage with 0s
            for (std::size_t idx = 0; idx < byteSize; ++idx)
            {
                bytes[idx] = 0;
            }

            m_baseAllocator.deallocate(ptr, size);
        }

    private:
        base_allocator_type m_baseAllocator;
    };

    /**
     * Convenience alias for std::basic_string, using the secure allocator.
     */
    template<
        class CharT,
        class Traits = std::char_traits<CharT>
    >
    using SecureBasicString = std::basic_string<CharT, Traits, SecureAllocator<CharT>>;

    /**
     * Convenience alias for std::string, using the secure allocator.
     */
    using SecureString = SecureBasicString<char>;
}
