#pragma once

#include <coroutine>
#include <exception>
#include <utility>

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
        std::exception_ptr ExtractException() {
            // Destructively return the stored exception pointer (if any), avoiding copies.
            return std::exchange(_Exception, {});
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
                // Note: the handle is not owned here.
                // It's owned by the Generator object and must only be destroyed there.
#ifdef _CPPUNWIND
                // Note: nullify the handle before rethrowing, for good measure.
                if (auto const exceptionPtr = std::exchange(m_handle, nullptr).promise().ExtractException())
                {
                    std::rethrow_exception(exceptionPtr);
                }
#else
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
            if (!m_handle.done()) {
                m_handle.resume();
            }
            if (m_handle.done()) {
#ifdef _CPPUNWIND
                auto const exceptionPtr = m_handle.promise().ExtractException();
#endif // _CPPUNWIND
                // Deterministically destroy the coroutine before throwing the exeption (if any).
                // This is for consistency: this would happen anyway if the owning Generator object
                // is destroyed as part of unwinding!
                // Exceptions should be thrown by value.
                m_handle.destroy();
                m_handle = {};
#ifdef _CPPUNWIND
                if (exceptionPtr)
                {
                    std::rethrow_exception(exceptionPtr);
                }
#endif // _CPPUNWIND
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
        std::swap(m_handle, _Right.m_handle);
        return *this;
    }

    ~Generator() {
        if (m_handle) {
            // Note: there can't be an exception here.
            // It's either already be extracted and thrown, or they never called begin() in the first place,
            // in which case the coroutine never got to run.
            m_handle.destroy();
        }
    }

private:
    std::coroutine_handle<promise_type> m_handle = nullptr;
};
