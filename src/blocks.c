/** @file */

#include "recnodb.h"
#include "extra.h"

#include <fcntl.h>
#include <errno.h>
#include <string.h>   // for memset()
#include <assert.h>

/**
 * Ensure that a file header is not blatently corrupt
 *
 * This function is primarily for test programs that construct
 * a bespoke handle, but neglect proper header initialization,
 * but also for confirming that an opened file is a valid *recnodb*
 * database file.
 */
RND_ERROR blocks_validate_head_file(const RND_HEAD_FILE *head_file)
{
   if (!head_file)
      return RND_BAD_PARAMETER;
   else if (
      // INFO_FILE confirmation
      head_file->fhead.chunk_size == 0
      || memcmp(head_file->fhead.magic, "RNDB", 4) != 0
      // INFO_BLOCK confirmation
      || head_file->bhead.block_type == 0
      || head_file->bhead.bytes_to_data == 0
      || head_file->bhead.block_size == 0
      )
   {
      return RND_INVALID_HEAD_FILE;
   }

   return RND_SUCCESS;
}

/**
 * Ensure that a handle is complete
 *
 * This function is primarily for test programs that construct
 * a bespoke handle, but neglect proper header initialization.
 */
RND_ERROR blocks_validate_handle(const RNDH *handle)
{
   if (!handle)
      return RND_BAD_PARAMETER;
   else if (!handle->file)
      return RND_FILE_NOT_OPEN;
   else
      return blocks_validate_head_file(&handle->head_file);
}

/**                                                                   
 * Mnemonic for extracting the chunk_size member of a HEAD_FILE struct
 *
 * @param hf    Pointer to HEAD_FILE header
 *
 * @return chunk_size value
 **********************************************************************************/
uint32_t blocks_head_file_chunk_size(const RND_HEAD_FILE* hf)
{
   return hf->fhead.chunk_size;
}

/**
 * Gets a chunk_size value, returning a default value if
 * the **handle** does not include a chunk_size (for testing).
 *
 * @param handle   pointer to basic RNDH handle
 *
 * @return chunk_size value.
 **********************************************************************************/
uint32_t blocks_handle_chunk_size(const RNDH *handle)
{
   if (handle)
      return blocks_head_file_chunk_size(&handle->head_file);
   else
      return 4096;
}

/**
 * Used for an assertion, detects if new block location preserves the
 * chunk_size multiples set out at file-creation time.
 *
 * @param handle              pointer to RNDH handle with a valid **fhead** member.
 * @param new_block_location  location whose address is to be validated.
 *
 * @return FALSE (0) if invalid, anything other than 0 if TRUE.
 **********************************************************************************/
bool blocks_validate_new_block_location(RNDH *handle, off_t new_block_location)
{
   
   return 0 == (new_block_location % blocks_handle_chunk_size(handle));
}

/**
 * Calculate the bytes_to_data value for the given block type.
 *
 * @param block_type   Block type for which the value is needed.
 *
 * @return Number of bytes from start of block to the start of data.
 *         Alternately, the size of the header information.
 **********************************************************************************/
uint16_t blocks_bytes_to_data(uint16_t block_type)
{
   switch(block_type)
   {
      case RBT_TABLE:
         return sizeof(RND_HEAD_TABLE);
      case RBT_FILE:
         return sizeof(RND_HEAD_FILE);
      default:
         return sizeof(RND_HEAD_BLOCK);
   }
}

/**
 * Returns number of bytes that can be used for data (block-size - size-of-header)
 */
uint32_t blocks_block_payload_size(const INFO_BLOCK *block)
{
   return block->block_size - block->bytes_to_data;
}


/**
 * Simple error-checking, block-reading function.
 *
 * NOTE: This utility function assumes that the calling function will have
 *       called `prime_handle`, and that *handle* is cleared in preparation
 *       setting sys_errno.
 *
 * @param handle     handle to open recnodb database
 * @param offset     offset to the beginning of the block to read
 * @param block      target memory block allocated in the calling function
 * @param block_len  length in bytes of block parameter
 *
 * @return RND_SUCCESS if it works, RND_SYSTEM_ERROR and handle::sys_errno set on failure.
 **********************************************************************************/
