#include <unity.h>
#include <math.h>
#include "vpd/vpd.h"

#define TOL 0.005f

struct Row { float t, rh, svp, air, leaf; };

static const Row ROWS[] = {
  {20, 60, 2.338f, 0.935f, 0.661f},
  {26, 70, 3.361f, 1.008f, 0.631f},
  {28, 75, 3.780f, 0.945f, 0.526f},
  {30, 50, 4.243f, 2.122f, 1.658f},
};

void setUp() {}
void tearDown() {}

void test_reference_table() {
  for (const Row &r : ROWS) {
    TEST_ASSERT_FLOAT_WITHIN(TOL, r.svp, vpdSvp(r.t));
    TEST_ASSERT_FLOAT_WITHIN(TOL, r.air, vpdAir(r.t, r.rh));
    TEST_ASSERT_FLOAT_WITHIN(TOL, r.leaf, vpdLeaf(r.t, r.rh, 2.0f));
  }
}

void test_rh_clamped() {
  TEST_ASSERT_FLOAT_WITHIN(1e-6f, 0.0f, vpdAir(25, 120));
  TEST_ASSERT_FLOAT_WITHIN(1e-6f, vpdSvp(25), vpdAir(25, -10));
}

void test_leaf_negative_not_clamped() {
  // RH 100 %, leaf 2 °C cooler than air -> negative leaf VPD
  TEST_ASSERT_TRUE(vpdLeaf(25, 100, 2.0f) < 0.0f);
}

void test_invalid_input() {
  TEST_ASSERT_FALSE(vpdInputValid(NAN, 50));
  TEST_ASSERT_FALSE(vpdInputValid(25, NAN));
  TEST_ASSERT_TRUE(vpdInputValid(25, 50));
}

int main(int, char **) {
  UNITY_BEGIN();
  RUN_TEST(test_reference_table);
  RUN_TEST(test_rh_clamped);
  RUN_TEST(test_leaf_negative_not_clamped);
  RUN_TEST(test_invalid_input);
  return UNITY_END();
}
