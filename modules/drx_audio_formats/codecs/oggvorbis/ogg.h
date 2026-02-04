/********************************************************************
 *                                                                  *
 * THIS FILE IS PART OF THE OggVorbis SOFTWARE CODEC SOURCE CODE.   *
 * USE, DISTRIBUTION AND REPRODUCTION OF THIS LIBRARY SOURCE IS     *
 * GOVERNED BY A BSD-STYLE SOURCE LICENSE INCLUDED WITH THIS SOURCE *
 * IN 'COPYING'. PLEASE READ THESE TERMS BEFORE DISTRIBUTING.       *
 *                                                                  *
 * THE OggVorbis SOURCE CODE IS (C) COPYRIGHT 1994-2007             *
 * by the Xiph.Org Foundation http://www.xiph.org/                  *
 *                                                                  *
 ********************************************************************

 function: toplevel libogg include

 ********************************************************************/
#ifndef _OGG_H
#define _OGG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include "os_types.h"

typedef struct {
  uk iov_base;
  size_t iov_len;
} ogg_iovec_t;

typedef struct {
  i64 endbyte;
  i32  endbit;

  u8 *buffer;
  u8 *ptr;
  i64 storage;
} oggpack_buffer;

/* ogg_page is used to encapsulate the data in one Ogg bitstream page *****/

typedef struct {
  u8 *header;
  i64 header_len;
  u8 *body;
  i64 body_len;
} ogg_page;

/* ogg_stream_state contains the current encode/decode state of a logical
   Ogg bitstream **********************************************************/

typedef struct {
  u8   *body_data;    /* bytes from packet bodies */
  i64    body_storage;          /* storage elements allocated */
  i64    body_fill;             /* elements stored; fill mark */
  i64    body_returned;         /* elements of fill returned */


  i32     *lacing_vals;      /* The values that will go to the segment table */
  ogg_int64_t *granule_vals; /* granulepos values for headers. Not compact
                                this way, but it is simple coupled to the
                                lacing fifo */
  i64    lacing_storage;
  i64    lacing_fill;
  i64    lacing_packet;
  i64    lacing_returned;

  u8    header[282];      /* working space for header encode */
  i32              header_fill;

  i32     e_o_s;          /* set when we have buffered the last packet in the
                             logical bitstream */
  i32     b_o_s;          /* set after we've written the initial page
                             of a logical bitstream */
  i64    serialno;
  i64    pageno;
  ogg_int64_t  packetno;  /* sequence number for decode; the framing
                             knows where there's a hole in the data,
                             but we need coupling so that the codec
                             (which is in a separate abstraction
                             layer) also knows about the gap */
  ogg_int64_t   granulepos;

} ogg_stream_state;

/* ogg_packet is used to encapsulate the data and metadata belonging
   to a single raw Ogg/Vorbis packet *************************************/

typedef struct {
  u8 *packet;
  i64  bytes;
  i64  b_o_s;
  i64  e_o_s;

  ogg_int64_t  granulepos;

  ogg_int64_t  packetno;     /* sequence number for decode; the framing
                                knows where there's a hole in the data,
                                but we need coupling so that the codec
                                (which is in a separate abstraction
                                layer) also knows about the gap */
} ogg_packet;

typedef struct {
  u8 *data;
  i32 storage;
  i32 fill;
  i32 returned;

  i32 unsynced;
  i32 headerbytes;
  i32 bodybytes;
} ogg_sync_state;

/* Ogg BITSTREAM PRIMITIVES: bitstream ************************/

extern z0  oggpack_writeinit(oggpack_buffer *b);
extern i32   oggpack_writecheck(oggpack_buffer *b);
extern z0  oggpack_writetrunc(oggpack_buffer *b,i64 bits);
extern z0  oggpack_writealign(oggpack_buffer *b);
extern z0  oggpack_writecopy(oggpack_buffer *b,uk source,i64 bits);
extern z0  oggpack_reset(oggpack_buffer *b);
extern z0  oggpack_writeclear(oggpack_buffer *b);
extern z0  oggpack_readinit(oggpack_buffer *b,u8 *buf,i32 bytes);
extern z0  oggpack_write(oggpack_buffer *b,u64 value,i32 bits);
extern i64  oggpack_look(oggpack_buffer *b,i32 bits);
extern i64  oggpack_look1(oggpack_buffer *b);
extern z0  oggpack_adv(oggpack_buffer *b,i32 bits);
extern z0  oggpack_adv1(oggpack_buffer *b);
extern i64  oggpack_read(oggpack_buffer *b,i32 bits);
extern i64  oggpack_read1(oggpack_buffer *b);
extern i64  oggpack_bytes(oggpack_buffer *b);
extern i64  oggpack_bits(oggpack_buffer *b);
extern u8 *oggpack_get_buffer(oggpack_buffer *b);