RND_ERROR blocks_read_block_head(RNDH *handle, off_t offset, INFO_BLOCK *block, int info_len)
{
   int bytes_to_read = sizeof(INFO_BLOCK);
   if (bytes_to_read > info_len)
      bytes_to_read = info_len;

   if (fseek(handle->file, offset, SEEK_SET)
       || !fread(block, bytes_to_read, 1, handle->file))
   {
      handle->sys_errno = errno;
      return RND_SYSTEM_ERROR;
   }
   else
      return RND_SUCCESS;
}

/**
 * Reads the next block head in a chain
 *
 * @param handle            handle to open recnodb database
 * @param block             pointer to handle from which the search is made
 * @param nextblock         [out] pointer to INFO_BLOCK struct into which the next
 *                          block info will be read.
 * @param nextblock_offset  [out] pointer to offset of `nextblock`
 *
 * @return RND_SUCCESS if the next block was found, an error message otherwise.
 */
RND_ERROR blocks_get_next_block_head(RNDH *handle,
                                     const INFO_BLOCK *block,
                                     INFO_BLOCK *nextblock,
                                     off_t *nextblock_offset)
{
   RND_ERROR rval = RND_REACHED_END_OF_BLOCK_CHAIN;

   if (block->next_block.offset)
   {
      // Save value before fread overwrites *block*
      off_t saved_nextblock_offset = block->next_block.offset;

      if (fseek(handle->file, block->next_block.offset, SEEK_SET)
          || !fread(nextblock, sizeof(INFO_BLOCK), 1, handle->file))
      {
         handle->sys_errno = errno;
         rval = RND_SYSTEM_ERROR;
      }
      else
      {
         *nextblock_offset = saved_nextblock_offset;
         rval = RND_SUCCESS;
      }
   }

   return rval;
}

/**
 * Simple error-checking, block-writing function.
 *
 * Gets appropriate header length from `blocks_bytes_to_data` and uses the
 * smaller of that value or *block_len* as the write length.
 *
 * NOTE: This utility function assumes that the calling function will have
 *       called `prime_handle`, and that *handle* is cleared in preparation
 *       setting sys_errno.
 *
 * @param handle     handle to open recnodb database
 * @param offset     offset to the beginning of the block to write
 * @param block      source memory block allocated in the calling function
 * @param block_len  length in bytes of block parameter
 *
 * @return RND_SUCCESS if it works, RND_SYSTEM_ERROR and handle::sys_errno set on failure.
 **********************************************************************************/
RND_ERROR blocks_write_block_head(RNDH *handle, off_t offset, INFO_BLOCK *block, int info_len)
{
   int len_to_write = blocks_bytes_to_data(block->block_type);
   if (len_to_write < info_len)
      len_to_write = info_len;
   
   if (fseek(handle->file, offset, SEEK_SET)
       || !fwrite(block, len_to_write, 1, handle->file))
   {
      handle->sys_errno = errno;
      return RND_SYSTEM_ERROR;
   }
   else
      return RND_SUCCESS;
}
/**
 * @brief Prepares a block header
 * 
 *
 * @param ib               [out] pointer to INFO_BLOCK memory
 * @param block_head_size  size of parameter *ib*.  This value should be
 *                         acquired with `blocks_bytes_to_data(block_type)`.
 * @param block_type       RBT_GENERIC, RBT_FILE, RBT_TABLE, RBT_DATA
 * @param block_size       size, in bytes, of the block that includes this header
 * @param rec_size         fixed record size if RBT_TABLE or RBT_FILE block type,
 *                         ignored otherwise
 * @param chunk_size       size of the smallest allocation of file 
 **********************************************************************************/
