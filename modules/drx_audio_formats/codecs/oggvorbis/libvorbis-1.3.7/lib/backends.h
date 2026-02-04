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

 function: libvorbis backend and mapping structures; needed for
           static mode headers

 ********************************************************************/

/* this is exposed up here because we need it for static modes.
   Lookups for each backend aren't exposed because there's no reason
   to do so */

#ifndef _vorbis_backend_h_
#define _vorbis_backend_h_

#include "codec_internal.h"

/* this would all be simpler/shorter with templates, but.... */
/* Floor backend generic *****************************************/
typedef struct{
  z0                   (*pack)  (vorbis_info_floor *,oggpack_buffer *);
  vorbis_info_floor     *(*unpack)(vorbis_info *,oggpack_buffer *);
  vorbis_look_floor     *(*look)  (vorbis_dsp_state *,vorbis_info_floor *);
  z0 (*free_info) (vorbis_info_floor *);
  z0 (*free_look) (vorbis_look_floor *);
  uk (*inverse1)  (struct vorbis_block *,vorbis_look_floor *);
  i32   (*inverse2)  (struct vorbis_block *,vorbis_look_floor *,
                     uk buffer,f32 *);
} vorbis_func_floor;

typedef struct{
  i32   order;
  i64  rate;
  i64  barkmap;

  i32   ampbits;
  i32   ampdB;

  i32   numbooks; /* <= 16 */
  i32   books[16];

  f32 lessthan;     /* encode-only config setting hacks for libvorbis */
  f32 greaterthan;  /* encode-only config setting hacks for libvorbis */

} vorbis_info_floor0;


#define VIF_POSIT 63
#define VIF_CLASS 16
#define VIF_PARTS 31
typedef struct{
  i32   partitions;                /* 0 to 31 */
  i32   partitionclass[VIF_PARTS]; /* 0 to 15 */

  i32   class_dim[VIF_CLASS];        /* 1 to 8 */
  i32   class_subs[VIF_CLASS];       /* 0,1,2,3 (bits: 1<<n poss) */
  i32   class_book[VIF_CLASS];       /* subs ^ dim entries */
  i32   class_subbook[VIF_CLASS][8]; /* [VIF_CLASS][subs] */


  i32   mult;                      /* 1 2 3 or 4 */
  i32   postlist[VIF_POSIT+2];    /* first two implicit */


  /* encode side analysis parameters */
  f32 maxover;
  f32 maxunder;
  f32 maxerr;

  f32 twofitweight;
  f32 twofitatten;

  i32   n;

} vorbis_info_floor1;

/* Residue backend generic *****************************************/
typedef struct{
  z0                 (*pack)  (vorbis_info_residue *,oggpack_buffer *);
  vorbis_info_residue *(*unpack)(vorbis_info *,oggpack_buffer *);
  vorbis_look_residue *(*look)  (vorbis_dsp_state *,
                                 vorbis_info_residue *);
  z0 (*free_info)    (vorbis_info_residue *);
  z0 (*free_look)    (vorbis_look_residue *);
  i64 **(*classx)      (struct vorbis_block *,vorbis_look_residue *,
			             i32 **,i32 *,i32);
  i32  (*forward)      (oggpack_buffer *,struct vorbis_block *,
                        vorbis_look_residue *,
                        i32 **,i32 *,i32,i64 **,i32);
  i32  (*inverse)      (struct vorbis_block *,vorbis_look_residue *,
                        f32 **,i32 *,i32);
} vorbis_func_residue;

typedef struct vorbis_info_residue0{
/* block-partitioned VQ coded straight residue */
  i64  begin;
  i64  end;

  /* first stage (lossless partitioning) */
  i32    grouping;         /* group n vectors per partition */
  i32    partitions;       /* possible codebooks for a partition */
  i32    partvals;         /* partitions ^ groupbook dim */
  i32    groupbook;        /* huffbook for partitioning */
  i32    secondstages[64]; /* expanded out to pointers in lookup */
  i32    booklist[512];    /* list of second stage books */

  /*const*/ i32 classmetric1[64];
  /*const*/ i32 classmetric2[64];
} vorbis_info_residue0;

/* Mapping backend generic *****************************************/
typedef struct{
  z0                 (*pack)  (vorbis_info *,vorbis_info_mapping *,
                                 oggpack_buffer *);
  vorbis_info_mapping *(*unpack)(vorbis_info *,oggpack_buffer *);
  z0 (*free_info)    (vorbis_info_mapping *);
  i32  (*forward)      (struct vorbis_block *vb);
  i32  (*inverse)      (struct vorbis_block *vb,vorbis_info_mapping *);
} vorbis_func_mapping;

typedef struct vorbis_info_mapping0{
  i32   submaps;  /* <= 16 */
  i32   chmuxlist[256];   /* up to 256 channels in a Vorbis stream */

  i32   floorsubmap[16];   /* [mux] submap to floors */
  i32   residuesubmap[16]; /* [mux] submap to residue */

  i32   coupling_steps;
  i32   coupling_mag[256];
  i32   coupling_ang[256];

} vorbis_info_mapping0;

#endif
