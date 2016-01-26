#include "tests.h"
#include "platform.h"
#include "routing_table.h"

#include "ordered_covering.h"


START_TEST(test_get_insertion_point_at_beginning_of_table)
{
  // Create a routing table with only generality 31 entries
  entry_t entries[4];
  entries[0].keymask.key = 0b00;
  entries[0].keymask.mask = 0b01;
  entries[1].keymask.key = 0b01;
  entries[1].keymask.mask = 0b01;
  entries[2].keymask.key = 0b00;
  entries[2].keymask.mask = 0b10;
  entries[3].keymask.key = 0b10;
  entries[3].keymask.mask = 0b10;
  table_t table = {4, entries};

  // A generality 30 entry should be inserted at the beginning of the table.
  ck_assert_int_eq(oc_get_insertion_point(&table, 30), 0);
}
END_TEST


START_TEST(test_get_insertion_point_after_same_generality)
{
  // Create a routing table with only generality 30 and 31 entries
  entry_t entries[5];
  entries[0].keymask.key = 0b00;
  entries[0].keymask.mask = 0b11;
  entries[1].keymask.key = 0b00;
  entries[1].keymask.mask = 0b01;
  entries[2].keymask.key = 0b01;
  entries[2].keymask.mask = 0b01;
  entries[3].keymask.key = 0b00;
  entries[3].keymask.mask = 0b10;
  entries[4].keymask.key = 0b10;
  entries[4].keymask.mask = 0b10;
  table_t table = {5, entries};

  // A generality 30 entry should be inserted after the 1 generality 30 entry
  ck_assert_int_eq(oc_get_insertion_point(&table, 30), 1);
}
END_TEST


START_TEST(test_get_insertion_point_at_end_of_table)
{
  // Create a routing table with only generality 30 and 31 entries
  entry_t entries[5];
  entries[1].keymask.key = 0b00;
  entries[1].keymask.mask = 0b11;
  entries[1].keymask.key = 0b00;
  entries[1].keymask.mask = 0b01;
  entries[2].keymask.key = 0b01;
  entries[2].keymask.mask = 0b01;
  entries[3].keymask.key = 0b00;
  entries[3].keymask.mask = 0b10;
  entries[4].keymask.key = 0b10;
  entries[4].keymask.mask = 0b10;
  table_t table = {5, entries};

  // Generality 31 and 32 entries should be inserted at the end of the table
  ck_assert_int_eq(oc_get_insertion_point(&table, 31), 5);
  ck_assert_int_eq(oc_get_insertion_point(&table, 32), 5);
}
END_TEST


START_TEST(test_oc_upcheck)
{
  // Initial routing table
  entry_t entries[6];
  table_t table = {6, entries};
  entries[0].keymask.key = 0b1101;
  entries[1].keymask.key = 0b1011;
  entries[2].keymask.key = 0b1001;
  entries[3].keymask.key = 0b0001;
  entries[4].keymask.key = 0b0000;
  for (unsigned int i = 0; i < 5; i++)
  {
    entries[i].keymask.mask = 0xf;
    entries[i].route = 0x8;
  }

  entries[5].keymask.key = entries[5].keymask.mask = 0b1001;
  entries[5].route = 0x4;

  // The first 5 entries cannot be merged as this would cause the first 3
  // entries to become covered; `oc_upcheck` should remove entries from the
  // merge set.
  merge_t m;
  merge_init(&m, &table);
  merge_add(&m, 0);
  merge_add(&m, 1);
  merge_add(&m, 2);
  merge_add(&m, 3);
  merge_add(&m, 4);

  oc_upcheck(&m, 0);

  // Check that entries were removed from the merge
  ck_assert(!merge_contains(&m, 0));
  ck_assert(!merge_contains(&m, 1));
  ck_assert(!merge_contains(&m, 2));
  ck_assert(merge_contains(&m, 3));
  ck_assert(merge_contains(&m, 4));
  ck_assert(!merge_contains(&m, 5));  // Never part of merge

  // Reset the merge and assert that if the merge is no better than the given
  // goodness an empty merge is returned.
  int goodness = merge_goodness(&m);

  merge_add(&m, 0);
  merge_add(&m, 1);
  merge_add(&m, 2);

  oc_upcheck(&m, goodness);

  // Merge should be empty
  ck_assert(!merge_contains(&m, 0));
  ck_assert(!merge_contains(&m, 1));
  ck_assert(!merge_contains(&m, 2));
  ck_assert(!merge_contains(&m, 3));
  ck_assert(!merge_contains(&m, 4));
  ck_assert(!merge_contains(&m, 5));  // Never part of merge

  // Tidy up
  merge_delete(&m);
}
END_TEST


Suite* ordered_covering_suite(void)
{
  Suite *s;
  TCase *tests;

  s = suite_create("Ordered Covering");
  tests = tcase_create("Core");
  suite_add_tcase(s, tests);

  // Add the tests
  tcase_add_test(tests, test_get_insertion_point_at_beginning_of_table);
  tcase_add_test(tests, test_get_insertion_point_after_same_generality);
  tcase_add_test(tests, test_get_insertion_point_at_end_of_table);

  tcase_add_test(tests, test_oc_upcheck);

  return s;
}
