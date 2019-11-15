/**
 * Copyright 2019 Nikita Danilov
 *
 * @file scopeguard.h
 * Simple RAII pattern implementation.
 */

#pragma once

#include <utility>

// ScopeGuard
template <class Ty>
struct ScopeGuard
{
    ScopeGuard(Ty&& obj) : func_(obj) { }
    ScopeGuard(ScopeGuard&& other) : func_(std::move(other.func_)), active_(other.active_)
    {
        other.active_ = false;
    }

    ~ScopeGuard()
    {
        if (active_)
            func_();
    }

    void disarm()
    {
        active_ = false;
    }

protected:
    Ty func_;
    bool active_ = true;

private:
    ScopeGuard(const ScopeGuard&) = delete;
    ScopeGuard& operator=(const ScopeGuard&) = delete;
};

template <class Ty> inline
auto makeScopeGuard(Ty&& fn) -> ScopeGuard<Ty>
{
    return std::forward<Ty>(fn);
}
