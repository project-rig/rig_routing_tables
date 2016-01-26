#include "tests.h"
#include "aliases.h"
#include "routing_table.h"


START_TEST(test_aliases_lifecycle)
{
  // Create a starting node
  aliases_t aliases = aliases_init();

  // Check that some keymasks aren't in the aliases map
  keymask_t km = {0x00000000, 0xffffffff};
  ck_assert(!aliases_contains(&aliases, km));

  km.key = km.mask;
  ck_assert(!aliases_contains(&aliases, km));

  km.key = km.mask = 0x00000000;
  ck_assert(!aliases_contains(&aliases, km));
  ck_assert(aliases_find(&aliases, km) == (void *) NULL);

  // Add a keymask to the aliases map with a pointer to an int
  km.key  = 0b1010;
  km.mask = 0b1110;
  int x = 11;
  aliases_insert(&aliases, km, (void *) &x);

  // Check that the lookup works as expected
  ck_assert(aliases_contains(&aliases, km));
  ck_assert(aliases_find(&aliases, km) == (void *) &x);

  // Add another, closely related element to the aliases map
  keymask_t km2 = {0b1000, 0b1111};
  int y = 12;

  aliases_insert(&aliases, km2, (void *) &y);

  // Check that the lookup still works
  ck_assert(aliases_contains(&aliases, km));
  ck_assert(aliases_find(&aliases, km) == (void *) &x);

  ck_assert(aliases_contains(&aliases, km2));
  ck_assert(aliases_find(&aliases, km2) == (void *) &y);

  keymask_t km3 = {0x00000000, 0x00000001};
  ck_assert(!aliases_contains(&aliases, km3));
  ck_assert(aliases_find(&aliases, km3) == (void *) NULL);

  // Remove an element from the aliases tree, check that the other may still be
  // accessed.
  aliases_remove(&aliases, km);

  ck_assert(!aliases_contains(&aliases, km));
  ck_assert(aliases_find(&aliases, km) == (void *) NULL);

  ck_assert(aliases_contains(&aliases, km2));
  ck_assert(aliases_find(&aliases, km2) == (void *) &y);

  // Clear the aliases entirely
  aliases_clear(&aliases);

  ck_assert(!aliases_contains(&aliases, km));
  ck_assert(aliases_find(&aliases, km) == (void *) NULL);

  ck_assert(!aliases_contains(&aliases, km2));
  ck_assert(aliases_find(&aliases, km2) == (void *) NULL);
}
END_TEST


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


Suite* aliases_suite(void)
{
  Suite *s;
  TCase *tests;

  s = suite_create("Aliases");
  tests = tcase_create("Core");
  suite_add_tcase(s, tests);

  // Add the tests
  tcase_add_test(tests, test_aliases_lifecycle);
  tcase_add_test(tests, test_aliases_list);

  return s;
}
