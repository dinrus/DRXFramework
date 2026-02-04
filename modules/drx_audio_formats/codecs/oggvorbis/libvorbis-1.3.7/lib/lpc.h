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

  function: LPC low level routines

 ********************************************************************/

#ifndef _V_LPC_H_
#define _V_LPC_H_

#include "../../codec.h"

/* simple linear scale LPC code */
extern f32 vorbis_lpc_from_data(f32 *data,f32 *lpc,i32 n,i32 m);

extern z0 vorbis_lpc_predict(f32 *coeff,f32 *prime,i32 m,
                               f32 *data,i64 n);

#endif
