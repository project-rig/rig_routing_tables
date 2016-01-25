#include "bitset.h"
#include "merge.h"
#include "routing_table.h"

#ifndef __ORDERED_COVERING_H__


typedef void aliases_t;


// Get the index where the routing table entry resulting from a merge should be
// inserted.
static inline unsigned int oc_get_insertion_point(merge_t *m)
{
  // TODO
}


// Remove from a merge any entries which would be covered by being existing
// entries if they were included in the given merge.
static inline void oc_upcheck(merge_t *m, int min_goodness)
{
  // Get the point where the merge will be inserted into the table.
  unsigned int insertion_index = oc_get_insertion_point(m);

  // For every entry in the merge check that the entry would not be covered by
  // any existing entries if it were to be merged.
  for (unsigned int _i = m->table->size, i = m->table->size - 1; _i > 0;
       _i--, i--)
  {
    if (!merge_contains(m, i))
    {
      // If this entry is not contained within the merge skip it
      continue;
    }

    // Get the keymask for this entry
    keymask_t km = m->table->entries[i].keymask;

    // Otherwise look through the table from the insertion point to the current
    // entry position to ensure that nothing covers the merge.
    for (unsigned int j = i + 1; j < insertion_index; j++)
    {
      keymask_t other_km = m->table->entries[j].keymask;

      // If the key masks intersect then remove this entry from the merge and
      // recalculate the insertion index.
      if (keymask_intersect(km, other_km))
      {
        merge_remove(m, i);  // Remove from the merge
        insertion_index = oc_get_insertion_point(m);
      }
    }
  }
}


// Remove entries from a merge such that the merge would not cover existing
// entries positioned below the merge.
static inline void oc_downcheck(
  merge_t *merge, int min_goodness, aliases_t *aliases
)
{
  // TODO
}


// Get the best merge which can be applied to a routing table
static inline merge_t oc_get_best_merge(table_t* table, aliases_t *aliases)
{
  // Keep track of which entries have been considered as part of merges.
  bitset_t considered;
  bitset_init(&considered, table->size);

  // Keep track of the current best merge and also provide a working merge
  merge_t best, working;
  merge_init(&best, table);
  merge_init(&working, table);

  // For every entry in the table see with which other entries it could be
  // merged.
  for (unsigned int i = 0; i < table->size; i++)
  {
    // If this entry has already been considered then skip to the next
    if (bitset_contains(&considered, i))
    {
      continue;
    }

    // Otherwise try to build a merge
    merge_clear(&working);       // Clear the working merge
    merge_add(&working, i);      // Add to the merge
    bitset_add(&considered, i);  // Mark this entry as considered

    entry_t entry = table->entries[i];  // Get the entry

    // Try to merge with other entries
    for (unsigned int j = i+1; j < table->size; j++)
    {
      entry_t other = table->entries[j];  // Get the other entry
      if (entry.route == other.route)
      {
        // If the routes are the same then the entries may be merged
        merge_add(&working, j);      // Add to the merge
        bitset_add(&considered, j);  // Mark the other entry as considered
      }
    }

    // If the working merge is better than the current best merge then continue
    // to refine it until it is valid.
    if (best.goodness < working.goodness)
    {
      // Perform the first downcheck
      oc_downcheck(&working, best.goodness, aliases);

      // If the working merge is still better than the current best merge then
      // continue to refine it.
      if (best.goodness < working.goodness)
      {
        // Perform the upcheck, seeing if this actually makes a change to the
        // size of the merge.
        unsigned int before = working.entries.count;
        oc_upcheck(&working, best.goodness);
        bool changed = (before != working.entries.count);

        // If the merge is still better than the current best merge AND the
        // number of entries was changed by the upcheck we need to perform
        // another downcheck.
        if (best.goodness < working.goodness && changed)
        {
          oc_downcheck(&working, best.goodness, aliases);
        }

        // If the merge is still better than the current best merge we swap the
        // current and best merges to record the new best merge.
        if (best.goodness < working.goodness)
        {
          merge_t other = working;
          working = best;
          best = other;
        }
      }
    }
  }

  // Tidy up
  merge_delete(&working);
  bitset_delete(&considered);

  // Return the best merge
  return best;
}


// Apply a merge to the table against which it is defined
static inline void oc_merge_apply(merge_t *m, aliases_t *aliases)
{
  // TODO
}


// Apply the ordered covering algorithm to a routing table
// Minimise the table until either the table is shorter than the target length
// or no more merges are possible.
static inline void oc_minimise(
  table_t *table,
  unsigned int target_length,
  aliases_t *aliases
)
{
  while (table->size > target_length)
  {
    // Get the best possible merge, if this merge is no good then break out of
    // the loop.
    merge_t merge = oc_get_best_merge(table, aliases);
    unsigned int count = merge.entries.count;

    if (count > 1)
    {
      // Apply the merge to the table if it would result in merging actually
      // occurring.
      oc_merge_apply(&merge, aliases);
    }

    // Free any memory used by the merge
    merge_delete(&merge);

    // Break out of the loop if no merge could be performed (indicating that no
    // more minimisation is possible).
    if (count < 2)
    {
      break;
    }
  }
}


#define __ORDERED_COVERING_H__
#endif  // __ORDERED_COVERING_H__
