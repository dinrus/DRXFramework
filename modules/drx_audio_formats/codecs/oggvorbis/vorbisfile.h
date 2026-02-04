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

 function: stdio-based convenience library for opening/seeking/decoding

 ********************************************************************/

#ifndef _OV_FILE_H_
#define _OV_FILE_H_

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#include <stdio.h>
#include "codec.h"

/* The function prototypes for the callbacks are basically the same as for
 * the stdio functions fread, fseek, fclose, ftell.
 * The one difference is that the FILE * arguments have been replaced with
 * a uk  - this is to be used as a pointer to whatever internal data these
 * functions might need. In the stdio case, it's just a FILE * cast to a uk 
 *
 * If you use other functions, check the docs for these functions and return
 * the right values. For seek_func(), you *MUST* return -1 if the stream is
 * unseekable
 */
typedef struct {
  size_t (*read_func)  (uk ptr, size_t size, size_t nmemb, uk datasource);
  i32    (*seek_func)  (uk datasource, ogg_int64_t offset, i32 whence);
  i32    (*close_func) (uk datasource);
  i64   (*tell_func)  (uk datasource);
} ov_callbacks;

#ifndef OV_EXCLUDE_STATIC_CALLBACKS

/* a few sets of convenient callbacks, especially for use under
 * Windows where ov_open_callbacks() should always be used instead of
 * ov_open() to avoid problems with incompatible crt.o version linking
 * issues. */

/*static i32 _ov_header_fseek_wrap(FILE *f,ogg_int64_t off,i32 whence){
  if(f==NULL)return(-1);

#ifdef __MINGW32__
  return fseeko64(f,off,whence);
#elif defined (_WIN32)
  return _fseeki64(f,off,whence);
#else
  return fseek(f,off,whence);
#endif
}*/

/* These structs below (OV_CALLBACKS_DEFAULT etc) are defined here as
 * static data. That means that every file which includes this header
 * will get its own copy of these structs whether it uses them or
 * not unless it #defines OV_EXCLUDE_STATIC_CALLBACKS.
 * These static symbols are essential on platforms such as Windows on
 * which several different versions of stdio support may be linked to
 * by different DLLs, and we need to be certain we know which one
 * we're using (the same one as the main application).
 */

/*static ov_callbacks OV_CALLBACKS_DEFAULT = {
  (size_t (*)(uk , size_t, size_t, uk ))  fread,
  (i32 (*)(uk , ogg_int64_t, i32))           _ov_header_fseek_wrap,
  (i32 (*)(uk ))                             fclose,
  (i64 (*)(uk ))                            ftell
};

static ov_callbacks OV_CALLBACKS_NOCLOSE = {
  (size_t (*)(uk , size_t, size_t, uk ))  fread,
  (i32 (*)(uk , ogg_int64_t, i32))           _ov_header_fseek_wrap,
  (i32 (*)(uk ))                             NULL,
  (i64 (*)(uk ))                            ftell
};

static ov_callbacks OV_CALLBACKS_STREAMONLY = {
  (size_t (*)(uk , size_t, size_t, uk ))  fread,
  (i32 (*)(uk , ogg_int64_t, i32))           NULL,
  (i32 (*)(uk ))                             fclose,
  (i64 (*)(uk ))                            NULL
};

static ov_callbacks OV_CALLBACKS_STREAMONLY_NOCLOSE = {
  (size_t (*)(uk , size_t, size_t, uk ))  fread,
  (i32 (*)(uk , ogg_int64_t, i32))           NULL,
  (i32 (*)(uk ))                             NULL,
  (i64 (*)(uk ))                            NULL
};*/

#endif

#define  NOTOPEN   0
#define  PARTOPEN  1
#define  OPENED    2
#define  STREAMSET 3
#define  INITSET   4