void blocks_set_info_block(INFO_BLOCK *ib,
                           uint32_t block_head_size,
                           BTYPE block_type,
                           unsigned block_size,
                           uint32_t rec_size,
                           uint32_t chunk_size)
{
   memset(ib, 0, block_head_size);
   
   // Calculate header size according to the block_type,
   // in case the caller sent a large block than necessary:
   uint16_t bytes_to_data = blocks_bytes_to_data(block_type);
   assert(bytes_to_data <= block_head_size);

   ib->block_type = block_type;
   ib->bytes_to_data = bytes_to_data;
   ib->block_size = block_size;

   // If includes INFO_TABLE:
   if (bytes_to_data > sizeof(INFO_BLOCK))
   {
      ((RND_HEAD_TABLE*)ib)->thead.rec_size = rec_size;

      // If includes INFO_FILExs
      if (bytes_to_data > sizeof(INFO_TABLE))
      {
         assert(chunk_size);

         ((RND_HEAD_FILE*)ib)->fhead.chunk_size = chunk_size;
         memcpy(((RND_HEAD_FILE*)ib)->fhead.magic, "RNDB", 4);
      }
   }
}

/**
 * Shortcut to for setting info block for blocks_append_block()
 *
 * @param ib               [out] pointer to INFO_BLOCK memory
 * @param block_head_size  size of parameter *ib*.
 * @param bdef             object containing block creation parameters
 **********************************************************************************/
void blocks_set_info_block_struct(INFO_BLOCK *ib,
                                  uint32_t block_head_size,
                                  RND_BLOCK_DEF *bdef)
{
   blocks_set_info_block(ib, block_head_size,
                         bdef->block_type,
                         bdef->block_size,
                         bdef->rec_size,
                         bdef->chunk_size);
}
                           
/**
 * Prepares block head
 *
 * @param hf           [out] pointer to file header that is to be modified
 * @param block_size   size of block this header describes
 * @param chunk_size   multiple of bytes for block sizes
 * @param rec_size     size of fixed-length records.  Eventually, *rec_size=0*
 *                     will indicate variable-length records.
 **********************************************************************************/
 void blocks_prep_head_file(RND_HEAD_FILE *hf,
                           uint32_t block_size,
                           uint32_t chunk_size,
                           uint32_t rec_size)
{
   assert(chunk_size > 0);
   
   memset(hf, 0, sizeof(RND_HEAD_FILE));

   // INFO_BLOCK
   ((INFO_BLOCK*)hf)->block_type = RBT_TABLE;
   ((INFO_BLOCK*)hf)->block_size = block_size;
   ((INFO_BLOCK*)hf)->bytes_to_data = sizeof(RND_HEAD_FILE);

   // INFO_TABLE
   hf->thead.rec_size = rec_size;

   // INFO_FILE
   memcpy(&hf->fhead.magic, "RNDB", 4);
   hf->fhead.chunk_size = chunk_size;
}

/**
 * Extends the file by the specified number of bytes.
 *
 * @param handle        RNDH handle of open database
 * @param bytes_to_add  Number of bytes requested for the new block
 *
 * @return 0 (RND_SUCCESS) if no errors, otherwise error index.
 * 
 * This function makes no effort to ensure exclusive access.
 * That must be done by the calling function to ensure that
 * two processes don't simultaneously attempt to extend the file.
 **********************************************************************************/
RND_ERROR blocks_extend_file(RNDH *handle, size_t bytes_to_add)
{
   assert(blocks_validate_handle(handle) == RND_SUCCESS);
   
   prime_handle(handle);

   RND_ERROR rval = RND_FAIL;

   if (fseek(handle->file, 0, SEEK_END))
   {
      handle->sys_errno = errno;
      rval = RND_SYSTEM_ERROR;
      goto abandon_function;
   }

   // Move to end of file, confirming previous blocks are well-placed
   off_t  new_block_location = ftell(handle->file);
   if (!blocks_validate_new_block_location(handle, new_block_location))
   {
      rval = RND_INVALID_BLOCK_LOCATION;
      goto abandon_function;
   }

   // Write the end-of-block to ensure no problems
   if (fseek(handle->file, bytes_to_add-1, SEEK_END)
       || !fwrite("\0", 1, 1, handle->file)
       || fseek(handle->file, new_block_location, SEEK_SET))
   {
      handle->sys_errno = errno;
      rval = RND_SYSTEM_ERROR;
      goto abandon_function;
   }

   rval = RND_SUCCESS;

  abandon_function:
   return rval;
}


