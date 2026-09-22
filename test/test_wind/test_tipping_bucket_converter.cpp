#include <gtest/gtest.h>
#include "helpers/wind/TippingBucketConverter.h"

TEST(TippingBucketConverterTest, ConvertsTipsToTenthsOfMillimetre) {
  EXPECT_EQ(TippingBucketConverter::tenthsMm(0), 0);
  EXPECT_EQ(TippingBucketConverter::tenthsMm(1), 3);       // 0.2794 mm -> 0.3 mm
  EXPECT_EQ(TippingBucketConverter::tenthsMm(100), 279);   // 27.94 mm -> 27.9 mm; rounding error doesn't accumulate per tip
  EXPECT_EQ(TippingBucketConverter::tenthsMm(3579), 10000);   // ~1000 mm
}

TEST(TippingBucketConverterTest, StopsAtSixteenBitsInsteadOfWrapping) {
  EXPECT_EQ(TippingBucketConverter::tenthsMm(23455), 65533);       // just under the limit
  EXPECT_EQ(TippingBucketConverter::tenthsMm(23456), 65535);       // 65536 tenths: wrapping would read 0
  EXPECT_EQ(TippingBucketConverter::tenthsMm(25000), 65535);       // 6985 mm would be 69850 tenths
  EXPECT_EQ(TippingBucketConverter::tenthsMm(0xFFFFFFFF), 65535);  // and the intermediate product must not overflow
}
