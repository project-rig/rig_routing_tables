/* SpiNNaker Implementation of Ordered Covering Routing Table Minimisation
 *
 * Expects tag 1 in SDRAM to contain:
 *    1 word indicating the length of the routing table, 1 word indicating the
 *    desired length of the routing table, followed by the routing table
 *    consisting of `entry_t` instances.
 *
 * When finished will overwrite the routing table in SDRAM with the new table.
 */
#define SPINNAKER
#include "spin1_api.h"
#include "ordered_covering.h"


// XXX: Will be included as part of next version of SCAMP/SARK.
// Get a pointer to a tagged allocation. If the "app_id" parameter is zero
// uses the core's app_id.
void *sark_tag_ptr (uint tag, uint app_id)
{
  if (app_id == 0)
  {
    app_id = sark_vec->app_id;
  }
  
  return (void *) sv->alloc_tag[(app_id << 8) + tag];
}


void c_main(void)
{
  // Read in the routing table from SDRAM
  uint32_t *sdram_table = (uint32_t *) sark_tag_ptr(1, 0);

  // Prepare the new routing table
  table_t table;
  table.size = sdram_table[0];
  uint32_t target_length = sdram_table[1];

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
  for (unsigned int i = 0; i < table.size; i++)
  {
    keymask_t km = table.entries[i].keymask;
    if (aliases_contains(&aliases, km))
    {
      alias_list_delete((alias_list_t *) aliases_find(&aliases, km));
      aliases_remove(&aliases, km);
    }
  }
}
