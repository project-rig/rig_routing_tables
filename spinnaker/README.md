# SpiNNaker executables

## Building SpiNNaker executables

To build SpiNNaker executables (`*.aplx`) ensure you have the [latest version
of SpiNNaker
tools](http://apt.cs.manchester.ac.uk/projects/SpiNNaker/downloads/) installed.
Then call `make` in this directory, two applications will be built.

## Using SpiNNaker executables

The script `minimise.py` illustrates how to load a routing table to a SpiNNaker
machine (with IP address `192.168.1.1`) for minimisation, how to read back the
minimized table and how to read back a profile of the memory usage.

[Rig](https://pypi.python.org/pypi/rig) must be installed for this script to
work. The [Rig documentation](http://rig.rtfd.org) may be useful when reading
this script.

## More details

On load `ordered_covering.aplx` and `ordered_covering_profiled.aplx` inspect
the SDRAM for a region with the tag ID 1. This region of memory is expected to contain:

 * 1 word indicating the length of the routing table
 * 1 word indicating the desired length of the table (or `0` to minimise as far as possible)

Followed by the indicated number of routing tables entries in the form:

```c
struct entry_t
{
  uint32_t key;
  uint32_t mask;
  uint32_t route;
}
```

**NOTE**: The entries are expected to be *already* sorted into increasing order
of the number of `X`s in the key and mask.

Once minimised, the size of the minimised table is written back into the first
word of this region and the compressed routing table is written over the
original table.

`minimise.py` includes examples of packing Rig routing tables into appropriate
structures to write into SDRAM (`pack_table`) and of reading back minimised
routing tables (`unpack_table`).

### Profiling memory usage

If `ordered_covering_profiled.aplx` is used then each call to `malloc` or
`free` is documented. The `user0` word of the VCPU struct will point at an
instance of `recording_region_t`:

```c
typedef struct _recording_region
{
  uint32_t n_entries;
  struct _recording_region *next;
  entry_t entries[];
} recording_region_t;
```

If `next` is NULL then then it is the last region. `entries` contains instances of:

```c
typedef struct _entry
{
  uint32_t bytes;  // 0 implies free
  uint32_t ptr;    // Address assigned/freed
} entry_t;
```

Each `malloc` of `n` bytes will add a new entry with `bytes` set to the number
of bytes and `ptr` indicating the address of the allocated memory. Each `free`
will add a new entry with `bytes` set to 0 and the `ptr` indicating the address
of the freed block of memory.

`minimise.py` includes an example (`get_memory_profile`) of reading back this
data to produce a list of samples of heap-usage over time.
