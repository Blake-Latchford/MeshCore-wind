#include <gtest/gtest.h>
#include "helpers/wind/AnemometerPulseConverter.h"

TEST(AnemometerPulseConverterTest, OneHertzIsTwoPointFourKmh) {
  // 1 closure/s = 2.4 km/h = 0.6667 m/s
  EXPECT_EQ(AnemometerPulseConverter::hundredthsMetersPerSecond(1, 1000), 67);
  EXPECT_EQ(AnemometerPulseConverter::hundredthsMetersPerSecond(3, 3000), 67);
  EXPECT_EQ(AnemometerPulseConverter::hundredthsMetersPerSecond(45, 1000), 3000);   // 45 Hz = 30.00 m/s
  EXPECT_EQ(AnemometerPulseConverter::hundredthsMetersPerSecond(0, 5000), 0);
  EXPECT_EQ(AnemometerPulseConverter::hundredthsMetersPerSecond(5, 0), 0);
}

TEST(AnemometerPulseConverterTest, UsesTheMeasuredInterval) {
  EXPECT_EQ(AnemometerPulseConverter::hundredthsMetersPerSecond(10, 5000), 133);   // 2 Hz
  EXPECT_EQ(AnemometerPulseConverter::hundredthsMetersPerSecond(3, 3300), 61);     // a late tick reads slower than 3 pulses in 3 s
}

TEST(AnemometerPulseConverterTest, SaturatesInsteadOfWrapping) {
  EXPECT_EQ(AnemometerPulseConverter::hundredthsMetersPerSecond(0xFFFFFF, 1000), 0xFFFF);
}
