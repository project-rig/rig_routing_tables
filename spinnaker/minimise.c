#include <stdbool.h>
#include <stdlib.h>
#include "spin1_api.h"
#include "ordered_covering.h"
#include "remove_default_routes.h"

/*****************************************************************************/
/* SpiNNaker routing table minimisation.
 *
 * Minimise a routing table loaded into SDRAM and load the minimised table into
 * the router using the specified application ID.
 *
 * There are two failure modes which will cause the application to raise a
 * run-time exception.
 *
 *  - RTE_MALLOC: Indicates that it might be possible to minimise the table
 *                but that it was not possible to do so on SpiNNaker due to
 *                memory constraints. Consider retrying the minimisation on
 *                host.
 *  - RTE_ABORT:  Indicates that it was not possible to minimise the routing
 *                table to fit within the space available in the router.
 *                Consider trying a different minimisation procedure than those
 *                contained within this executable.
 *
 * The memory address with tag "1" is expected contain the following struct
 * (entry_t is defined in `routing_table.h` but is described below).
 */

typedef struct
{
  // Application ID to use to load the routing table. This can be left as `0'
  // to load routing entries with the same application ID that was used to load
  // this application.
  uint32_t app_id;

  uint32_t flags;       // Currently there are no flags
  uint32_t table_size;  // Initial size of the routing table.
  entry_t entries[];    // Routing table entries
} header_t;

/* entry_t is defined as:
 *
 *     typedef struct
 *     {
 *       uint32_t key;
 *       uint32_t mask;
 *       uint32_t route;   // Routing direction
 *       uint32_t source;  // Source of packets arriving at this entry
 *     } entry_t;
 *
 * The `source` field is used to determine if the entry could be replaced by
 * default routing, it can be left blank if removing default entries is not to
 * be used. Otherwise indicate which links will be used by packets expected to
 * match the specified entry.
 * 
 * NOTE: The routing table provided to this application MUST include all of the
 * entries which are expected to arrive at this router (i.e., entries which
 * could be replaced by default routing MUST be included in the table provided
 * to this application).
 *
 * NOTE: The block of memory containing the header and initial routing table
 * will be freed on exit by this application.
 */
/*****************************************************************************/

/*****************************************************************************/
/* Read a new copy of the routing table from SDRAM.                          */
void read_table(table_t *table, header_t *header)
{
  // Copy the size of the table
  table->size = header->table_size;

  // Allocate space for the routing table entries
  table->entries = MALLOC(table->size * sizeof(entry_t));

  // Copy in the routing table entries
  spin1_memcpy((void *) table->entries,
               (void *) header->entries,
               sizeof(entry_t) * table->size);
}
/*****************************************************************************/

/*****************************************************************************/
/* Load a routing table to the router.                                       */
bool load_routing_table(table_t *table, uint32_t app_id)
{
  // Try to allocate sufficient room for the routing table.
  uint32_t entry_id = rtr_alloc_id(table->size, app_id);

  if (entry_id != 0)
  {
    // Load entries into the table (provided the allocation succeeded). Note
    // that although the allocation included the specified application ID we
    // also need to include it as the most significant byte in the route (see
    // `sark_hw.c`).
    for (uint32_t i = 0; i < table->size; i++)
    {
      entry_t entry = table->entries[i];
      uint32_t route = entry.route | (app_id << 24);
      rtr_mc_set(entry_id + i, entry.keymask.key, entry.keymask.mask, route);
    }
  }

  // Indicate if we were able to allocate routing table entries.
  return (entry_id != 0);
};
/*****************************************************************************/

/*****************************************************************************/
/* Method used to sort routing table entries.                                */
int compare_rte(const void *va, const void *vb)
{
  // Grab the keys and masks
  keymask_t a = ((entry_t *) va)->keymask;
  keymask_t b = ((entry_t *) vb)->keymask;

  // Perform the comparison
  return ((int) keymask_count_xs(a)) - ((int) keymask_count_xs(b));
}
/*****************************************************************************/

/*****************************************************************************/
void c_main(void)
{
  // Prepare to minimise the routing tables
  header_t *header = (header_t *) sark_tag_ptr(1, 0);

  // Load the routing table
  table_t table;
  read_table(&table, header);

  // Store intermediate sizes for later reporting (if we fail to minimise)
  uint32_t size_original, size_rde, size_oc;
  size_original = table.size;

  // Try to load the table
  if (!load_routing_table(&table, header->app_id))
  {
    // Otherwise remove default routes.
    remove_default_routes_minimise(&table);
    size_rde = table.size;

    // Try to load the table
    if (!load_routing_table(&table, header->app_id))
    {
      // Try to use Ordered Covering the minimise the table. This requires
      // that the table be reloaded from memory and that it be sorted in
      // ascending order of generality.
      FREE(table.entries);
      read_table(&table, header);
      qsort(table.entries, table.size, sizeof(entry_t), compare_rte);

      // Get the target length of the routing table
      uint32_t target_length = rtr_alloc_max();

      // Perform the minimisation
      aliases_t aliases = aliases_init();
      oc_minimise(&table, target_length, &aliases);
      size_oc = table.size;

      // Clean up the memory used by the aliases table
      aliases_clear(&aliases);

      // Try to load the routing table
      if (!load_routing_table(&table, header->app_id))
      {
        // Otherwise give up and exit with a runtime error
        io_printf(
          IO_BUF, "Failed to minimise routing table to fit %u entries.\n"
                  "(Original table: %u\n",
                  " after removing default entries: %u\n,"
                  " after Ordered Covering: %u).\n",
          size_original, size_rde, size_oc
        );

        // Free the block of SDRAM used to load the routing table.
        sark_xfree(sv->sdram_heap, (void *) header, ALLOC_LOCK);

        // Raise the runtime error
        rt_error(RTE_ABORT);
      }
    }
  }

  // Free the memory used by the routing table.
  FREE(table.entries);

  // Free the block of SDRAM used to load the routing table.
  sark_xfree(sv->sdram_heap, (void *) header, ALLOC_LOCK);
}
/*****************************************************************************/
