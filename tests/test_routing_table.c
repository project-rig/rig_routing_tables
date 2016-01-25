#include "tests.h"
#include "routing_table.h"


START_TEST(test_keymask_get_xs_and_count_xs)
{
  // Xs in all positions
  keymask_t km = {0x0, 0x0};
  ck_assert(keymask_get_xs(km) == (uint32_t) 0xffffffff);
  ck_assert_int_eq(keymask_count_xs(km), 32);

  // No Xs at all
  km.mask = 0xffffffff;
  ck_assert(keymask_get_xs(km) == (uint32_t) 0x00000000);
  ck_assert_int_eq(keymask_count_xs(km), 0);

  km.key = km.mask;
  ck_assert(keymask_get_xs(km) == (uint32_t) 0x00000000);
  ck_assert_int_eq(keymask_count_xs(km), 0);

  km.mask = 0x0;
  ck_assert(keymask_get_xs(km) == (uint32_t) 0x00000000);
  ck_assert_int_eq(keymask_count_xs(km), 0);

  // Some Xs
  km.key = 0x0;
  km.mask = 0x7fffffff;
  ck_assert(keymask_get_xs(km) == (uint32_t) 0x80000000);
  ck_assert_int_eq(keymask_count_xs(km), 1);

  km.mask = 0xfffffffe;
  ck_assert(keymask_get_xs(km) == (uint32_t) 0x00000001);
  ck_assert_int_eq(keymask_count_xs(km), 1);
}
END_TEST


START_TEST(test_keymask_intersect)
{
  keymask_t a, b;

  // All Xs intersects with all Xs
  a.key = 0x0; a.mask = 0x0;
  b.key = 0x0; b.mask = 0x0;
  ck_assert(keymask_intersect(a, b));

  // All 0s intersects with all Xs
  a.mask = 0xffffffff;
  ck_assert(keymask_intersect(a, b));

  // All 1s intersects with all Xs
  a.key = 0xffffffff;
  ck_assert(keymask_intersect(a, b));

  // All 0s doesn't intersect with all 1s
  b.mask = a.mask;
  ck_assert(!keymask_intersect(a, b));

  // More complex mix (not intersecting)
  a.key = 0x80000000; a.mask = 0xc0000000;  // 10XXXX...
  b.key = 0x00000000; b.mask = 0x80000000;  // 0XXXXX...
  ck_assert(!keymask_intersect(a, b));

  // More complex mix (intersecting)
  b.key = a.key;  // 1XXXX...
  ck_assert(keymask_intersect(a, b));
}
END_TEST


START_TEST(test_keymask_merge)
{
  keymask_t a, b, c;
  // All non Xs to all Xs
  a.key = 0x00000000; a.mask = 0xffffffff;
  b.key = 0xffffffff; b.mask = 0xffffffff;

  c = keymask_merge(a, b);
  ck_assert(c.key == 0x0 && c.mask == 0x0);

  c = keymask_merge(b, a);
  ck_assert(c.key == 0x0 && c.mask == 0x0);

  // Mix of existing Xs and new Xs
  // X and X -> X
  // 0 and X -> X
  // 1 and X -> X
  // X and 0 -> X
  // X and 1 -> X
  // 0 and 0 -> 0
  // 0 and 1 -> X
  // 1 and 1 -> 1
  // 1 and 0 -> X
  a.key = 0b001000011; a.mask = 0b011001111;  // X01XX0011
  b.key = 0b000010110; b.mask = 0b000111111;  // XXX010110
                                              // ---------
  c = keymask_merge(a, b);                    // XXXXX0X1X
  ck_assert(c.key == 0b000000010 && c.mask == 0b000001010);
}
END_TEST


Suite* routing_table_suite(void)
{
  Suite *s;
  TCase *tests;

  s = suite_create("Routing Table");
  tests = tcase_create("KeyMask");
  suite_add_tcase(s, tests);

  // Add the tests
  tcase_add_test(tests, test_keymask_get_xs_and_count_xs);
  tcase_add_test(tests, test_keymask_intersect);
  tcase_add_test(tests, test_keymask_merge);

  return s;
}
