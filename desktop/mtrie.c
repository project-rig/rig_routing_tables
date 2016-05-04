#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "routing_table.h"
#include "mtrie.h"


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


int main(int argc, char *argv[])
{
  // Usage:
  // mtrie in_file out_file
  if (argc < 3)
  {
    fprintf(stderr, "Usage: mtrie in_file out_file");
    return EXIT_FAILURE;
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
    }

    // Perform the minimisation
    mtrie_minimise(&table);

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
        0x0,  // No source
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
