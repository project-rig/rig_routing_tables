/* SpiNNaker Implementation of Ordered Covering Routing Table Minimisation
 *
 * Expects tag 1 in SDRAM to contain:
 *    1 word indicating the length of the routing table, 1 word indicating the
 *    desired length of the routing table, followed by the routing table
 *    consisting of `entry_t` instances.
 *
 * When finished will overwrite the routing table in SDRAM with the new table.
 */
#include "spin1_api.h"
#include "ordered_covering.h"

void c_main(void)
{
  // Read in the routing table from SDRAM
  uint32_t *sdram_table = (uint32_t *) sark_tag_ptr(1, 0);

  // Prepare the new routing table
  table_t table;
  table.size = sdram_table[0];
  uint32_t target_length = sdram_table[1];

  // Prepare the memory profiling if desired
#ifdef PROFILED
  profile_init();
#endif

  // Copy in the original table
  table.entries = MALLOC(table.size * sizeof(entry_t));
  spin1_memcpy((void *) table.entries,
               (void *) &sdram_table[2],
               sizeof(entry_t) * table.size);

  // Perform the minimisation
  aliases_t aliases = aliases_init();
  oc_minimise(&table, target_length, &aliases);

  // Copy the new table back into SDRAM
  sdram_table[0] = table.size;
  spin1_memcpy((void *) &sdram_table[2],
               (void *) table.entries,
               sizeof(entry_t) * table.size);

  // Clean up the memory used by the aliases table
  aliases_clear(&aliases);
}
