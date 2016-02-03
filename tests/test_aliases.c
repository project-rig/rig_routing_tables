#include "tests.h"
#include "aliases.h"
#include "routing_table.h"


START_TEST(test_aliases_list)
{
  // Create a new aliases list for 5 elements
  alias_list_t *l1 = alias_list_new(5);
  ck_assert_int_eq(l1->n_elements, 0);  // Not added any elements
  ck_assert_int_eq(l1->max_size, 5);  // Can contain 5 elements

  // Add an element
  keymask_t km = {0x0, 0xf};
  alias_list_append(l1, km);
  ck_assert_int_eq(l1->n_elements, 1);  // Now contains 1 element
  ck_assert(alias_list_get(l1, 0).key == km.key);  // Check the element is expected
  ck_assert(alias_list_get(l1, 0).mask == km.mask);  // Check the element is expected

  // This should cause no problems with memory accesses
  ck_assert(alias_list_append(l1, km));
  ck_assert(alias_list_append(l1, km));
  ck_assert(alias_list_append(l1, km));
  ck_assert(alias_list_append(l1, km));
  ck_assert(!alias_list_append(l1, km));  // This will be a problem!

  // Create a new alias list and append it to the existing list
  alias_list_t *l2 = alias_list_new(10);
  alias_list_join(l1, l2);
  ck_assert(l1->next == l2);

  // Create a new alias list and append it to the existing list
  alias_list_t *l3 = alias_list_new(7);
  alias_list_join(l1, l3);
  ck_assert(l1->next == l2);
  ck_assert(l2->next == l3);

  // Tidy up, should delete everything
  alias_list_delete(l1);
}
END_TEST


START_TEST(test_aliases_insert)
{
  // Create a new tree
  aliases_t aliases = aliases_init();

  // Simple insert at root of tree
  keymask_t km0 = {0x0, 0x1};
  alias_list_t* al0 = alias_list_new(3);
  aliases_insert(&aliases, km0, al0);

  // Insert to left
  keymask_t km1 = {0x0, 0x0};
  alias_list_t* al1 = alias_list_new(3);
  aliases_insert(&aliases, km1, al1);

  // Insert to right
  keymask_t km2 = {0x0, 0x2};
  alias_list_t* al2 = alias_list_new(3);
  aliases_insert(&aliases, km2, al2);

  // Insert another
  keymask_t km3 = {0x0, 0x3};
  alias_list_t* al3 = alias_list_new(3);
  aliases_insert(&aliases, km3, al3);

  // Check contains and retrieval for these elements
  ck_assert(aliases_contains(&aliases, km0));
  ck_assert(aliases_find(&aliases, km0) == al0);

  ck_assert(aliases_contains(&aliases, km1));
  ck_assert(aliases_find(&aliases, km1) == al1);

  ck_assert(aliases_contains(&aliases, km2));
  ck_assert(aliases_find(&aliases, km2) == al2);

  ck_assert(aliases_contains(&aliases, km3));
  ck_assert(aliases_find(&aliases, km3) == al3);

  // Check that removing elements works
  aliases_remove(&aliases, km3);
  ck_assert(!aliases_contains(&aliases, km3));
  ck_assert(aliases_find(&aliases, km3) == NULL);

  // Tidy up
  aliases_clear(&aliases);
  alias_list_delete(al3);
}
END_TEST


Suite* aliases_suite(void)
{
  Suite *s;
  TCase *tests;

  s = suite_create("Aliases");
  tests = tcase_create("Core");
  suite_add_tcase(s, tests);

  // Add the tests
  tcase_add_test(tests, test_aliases_list);

  tcase_add_test(tests, test_aliases_insert);

  return s;
}
