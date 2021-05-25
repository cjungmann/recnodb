/** @file */

#ifndef BLOCKS_H
#define BLOCKS_H

#include <stdint.h>   // defines uint32_t, etc
#include <fcntl.h>    // defines off_t

#include "recnodb.h"

typedef enum {
   RBT_UNDEFINED = 0,
   RBT_GENERIC,      /**< Unspecified block type                           */
   RBT_DATA,         /**< Block with free-form data                        */
   RBT_TABLE,        /**< Block with fixed-length records                  */
   RBT_FILE          /**< First block in file, includes INFO_TABLE members */
} BTYPE;

/*************************
 * Block-header structures
 ************************/
typedef struct rnd_block_location {
   off_t offset;             /**< Offset in file of referenced block  */
   off_t size;               /**< Size, in bytes, of referenced block */
} BLOCK_LOC;

struct rnd_info_block {
   uint16_t   block_type;    /**< BTYPE enum from above                                 */
   uint16_t   bytes_to_data; /**< Offset to data from top of block                      */
   uint32_t   block_size;    /**< Size, in bytes, of block                              */
   uint64_t   first_recno;   /**< Record number of first record in this block           */
   BLOCK_LOC  next_block;    /**< Reference to following block (0s if this is the tail) */
};

struct rnd_info_chain {
   off_t chain_offset;        /**< sum of the lengths of all the previous links in this chain   */
                              /**< This value, along with this::block_size, this::rec_size, and */
                              /**< this::last_recno, allows us to calculate the offset to a new */
                              /**< record.                                                      */
   off_t block_penultimate;   /**< If the last block can be extended (because it's the */
                              /**< last block in the file), the second-to-last block   */
                              /**< must be updated with the new block-size.            */
   off_t block_last;          /**< Offset to the last block in the chain. */
};

struct rnd_info_table {
   int32_t rec_size;         /**< Record size of this table          */
   int32_t last_recno;       /**< Record number of last-added record */
};

struct rnd_info_file {
   char     magic[4];        /**< "RCNO"                                               */
   uint32_t chunk_size;      /**< Minimum-divisible size of newly-allocated file space */
};

typedef struct rnd_info_block INFO_BLOCK;
typedef struct rnd_info_chain INFO_CHAIN;
typedef struct rnd_info_table INFO_TABLE;
typedef struct rnd_info_file  INFO_FILE;

typedef struct rnd_info_block RND_HEAD_BLOCK;

typedef struct rnd_head_table {
   INFO_BLOCK  bhead;
   INFO_CHAIN  chead;
   INFO_TABLE  thead;
} RND_HEAD_TABLE;

typedef struct rnd_head_file {
   INFO_BLOCK  bhead;   /**< basic block information                                            */
   INFO_CHAIN  chead;   /**< only needed for first block in a chain                             */
   INFO_TABLE  thead;   /**< All tables are chains, not all chains are table (i.e. data chain)  */
   INFO_FILE   fhead;   /**< Only one file head per file, it's the rarest and thus last element */
} RND_HEAD_FILE;

/** *********************
 * Block creation structs
 ***********************/
typedef struct rnd_new_block_definition {
   BTYPE     block_type;      /**< BTYPE enum value                              */
   unsigned  block_size;      /**< Number of bytes in new block                  */
   uint32_t  rec_size;        /**< Number of bytes in fixed-length records       */
   uint32_t  chunk_size;      /**< Minimum allocation multiplier (RBT_FILE only) */
   BLOCK_LOC new_block;       /**< [out] where new block can be found            */
} RND_BLOCK_DEF;

RND_ERROR blocks_validate_head_file(const RND_HEAD_FILE *head_file);
RND_ERROR blocks_validate_handle(const RNDH *handle);

uint16_t blocks_bytes_to_data(uint16_t block_type);
RND_ERROR blocks_read_block_head(RNDH *handle, off_t offset, INFO_BLOCK *block, int block_len);
RND_ERROR blocks_write_block_head(RNDH *handle, off_t offset, INFO_BLOCK *block, int block_len);


void blocks_set_info_block(INFO_BLOCK *ib,
                           uint32_t block_head_size,
                           BTYPE    block_type,
                           unsigned block_size,
                           uint32_t rec_size,
                           uint32_t chunk_size);

void blocks_prep_head_file(RND_HEAD_FILE *hf,
                           uint32_t block_size,
                           uint32_t chunk_size,
                           uint32_t rec_size);

RND_ERROR blocks_file_open(const char *path,
                           RND_FLAGS flags,
                           uint32_t chunk_size,  // Ignored if not flags |= RND_CREATE
                           uint32_t rec_size,    // Ignored if not flags |= RND_CREATE
                           RNDH *handle);

void blocks_file_close(RNDH *handle);

RND_ERROR blocks_file_use(const char *path,
                          RND_FLAGS flags,
                          uint32_t chunk_size,
                          uint32_t rec_size,
                          rnd_user user,
                          void *closure);

RND_ERROR blocks_extend_file(RNDH *handle, size_t bytes_to_add);
RND_ERROR blocks_append_block(RNDH *handle, RND_BLOCK_DEF *bdef);

RND_ERROR blocks_extend_chain(RNDH *handle, off_t parent, RND_BLOCK_DEF *bref);


#endif
