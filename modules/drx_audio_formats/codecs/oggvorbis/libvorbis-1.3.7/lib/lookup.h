/********************************************************************
 *                                                                  *
 * THIS FILE IS PART OF THE OggVorbis SOFTWARE CODEC SOURCE CODE.   *
 * USE, DISTRIBUTION AND REPRODUCTION OF THIS LIBRARY SOURCE IS     *
 * GOVERNED BY A BSD-STYLE SOURCE LICENSE INCLUDED WITH THIS SOURCE *
 * IN 'COPYING'. PLEASE READ THESE TERMS BEFORE DISTRIBUTING.       *
 *                                                                  *
 * THE OggVorbis SOURCE CODE IS (C) COPYRIGHT 1994-2009             *
 * by the Xiph.Org Foundation https://xiph.org/                     *
 *                                                                  *
 ********************************************************************

  function: lookup based functions

 ********************************************************************/

#ifndef _V_LOOKUP_H_

#ifdef FLOAT_LOOKUP
extern f32 vorbis_coslook(f32 a);
extern f32 vorbis_invsqlook(f32 a);
extern f32 vorbis_invsq2explook(i32 a);
extern f32 vorbis_fromdBlook(f32 a);
#endif
#ifdef INT_LOOKUP
extern i64 vorbis_invsqlook_i(i64 a,i64 e);
extern i64 vorbis_coslook_i(i64 a);
extern f32 vorbis_fromdBlook_i(i64 a);
#endif

#endif
