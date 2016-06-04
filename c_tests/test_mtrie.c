#include "tests.h"
#include "mtrie.h"


START_TEST(test_insert_and_count)
{
  // Test inserting entries into an m-Trie and then counting the number of
  // entries.
  mtrie_t *root = mtrie_new();  // Create the new m-Trie

  // There should be no entries at this point
  ck_assert_int_eq(mtrie_count(root), 0);

  // Add a single entry, check that a single entry is reported
  mtrie_insert(root, 0x00000000, 0xffffffff, 0x0);
  ck_assert_int_eq(mtrie_count(root), 1);

  // Add a different entry, check that two entries are reported
  mtrie_insert(root, 0x00000011, 0xffffffff, 0x0);
  ck_assert_int_eq(mtrie_count(root), 2);

  // Add a different entry, check that three entries are reported
  mtrie_insert(root, 0x00000111, 0xfffff7ff, 0x0);
  ck_assert_int_eq(mtrie_count(root), 3);

  // Clear the tree up
  mtrie_delete(root);
}
END_TEST


START_TEST(test_serialise)
{
  // Test extracting keys and masks from a trie
  mtrie_t *root = mtrie_new();  // Create the new m-Trie

  // Add some entries to the trie
  mtrie_insert(root, 0x00000000, 0xffffffff, 0b001);
  mtrie_insert(root, 0x00000011, 0xffffffff, 0b010);
  mtrie_insert(root, 0x00000111, 0xfffff7ff, 0b100);

  // Extract entries
  mtrie_entry_t entries[3];
  mtrie_get_entries(root, entries);

  // Check the entries are correct
  ck_assert_int_eq(entries[0].keymask.key, 0x0);
  ck_assert_int_eq(entries[0].keymask.mask, 0xffffffff);
  ck_assert_int_eq(entries[0].source, 0b001);

  ck_assert_int_eq(entries[1].keymask.key, 0x11);
  ck_assert_int_eq(entries[1].keymask.mask, 0xffffffff);
  ck_assert_int_eq(entries[1].source, 0b010);

  ck_assert_int_eq(entries[2].keymask.key, 0x111);
  ck_assert_int_eq(entries[2].keymask.mask, 0xfffff7ff);
  ck_assert_int_eq(entries[2].source, 0b100);

  // Clear the tree up
  mtrie_delete(root);
}
END_TEST

typedef struct _stim
{
  keymask_t a, b;
} stim_t;

stim_t leaf_stims[] = {
  {{0x0000000, 0xffffffff}, {0x00000001, 0xffffffff}},  // ...0 and ...1
  {{0x0000000, 0xffffffff}, {0x00000000, 0xfffffffe}},  // ...0 and ...X
  {{0x0000000, 0xfffffffe}, {0x00000000, 0xffffffff}},  // ...X and ...0
  {{0x0000001, 0xffffffff}, {0x00000000, 0xffffffff}},  // ...1 and ...0
  {{0x0000001, 0xffffffff}, {0x00000000, 0xfffffffe}},  // ...1 and ...X
  {{0x0000000, 0xfffffffe}, {0x00000001, 0xffffffff}},  // ...X and ...1
};

START_TEST(test_insert_and_merge_leaves)
{
  stim_t stim = leaf_stims[_i];

  // Test inserting entries into an m-Trie and then counting the number of
  // entries.
  mtrie_t *root = mtrie_new();  // Create the new m-Trie

  // Add two entries which can be merged and ensure that they are merged
  mtrie_insert(root, stim.a.key, stim.a.mask, 0b1000);
  mtrie_insert(root, stim.b.key, stim.b.mask, 0b0100);
  ck_assert_int_eq(mtrie_count(root), 1);

  // Check the entry is correct
  mtrie_entry_t entry;
  mtrie_get_entries(root, &entry);

  ck_assert_int_eq(entry.keymask.key, 0x0);
  ck_assert_int_eq(entry.keymask.mask, 0xfffffffe);
  ck_assert_int_eq(entry.source, 0b1100);

  // Clear the tree up
  mtrie_delete(root);
}
END_TEST

stim_t node_stims[] = {
  {{0x0000000, 0xffffffff}, {0x00000002, 0xffffffff}},  // ...00 and ...10
  {{0x0000000, 0xffffffff}, {0x00000000, 0xfffffffd}},  // ...00 and ...X0
  {{0x0000000, 0xfffffffd}, {0x00000000, 0xffffffff}},  // ...X0 and ...00
  {{0x0000002, 0xffffffff}, {0x00000000, 0xffffffff}},  // ...10 and ...00
  {{0x0000002, 0xffffffff}, {0x00000000, 0xfffffffd}},  // ...10 and ...X0
  {{0x0000000, 0xfffffffd}, {0x00000002, 0xffffffff}},  // ...X0 and ...10
};

START_TEST(test_insert_and_merge_nodes)
{
  stim_t stim = node_stims[_i];

  // Test inserting entries into an m-Trie and then counting the number of
  // entries.
  mtrie_t *root = mtrie_new();  // Create the new m-Trie

  // Add two entries which can be merged and ensure that they are merged
  mtrie_insert(root, stim.a.key, stim.a.mask, 0b01);
  mtrie_insert(root, stim.b.key, stim.b.mask, 0b10);
  ck_assert_int_eq(mtrie_count(root), 1);

  // Check the entry is correct
  mtrie_entry_t entry;
  mtrie_get_entries(root, &entry);

  ck_assert_int_eq(entry.keymask.key, 0x0);
  ck_assert_int_eq(entry.keymask.mask, 0xfffffffd);
  ck_assert_int_eq(entry.source, 0b11);

  // Clear the tree up
  mtrie_delete(root);
}
END_TEST

