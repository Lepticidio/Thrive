#include "engine/rng.h"
#include "engine/tests/test_component.h"
#include <unordered_set>
#include <set>
#include <gtest/gtest.h>


using namespace thrive;


TEST(RNG, getIntBetween) {
    RNG rng;
    std::set<int> rngIntValues;
    // A series of random integers should not result in the same number repeatedly. (These test will unintentionally fail once every 1x10^200 times, feeling lucky?)
    for (int i = 0; i < 100; ++i)
        rngIntValues.insert(rng.getIntBetween(1, 100));
    EXPECT_FALSE(rngIntValues.size()==1);
    // Random numbers produced must be in the provided range
    EXPECT_TRUE((*rngIntValues.begin())>=1);
    EXPECT_TRUE((*rngIntValues.end())<=100);
}

TEST(RNG, getDoubleBetween) {
    RNG rng;
    // A series of random doubles should not result in the same number repeatedly.
    std::set<double> rngDoubleValues;
    for (int i = 0; i < 100; ++i)
        rngDoubleValues.insert(rng.getDoubleBetween(1.0, 100.0));
    EXPECT_FALSE(rngDoubleValues.size()==1);
    // Random numbers produced must be in the provided range
    EXPECT_TRUE((*rngDoubleValues.begin())>=1.0);
    EXPECT_TRUE((*rngDoubleValues.end())<=100.0);
}

TEST(RNG, generateRandomSeed) {
    RNG rng;
    std::unordered_set<int> rngSeedValues;
    // A series of random seeds should not result in the same number repeatedly.
    for (int i = 0; i < 100; ++i)
        rngSeedValues.insert(rng.generateRandomSeed());
    EXPECT_FALSE(rngSeedValues.size()==1);
}

TEST(RNG, getSeed) {
    RNG rng(1337);
    EXPECT_EQ(1337, rng.getSeed());
}

TEST(RNG, setSeed) {
    RNG rng(1337);
    rng.setSeed(1234);
    EXPECT_EQ(1234, rng.getSeed());
}
