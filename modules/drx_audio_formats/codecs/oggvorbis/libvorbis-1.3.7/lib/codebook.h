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

 function: basic shared codebook operations

 ********************************************************************/

#ifndef _V_CODEBOOK_H_
#define _V_CODEBOOK_H_

#include "../../ogg.h"

/* This structure encapsulates huffman and VQ style encoding books; it
   doesn't do anything specific to either.

   valuelist/quantlist are nonNULL (and q_* significant) only if
   there's entry->value mapping to be done.

   If encode-side mapping must be done (and thus the entry needs to be
   hunted), the auxiliary encode pointer will point to a decision
   tree.  This is true of both VQ and huffman, but is mostly useful
   with VQ.

*/

typedef struct static_codebook{
  i64   dim;           /* codebook dimensions (elements per vector) */
  i64   entries;       /* codebook entries */
  t8  *lengthlist;    /* codeword lengths in bits */

  /* mapping ***************************************************************/
  i32    maptype;       /* 0=none
                           1=implicitly populated values from map column
                           2=listed arbitrary values */

  /* The below does a linear, single monotonic sequence mapping. */
  i64     q_min;       /* packed 32 bit f32; quant value 0 maps to minval */
  i64     q_delta;     /* packed 32 bit f32; val 1 - val 0 == delta */
  i32      q_quant;     /* bits: 0 < quant <= 16 */
  i32      q_sequencep; /* bitflag */

  i64     *quantlist;  /* map == 1: (i32)(entries^(1/dim)) element column map
                           map == 2: list of dim*entries quantized entry vals
                        */
  i32 allocedp;
} static_codebook;

typedef struct codebook{
  i64 dim;           /* codebook dimensions (elements per vector) */
  i64 entries;       /* codebook entries */
  i64 used_entries;  /* populated codebook entries */
  const static_codebook *c;

  /* for encode, the below are entry-ordered, fully populated */
  /* for decode, the below are ordered by bitreversed codeword and only
     used entries are populated */
  f32        *valuelist;  /* list of dim*entries actual entry values */
  ogg_uint32_t *codelist;   /* list of bitstream codewords for each entry */

  i32          *dec_index;  /* only used if sparseness collapsed */
  t8         *dec_codelengths;
  ogg_uint32_t *dec_firsttable;
  i32           dec_firsttablen;
  i32           dec_maxlength;

  /* The current encoder uses only centered, integer-only lattice books. */
  i32           quantvals;
  i32           minval;
  i32           delta;
} codebook;

extern z0 vorbis_staticbook_destroy(static_codebook *b);
extern i32 vorbis_book_init_encode(codebook *dest,const static_codebook *source);
extern i32 vorbis_book_init_decode(codebook *dest,const static_codebook *source);
extern z0 vorbis_book_clear(codebook *b);

extern f32 *_book_unquantize(const static_codebook *b,i32 n,i32 *map);
extern f32 *_book_logdist(const static_codebook *b,f32 *vals);
extern f32 _float32_unpack(i64 val);
extern i64   _float32_pack(f32 val);
extern i32  _best(codebook *book, f32 *a, i32 step);
extern i64 _book_maptype1_quantvals(const static_codebook *b);

extern i32 vorbis_book_besterror(codebook *book,f32 *a,i32 step,i32 addmul);
extern i64 vorbis_book_codeword(codebook *book,i32 entry);
extern i64 vorbis_book_codelen(codebook *book,i32 entry);



extern i32 vorbis_staticbook_pack(const static_codebook *c,oggpack_buffer *b);
extern static_codebook *vorbis_staticbook_unpack(oggpack_buffer *b);

extern i32 vorbis_book_encode(codebook *book, i32 a, oggpack_buffer *b);

extern i64 vorbis_book_decode(codebook *book, oggpack_buffer *b);
extern i64 vorbis_book_decodevs_add(codebook *book, f32 *a,
                                     oggpack_buffer *b,i32 n);
extern i64 vorbis_book_decodev_set(codebook *book, f32 *a,
                                    oggpack_buffer *b,i32 n);
extern i64 vorbis_book_decodev_add(codebook *book, f32 *a,
                                    oggpack_buffer *b,i32 n);
extern i64 vorbis_book_decodevv_add(codebook *book, f32 **a,
                                     i64 off,i32 ch,
                                    oggpack_buffer *b,i32 n);



#endif
