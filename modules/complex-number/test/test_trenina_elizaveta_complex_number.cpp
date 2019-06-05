// Copyright 2019 Trenina Elizaveta

#include <gtest/gtest.h>

#include "include/complex_number.h"

TEST(Trenina_Elizaveta_ComplexNumberTest, Can_Create_Without_Parameters) {
    // Act & Assert
    ASSERT_NO_THROW(ComplexNumber{});
}

TEST(Trenina_Elizaveta_ComplexNumberTest, Can_Create_Copy) {
    // Arrange
    ComplexNumber a(11.5, 17.8);

    // Act
    ComplexNumber b(a);

    // Assert
    EXPECT_EQ(a, b);
}

TEST(Trenina_Elizaveta_ComplexNumberTest, Can_Multiply_Negative_Numbers) {
    // Arrange
    ComplexNumber a(-1.1, -1.0);
    ComplexNumber b(-10.0, -1.0);
    ComplexNumber exp(10.0, 11.1);

    // Act
    ComplexNumber act(a*b);

    // Assert
    EXPECT_EQ(exp, act);
}

TEST(Trenina_Elizaveta_ComplexNumberTest, Can_Assign_Some_Numbers) {
    // Arrange
    ComplexNumber a(-1.1, -1.0);
    ComplexNumber b(-10.0, -1.0);
    ComplexNumber c(10.0, 11.1);

    // Act
    a = b = c;

    // Assert
    EXPECT_TRUE(a == c);
}

TEST(Trenina_Elizaveta_ComplexNumberTest, Can_Calculate_With_Parentheses) {
    // Arrange
    ComplexNumber a(1.1, 2.1);
    ComplexNumber b(8.9, 17.9);
    ComplexNumber c(2.0, 2.0);
    ComplexNumber exp(-20.0, 60.0);
    // Act
    ComplexNumber act = (a + b)*c;

    // Assert
    EXPECT_EQ(exp, act);
}
