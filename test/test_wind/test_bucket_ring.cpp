#include <gtest/gtest.h>
#include "helpers/wind/BucketRing.h"

TEST(BucketRingTest, EmptyReadsZero) {
  BucketRing<4> ring;
  EXPECT_EQ(ring.mean(), 0);
  EXPECT_EQ(ring.max(), 0);
}

TEST(BucketRingTest, MeanAndMaxBeforeItFills) {
  BucketRing<4> ring;
  ring.record(10);
  ring.record(20);
  EXPECT_EQ(ring.mean(), 15);
  EXPECT_EQ(ring.max(), 20);
}

TEST(BucketRingTest, MeanAndMaxOnceFull) {
  BucketRing<4> ring;
  ring.record(10);
  ring.record(20);
  ring.record(30);
  ring.record(40);
  EXPECT_EQ(ring.mean(), 25);
  EXPECT_EQ(ring.max(), 40);
}

TEST(BucketRingTest, OldestBucketIsDroppedOnceFull) {
  BucketRing<4> ring;
  ring.record(100);   // this one should fall off
  ring.record(10);
  ring.record(20);
  ring.record(30);
  ring.record(40);
  EXPECT_EQ(ring.mean(), 25);   // not (100+10+20+30)/4
  EXPECT_EQ(ring.max(), 40);    // not 100
}

TEST(BucketRingTest, RecordingSeveralBucketsAtOnceFillsThemAll) {
  BucketRing<4> ring;
  ring.record(50, 3);   // as if 3 buckets were missed and backfilled with the same value
  EXPECT_EQ(ring.mean(), 50);
  EXPECT_EQ(ring.max(), 50);

  ring.record(10);
  EXPECT_EQ(ring.mean(), (50 + 50 + 50 + 10) / 4);
  EXPECT_EQ(ring.max(), 50);
}

TEST(BucketRingTest, RecordingMoreBucketsThanCapacityDoesNotOverrun) {
  BucketRing<4> ring;
  ring.record(5, 100);   // must not write past _values[4)
  EXPECT_EQ(ring.mean(), 5);
  EXPECT_EQ(ring.max(), 5);
}

TEST(BucketRingTest, MaxTracksTheHighestBucketNotJustTheLatest) {
  BucketRing<4> ring;
  ring.record(5);
  ring.record(90);
  ring.record(5);
  EXPECT_EQ(ring.max(), 90);
}
