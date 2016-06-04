#include "routing_table.h"

// Table management methods (mostly here for the CFFI wrapper)
table_t *new_table(unsigned int size);
void free_table(table_t *table);
