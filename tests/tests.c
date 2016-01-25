#include "tests.h"


int main(void)
{
  int n_failed;
  Suite *s = bitset_suite();
  SRunner *sr = srunner_create(s);

  // Run the tests
  srunner_run_all(sr, CK_NORMAL);

  n_failed = srunner_ntests_failed(sr);
  srunner_free(sr);

  return (n_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
