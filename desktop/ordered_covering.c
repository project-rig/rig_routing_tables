#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "routing_table.h"
#include "ordered_covering.h"


// Data header in file format
typedef struct _header_t
{
  uint8_t x, y;
  uint16_t length;
} header_t;


// Table entry type in file format
typedef struct _fentry
{
  uint32_t key, mask, source, route;
} fentry_t;


// Routing table entry comparison method
int entry_cmp(const void *a, const void *b)
{
  // Get the generalities
  const unsigned int gen_a = keymask_count_xs(((entry_t *) a)->keymask);
  const unsigned int gen_b = keymask_count_xs(((entry_t *) b)->keymask);

  // Compare
  if (gen_a < gen_b)
  {
    return -1;
  }
  else if (gen_a == gen_b)
  {
    return 0;
  }
  else
  {
    return 1;
  }
}


void minimise(table_t *table, unsigned int target_length)
{
  // Create an empty aliases table
  aliases_t aliases = aliases_init();

  // Minimise
  oc_minimise(table, target_length, &aliases);

  // Tidy up the aliases table
  for (unsigned int i = 0; i < table->size; i++)
  {
    keymask_t km = table->entries[i].keymask;
    if (aliases_contains(&aliases, km))
    {
      alias_list_delete((alias_list_t *) aliases_find(&aliases, km));
      aliases_remove(&aliases, km);
    }
  }
}


int main(int argc, char *argv[])
{
  // Usage:
  // ordered_covering in_file out_file [target_length]
  if (argc < 3)
  {
    fprintf(stderr, "Usage: ordered_covering in_file out_file [target_length]\n");
    return EXIT_FAILURE;
  }

  unsigned int target_length = 0;
  if (argc >= 4)
  {
    target_length = atoi(argv[3]);
  }

  // Open the input and output files
  FILE *in_file = fopen(argv[1], "rb");
  if (in_file == NULL)
  {
    fprintf(stderr, "Could not open input file %s\n", argv[1]);
    return EXIT_FAILURE;
  }

  FILE *out_file = fopen(argv[2], "wb+");
  if (out_file == NULL)
  {
    fprintf(stderr, "Could not open output file %s\n", argv[2]);
    return EXIT_FAILURE;
  }

  // Stream through the input file
  header_t h;
  while (fread(&h, sizeof(header_t), 1, in_file))
  {
    // Print information about the current table
    printf("(%3u, %3u)\t%4u\t", h.x, h.y, h.length);
    fflush(stdout);

    // Read in the routing table
    table_t table;
    table.size = h.length;
    table.entries = malloc(sizeof(entry_t) * h.length);

    for (unsigned int i = 0; i < table.size; i++)
    {
      fentry_t t;
      if (!fread(&t, sizeof(fentry_t), 1, in_file))
      {
        fprintf(stderr, "ERROR: Incomplete routing table\n");
        return EXIT_FAILURE;
      }

      // Copy relevant fields across
      table.entries[i].keymask.key = t.key;
      table.entries[i].keymask.mask = t.mask;
      table.entries[i].route = t.route;
      table.entries[i].source = t.source;
    }

    // Sort the table
    qsort(table.entries, table.size, sizeof(entry_t), entry_cmp);

    // Perform the minimisation
    minimise(&table, target_length);

    printf("%u\n", table.size);

    // Dump the table into the output file
    h.length = table.size;
    fwrite(&h, sizeof(header_t), 1, out_file);

    for (unsigned int i = 0; i < table.size; i++)
    {
      fentry_t t = {
        // Copy relevant fields across
        table.entries[i].keymask.key,
        table.entries[i].keymask.mask,
        table.entries[i].source,
        table.entries[i].route,
      };
      fwrite(&t, sizeof(fentry_t), 1, out_file);
    }
  }

  // Close the input and output files
  fclose(in_file);
  fclose(out_file);

  return EXIT_SUCCESS;
}