/**
 * Adds a block to the end of the file.
 *
 * This function attempts to add a block to the end of the file.
 * It makes no safety provisions, so locks and links must be made
 * by calling functions.
 *
 * With the exception of the initial RBT_FILE block, if this block is not
 * subseqently linked with a *chains* function, it will be unreachable.
 *
 * @param handle    Open handle
 * @param bdef      Block definition
 *
 * @return RND_ERROR, RND_SUCCESS==0, non-zero indicates an error.
 **********************************************************************************/
RND_ERROR blocks_append_block(RNDH *handle, RND_BLOCK_DEF *bdef)
{
   assert(blocks_validate_handle(handle) == RND_SUCCESS);
   
   prime_handle(handle);
   
   RND_ERROR rval = RND_FAIL;

   // Saving file positions at start and EOF
   off_t original_file_position = 0, new_block_position;
   
   if (-1 == (original_file_position = ftell(handle->file))
       || fseek(handle->file, 0, SEEK_END)
       || -1 == (new_block_position = ftell(handle->file)))
   {
      handle->sys_errno = errno;
      rval = RND_SYSTEM_ERROR;

      if (original_file_position)
         goto restore_file_position;
      else
         goto abandon_function;
   }

   // Extend file
   size_t bytes_to_add = bdef->block_size;
   if ((rval = blocks_extend_file(handle, bytes_to_add)))
      goto restore_file_position;

   // Prepare and write block head of new block.
   // create scope to manage lifetime of blockbuff VLA
   {
      // Prepare new block head
      size_t head_size = blocks_bytes_to_data(bdef->block_type);
      char blockbuff[head_size];
      blocks_set_info_block_struct((INFO_BLOCK*)blockbuff, head_size, bdef);

      if ((rval = blocks_write_block_head(handle, new_block_position, (INFO_BLOCK*)blockbuff, head_size)))
         goto restore_file_position;
   }

   // Everything has worked, prepare return values (rval and [out] data member):
   bdef->new_block.offset = new_block_position;
   bdef->new_block.size = bytes_to_add;
   rval = RND_SUCCESS;
         
  restore_file_position:
   fseek(handle->file, original_file_position, SEEK_SET);

  abandon_function:
   return rval;
}

/**
 * Test to prevent orphaning already linked/allocated block.
 **********************************************************************************/
bool blocks_block_points_to_child_test(INFO_BLOCK *ib)
{
   return ib->next_block.offset != 0;
}

/**
 *
 **********************************************************************************/
void blocks_update_link_with_child(INFO_BLOCK *ib, BLOCK_LOC *new_block)
{
   memcpy(&ib->next_block, new_block, sizeof(BLOCK_LOC));
}

/**
 * Attempts to allocate and initialize a new block and attach to the end of a chain.
 *
 * @param handle     handle to an open recno database
 * @param chain_end  offset to the current terminating link of this chain
 * @param bdef       parameters for block request
 *
 * @return standard library return value
 **********************************************************************************/
