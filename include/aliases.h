/* Aliases are built using a trie structure as this avoids the need for
 * rebalancing at the cost of more memory.
 */
#include "platform.h"
#include "routing_table.h"
#include <stdio.h>


#ifndef __ALIASES_H__

typedef struct _node_t
{
  // There is a child in each of three directions
  void *child_0, *child_1, *child_X;
} node_t;
typedef node_t aliases_t;  // Users need not know how this works


// Create a new, empty, aliases container
static inline aliases_t aliases_init(void)
{
  aliases_t aliases = {NULL, NULL, NULL};
  return aliases;
}


static inline aliases_t* aliases_new(void)
{
  aliases_t *aliases = MALLOC(sizeof(aliases_t));
  aliases->child_X = NULL;
  aliases->child_0 = NULL;
  aliases->child_1 = NULL;
  return aliases;
}


// Retrieve an element from an aliases container
static inline void* aliases_find(aliases_t *a, keymask_t key)
{
  // Traverse the aliases tree to reach the desired element.
  for (uint32_t bit = (1 << 31);
       bit > 0 && a != NULL;
       bit >>= 1)
  {
    // Inspect the current bit to determine which route to take
    if (!(key.key & bit) && !(key.mask & bit))  // X
    {
      a = (aliases_t *) a->child_X;
    }
    else if (!(key.key & bit) && (key.mask & bit)) // 0
    {
      a = (aliases_t *) a->child_0;
    }
    else if ((key.key & bit) && (key.mask & bit)) // 1
    {
      a = (aliases_t *) a->child_1;
    }
    else  // !
    {
      a = NULL;
    }
  }
  return (void *) a;
}


// See if the aliases contain holds an element
static inline bool aliases_contains(aliases_t *a, keymask_t key)
{
  return aliases_find(a, key) != NULL;
}


// Add/overwrite an element into an aliases tree
static inline void aliases_insert(aliases_t *a, keymask_t key, void *value)
{
  // Traverse the tree, inserting new elements as we go
  for (uint32_t bit = (1 << 31); bit > 0; bit >>= 1)
  {
    void** child;
    if (!(key.key & bit) && !(key.mask & bit))  // X
    {
      child = &(a->child_X);
    }
    else if (!(key.key & bit) && (key.mask & bit)) // 0
    {
      child = &(a->child_0);
    }
    else if ((key.key & bit) && (key.mask & bit)) // 1
    {
      child = &(a->child_1);
    }
    else  // !
    {
      return;  // Cannot have elements associated with this key
    }

    if (bit > 1)
    {
      // If the child is uninitialised then initialise it
      if (*child == NULL)
      {
        *child = aliases_new();
      }
      a = (aliases_t*) *child;
    }
    else
    {
      // Insert the value
      *child = value;
    }
  }
}


static inline bool _aliases_remove(aliases_t *a, keymask_t key, uint32_t bit)
{
  // Determine the child we're inspecting
  void** child;
  if (!(key.key & bit) && !(key.mask & bit))  // X
  {
    child = &(a->child_X);
  }
  else if (!(key.key & bit) && (key.mask & bit)) // 0
  {
    child = &(a->child_0);
  }
  else if ((key.key & bit) && (key.mask & bit)) // 1
  {
    child = &(a->child_1);
  }
  else  // !
  {
    return false;  // Cannot have elements associated with this key
  }

  if (bit == 1)
  {
    // If this is a leaf then just detach, don't free the connected element
    *child = NULL;
  }
  else if (*child != NULL)
  {
    // Recurse
    bool remove_child = _aliases_remove((aliases_t *) *child, key, bit >> 1);
    if (remove_child)
    {
      // If the child is itself childless we remove it
      FREE(*child);
      *child = NULL;
    }
  }

  // Return true if we have no children
  return (a->child_X == NULL && a->child_0 == NULL && a->child_1 == NULL);
}


// Remove an element from an aliases tree
static inline void aliases_remove(aliases_t *a, keymask_t key)
{
  _aliases_remove(a, key, 1 << 31);
}


static inline void _aliases_clear(aliases_t *a, unsigned int level)
{
  // If we're a leaf then we don't need to do anything
  if (level > 0)
  {
    // Recurse and then free children
    if (a->child_X != NULL)
    {
      _aliases_clear((aliases_t *) a->child_X, level - 1);
      FREE(a->child_X);
      a->child_X = NULL;
    }

    if (a->child_0 != NULL)
    {
      _aliases_clear((aliases_t *) a->child_0, level - 1);
      FREE(a->child_0);
      a->child_0 = NULL;
    }

    if (a->child_1 != NULL)
    {
      _aliases_clear((aliases_t *) a->child_1, level - 1);
      FREE(a->child_1);
      a->child_1 = NULL;
    }
  }
}


// Remove all elements from an aliases container
static inline void aliases_clear(aliases_t *a)
{
  _aliases_clear(a, 31);
}

#define __ALIASES_H__
#endif  // __ALIASES_H__
