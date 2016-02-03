#include "tests.h"
#include "routing_table.h"
#include "merge.h"


START_TEST(test_merge_lifecycle)
{
  // Create a routing table from which we'll build merges
  entry_t entries[] = {
    {{0x0, 0xf}, 0b001},
    {{0x1, 0xf}, 0b001},
    {{0x2, 0xf}, 0b001},
    {{0x6, 0xf}, 0b001},
    {{0x0, 0x0}, 0b111},
  };
  table_t table = {5, entries};

  // Create a new merge based around the above table
  merge_t m;
  merge_init(&m, &table);

  // Check that no entries are in the merge
  ck_assert_int_eq(m.entries.count, 0);
  for (unsigned int i = 0; i < table.size; i++)
  {
    ck_assert(!merge_contains(&m, i));
  }

  // Add an entry to a merge and then check that the resultant key-mask is
  // correct.
  merge_add(&m, 2);
  ck_assert_int_eq(m.keymask.key, 0x2);
  ck_assert_int_eq(m.keymask.mask, 0xf);
  ck_assert_int_eq(m.route, 0x1);

  // Add another entry to a merge and then check that the resultant key-mask is
  // correct.
  merge_add(&m, 3);
  ck_assert_int_eq(m.keymask.key,  0b0010);
  ck_assert_int_eq(m.keymask.mask, 0b1011);
  ck_assert_int_eq(m.route, 0x1);

  ck_assert(!merge_contains(&m, 0));
  ck_assert(!merge_contains(&m, 1));
  ck_assert(merge_contains(&m, 2));
  ck_assert(merge_contains(&m, 3));

  // Add another entry to the merge
  merge_add(&m, 1);
  ck_assert_int_eq(m.keymask.key,  0b0000);
  ck_assert_int_eq(m.keymask.mask, 0b1000);
  ck_assert_int_eq(m.route, 0x1);

  // Remove an entry from the table and ensure that the keymask is recalculated
  // correctly and that the route is correct.
  merge_remove(&m, 3);  // Merge is now a merge of 0010 and 0001
  ck_assert_int_eq(m.keymask.key,  0b0000);
  ck_assert_int_eq(m.keymask.mask, 0b1100);
  ck_assert_int_eq(m.route, 0x1);

  merge_remove(&m, 2);  // Merge is now a merge of only 0001
  ck_assert_int_eq(m.keymask.key,  0b0001);
  ck_assert_int_eq(m.keymask.mask, 0b1111);
  ck_assert_int_eq(m.route, 0x1);

  merge_remove(&m, 1);  // Merge is now empty
  ck_assert_int_eq(m.keymask.key,  0xffffffff);
  ck_assert_int_eq(m.keymask.mask, 0x00000000);
  ck_assert_int_eq(m.route, 0x0);

  // Delete the merge
  merge_delete(&m);
}
END_TEST


Suite* merge_suite(void)
{
  Suite *s;
  TCase *tests;

  s = suite_create("Merge");
  tests = tcase_create("Core");
  suite_add_tcase(s, tests);

  // Add the tests
  tcase_add_test(tests, test_merge_lifecycle);

  return s;
}
