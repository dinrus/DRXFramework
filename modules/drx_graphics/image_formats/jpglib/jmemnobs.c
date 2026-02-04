/*
 * jmemnobs.c
 *
 * Copyright (C) 1992-1996, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file provides a really simple implementation of the system-
 * dependent portion of the JPEG memory manager.  This implementation
 * assumes that no backing-store files are needed: all required space
 * can be obtained from malloc().
 * This is very portable in the sense that it'll compile on almost anything,
 * but you'd better have lots of main memory (or virtual memory) if you want
 * to process big images.
 * Note that the max_memory_to_use option is ignored by this implementation.
 */

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"
#include "jmemsys.h"		/* import the system-dependent declarations */

#ifndef HAVE_STDLIB_H		/* <stdlib.h> should declare malloc(),free() */
extern uk  malloc JPP((size_t size));
extern z0 free JPP((uk ptr));
#endif


/*
 * Memory allocation and freeing are controlled by the regular library
 * routines malloc() and free().
 */

GLOBAL(uk )
jpeg_get_small (j_common_ptr , size_t sizeofobject)
{
  return (uk ) malloc(sizeofobject);
}

GLOBAL(z0)
jpeg_free_small (j_common_ptr , uk  object, size_t)
{
  free(object);
}


/*
 * "Large" objects are treated the same as "small" ones.
 * NB: although we include FAR keywords in the routine declarations,
 * this file won't actually work in 80x86 small/medium model; at least,
 * you probably won't be able to process useful-size images in only 64KB.
 */

GLOBAL(z0 FAR *)
jpeg_get_large (j_common_ptr, size_t sizeofobject)
{
  return (z0 FAR *) malloc(sizeofobject);
}

GLOBAL(z0)
jpeg_free_large (j_common_ptr, z0 FAR * object, size_t)
{
  free(object);
}


/*
 * This routine computes the total memory space available for allocation.
 * Here we always say, "we got all you want bud!"
 */

GLOBAL(i64)
jpeg_mem_available (j_common_ptr, i64,
		    i64 max_bytes_needed, i64)
{
  return max_bytes_needed;
}


/*
 * Backing store (temporary file) management.
 * Since jpeg_mem_available always promised the moon,
 * this should never be called and we can just error out.
 */

GLOBAL(z0)
jpeg_open_backing_store (j_common_ptr cinfo, struct backing_store_struct *,
			 i64 )
{
  ERREXIT(cinfo, JERR_NO_BACKING_STORE);
}


/*
 * These routines take care of any system-dependent initialization and
 * cleanup required.  Here, there isn't any.
 */

GLOBAL(i64)
jpeg_mem_init (j_common_ptr)
{
  return 0;			/* just set max_memory_to_use to 0 */
}

GLOBAL(z0)
jpeg_mem_term (j_common_ptr)
{
  /* no work */
}