typedef struct OggVorbis_File {
  z0            *datasource; /* Pointer to a FILE *, etc. */
  i32              seekable;
  ogg_int64_t      offset;
  ogg_int64_t      end;
  ogg_sync_state   oy;

  /* If the FILE handle isn't seekable (eg, a pipe), only the current
     stream appears */
  i32              links;
  ogg_int64_t     *offsets;
  ogg_int64_t     *dataoffsets;
  i64            *serialnos;
  ogg_int64_t     *pcmlengths; /* overloaded to maintain binary
                                  compatibility; x2 size, stores both
                                  beginning and end values */
  vorbis_info     *vi;
  vorbis_comment  *vc;

  /* Decoding working state local storage */
  ogg_int64_t      pcm_offset;
  i32              ready_state;
  i64             current_serialno;
  i32              current_link;

  f64           bittrack;
  f64           samptrack;

  ogg_stream_state os; /* take physical pages, weld into a logical
                          stream of packets */
  vorbis_dsp_state vd; /* central working state for the packet->PCM decoder */
  vorbis_block     vb; /* local working space for packet->PCM decode */

  ov_callbacks callbacks;

} OggVorbis_File;


extern i32 ov_clear(OggVorbis_File *vf);
extern i32 ov_fopen(const t8 *path,OggVorbis_File *vf);
extern i32 ov_open(FILE *f,OggVorbis_File *vf,const t8 *initial,i64 ibytes);
extern i32 ov_open_callbacks(uk datasource, OggVorbis_File *vf,
                const t8 *initial, i64 ibytes, ov_callbacks callbacks);

extern i32 ov_test(FILE *f,OggVorbis_File *vf,const t8 *initial,i64 ibytes);
extern i32 ov_test_callbacks(uk datasource, OggVorbis_File *vf,
                const t8 *initial, i64 ibytes, ov_callbacks callbacks);
extern i32 ov_test_open(OggVorbis_File *vf);

extern i64 ov_bitrate(OggVorbis_File *vf,i32 i);
extern i64 ov_bitrate_instant(OggVorbis_File *vf);
extern i64 ov_streams(OggVorbis_File *vf);
extern i64 ov_seekable(OggVorbis_File *vf);
extern i64 ov_serialnumber(OggVorbis_File *vf,i32 i);

extern ogg_int64_t ov_raw_total(OggVorbis_File *vf,i32 i);
extern ogg_int64_t ov_pcm_total(OggVorbis_File *vf,i32 i);
extern f64 ov_time_total(OggVorbis_File *vf,i32 i);

extern i32 ov_raw_seek(OggVorbis_File *vf,ogg_int64_t pos);
extern i32 ov_pcm_seek(OggVorbis_File *vf,ogg_int64_t pos);
extern i32 ov_pcm_seek_page(OggVorbis_File *vf,ogg_int64_t pos);
extern i32 ov_time_seek(OggVorbis_File *vf,f64 pos);
extern i32 ov_time_seek_page(OggVorbis_File *vf,f64 pos);

extern i32 ov_raw_seek_lap(OggVorbis_File *vf,ogg_int64_t pos);
extern i32 ov_pcm_seek_lap(OggVorbis_File *vf,ogg_int64_t pos);
extern i32 ov_pcm_seek_page_lap(OggVorbis_File *vf,ogg_int64_t pos);
extern i32 ov_time_seek_lap(OggVorbis_File *vf,f64 pos);
extern i32 ov_time_seek_page_lap(OggVorbis_File *vf,f64 pos);

extern ogg_int64_t ov_raw_tell(OggVorbis_File *vf);
extern ogg_int64_t ov_pcm_tell(OggVorbis_File *vf);
extern f64 ov_time_tell(OggVorbis_File *vf);

extern vorbis_info *ov_info(OggVorbis_File *vf,i32 link);
extern vorbis_comment *ov_comment(OggVorbis_File *vf,i32 link);

extern i64 ov_read_float(OggVorbis_File *vf,f32 ***pcm_channels,i32 samples,
                          i32 *bitstream);
extern i64 ov_read_filter(OggVorbis_File *vf,t8 *buffer,i32 length,
                          i32 bigendianp,i32 word,i32 sgned,i32 *bitstream,
                          z0 (*filter)(f32 **pcm,i64 channels,i64 samples,uk filter_param),uk filter_param);
extern i64 ov_read(OggVorbis_File *vf,t8 *buffer,i32 length,
                    i32 bigendianp,i32 word,i32 sgned,i32 *bitstream);
extern i32 ov_crosslap(OggVorbis_File *vf1,OggVorbis_File *vf2);

extern i32 ov_halfrate(OggVorbis_File *vf,i32 flag);
extern i32 ov_halfrate_p(OggVorbis_File *vf);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

