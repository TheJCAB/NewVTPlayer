#pragma once

#include <coroutine>
#include <exception>

template <class _Ty>
struct Generator {
    struct promise_type {
        const _Ty* _Value;
#ifdef _CPPUNWIND
        std::exception_ptr _Exception;
#endif // _CPPUNWIND

        Generator get_return_object() noexcept {
            return Generator{*this};
        }

        std::suspend_always initial_suspend() noexcept {
            return {};
        }

        std::suspend_always final_suspend() noexcept {
            return {};
        }

#ifndef _KERNEL_MODE
#ifdef _CPPUNWIND
        void unhandled_exception() noexcept {
            _Exception = std::current_exception();
        }
#else // ^^^ defined(_CPPUNWIND) / !defined(_CPPUNWIND) vvv
        void unhandled_exception() noexcept {}
#endif // _CPPUNWIND
#endif // _KERNEL_MODE

#ifdef _CPPUNWIND
        void _Rethrow_if_exception() {
            if (_Exception) {
                std::rethrow_exception(_Exception);
            }
        }
#endif // _CPPUNWIND

        std::suspend_always yield_value(const _Ty& _Val) noexcept {
            _Value = std::addressof(_Val);
            return {};
        }

        void return_void() noexcept {}

        template <class _Uty>
        _Uty&& await_transform(_Uty&& _Whatever) {
            static_assert(sizeof(_Uty) == 0,
                "co_await is not supported in coroutines of type Generator");
            return std::forward<_Uty>(_Whatever);
        }
    };

    struct iterator {
        using iterator_category = std::input_iterator_tag;
        using difference_type   = ptrdiff_t;
        using value_type        = _Ty;
        using reference         = const _Ty&;
        using pointer           = const _Ty*;

        std::coroutine_handle<promise_type> m_handle = nullptr;

        iterator() = default;
        explicit iterator(std::coroutine_handle<promise_type> _Coro_) noexcept : m_handle(_Coro_) {}

        iterator& operator++() {
            m_handle.resume();
            if (m_handle.done()) {
#ifdef _CPPUNWIND
                std::exchange(m_handle, nullptr).promise()._Rethrow_if_exception();
#else // ^^^ defined(_CPPUNWIND) / !defined(_CPPUNWIND) vvv
                m_handle = nullptr;
#endif // _CPPUNWIND
            }

            return *this;
        }

        void operator++(int) {
            // This operator meets the requirements of the C++20 input_iterator concept,
            // but not the Cpp17InputIterator requirements.
            ++*this;
        }

        [[nodiscard]] bool operator==(const iterator& _Right) const noexcept {
            return m_handle == _Right.m_handle;
        }

        [[nodiscard]] bool operator!=(const iterator& _Right) const noexcept {
            return !(*this == _Right);
        }

        [[nodiscard]] reference operator*() const noexcept {
            return *m_handle.promise()._Value;
        }

        [[nodiscard]] pointer operator->() const noexcept {
            return m_handle.promise()._Value;
        }
    };

    [[nodiscard]] iterator begin() {
        if (m_handle) {
            m_handle.resume();
            if (m_handle.done()) {
#ifdef _CPPUNWIND
                m_handle.promise()._Rethrow_if_exception();
#endif // _CPPUNWIND
                return {};
            }
        }

        return iterator{m_handle};
    }

    [[nodiscard]] iterator end() noexcept {
        return {};
    }

    explicit Generator(promise_type& _Prom) noexcept : m_handle(std::coroutine_handle<promise_type>::from_promise(_Prom)) {}

    Generator() = default;

    Generator(Generator&& _Right) noexcept : m_handle(std::exchange(_Right.m_handle, nullptr)) {}

    Generator& operator=(Generator&& _Right) noexcept {
        m_handle = std::exchange(_Right.m_handle, nullptr);
        return *this;
    }

    ~Generator() {
        if (m_handle) {
            m_handle.destroy();
        }
    }

private:
    std::coroutine_handle<promise_type> m_handle = nullptr;
};
