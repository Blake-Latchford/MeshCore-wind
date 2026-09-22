#include <gtest/gtest.h>
#include <cmath>
#include <set>
#include "helpers/wind/ResistiveVaneDecoder.h"

static uint32_t counts(uint32_t vane_ohms, uint32_t pullup, uint32_t adc_max) {
  return (uint32_t)std::lround(adc_max * (double)vane_ohms / (vane_ohms + pullup));
}

TEST(ResistiveVaneDecoderTest, TableCoversAllSixteenHeadings) {
  std::set<uint16_t> headings;
  for (int i = 0; i < WIND_VANE_POSITION_COUNT; i++) headings.insert(ResistiveVaneDecoder::positionHeadingTenths(i));
  ASSERT_EQ(headings.size(), 16u);
  for (int i = 0; i < 16; i++) EXPECT_TRUE(headings.count((uint16_t)(i * 225))) << i * 22.5;
}

TEST(ResistiveVaneDecoderTest, TableIsAscendingByResistance) {
  for (int i = 1; i < WIND_VANE_POSITION_COUNT; i++)
    EXPECT_GT(ResistiveVaneDecoder::positionOhms(i), ResistiveVaneDecoder::positionOhms(i - 1));
}

TEST(ResistiveVaneDecoderTest, EveryPositionDecodesToItsHeading) {
  for (int i = 0; i < WIND_VANE_POSITION_COUNT; i++) {
    uint32_t ohms = ResistiveVaneDecoder::positionOhms(i);
    uint16_t heading = 9999;
    ASSERT_TRUE(ResistiveVaneDecoder::decode(counts(ohms, 10000, 4095), 4095, 10000, heading)) << ohms;
    EXPECT_EQ(heading, ResistiveVaneDecoder::positionHeadingTenths(i)) << ohms;
  }
}

TEST(ResistiveVaneDecoderTest, ToleratesADCNoiseAndResistorTolerance) {
  // 1% pull-up shifts every reading by at most ~10 counts; add a little noise on top.
  for (int i = 0; i < WIND_VANE_POSITION_COUNT; i++) {
    uint32_t ohms = ResistiveVaneDecoder::positionOhms(i);
    uint32_t c = counts(ohms, 10000, 4095);
    for (int delta : {-12, -6, 6, 12}) {
      uint16_t heading = 9999;
      ASSERT_TRUE(ResistiveVaneDecoder::decode(c + delta, 4095, 10000, heading)) << ohms << " " << delta;
      EXPECT_EQ(heading, ResistiveVaneDecoder::positionHeadingTenths(i)) << ohms << " " << delta;
    }
  }
}

TEST(ResistiveVaneDecoderTest, OpenCircuitIsRejected) {
  // All reeds open (magnet between two positions, or vane unplugged): the pull-up takes the pin to full scale.
  uint16_t heading = 1234;
  EXPECT_FALSE(ResistiveVaneDecoder::decode(4095, 4095, 10000, heading));
  EXPECT_EQ(heading, 1234);   // untouched
}

TEST(ResistiveVaneDecoderTest, ShortIsRejected) {
  uint16_t heading = 1234;
  EXPECT_FALSE(ResistiveVaneDecoder::decode(0, 4095, 10000, heading));
  EXPECT_EQ(heading, 1234);
}

TEST(ResistiveVaneDecoderTest, ToleratesLargeDriftAtEitherEnd) {
  // The gate only has to catch open and short circuits, so a reading well off at the extreme positions
  // (e.g. a few percent of gain error) still decodes.
  uint16_t heading = 9999;
  ASSERT_TRUE(ResistiveVaneDecoder::decode(counts(120000, 10000, 4095) + 100, 4095, 10000, heading));
  EXPECT_EQ(heading, 2700);
  ASSERT_TRUE(ResistiveVaneDecoder::decode(counts(688, 10000, 4095) - 100, 4095, 10000, heading));
  EXPECT_EQ(heading, 1125);
}

TEST(ResistiveVaneDecoderTest, OtherPullupAndResolutionStillDecode) {
  for (int i = 0; i < WIND_VANE_POSITION_COUNT; i++) {
    uint32_t ohms = ResistiveVaneDecoder::positionOhms(i);
    uint16_t heading = 9999;
    ASSERT_TRUE(ResistiveVaneDecoder::decode(counts(ohms, 4700, 16383), 16383, 4700, heading)) << ohms;
    EXPECT_EQ(heading, ResistiveVaneDecoder::positionHeadingTenths(i)) << ohms;
  }
}
