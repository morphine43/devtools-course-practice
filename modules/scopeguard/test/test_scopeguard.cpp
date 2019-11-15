// Copyright 2019 Nikita Danilov

#include <gtest/gtest.h>

#include <functional>

#include "include/scopeguard.hxx"

namespace {
    auto count = 0u;
    void inc(unsigned* c) { ++(*c); }
    void incCount() { inc(&count); }
    void resetCount(unsigned* c) { *c = 0u; }
    void resetCount() { resetCount(&count); }
}  // namespace

TEST(ScopeGuard, Sg_can_be_created_via_API_function) {
    ASSERT_NO_THROW(makeScopeGuard(incCount));
}

TEST(ScopeGuard, Plain_function_can_be_used_to_create_a_sg) {
    makeScopeGuard(incCount);
}

TEST(ScopeGuard, Sg_calls_func_once) {
    resetCount();

    {
        const auto guard = makeScopeGuard(incCount);
        ASSERT_EQ(count, 0u);
    }

    ASSERT_EQ(count, 1u);
}

TEST(ScopeGuard, Disarmed_sg_does_not_call_func) {
    resetCount();

    {
        auto guard = makeScopeGuard(incCount);
        ASSERT_EQ(count, 0u);

        guard.disarm();
        ASSERT_EQ(count, 0u);
    }

    ASSERT_EQ(count, 0u);
}


TEST(ScopeGuard, Lval_ref_can_be_used_to_create_a_sg) {
    auto& inc_ref = incCount;

    makeScopeGuard(inc_ref);
}


TEST(ScopeGuard, Lval_ref_based_sg_calls_func_once) {
    resetCount();

    {
        auto& inc_ref = incCount;
        const auto guard = makeScopeGuard(inc_ref);
        ASSERT_EQ(count, 0u);
    }

    ASSERT_EQ(count, 1u);
}


TEST(ScopeGuard, Disarmed_lval_ref_based_sg_does_not_call_func) {
    resetCount();

    {
        auto& inc_ref = incCount;
        auto guard = makeScopeGuard(inc_ref);
        ASSERT_EQ(count, 0u);

        guard.disarm();
        ASSERT_EQ(count, 0u);
    }

    ASSERT_EQ(count, 0u);
}


TEST(ScopeGuard, Ref_wrapper_can_be_used_to_create_a_sg) {
    makeScopeGuard(std::ref(incCount));
}


TEST(ScopeGuard, Ref_wrapper_based_sg_calls_func_once) {
    resetCount();

    {
        const auto guard = makeScopeGuard(std::ref(incCount));
        ASSERT_EQ(count, 0u);
    }

    ASSERT_EQ(count, 1u);
}


TEST(ScopeGuard, Ref_wrapper_based_sg_can_be_disarmed) {
    makeScopeGuard(std::ref(incCount)).disarm();
}


TEST(ScopeGuard, Disarmed_ref_wrapper_based_sg_does_not_call_func) {
    resetCount();

    {
        auto guard = makeScopeGuard(std::ref(incCount));
        ASSERT_EQ(count, 0u);

        guard.disarm();
        ASSERT_EQ(count, 0u);
    }

    ASSERT_EQ(count, 0u);
}


TEST(ScopeGuard, Lval_function_ptr_can_be_used_to_create_a_sg) {
    const auto fp = &incCount;

    makeScopeGuard(fp);
}


TEST(ScopeGuard, Lval_function_ptr_based_sg_calls_func_once) {
    resetCount();

    {
        const auto fp = &incCount;
        const auto guard = makeScopeGuard(fp);
        ASSERT_EQ(count, 0u);
    }

    ASSERT_EQ(count, 1u);
}


TEST(ScopeGuard, Lval_function_ptr_based_sg_can_be_disarmed) {
    const auto fp = &incCount;

    makeScopeGuard(fp).disarm();
}


TEST(ScopeGuard, Disarmed_lval_function_ptr_based_sg_does_not_call_func) {
    resetCount();

    {
        const auto fp = &incCount;
        auto guard = makeScopeGuard(fp);
        ASSERT_EQ(count, 0u);

        guard.disarm();
        ASSERT_EQ(count, 0u);
    }

    ASSERT_EQ(count, 0u);
}


TEST(ScopeGuard, Rval_ref_to_function_ptr_can_be_used_to_create_a_sg) {
    const auto fp = &incCount;

    makeScopeGuard(std::move(fp));
}


TEST(ScopeGuard, Function_ptr_rval_ref_based_sg_calls_func_once) {
    resetCount();

    {
        const auto fp = &incCount;
        const auto guard = makeScopeGuard(std::move(fp));
        ASSERT_EQ(count, 0u);
    }

    ASSERT_EQ(count, 1u);
}


TEST(ScopeGuard, Function_ptr_rval_ref_based_sg_can_be_disarmed) {
    const auto fp = &incCount;
    makeScopeGuard(std::move(fp)).disarm();
}


TEST(ScopeGuard, Disarmed_function_ptr_rval_ref_based_sg_does_not_call_func) {
    resetCount();

    {
        const auto fp = &incCount;
        auto guard = makeScopeGuard(std::move(fp));
        ASSERT_EQ(count, 0u);

        guard.disarm();
        ASSERT_EQ(count, 0u);
    }

    ASSERT_EQ(count, 0u);
}