START_TEST(test_insert_and_merge_partial)
{
  // Test inserting entries into an m-Trie and then counting the number of
  // entries.
  mtrie_t *root = mtrie_new();  // Create the new m-Trie

  // Add three entries, of which only the latter two can be merged
  mtrie_insert(root, 0b0101, 0b1111, 0b001);
  mtrie_insert(root, 0b0000, 0b1111, 0b010);
  mtrie_insert(root, 0b1000, 0b1111, 0b100);
  ck_assert_int_eq(mtrie_count(root), 2);

  // Check the entry is correct
  mtrie_entry_t entries[2];
  mtrie_get_entries(root, entries);

  ck_assert_int_eq(entries[0].keymask.key, 0b0101);
  ck_assert_int_eq(entries[0].keymask.mask, 0b1111);
  ck_assert_int_eq(entries[0].source, 0b001);

  ck_assert_int_eq(entries[1].keymask.key, 0b0000);
  ck_assert_int_eq(entries[1].keymask.mask, 0b0111);
  ck_assert_int_eq(entries[1].source, 0b110);

  // Clear the tree up
  mtrie_delete(root);
}
END_TEST

START_TEST(test_mtrie_minimise)
{
  // Test minimisation of a routing table using m-Trie
  // Test that the given table is minimised correctly:
  //
  //   S  -> 0000 -> N NE
  //   SW -> 0001 -> N NE
  //   N  -> 0010 -> E
  //   S  -> 0011 -> E
  //   N  -> 010X -> SW
  //   N  -> 0110 -> SW
  //   N  -> 0111 -> SW
  //   W  -> 1010 -> N
  //   W  -> 1001 -> N
  //
  // The result (worked out by hand) should be:
  //
  //   SW S -> 000X -> N NE
  //   N S  -> 001X -> E
  //   N    -> 01XX -> SW
  //   W    -> 1001 -> N
  //   W    -> 1010 -> N
  //
  entry_t entries[] = {
    {{0b0000, 0xf}, 0b000110, 0b100000},
    {{0b0001, 0xf}, 0b000110, 0b010000},
    {{0b0010, 0xf}, 0b000001, 0b000100},
    {{0b0011, 0xf}, 0b000001, 0b100000},
    {{0b0100, 0xe}, 0b010000, 0b000100},
    {{0b0110, 0xf}, 0b010000, 0b000100},
    {{0b0111, 0xf}, 0b010000, 0b000100},
    {{0b1010, 0xf}, 0b000100, 0b001000},
    {{0b1001, 0xf}, 0b000100, 0b001000},
  };
  table_t table = {sizeof(entries) / sizeof(entry_t), entries};

  // Minimise the table
  mtrie_minimise(&table);

  // Check the returned table
  ck_assert_int_eq(table.size, 5);

  // Check each entry
  ck_assert_int_eq(table.entries[0].keymask.key, 0b0000);
  ck_assert_int_eq(table.entries[0].keymask.mask, 0b1110);
  ck_assert_int_eq(table.entries[0].route, 0b000110);
  ck_assert_int_eq(table.entries[0].source, 0b110000);

  ck_assert_int_eq(table.entries[1].keymask.key, 0b0010);
  ck_assert_int_eq(table.entries[1].keymask.mask, 0b1110);
  ck_assert_int_eq(table.entries[1].route, 0b000001);
  ck_assert_int_eq(table.entries[1].source, 0b100100);

  ck_assert_int_eq(table.entries[2].keymask.key, 0b0100);
  ck_assert_int_eq(table.entries[2].keymask.mask, 0b1100);
  ck_assert_int_eq(table.entries[2].route, 0b010000);
  ck_assert_int_eq(table.entries[2].source, 0b000100);

  ck_assert_int_eq(table.entries[3].keymask.key, 0b1001);
  ck_assert_int_eq(table.entries[3].keymask.mask, 0b1111);
  ck_assert_int_eq(table.entries[3].route, 0b000100);
  ck_assert_int_eq(table.entries[3].source, 0b001000);

  ck_assert_int_eq(table.entries[4].keymask.key, 0b1010);
  ck_assert_int_eq(table.entries[4].keymask.mask, 0b1111);
  ck_assert_int_eq(table.entries[4].route, 0b000100);
  ck_assert_int_eq(table.entries[4].source, 0b001000);
}
END_TEST


Suite* mtrie_suite(void)
{
  Suite *s;
  TCase *tests;

  s = suite_create("m-Trie");
  tests = tcase_create("Core");
  suite_add_tcase(s, tests);

  // Add the tests
  tcase_add_test(tests, test_insert_and_count);
  tcase_add_test(tests, test_serialise);

  tcase_add_loop_test(tests, test_insert_and_merge_leaves,
                      0, sizeof(leaf_stims) / sizeof(stim_t));
  tcase_add_loop_test(tests, test_insert_and_merge_nodes,
                      0, sizeof(node_stims) / sizeof(stim_t));

  tcase_add_test(tests, test_insert_and_merge_partial);

  tcase_add_test(tests, test_mtrie_minimise);

  return s;
}
