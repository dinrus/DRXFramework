/********************************************************************
 *                                                                  *
 * THIS FILE IS PART OF THE OggVorbis SOFTWARE CODEC SOURCE CODE.   *
 * USE, DISTRIBUTION AND REPRODUCTION OF THIS LIBRARY SOURCE IS     *
 * GOVERNED BY A BSD-STYLE SOURCE LICENSE INCLUDED WITH THIS SOURCE *
 * IN 'COPYING'. PLEASE READ THESE TERMS BEFORE DISTRIBUTING.       *
 *                                                                  *
 * THE OggVorbis SOURCE CODE IS (C) COPYRIGHT 1994-2015             *
 * by the Xiph.Org Foundation https://xiph.org/                     *
 *                                                                  *
 ********************************************************************

 function: miscellaneous prototypes

 ********************************************************************/

#ifndef _V_RANDOM_H_
#define _V_RANDOM_H_
#include "../../codec.h"

extern uk _vorbis_block_alloc(vorbis_block *vb,i64 bytes);
extern z0 _vorbis_block_ripcord(vorbis_block *vb);
extern i32 ov_ilog(ogg_uint32_t v);

#ifdef ANALYSIS
extern i32 analysis_noisy;
extern z0 _analysis_output(t8 *base,i32 i,f32 *v,i32 n,i32 bark,i32 dB,
                             ogg_int64_t off);
extern z0 _analysis_output_always(t8 *base,i32 i,f32 *v,i32 n,i32 bark,i32 dB,
                             ogg_int64_t off);
#endif

#ifdef DEBUG_MALLOC

#define _VDBG_GRAPHFILE "malloc.m"
#undef _VDBG_GRAPHFILE
extern uk _VDBG_malloc(uk ptr,i64 bytes,t8 *file,i64 line);
extern z0 _VDBG_free(uk ptr,t8 *file,i64 line);

#ifndef MISC_C
#undef _ogg_malloc
#undef _ogg_calloc
#undef _ogg_realloc
#undef _ogg_free

#define _ogg_malloc(x) _VDBG_malloc(NULL,(x),__FILE__,__LINE__)
#define _ogg_calloc(x,y) _VDBG_malloc(NULL,(x)*(y),__FILE__,__LINE__)
#define _ogg_realloc(x,y) _VDBG_malloc((x),(y),__FILE__,__LINE__)
#define _ogg_free(x) _VDBG_free((x),__FILE__,__LINE__)
#endif
#endif

#endif
