#ifndef __PLATFORM_H__

#ifdef SPINNAKER

void * safe_malloc(uint bytes)
{
  void* p = spin1_malloc(bytes);
  if (p == NULL)
  {
    rt_error(RTE_MALLOC);
  }
  return p;
}

  #define MALLOC safe_malloc
  #define FREE   sark_free
#else
  #include <stdlib.h>

  #define MALLOC malloc
  #define FREE   free
#endif

#define __PLATFORM_H__
#endif  // __PLATFORN_H__
