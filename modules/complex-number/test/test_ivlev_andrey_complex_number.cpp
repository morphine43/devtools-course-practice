// Copyright 2019 Ivlev Andrey

#include <gtest/gtest.h>
#include "include/complex_number.h"

TEST(Ivlev_Andrey_ComplexNumberTest, Sum_Two_Without_Explicit_Init_ImPart) {
    // Arrange
    ComplexNumber a;
    ComplexNumber b;
    ComplexNumber c(14, 0);

    // Act
    a.setRe(7);
    b.setRe(7);
    ComplexNumber res = a + b;

    // Act & Assert
    EXPECT_EQ(c, res);
}

TEST(Ivlev_Andrey_ComplexNumberTest, Sum_Two_Without_Explicit_Init_RealPart) {
    // Arrange
    ComplexNumber a;
    ComplexNumber b;
    ComplexNumber c(0, 14);

    // Act
    a.setIm(7);
    b.setIm(7);
    ComplexNumber res = a + b;

    // Assert
    EXPECT_EQ(c, res);
}

TEST(Ivlev_Andrey_ComplexNumberTest, Sub_Two_Without_Explicit_Init_ImPart) {
    // Arrange
    ComplexNumber a;
    ComplexNumber b;
    ComplexNumber c(7, 0);

    // Act
    a.setRe(14);
    b.setRe(7);
    ComplexNumber res = a - b;

    // Act & Assert
    EXPECT_EQ(c, res);
}

TEST(Ivlev_Andrey_ComplexNumberTest, Sub_Two_Without_Explicit_Init_RealPart) {
    // Arrange
    ComplexNumber a;
    ComplexNumber b;
    ComplexNumber c(0, 7);

    // Act
    a.setIm(14);
    b.setIm(7);
    ComplexNumber res = a - b;

    // Assert
    EXPECT_EQ(c, res);
}
