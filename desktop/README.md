# Desktop executable

Build desktop implementations of Ordered-Covering and m-Trie by running `make`
in this directory.

The resulting executables can be called with:

```bash
$ ./ordered_covering in_file out_file [target length]
$ ./mtrie in_file out_file
```

## Input/Output file format

Input and output files are expected to be binary serialisations of
[Rig-like routing table entries](http://rig.readthedocs.org/en/stable/routing_table_tools_doctest.html#routingtableentry-and-routes-routing-table-data-structures)
in the form:

```c
typedef struct _entry
{
  uint32_t key;
  uint32_t mask;
  uint32_t source;  // Mask of links/cores from which packets may originate
  uint32_t route;   // Mask of links/cores to which packets should be sent
} entry_t;
```

**NOTE**: The `source` field is currently ignored and will be returned blank,
indicating that the entry should never be removed to be handled to by default
routing.

If a target length is provided then minimisation will stop once the table is at
most as long as the target, otherwise the table will be minimised as far as
possible.
