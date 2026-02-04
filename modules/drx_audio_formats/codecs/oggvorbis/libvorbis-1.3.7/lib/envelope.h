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

 function: PCM data envelope analysis and manipulation

 ********************************************************************/

#ifndef _V_ENVELOPE_
#define _V_ENVELOPE_

#include "mdct.h"

#define VE_PRE    16
#define VE_WIN    4
#define VE_POST   2
#define VE_AMP    (VE_PRE+VE_POST-1)

#define VE_BANDS  7
#define VE_NEARDC 15

#define VE_MINSTRETCH 2   /* a bit less than short block */
#define VE_MAXSTRETCH 12  /* one-third full block */

typedef struct {
  f32 ampbuf[VE_AMP];
  i32   ampptr;

  f32 nearDC[VE_NEARDC];
  f32 nearDC_acc;
  f32 nearDC_partialacc;
  i32   nearptr;

} envelope_filter_state;

typedef struct {
  i32 begin;
  i32 end;
  f32 *window;
  f32 total;
} envelope_band;

typedef struct {
  i32 ch;
  i32 winlength;
  i32 searchstep;
  f32 minenergy;

  mdct_lookup  mdct;
  f32       *mdct_win;

  envelope_band          band[VE_BANDS];
  envelope_filter_state *filter;
  i32   stretch;

  i32                   *mark;

  i64 storage;
  i64 current;
  i64 curmark;
  i64 cursor;
} envelope_lookup;

extern z0 _ve_envelope_init(envelope_lookup *e,vorbis_info *vi);
extern z0 _ve_envelope_clear(envelope_lookup *e);
extern i64 _ve_envelope_search(vorbis_dsp_state *v);
extern z0 _ve_envelope_shift(envelope_lookup *e,i64 shift);
extern i32  _ve_envelope_mark(vorbis_dsp_state *v);


#endif
