// Copyright 2019 Nikita Danilov

#include <gtest/gtest.h>

#include <functional>

#include "include/scopeguard.hxx"

namespace
{
  auto count = 0u;
  void inc(unsigned& c) { ++c; }
  void incCount() { inc(count); }
  void resetCount(unsigned& c) { c = 0u; }
  void resetCount() { resetCount(count); }
} // namespace

TEST(ScopeGuard, ScopeGuard_can_be_created_via_API_function)
{
        ASSERT_NO_THROW(makeScopeGuard(incCount));
}

TEST(ScopeGuard, Plain_function_can_be_used_to_create_a_scope_guard)
{
  makeScopeGuard(incCount);
}

////////////////////////////////////////////////////////////////////////////////
TEST(ScopeGuard, Scope_guard_executes_the_function_exactly_once_when_leaving_scope)
{
  resetCount();

  {
    const auto guard = makeScopeGuard(incCount);
    ASSERT_EQ(count, 0u);
  }

  ASSERT_EQ(count, 1u);
}

////////////////////////////////////////////////////////////////////////////////
TEST(ScopeGuard, Disarmed_scope_guard_does_not_execute_its_callback_at_all)
{
  resetCount();

  {
    auto guard = makeScopeGuard(incCount);
    ASSERT_EQ(count, 0u);

    guard.disarm();
    ASSERT_EQ(count, 0u);
  }

  ASSERT_EQ(count, 0u);
}

////////////////////////////////////////////////////////////////////////////////
TEST(ScopeGuard, Lvalue_reference_can_be_used_to_create_a_scope_guard)
{
  auto& inc_ref = incCount;
  makeScopeGuard(inc_ref);
}

////////////////////////////////////////////////////////////////////////////////
TEST(ScopeGuard, Lvalue_reference_based_scope_guard_executes_the_function_exactly_once_when_leaving_scope)
{
  resetCount();

  {
    auto& inc_ref = incCount;
    const auto guard = makeScopeGuard(inc_ref);
    ASSERT_EQ(count, 0u);
  }

  ASSERT_EQ(count, 1u);
}

////////////////////////////////////////////////////////////////////////////////
TEST(ScopeGuard, Disarmed_lvalue_reference_based_scope_guard_does_not_execute_its_callback_at_all)
{
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

////////////////////////////////////////////////////////////////////////////////
TEST(ScopeGuard, Reference_wrapper_can_be_used_to_create_a_scope_guard)
{
  makeScopeGuard(std::ref(incCount));
}

////////////////////////////////////////////////////////////////////////////////
TEST(ScopeGuard, Reference_wrapper_based_scope_guard_executes_the_function_exactly_once_when_leaving_scope)
{
  resetCount();

  {
    const auto guard = makeScopeGuard(std::ref(incCount));
    ASSERT_EQ(count, 0u);
  }

  ASSERT_EQ(count, 1u);
}

////////////////////////////////////////////////////////////////////////////////
TEST(ScopeGuard, Reference_wrapper_based_scope_guard_can_be_disarmed)
{
  makeScopeGuard(std::ref(incCount)).disarm();
}

////////////////////////////////////////////////////////////////////////////////
TEST(ScopeGuard, Disarmed_reference_wrapper_based_scope_guard_does_not_execute_its_callback_at_all)
{
  resetCount();

  {
    auto guard = makeScopeGuard(std::ref(incCount));
    ASSERT_EQ(count, 0u);

    guard.disarm();
    ASSERT_EQ(count, 0u);
  }

  ASSERT_EQ(count, 0u);
}

////////////////////////////////////////////////////////////////////////////////
TEST(ScopeGuard, Lvalue_function_pointer_can_be_used_to_create_a_scope_guard)
{
  const auto fp = &incCount;
  makeScopeGuard(fp);
}

////////////////////////////////////////////////////////////////////////////////
TEST(ScopeGuard, Lvalue_function_pointer_based_scope_guard_executes_the_function_exactly_once_when_leaving_scope)
{
  resetCount();

  {
    const auto fp = &incCount;
    const auto guard = makeScopeGuard(fp);
    ASSERT_EQ(count, 0u);
  }

  ASSERT_EQ(count, 1u);
}

////////////////////////////////////////////////////////////////////////////////
TEST(ScopeGuard, Lvalue_function_pointer_based_scope_guard_can_be_disarmed)
{
  const auto fp = &incCount;
  makeScopeGuard(fp).disarm();
}

////////////////////////////////////////////////////////////////////////////////
TEST(ScopeGuard, Disarmed_lvalue_function_pointer_based_scope_guard_does_not_execute_its_callback_at_all)
{
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

////////////////////////////////////////////////////////////////////////////////
TEST(ScopeGuard, Rvalue_reference_to_function_pointer_can_be_used_to_create_a_scope_guard)
{
  const auto fp = &incCount;
  makeScopeGuard(std::move(fp));
}

////////////////////////////////////////////////////////////////////////////////
TEST(ScopeGuard, Function_pointer_rvalue_reference_based_scope_guard_executes_the_function_exactly_once_when_leaving_scope)
{
  resetCount();

  {
    const auto fp = &incCount;
    const auto guard = makeScopeGuard(std::move(fp));
    ASSERT_EQ(count, 0u);
  }

  ASSERT_EQ(count, 1u);
}

////////////////////////////////////////////////////////////////////////////////
TEST(ScopeGuard, Function_pointer_rvalue_reference_based_scope_guard_can_be_disarmed)
{
  const auto fp = &incCount;
  makeScopeGuard(std::move(fp)).disarm();
}

////////////////////////////////////////////////////////////////////////////////
TEST(ScopeGuard, Disarmed_function_pointer_rvalue_reference_based_scope_guard_does_not_execute_its_callback_at_all)
{
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