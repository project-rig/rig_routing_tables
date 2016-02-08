#include "spin1_api.h"
#include <stdbool.h>
#include <stdint.h>
#include "platform.h"

const unsigned int max_entries = 2048;

typedef struct _entry
{
  uint32_t bytes;  // 0 implies free
  uint32_t ptr;    // Address assigned/freed
} entry_t;


typedef struct _recording_region
{
  uint32_t n_entries;
  struct _recording_region *next;
  entry_t entries[];
} recording_region_t;


recording_region_t *dump;


recording_region_t *get_new_buffer(bool tag)
{
  recording_region_t *b = sark_xalloc(
    sv->sdram_heap,
    sizeof(uint32_t) + sizeof(recording_region_t *) + max_entries*sizeof(entry_t),
    0, 0
  );

  if (b == NULL)
  {
    io_printf(IO_BUF, "Failed to malloc recording region.\n");
    rt_error(RTE_MALLOC);
  }

  if (tag)
  {
    sv->vcpu_base[spin1_get_core_id()].user0 = (unsigned int) b;
  }

  b->n_entries = 0;
  b->next = NULL;
  return b;
}


void profile_init()
{
  // Get a recording region
  dump = get_new_buffer(true);
}


void *profiled_malloc(unsigned int bytes)
{
  void * ptr = safe_malloc(bytes);

  // Record the entry
  dump->entries[dump->n_entries].bytes = bytes;
  dump->entries[dump->n_entries].ptr = (uint32_t) ptr;
  dump->n_entries++;

  if (dump->n_entries == max_entries)
  {
    dump->next = get_new_buffer(false);
    dump = dump->next;
  }
  
  return ptr;
}


void profiled_free(void * ptr)
{
  sark_free(ptr);

  // Record the entry
  dump->entries[dump->n_entries].bytes = 0;
  dump->entries[dump->n_entries].ptr = (uint32_t) ptr;
  dump->n_entries++;

  if (dump->n_entries == max_entries)
  {
    dump->next = get_new_buffer(false);
    dump = dump->next;
  }
}
