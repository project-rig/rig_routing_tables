#include "platform.h"
#include "bitset.h"
#include "routing_table.h"
#include <stdbool.h>
#include <stdint.h>

#ifndef __MTRIE_H__

// m-Trie structure
typedef struct _mtrie_node_t
{
  struct _mtrie_node_t *parent;  // Our parent
  uint32_t bit;  // Bit represented by this Node
  struct _mtrie_node_t *child_0, *child_1, *child_X;  // Children of this Node
} mtrie_t;

// Create a new (empty) node
static inline mtrie_t *mtrie_new_node(mtrie_t *parent, uint32_t bit)
{
  // Malloc room for the new m-Trie, then set the bit to be the MSB and ensure
  // the children are NULL.
  mtrie_t *node = MALLOC(sizeof(mtrie_t));
  node->parent = parent;
  node->bit = bit;
  node->child_0 = node->child_1 = node->child_X = NULL;

  return node;
}

// Create a new (empty) tree
static inline mtrie_t* mtrie_new(void)
{
  return mtrie_new_node(NULL, 1 << 31);
}

// Delete an existing tree
static inline void mtrie_delete(mtrie_t *node)
{
  if (node != NULL)
  {
    // Free any children
    mtrie_delete(node->child_0);
    mtrie_delete(node->child_1);
    mtrie_delete(node->child_X);

    // Free ourselves
    FREE(node);
  }
}

// Count the number of paths travelling through a node
static inline unsigned int mtrie_count(mtrie_t *node)
{
  if (node == NULL)
  {
    return 0;  // Not a node, so return 0
  }
  else if (!node->bit)
  {
    return 1;  // Node is a leaf, so return a 1
  }
  else
  {
    // Get the contribution from each child (if there are any)
    return mtrie_count(node->child_0) +
           mtrie_count(node->child_1) +
           mtrie_count(node->child_X);
  }
}

// Extract routing table entries from a trie
static inline keymask_t* _get_entries(
  mtrie_t *node, keymask_t *table, uint32_t pkey, uint32_t pmask
)
{
  if (node == NULL)
  {
    // Do nothing as this isn't a valid node.
  }
  else if (!node->bit)
  {
    // If this is a leaf then add an entry to the table representing this entry
    // and return a pointer to the next entry in the table.
    table->key = pkey;
    table->mask = pmask;

    // Point to the next table entry
    table++;
  }
  else
  {
    // If this is not a leaf then get entries from any children we may have.
    uint32_t b = node->bit;  // Bit to set
    table = _get_entries(node->child_0, table, pkey, pmask | b);
    table = _get_entries(node->child_1, table, pkey | b, pmask | b);
    table = _get_entries(node->child_X, table, pkey, pmask);
  }

  return table;
}

static inline void mtrie_get_entries(mtrie_t *node, keymask_t *table)
{
  _get_entries(node, table, 0x0, 0x0);
}

// Get the relevant child with which to follow a path
static inline mtrie_t** get_child(mtrie_t *node, uint32_t key, uint32_t mask)
{
  if (mask & node->bit)  // Either a 0 or a 1
  {
    if (!(key & node->bit))
    {
      // A 0 at this bit
      return &(node->child_0);
    }
    else
    {
      // A 1 at this bit
      return &(node->child_1);
    }
  }
  else if (!(key & node->bit))
  {
    // An X at this bit
    return &(node->child_X);
  }
  else
  {
    return NULL;  // A `!' at this bit, abort
  }
}

// Traverse a path through the tree, adding elements as necessary
static inline mtrie_t* mtrie_traverse(mtrie_t *node, uint32_t key, uint32_t mask)
{
  if (node->bit)  // If not a leaf
  {
    // See where to turn at this node
    mtrie_t **child = get_child(node, key, mask);

    // If no child was returned then the given key and mask were invalid
    if (!child)
    {
      return NULL;
    }

    // If the child is NULL then create a new child
    if (!*child)
    {
      *child = mtrie_new_node(node, node->bit >> 1);
    }

    // Delegate the traversal to the child
    return mtrie_traverse(*child, key, mask);
  }
  else
  {
    // If we are a leaf then return our parent
    return node->parent;
  }
}

// Check if a path exists in a sub-trie
static inline bool path_exists(mtrie_t *node, uint32_t key, uint32_t mask)
{
  if (node->bit)  // If not a leaf
  {
    // See where to turn at this node
    mtrie_t **child = get_child(node, key, mask);

    // If there is no child then the path does not exist or the given path was
    // invalid.
    if (!child || !*child)
    {
      return false;
    }

    // Delegate the traversal to the child
    return path_exists(*child, key, mask);
  }
  else
  {
    // If we are a leaf then the path must exist
    return true;
  }
}

static inline bool mtrie_untraverse(mtrie_t *node, uint32_t key, uint32_t mask)
{
  if (node->bit)  // If not a leaf
  {
    // Get the child to attempt to untraverse
    mtrie_t **child = get_child(node, key, mask);

    if (mtrie_untraverse(*child, key, mask))
    {
      // The child has been freed, so remove our reference to it
      *child = NULL;
    }

    // If we have no children left then free ourselves and return true;
    // otherwise return false.
    if (node->child_0 == NULL && node->child_1 == NULL && node->child_X == NULL)
    {
      FREE(node);
      return true;
    }
    else
    {
      return false;
    }
  }
  else
  {
    // If a leaf then free ourselves and then return to true to indicate that
    // this has occurred.
    FREE(node);
    return true;
  }
}

static inline void untraverse_in_child(mtrie_t **child, uint32_t key, uint32_t mask)
{
  if (mtrie_untraverse(*child, key, mask))
  {
    *child = NULL;
  }
}

