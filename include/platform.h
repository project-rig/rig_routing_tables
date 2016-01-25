#ifndef __PLATFORM_H__

#ifdef SPINNAKER
  #define MALLOC spin1_malloc
  #define FREE   sark_free
#else
  #include <stdlib.h>

  #define MALLOC malloc
  #define FREE   free
#endif

#define __PLATFORM_H__
#endif  // __PLATFORN_H__
