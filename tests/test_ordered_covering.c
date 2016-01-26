#include "tests.h"
#include "platform.h"
#include "routing_table.h"
#include "aliases.h"

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


START_TEST(test_merge_apply_at_beginning_of_table)
{
  // Merge the first two entries:
  //
  //     0000 -> N
  //     0001 -> N
  //     XXXX -> S
  //
  // The result should be:
  //
  //     000X -> N {0000, 0001}
  //     XXXX -> S
  entry_t entries[3];
  table_t table = {3, entries};

  entries[0].keymask.key = 0x0;
  entries[0].keymask.mask = 0xf;
  entries[0].route = 0b100;

  entries[1].keymask.key = 0x1;
  entries[1].keymask.mask = 0xf;
  entries[1].route = 0b100;

  entries[2].keymask.key = 0x0;
  entries[2].keymask.mask = 0x0;
  entries[2].route = 0b100000;

  // Create the merge
  merge_t m;
  merge_init(&m, &table);
  merge_add(&m, 0);
  merge_add(&m, 1);

  // Create an empty aliases dict
  aliases_t aliases = aliases_init();

  // Apply the merge
  oc_merge_apply(&m, &aliases);

  // Check that the table was modified as expected
  ck_assert_int_eq(table.size, 2);  // Should now contain two entries

  // Check the first entry -- this should be the new entry
  ck_assert_int_eq(table.entries[0].keymask.key, 0x0);
  ck_assert_int_eq(table.entries[0].keymask.mask, 0xe);
  ck_assert_int_eq(table.entries[0].route, 0b100);

  // Check that the last table in the entry was as it was originally
  ck_assert(table.entries[1].keymask.key == 0x0);
  ck_assert(table.entries[1].keymask.mask == 0x0);
  ck_assert(table.entries[1].route == 0b100000);

  // Check that the aliases map contains an entry for the new routing table
  // entry
  ck_assert(aliases_contains(&aliases, table.entries[0].keymask));

  // Check that the aliases map entry contains the two original keymasks
  alias_list_t *l = (alias_list_t *) aliases_find(
    &aliases, table.entries[0].keymask);

  ck_assert_int_eq(l->n_elements, 2);
  ck_assert(alias_list_get(l, 0).key == 0x0);
  ck_assert(alias_list_get(l, 0).mask == 0xf);

  ck_assert(alias_list_get(l, 1).key == 0x1);
  ck_assert(alias_list_get(l, 1).mask == 0xf);

  // Tidy up
  merge_delete(&m);
  alias_list_delete(l);
  aliases_clear(&aliases);
}
END_TEST


START_TEST(test_merge_apply_at_end_of_table)
{
  // Merge the first two entries:
  //
  //     0000 -> N
  //     001X -> N {0010, 0011}
  //     1111 -> S
  //
  // The result should be:
  //
  //     1111 -> S
  //     00XX -> N {0000, 0010, 0011}
  entry_t entries[3];
  table_t table = {3, entries};

  entries[0].keymask.key = 0x0;
  entries[0].keymask.mask = 0xf;
  entries[0].route = 0b100;

  entries[1].keymask.key = 0x2;
  entries[1].keymask.mask = 0xe;
  entries[1].route = 0b100;

  entries[2].keymask.key = 0xf;
  entries[2].keymask.mask = 0xf;
  entries[2].route = 0b100000;

  // Create the merge
  merge_t m;
  merge_init(&m, &table);
  merge_add(&m, 0);
  merge_add(&m, 1);

  // Create an aliases dict
  aliases_t aliases = aliases_init();

  // Add a new aliases entry for 001X
  alias_list_t *l1 = alias_list_new(2);
  keymask_t km1 = {0x2, 0xf}, km2 = {0x3, 0xf};
  alias_list_append(l1, km1);
  alias_list_append(l1, km2);
  aliases_insert(&aliases, entries[1].keymask, (void *) l1);

  // Apply the merge
  oc_merge_apply(&m, &aliases);

  // Check that the table was modified as expected
  ck_assert_int_eq(table.size, 2);  // Should now contain two entries

  // Check the first entry -- this should be the same as the last old entry
  ck_assert_int_eq(table.entries[0].keymask.key, 0xf);
  ck_assert_int_eq(table.entries[0].keymask.mask, 0xf);
  ck_assert_int_eq(table.entries[0].route, 0b100000);

  // Check that the last entry is the result of the merge
  ck_assert(table.entries[1].keymask.key == 0x0);
  ck_assert(table.entries[1].keymask.mask == 0xc);
  ck_assert(table.entries[1].route == 0b100);

  // Check that the aliases map contains an entry for the new routing table
  // entry
  ck_assert(aliases_contains(&aliases, table.entries[1].keymask));

  // Check that the aliases map entry contains the two original keymasks
  alias_list_t *l = (alias_list_t *) aliases_find(
    &aliases, table.entries[1].keymask);

  ck_assert_int_eq(l->n_elements, 1);
  ck_assert(alias_list_get(l, 0).key == 0x0);
  ck_assert(alias_list_get(l, 0).mask == 0xf);

  ck_assert(l->next != NULL);
  ck_assert_int_eq(l->next->n_elements, 2);

  ck_assert(alias_list_get(l->next, 0).key == 0x2);
  ck_assert(alias_list_get(l->next, 0).mask == 0xf);

  ck_assert(alias_list_get(l->next, 1).key == 0x3);
  ck_assert(alias_list_get(l->next, 1).mask == 0xf);

  // Tidy up
  merge_delete(&m);
  alias_list_delete(l);
  aliases_clear(&aliases);
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

  tcase_add_test(tests, test_merge_apply_at_beginning_of_table);
  tcase_add_test(tests, test_merge_apply_at_end_of_table);

  return s;
}
