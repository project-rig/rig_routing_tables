#ifndef __PLATFORM_H__

#ifdef SPINNAKER
  static inline void * safe_malloc(uint bytes)
  {
    void* p = spin1_malloc(bytes);
    if (p == NULL)
    {
      io_printf(IO_BUF, "Failed to malloc %u bytes.\n", bytes);
      rt_error(RTE_MALLOC);
    }
    return p;
  }

  #ifdef PROFILED
    void profile_init();
    void *profiled_malloc(uint bytes);
    void profiled_free(void * ptr);

    #define MALLOC profiled_malloc
    #define FREE   profiled_free
  #else
    #define MALLOC safe_malloc
    #define FREE   sark_free
  #endif

#else
  #include <stdlib.h>

  #define MALLOC malloc
  #define FREE   free
#endif

#define __PLATFORM_H__
#endif  // __PLATFORN_H__