// Insert a new entry into the trie
static inline void mtrie_insert(mtrie_t *root, uint32_t key, uint32_t mask)
{
  // Traverse a path through the trie and keep a reference to the leaf we reach
  mtrie_t *leaf = mtrie_traverse(root, key, mask);

  // Attempt to find overlapping paths
  while (leaf)
  {
    mtrie_t **child_0 = &(leaf->child_0);
    mtrie_t **child_1 = &(leaf->child_1);
    mtrie_t **child_X = &(leaf->child_X);

    if (*child_0 != NULL && path_exists(*child_0, key, mask) &&
        *child_1 != NULL && path_exists(*child_1, key, mask))
    {
      // Traverse the path in X and then untraverse in 0 and 1
      if (*child_X == NULL)
      {
        *child_X = mtrie_new_node(leaf, leaf->bit >> 1);
      }
      mtrie_traverse(*child_X, key, mask);

      // Untraverse in `0' and `1'
      untraverse_in_child(child_0, key, mask);
      untraverse_in_child(child_1, key, mask);

      // Update the key and mask
      key &= ~(leaf->bit);
      mask &= ~(leaf->bit);
    }
    else if (*child_X != NULL && path_exists(*child_X, key, mask) &&
             *child_0 != NULL && path_exists(*child_0, key, mask))
    {
      // Untraverse in `0'
      untraverse_in_child(child_0, key, mask);

      // Update the key and mask
      key &= ~(leaf->bit);
      mask &= ~(leaf->bit);
    }
    else if (*child_X != NULL && path_exists(*child_X, key, mask) &&
             *child_1 != NULL && path_exists(*child_1, key, mask))
    {
      // Untraverse in `1'
      untraverse_in_child(child_1, key, mask);

      // Update the key and mask
      key &= ~(leaf->bit);
      mask &= ~(leaf->bit);
    }

    // Move up a level
    leaf = leaf->parent;
  }
}

// Subtable structure used to hold partially-minimised routing tables
typedef struct _subtable
{
  unsigned int n_entries;  // Number of entries in the subtable
  uint32_t route;          // Route of all entries in the subtable
  keymask_t *entries;      // Entries in the subtable

  struct _subtable *next;  // Next subtable in the chain
} subtable_t;


// Create a new subtable, possibly appending to an existing list of subtables
static inline subtable_t* subtable_new(
  subtable_t **sb, unsigned int size, uint32_t route
)
{
  while (*sb != NULL)
  {
    sb = &((*sb)->next);
  }

  // Create a new subtable of the desired size
  *sb = MALLOC(sizeof(subtable_t));
  (*sb)->entries = MALLOC(sizeof(keymask_t) * size);
  (*sb)->n_entries = size;
  (*sb)->route = route;
  (*sb)->next = NULL;

  return *sb;
}

// Expand a subtable into an existing routing table
static inline void subtable_expand(subtable_t *sb, table_t *table)
{
  // Keep a count of the new number of entries
  table->size = 0;

  // For each subtable copy in the new entries
  entry_t *next = table->entries;
  while (sb != NULL)
  {
    // Expand the current subtable
    for (unsigned int i = 0; i < sb->n_entries; i++)
    {
      next->keymask.key = sb->entries[i].key;
      next->keymask.mask = sb->entries[i].mask;
      next->route = sb->route;

      next++;
    }

    // Increase the size of the output table
    table->size += sb->n_entries;

    // Get the next subtable
    sb = sb->next;
  }
}

// Delete a set of subtables
static inline void subtable_delete(subtable_t *sb)
{
  // Delete local entries
  FREE(sb->entries);
  sb->entries = NULL;
  sb->n_entries = 0;

  // Recurse
  if (sb->next != NULL)
  {
    subtable_delete(sb->next);
    sb->next = NULL;
  }

  // Delete ourselves
  FREE(sb);
}

// Use m-Tries to minimise a routing table
static inline void mtrie_minimise(table_t *table)
{
  // For each set of unique routes in the table we construct an m-Trie to
  // minimise the entries; we then write the minimised table back in on-top of
  // the original table.
  
  // Keep a reference to entries we've already dealt with
  bitset_t visited;
  bitset_init(&visited, table->size);

  // Maintain a chain of partially minimised tables
  subtable_t *subtables = NULL;

  // For every entry in the table that hasn't been inspected create a new
  // m-Trie, then add to it every entry with an equivalent route.
  for (unsigned int i = 0; i < table->size; i++)
  {
    if (bitset_contains(&visited, i))
    {
      continue;
    }

    // Create a new m-Trie
    mtrie_t *trie = mtrie_new();
    uint32_t route = table->entries[i].route;

    // Add all equivalent entries to the trie
    for (unsigned int j = i; j < table->size; j++)
    {
      if (table->entries[j].route == route)
      {
        // Mark the entry as visited
        bitset_add(&visited, j);

        // Add to the trie
        uint32_t key = table->entries[j].keymask.key;
        uint32_t mask = table->entries[j].keymask.mask;
        mtrie_insert(trie, key, mask);
      }
    }

    // Read out all the minimised entries into a new subtable
    subtable_t *sb = subtable_new(&subtables, mtrie_count(trie), route);
    mtrie_get_entries(trie, sb->entries);

    // Delete the m-Trie
    mtrie_delete(trie);
  }

  // Overwrite the original routing table by copying entries back from the
  // subtables.
  subtable_expand(subtables, table);

  // Clear up the subtables and bitset.
  subtable_delete(subtables); subtables = NULL;
  bitset_delete(&visited);
}

#define __MTRIE_H__
#endif // __MTRIE_H__
