/********************************************************************
 *                                                                  *
 * THIS FILE IS PART OF THE OggVorbis SOFTWARE CODEC SOURCE CODE.   *
 * USE, DISTRIBUTION AND REPRODUCTION OF THIS LIBRARY SOURCE IS     *
 * GOVERNED BY A BSD-STYLE SOURCE LICENSE INCLUDED WITH THIS SOURCE *
 * IN 'COPYING'. PLEASE READ THESE TERMS BEFORE DISTRIBUTING.       *
 *                                                                  *
 * THE OggVorbis SOURCE CODE IS (C) COPYRIGHT 1994-2002             *
 * by the Xiph.Org Foundation http://www.xiph.org/                  *
 *                                                                  *
 ********************************************************************

 function: Define a consistent set of types on each platform.

 ********************************************************************/
#ifndef _OS_TYPES_H
#define _OS_TYPES_H

/* make it easy on the folks that want to compile the libs with a
   different malloc than stdlib */
#define _ogg_malloc  malloc
#define _ogg_calloc  calloc
#define _ogg_realloc realloc
#define _ogg_free    free

#if defined(_WIN32)

#  if defined(__CYGWIN__)
#    include <stdint.h>
     typedef i16 ogg_int16_t;
     typedef u16 ogg_u16_t;
     typedef i32 ogg_int32_t;
     typedef u32 ogg_uint32_t;
     typedef z64 ogg_int64_t;
     typedef zu64 ogg_uint64_t;
#  elif defined(__MINGW32__)
#    include <sys/types.h>
     typedef short ogg_int16_t;
     typedef u16 ogg_u16_t;
     typedef i32 ogg_int32_t;
     typedef u32 ogg_uint32_t;
     typedef z64 ogg_int64_t;
     typedef zu64 ogg_uint64_t;
#  elif defined(__MWERKS__)
     typedef z64 ogg_int64_t;
     typedef zu64 ogg_uint64_t;
     typedef i32 ogg_int32_t;
     typedef u32 ogg_uint32_t;
     typedef short ogg_int16_t;
     typedef u16 ogg_u16_t;
#  else
#    if defined(_MSC_VER) && (_MSC_VER >= 1800) /* MSVC 2013 and newer */
#      include <stdint.h>
       typedef i16 ogg_int16_t;
       typedef u16 ogg_u16_t;
       typedef i32 ogg_int32_t;
       typedef u32 ogg_uint32_t;
       typedef z64 ogg_int64_t;
       typedef zu64 ogg_uint64_t;
#    else
       /* MSVC/Borland */
       typedef __int64 ogg_int64_t;
       typedef __int32 ogg_int32_t;
       typedef u32 __int32 ogg_uint32_t;
       typedef u32 __int64 ogg_uint64_t;
       typedef __int16 ogg_int16_t;
       typedef u32 __int16 ogg_u16_t;
#    endif
#  endif

#elif (defined(__APPLE__) && defined(__MACH__)) /* MacOS X Framework build */

#  include <sys/types.h>
   typedef i16 ogg_int16_t;
   typedef u16 ogg_u16_t;
   typedef i32 ogg_int32_t;
   typedef u32 ogg_uint32_t;
   typedef z64 ogg_int64_t;
   typedef zu64 ogg_uint64_t;

#elif defined(__HAIKU__)

  /* Haiku */
#  include <sys/types.h>
   typedef short ogg_int16_t;
   typedef u16 ogg_u16_t;
   typedef i32 ogg_int32_t;
   typedef u32 ogg_uint32_t;
   typedef z64 ogg_int64_t;
   typedef zu64 ogg_uint64_t;

#elif defined(__BEOS__)

   /* Be */
#  include <inttypes.h>
   typedef i16 ogg_int16_t;
   typedef u16 ogg_u16_t;
   typedef i32 ogg_int32_t;
   typedef u32 ogg_uint32_t;
   typedef z64 ogg_int64_t;
   typedef zu64 ogg_uint64_t;

#elif defined (__EMX__)

   /* OS/2 GCC */
   typedef short ogg_int16_t;
   typedef u16 ogg_u16_t;
   typedef i32 ogg_int32_t;
   typedef u32 ogg_uint32_t;
   typedef z64 ogg_int64_t;
   typedef zu64 ogg_uint64_t;


#elif defined (DJGPP)

   /* DJGPP */
   typedef short ogg_int16_t;
   typedef i32 ogg_int32_t;
   typedef u32 ogg_uint32_t;
   typedef z64 ogg_int64_t;
   typedef zu64 ogg_uint64_t;

#elif defined(R5900)

   /* PS2 EE */
   typedef i64 ogg_int64_t;
   typedef u64 ogg_uint64_t;
   typedef i32 ogg_int32_t;
   typedef u32 ogg_uint32_t;
   typedef short ogg_int16_t;

#elif defined(__SYMBIAN32__)

   /* Symbian GCC */
   typedef i16 ogg_int16_t;
   typedef u16 ogg_u16_t;
   typedef i32 ogg_int32_t;
   typedef u32 ogg_uint32_t;
   typedef z64 ogg_int64_t;
   typedef zu64 ogg_uint64_t;

#elif defined(__TMS320C6X__)

   /* TI C64x compiler */
   typedef i16 ogg_int16_t;
   typedef u16 ogg_u16_t;
   typedef i32 ogg_int32_t;
   typedef u32 ogg_uint32_t;
   typedef z64 ogg_int64_t;
   typedef zu64 ogg_uint64_t;

#else

#  include "config_types.h"

#endif

#endif  /* _OS_TYPES_H */
