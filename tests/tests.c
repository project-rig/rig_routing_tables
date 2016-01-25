#include "tests.h"


int main(void)
{
  int n_failed;

  Suite *s_bitset = bitset_suite();
  SRunner *sr = srunner_create(s_bitset);

  Suite *s_routing_table = routing_table_suite();
  srunner_add_suite(sr, s_routing_table);

  Suite *s_merge = merge_suite();
  srunner_add_suite(sr, s_merge);

  // Run the tests
  srunner_run_all(sr, CK_NORMAL);

  n_failed = srunner_ntests_failed(sr);
  srunner_free(sr);

  return (n_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
