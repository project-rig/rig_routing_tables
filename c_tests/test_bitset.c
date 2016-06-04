#include "tests.h"
#include "platform.h"

#include "bitset.h"


START_TEST(test_bitset_use)
{
  // Initialise the bitset
  bitset_t b;
  bitset_init(&b, 100);  // 100 element bitset

  // Test that nothing is in the set
  ck_assert_int_eq(b.count, 0);
  ck_assert_int_eq(b.n_elements, 100);
  for (unsigned int i = 0; i < 100; i++)
  {
    ck_assert(!bitset_contains(&b, i));
  }

  // Add the first element to the set
  ck_assert(bitset_add(&b, 0));
  ck_assert_int_eq(b.count, 1);
  ck_assert(bitset_contains(&b, 0));

  // Add the last element to the set
  ck_assert(bitset_add(&b, 99));
  ck_assert_int_eq(b.count, 2);
  ck_assert(bitset_contains(&b, 99));

  // Add an element that's out of range
  ck_assert(!bitset_add(&b, 100));
  ck_assert_int_eq(b.count, 2);

  // Remove a non-element
  ck_assert(!bitset_remove(&b, 98));
  ck_assert_int_eq(b.count, 2);  // No change

  // Remove a complete non-element
  ck_assert(!bitset_remove(&b, 100));
  ck_assert_int_eq(b.count, 2);  // No change

  // Remove an non-element
  ck_assert(bitset_remove(&b, 0));
  ck_assert_int_eq(b.count, 1);

  // Clear the bitset and check
  ck_assert(bitset_clear(&b));
  ck_assert_int_eq(b.count, 0);
  for (unsigned int i = 0; i < 100; i++)
  {
    ck_assert(!bitset_contains(&b, i));
  }

  // Delete the bitset
  bitset_delete(&b);

  // All previous operations should fail
  ck_assert(!bitset_add(&b, 0));
  ck_assert_int_eq(b.count, 0);
  ck_assert(!bitset_contains(&b, 0));
}
END_TEST


Suite* bitset_suite(void)
{
  Suite *s;
  TCase *tests;

  s = suite_create("BitSet");
  tests = tcase_create("Core");
  suite_add_tcase(s, tests);

  // Add the tests
  tcase_add_test(tests, test_bitset_use);

  return s;
}
