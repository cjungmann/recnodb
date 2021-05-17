#ifndef RECNODB_PAGES_H
#define RECNODB_PAGES_H

#include "extra.h"
#include "recnodb.h"

/*******
 * Names for memory areas:
 * chunk    smallest block allocation size
 * block    one or more contiguous chunks
 */


/* typedef off_t    RND_BLOCK_OFFSET; */
/* typedef off_t    RND_BLOCK_SIZE; */

typedef uint32_t RND_BLOCK_OFFSET;
typedef uint32_t RND_BLOCK_SIZE;

typedef uint16_t RND_BLOCK_TYPE;
typedef uint16_t RND_BYTES_TO_DATA;
typedef uint32_t RND_CHUNK_SIZE;
/******************************
 * RND_REC_SIZE and RND_RECNO defined in recnodb.h
 */

typedef struct rnd_block_handle {
   RND_BLOCK_OFFSET offset;
   RND_BLOCK_SIZE size;
} RND_BHANDLE;


/**************
 * File headers
 *************/
typedef enum {
   BT_UNDEFINED = 0,
   BT_GENERIC,
   BT_FILE,
   BT_TABLE,
   BT_DATA
} BLOCK_TYPE;

struct rnd_block_info {
   uint16_t  block_type;
   uint16_t  bytes_to_data;
   uint32_t  block_size;        // (in bytes) number of pages * page_size
   off_t     next_block;
   off_t     next_block_size;
};

struct rnd_table_info {
   uint32_t     rec_size;
   uint32_t     last_recno;
};

struct rnd_file_info {
   char           magic[4];
   uint32_t       chunk_size;
};

struct rnd_table_head {
   struct rnd_block_info binfo;
   struct rnd_table_info tinfo;
};

struct rnd_file_extra {
   struct rnd_table_info tinfo;
   struct rnd_file_info  finfo;
};

struct rnd_file_head {
   struct rnd_block_info binfo;
   struct rnd_table_info tinfo;
   struct rnd_file_info  finfo;
};

typedef struct rnd_block_info RND_BHEAD;
typedef struct rnd_table_head RND_THEAD;
typedef struct rnd_file_head  RND_FHEAD;

/***
 * The following types are used with **rnd_add_block()** to
 * complete the new block's header information.
 */
typedef struct rnd_table_head RND_TABLE_EXTRA;
typedef struct rnd_file_extra RND_FILE_EXTRA;

typedef struct rnd_get_block_request {
   RND_BHANDLE *new_block;
   BLOCK_TYPE  type;
   int         chunks_needed;
   RND_DATA    *extra_header;
} RND_NEW_BLOCK_REQ;

typedef struct rnd_append_block_closure {
   RND_BHANDLE *new_block;
   BLOCK_TYPE  type;
   int         chunks_needed;
   RND_DATA    *extra_header;
   RND_BHANDLE *chain_end;
   RND_ERROR   result;
} RND_BLOCK_REQ;

static inline RND_BHANDLE* get_bhandle_from_BHEAD(struct rnd_block_info *bi)
{ return (RND_BHANDLE*)bi->next_block; }


#include "inlines.h"

RND_ERROR rnd_add_block(RNDH *handle,
                        RND_BHANDLE *new_block,
                        int chunks_needed,
                        BLOCK_TYPE type,
                        RND_DATA *extra_header,
                        RND_BHANDLE *link);

RND_ERROR rnd_add_block_request(RNDH *handle, RND_BLOCK_REQ *breq);

RND_ERROR rnd_add_table_block(RNDH *handle,
                              RND_BHANDLE *new_block,
                              int rec_size,
                              int pages_needed,
                              RND_BHANDLE *link);

RND_ERROR rnd_add_data_block(RNDH *handle,
                             RND_BHANDLE *new_block,
                             int pages_needed,
                             RND_BHANDLE *link);

RND_ERROR rnd_read_block(RNDH *handle, RND_BHANDLE *bhandle, void *buffer, int buffer_size);
 


RND_ERROR rnd_prepare_new_file(RNDH *handle, uint32_t chunk_size, uint16_t record_size);
RND_ERROR rnd_prepare_handle_from_file(RNDH *handle);

#endif

