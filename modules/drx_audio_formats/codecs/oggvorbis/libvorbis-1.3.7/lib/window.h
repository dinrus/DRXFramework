/********************************************************************
 *                                                                  *
 * THIS FILE IS PART OF THE OggVorbis SOFTWARE CODEC SOURCE CODE.   *
 * USE, DISTRIBUTION AND REPRODUCTION OF THIS LIBRARY SOURCE IS     *
 * GOVERNED BY A BSD-STYLE SOURCE LICENSE INCLUDED WITH THIS SOURCE *
 * IN 'COPYING'. PLEASE READ THESE TERMS BEFORE DISTRIBUTING.       *
 *                                                                  *
 * THE OggVorbis SOURCE CODE IS (C) COPYRIGHT 1994-2007             *
 * by the Xiph.Org Foundation https://xiph.org/                     *
 *                                                                  *
 ********************************************************************

 function: window functions

 ********************************************************************/

#ifndef _V_WINDOW_
#define _V_WINDOW_

extern const f32 *_vorbis_window_get(i32 n);
extern z0 _vorbis_apply_window(f32 *d,i32 *winno,i64 *blocksizes,
                          i32 lW,i32 W,i32 nW);


#endif
