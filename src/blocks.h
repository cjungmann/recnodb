#ifndef BLOCKS_H
#define BLOCKS_H

#include <stdint.h>   // defines uint32_t, etc
#include <fcntl.h>    // defines off_t

#include "recnodb.h"

typedef enum {
   RBT_UNDEFINED = 0,
   RBT_GENERIC,      //!< Unspecified block type
   RBT_DATA,         //!< Block with free-form data
   RBT_TABLE,        //!< Block with fixed-length records
   RBT_FILE          //!< First block in file, includes INFO_TABLE members
} BTYPE;

/*************************
 * Block-header structures
 ************************/
typedef struct rnd_block_location {
   off_t offset;             //!< Offset in file of referenced block
   off_t size;               //!< Size, in bytes, of referenced block
} BLOCK_LOC;

struct rnd_info_block {
   uint16_t   block_type;    //!< BTYPE enum from above
   uint16_t   bytes_to_data; //!< Offset to data from top of block
   uint32_t   block_size;    //!< Size, in bytes, of block
   BLOCK_LOC  next_block;    //!< Reference to following block (0s if this is the tail)
};

struct rnd_info_table {
   int32_t rec_size;         //!< Record size of this table
   int32_t last_recno;       //!< Record number of last-added record
};

struct rnd_info_file {
   char     magic[4];        //!< "RCNO"
   uint32_t chunk_size;      //!< Minimum-divisible size of newly-allocated file space
};

typedef struct rnd_info_block INFO_BLOCK;
typedef struct rnd_info_table INFO_TABLE;
typedef struct rnd_info_file  INFO_FILE;

typedef struct rnd_info_block RND_HEAD_BLOCK;

typedef struct rnd_head_table {
   INFO_BLOCK  bhead;
   INFO_TABLE  thead;
} RND_HEAD_TABLE;

typedef struct rnd_head_file {
   INFO_BLOCK  bhead;
   INFO_TABLE  thead;
   INFO_FILE   fhead;
} RND_HEAD_FILE;

/************************
 * Block creation structs
 ***********************/
typedef struct rnd_new_block_definition {
   BLOCK_LOC *new_block;      //!< [out] where new block can be found
   unsigned  block_type;      //!< BTYPE enum value
   unsigned  block_size;      //!< Number of bytes in new block
   uint32_t  rec_size;        //!< Number of bytes in fixed-length records
                              //!< (RBT_TABLE and RBT_FILE)
   uint32_t  chunk_size;      //!< Minimum allocation multiplier (RBT_FILE only)
} RND_BLOCK_DEF;

void blocks_set_info_block(INFO_BLOCK *ib,
                           uint32_t block_head_size,
                           unsigned block_type,
                           unsigned block_size,
                           uint32_t rec_size,
                           uint32_t chunk_size);

void blocks_prep_head_file(RND_HEAD_FILE *hf,
                           uint32_t block_size,
                           uint32_t chunk_size,
                           uint32_t rec_size);

RND_ERROR blocks_file_open(const char *path,
                           uint32_t chunk_size,  // Ignored if not flags |= RND_CREATE
                           uint32_t rec_size,    // Ignored if not flags |= RND_CREATE
                           RND_FLAGS flags,
                           rnd_user user);

RND_ERROR blocks_extend_file(RNDH *handle, size_t bytes_to_add);
RND_ERROR blocks_append_block(RNDH *handle, RND_BLOCK_DEF *bdef);


#endif
