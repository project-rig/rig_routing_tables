#include "tests.h"
#include "routing_table.h"
#include "remove_default_routes.h"

// Remove default routes from the table:
//
// S   -> 0000 -> N    # Remove
// N   -> 0001 -> N    # Keep
// ?   -> 0010 -> N    # Keep
// N S -> 0011 -> N S  # Keep
// 1   -> 0100 -> 1    # Keep
entry_t orthogonal_entries[] = {
  {{0x0, 0xf}, 0b000100, 0b100000},
  {{0x1, 0xf}, 0b000100, 0b000100},
  {{0x2, 0xf}, 0b000100, (1 << 24)},
  {{0x3, 0xf}, 0b100100, 0b100100},
  {{0x4, 0xf}, 0b1000000, 0b1000000},
};
entry_t orthogonal_entries_results[] = {
  {{0x1, 0xf}, 0b000100, 0b000100},
  {{0x2, 0xf}, 0b000100, (1 << 24)},
  {{0x3, 0xf}, 0b100100, 0b100100},
  {{0x4, 0xf}, 0b1000000, 0b1000000},
};

// Remove default routes from the table:
//
// S   -> 0000 -> N    # Remove
// N   -> 0001 -> N    # Keep
// ?   -> 0010 -> N    # Keep
// N S -> 0011 -> N S  # Keep
// W   -> 0100 -> E    # Remove
entry_t orthogonal_entries2[] = {
  {{0x0, 0xf}, 0b000100, 0b100000},
  {{0x1, 0xf}, 0b000100, 0b000100},
  {{0x2, 0xf}, 0b000100, (1 << 24)},
  {{0x3, 0xf}, 0b100100, 0b100100},
  {{0x4, 0xf}, 0b000001, 0b001000},
};
entry_t orthogonal_entries2_results[] = {
  {{0x1, 0xf}, 0b000100, 0b000100},
  {{0x2, 0xf}, 0b000100, (1 << 24)},
  {{0x3, 0xf}, 0b100100, 0b100100},
};

// Remove default routes from the table:
//
// S -> 1000 -> N    # Remove
// S -> 0000 -> N    # Keep
// ? -> 0XXX -> N    # Keep
entry_t nonorthogonal_entries[] = {
  {{0x8, 0xf}, 0b000100, 0b100000},
  {{0x0, 0xf}, 0b000100, 0b100000},
  {{0x0, 0x8}, 0b000100, (1 << 24)},
};
entry_t nonorthogonal_entries_results[] = {
  {{0x0, 0xf}, 0b000100, 0b100000},
  {{0x0, 0x8}, 0b000100, (1 << 24)},
};

// Set up the fixtures
table_t test_tables[] = {
  {5, orthogonal_entries},
  {5, orthogonal_entries2},
  {3, nonorthogonal_entries},
};

table_t expected_tables[] = {
  {4, orthogonal_entries_results},
  {3, orthogonal_entries2_results},
  {2, nonorthogonal_entries_results},
};


START_TEST(test_removal)
{
  // Grab the tables
  table_t table = test_tables[_i];
  table_t expected = expected_tables[_i];

  // Minimise
  remove_default_routes_minimise(&table);

  // Check the result
  ck_assert_int_eq(table.size, expected.size);

  // Check the tables are equivalent
  for (unsigned int i = 0; i < table.size; i++)
  {
    // Get the entries
    entry_t e = table.entries[i];
    entry_t f = expected.entries[i];

    ck_assert_int_eq(e.keymask.key, f.keymask.key);
    ck_assert_int_eq(e.keymask.mask, f.keymask.mask);
    ck_assert_int_eq(e.route, f.route);
    ck_assert_int_eq(e.source, f.source);
  }
}
END_TEST


Suite* remove_default_suite(void)
{
  Suite *s;
  TCase *tests;

  s = suite_create("Remove Default Routes");
  tests = tcase_create("Core");
  suite_add_tcase(s, tests);

  // Add the tests
  tcase_add_loop_test(tests, test_removal,
                      0, sizeof(test_tables) / sizeof(table_t));

  return s;
}
