#ifndef RECNODB_PAGES_H
#define RECNODB_PAGES_H

#include "extra.h"
#include "recnodb.h"

typedef unsigned int RND_BLOCK_OFFSET;
typedef unsigned int RND_BLOCK_LENGTH;

typedef enum {
   BT_UNDEFINED = 0,
   BT_FILE,
   BT_TABLE,
   BT_DATA
} BLOCK_TYPE;

typedef struct rnd_block_head {
   uint16_t block_type;        // Use values from enum BLOCK_TYPE
   uint16_t head_size;         // used to calculate offset to data
   uint32_t block_size;        // (in bytes) number of pages * page_size
   off_t    next_block;
   off_t    next_block_size;
} RND_BHEAD;

typedef struct rnd_table_head {
   RND_BHEAD bhead;
   uint32_t rec_size;
   uint32_t last_recno;
} RND_THEAD;

typedef struct rnd_file_head {
   RND_THEAD thead;
   char      magic[4];
   uint32_t  page_size;
} RND_FHEAD;

typedef struct rnd_block_handle {
   RND_BLOCK_OFFSET offset;
   RND_BLOCK_LENGTH length;
} RND_BHANDLE;

RND_ERROR rnd_add_block(RNDH *handle,
                        RND_BHANDLE *new_block,
                        int pages_needed,
                        BLOCK_TYPE type,
                        RND_DATA *extra_header,
                        RND_BHANDLE *link);

RND_ERROR rnd_add_table_block(RNDH *handle,
                              RND_BHANDLE *new_block,
                              int rec_size,
                              int pages_needed,
                              RND_BHANDLE *link);

RND_ERROR rnd_add_data_block(RNDH *handle,
                             RND_BHANDLE *new_block,
                             int pages_needed,
                             RND_BHANDLE *link);



RND_ERROR rnd_prepare_new_file(RNDH *handle, uint16_t page_size, uint16_t record_size);
RND_ERROR rnd_prepare_handle_from_file(RNDH *handle);

#endif