RND_ERROR blocks_extend_chain(RNDH *handle, off_t chain_end, RND_BLOCK_DEF *bdef)
{
   prime_handle(handle);

   RND_ERROR rval;
   
   // Add the new block
   rval = blocks_append_block(handle, bdef);
   if (rval)
      goto abandon_function;

   // Scope for VLA blockbuff, cast to INFO_BLOCK* ib
   {
      // Update current end-link to point to new block
      size_t head_size = blocks_bytes_to_data(bdef->block_type);
      char blockbuff[head_size];
      INFO_BLOCK *ib = (INFO_BLOCK*)&blockbuff;
      blocks_set_info_block_struct((INFO_BLOCK*)&blockbuff, head_size, bdef);

      if ((rval = blocks_read_block_head(handle, chain_end, ib, head_size)))
         goto abandon_function;

      if (blocks_block_points_to_child_test(ib))
      {
         rval = RND_ATTEMPT_TO_ORPHAN_BLOCK;
         goto abandon_function;
      }
   
      blocks_update_link_with_child(ib, &bdef->new_block);

      if ((rval = blocks_write_block_head(handle, chain_end, ib, head_size)))
         goto abandon_function;
   }

  abandon_function:
   return rval;
}

/** *******************************************************************************
 * Open file, preparing supplied RNDH handle.
 *
 * @param path       path to database to open
 * @param flags      options for opening database
 * @param chunk_size minimum length for which a file is extended
 * @param rec_size   size of fixed-length record.  Use 0 for variable-length record.
 * @param handle     [in/out] handle that, upon successful return, can be used
 *                   for database operations
 **********************************************************************************/
RND_ERROR blocks_file_open(const char *path,
                           RND_FLAGS flags,
                           uint32_t chunk_size,
                           uint32_t rec_size,
                           RNDH *handle)
{
   prime_handle(handle);
   
   RND_ERROR rval;

   if (!handle)
   {
      rval = RND_BAD_PARAMETER;
      goto abandon_function;
   }
   else
      rval = RND_SUCCESS;

   // Clear any [out] parameters provided
   memset(handle, 0, sizeof(RNDH));

   bool create_mode = (flags & RND_CREATE) != 0;
   const char *fopen_mode = create_mode ? "w+b" : "r+b";
   FILE *f = fopen(path, fopen_mode);
   if (!f)
   {
      handle->sys_errno = errno;
      rval = RND_SYSTEM_ERROR;
      goto abandon_function;
   }

   if (create_mode)
   {
      // Establish first empty block and leave file stream
      // pointer at the beginning of the file
      if (fseek(f, chunk_size-1, SEEK_SET)
          || !fwrite("0", 1, 1, f)
          || fseek(f, 0, SEEK_SET))
      {
         handle->sys_errno = errno;
         rval = RND_SYSTEM_ERROR;
         goto abandon_file;
      }

      // Prepare and write the file header
      blocks_prep_head_file(&handle->head_file, chunk_size, chunk_size, rec_size);
      if (!fwrite(&handle->head_file, sizeof(RND_HEAD_FILE), 1, f))
      {
         handle->sys_errno = errno;
         rval = RND_SYSTEM_ERROR;
         goto abandon_file;
      }
   }
   else
   {
      if (!fread(&handle->head_file, sizeof(RND_HEAD_FILE), 1, f))
      {
         handle->sys_errno = errno; 
         rval = RND_SYSTEM_ERROR;
         goto abandon_file;
      }
   }

   handle->file = f;

   if ((rval = blocks_validate_handle(handle)))
   {
      // rval = RND_INVALID_RECNODB_FILE;
      goto abandon_file;
   }

   assert(rval == RND_SUCCESS);
   goto exit_function;

  abandon_file:
   fclose(f);

  exit_function:
  abandon_function:
   return rval;
}

void blocks_file_close(RNDH *handle)
{
   if (handle && handle->file)
   {
      fclose(handle->file);
      handle->file = NULL;
   }
}


RND_ERROR blocks_file_use(const char *path,
                          RND_FLAGS flags,
                          uint32_t chunk_size,
                          uint32_t rec_size,
                          rnd_user user,
                          void *closure)
{
   // omits `prime_handle` because `blocks_file_open` will call it

   RNDH handle;

   RND_ERROR rval = blocks_file_open(path, flags, chunk_size, rec_size, &handle);
   if (rval == RND_SUCCESS)
   {
      (*user)(&handle, closure);

      blocks_file_close(&handle);
   }

   return rval;
}
