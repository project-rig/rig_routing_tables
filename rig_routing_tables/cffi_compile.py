import os
from cffi import FFI
ffi = FFI()


# Get the include directory
include_dir = os.path.join(os.path.dirname(__file__), "..", "include")

ffi.set_source(
    "_rig_routing_tables",
    """
        #include "mtrie.h"
        #include "routing_table.h"
        #include "ordered_covering.h"
        #include "cffi_utils.h"
    """,
    include_dirs=[include_dir, os.path.dirname(__file__)],
    sources=[os.path.join(os.path.dirname(__file__), "cffi_utils.c")],
)

ffi.cdef("""
    // Data structures
    typedef struct _keymask_t
    {
      uint32_t key;   // Key for the keymask
      uint32_t mask;  // Mask for the keymask
      ...;
    } keymask_t;

    typedef struct _entry_t
    {
      keymask_t keymask;  // Key and mask
      uint32_t route;     // Routing direction
      uint32_t source;    // Source of packets arriving at this entry
      ...;
    } entry_t;

    typedef struct _table_t
    {
      unsigned int size;  // Number of entries in the table
      entry_t *entries;   // Entries in the table
      ...;
    } table_t;

    // Table management
    table_t* new_table(unsigned int size);
    void free_table(table_t *table);

    // Minimisation methods
    void mtrie_minimise(table_t *table);
    void oc_minimise_na(table_t *table, unsigned int target_length);
""")

if __name__ == "__main__":
    ffi.compile()
