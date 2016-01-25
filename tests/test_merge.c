#include "tests.h"
#include "routing_table.h"
#include "merge.h"


START_TEST(test_merge_lifecycle)
{
  // Create a routing table from which we'll build merges
  table_t table;
  table.size = 4;
  entry_t entries[4];
  table.entries = entries;

  // 0000 -> E
  entries[0].keymask.key = 0x0;
  entries[0].keymask.key = 0xf;
  entries[0].route = 0x1;

  // 0001 -> E
  entries[1].keymask.key = 0x1;
  entries[1].keymask.key = 0xf;
  entries[1].route = 0x1;

  // 0010 -> E
  entries[2].keymask.key = 0x2;
  entries[2].keymask.key = 0xf;
  entries[2].route = 0x1;

  // 0110 -> E
  entries[3].keymask.key = 0x6;
  entries[3].keymask.key = 0xf;
  entries[3].route = 0x1;
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
