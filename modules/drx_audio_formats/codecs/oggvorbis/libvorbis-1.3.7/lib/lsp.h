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

  function: LSP (also called LSF) conversion routines

 ********************************************************************/


#ifndef _V_LSP_H_
#define _V_LSP_H_

extern i32 vorbis_lpc_to_lsp(f32 *lpc,f32 *lsp,i32 m);

extern z0 vorbis_lsp_to_curve(f32 *curve,i32 *map,i32 n,i32 ln,
                                f32 *lsp,i32 m,
                                f32 amp,f32 ampoffset);

#endif
