#include "tests.h"
#include "platform.h"

#include "ordered_covering.h"


Suite* ordered_covering_suite(void)
{
  Suite *s;
  TCase *tests;

  s = suite_create("Ordered Covering");
  tests = tcase_create("Core");
  suite_add_tcase(s, tests);

  // Add the tests
  // tcase_add_test(tests, test_bitset_use);

  return s;
}