extern z0  oggpackB_writeinit(oggpack_buffer *b);
extern i32   oggpackB_writecheck(oggpack_buffer *b);
extern z0  oggpackB_writetrunc(oggpack_buffer *b,i64 bits);
extern z0  oggpackB_writealign(oggpack_buffer *b);
extern z0  oggpackB_writecopy(oggpack_buffer *b,uk source,i64 bits);
extern z0  oggpackB_reset(oggpack_buffer *b);
extern z0  oggpackB_writeclear(oggpack_buffer *b);
extern z0  oggpackB_readinit(oggpack_buffer *b,u8 *buf,i32 bytes);
extern z0  oggpackB_write(oggpack_buffer *b,u64 value,i32 bits);
extern i64  oggpackB_look(oggpack_buffer *b,i32 bits);
extern i64  oggpackB_look1(oggpack_buffer *b);
extern z0  oggpackB_adv(oggpack_buffer *b,i32 bits);
extern z0  oggpackB_adv1(oggpack_buffer *b);
extern i64  oggpackB_read(oggpack_buffer *b,i32 bits);
extern i64  oggpackB_read1(oggpack_buffer *b);
extern i64  oggpackB_bytes(oggpack_buffer *b);
extern i64  oggpackB_bits(oggpack_buffer *b);
extern u8 *oggpackB_get_buffer(oggpack_buffer *b);

/* Ogg BITSTREAM PRIMITIVES: encoding **************************/

extern i32      ogg_stream_packetin(ogg_stream_state *os, ogg_packet *op);
extern i32      ogg_stream_iovecin(ogg_stream_state *os, ogg_iovec_t *iov,
                                   i32 count, i64 e_o_s, ogg_int64_t granulepos);
extern i32      ogg_stream_pageout(ogg_stream_state *os, ogg_page *og);
extern i32      ogg_stream_pageout_fill(ogg_stream_state *os, ogg_page *og, i32 nfill);
extern i32      ogg_stream_flush(ogg_stream_state *os, ogg_page *og);
extern i32      ogg_stream_flush_fill(ogg_stream_state *os, ogg_page *og, i32 nfill);

/* Ogg BITSTREAM PRIMITIVES: decoding **************************/

extern i32      ogg_sync_init(ogg_sync_state *oy);
extern i32      ogg_sync_clear(ogg_sync_state *oy);
extern i32      ogg_sync_reset(ogg_sync_state *oy);
extern i32      ogg_sync_destroy(ogg_sync_state *oy);
extern i32      ogg_sync_check(ogg_sync_state *oy);

extern t8    *ogg_sync_buffer(ogg_sync_state *oy, i64 size);
extern i32      ogg_sync_wrote(ogg_sync_state *oy, i64 bytes);
extern i64     ogg_sync_pageseek(ogg_sync_state *oy,ogg_page *og);
extern i32      ogg_sync_pageout(ogg_sync_state *oy, ogg_page *og);
extern i32      ogg_stream_pagein(ogg_stream_state *os, ogg_page *og);
extern i32      ogg_stream_packetout(ogg_stream_state *os,ogg_packet *op);
extern i32      ogg_stream_packetpeek(ogg_stream_state *os,ogg_packet *op);

/* Ogg BITSTREAM PRIMITIVES: general ***************************/

extern i32      ogg_stream_init(ogg_stream_state *os,i32 serialno);
extern i32      ogg_stream_clear(ogg_stream_state *os);
extern i32      ogg_stream_reset(ogg_stream_state *os);
extern i32      ogg_stream_reset_serialno(ogg_stream_state *os,i32 serialno);
extern i32      ogg_stream_destroy(ogg_stream_state *os);
extern i32      ogg_stream_check(ogg_stream_state *os);
extern i32      ogg_stream_eos(ogg_stream_state *os);

extern z0     ogg_page_checksum_set(ogg_page *og);

extern i32      ogg_page_version(const ogg_page *og);
extern i32      ogg_page_continued(const ogg_page *og);
extern i32      ogg_page_bos(const ogg_page *og);
extern i32      ogg_page_eos(const ogg_page *og);
extern ogg_int64_t  ogg_page_granulepos(const ogg_page *og);
extern i32      ogg_page_serialno(const ogg_page *og);
extern i64     ogg_page_pageno(const ogg_page *og);
extern i32      ogg_page_packets(const ogg_page *og);

extern z0     ogg_packet_clear(ogg_packet *op);


#ifdef __cplusplus
}
#endif

#endif  /* _OGG_H */
