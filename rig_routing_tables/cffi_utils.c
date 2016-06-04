#include <stdlib.h>
#include <stddef.h>
#include "cffi_utils.h"

table_t *new_table(unsigned int size)
{
  // Malloc space for the table and associated entries
  table_t *table = malloc(sizeof(table_t));
  table->size = 0;
  table->entries = NULL;

  if (table != NULL)
  {
    table->entries = malloc(sizeof(entry_t) * size);

    if (table->entries != NULL)
    {
      table->size = size;
      return table;
    }
  }

  return NULL;
}

void free_table(table_t *table)
{
  // Free the entries and then the table
  free(table->entries);
  table->entries = NULL;
  table->size = 0;
  free(table);
}
