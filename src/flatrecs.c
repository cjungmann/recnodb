#include "flatrecs.h"
#include "extra.h"
#include "locks.h"

#include <assert.h>

typedef char rec_prefix;

typedef struct flatrecs_get_next_offset_locks_closure {
   void                    *caller_closure;
   flatrecs_use_new_record user;
} FGN_CLO;

typedef struct flatrecs_block_dimensions {
   off_t offset_to_data;
   off_t offset_to_end_of_block;
} FBD;

/**
 * Add the prefix size to the payload data for the true record size.
 *
 * @param htable  pointer to table header from which to acquire payload recsize.
 *
 * @return size of record including prefix.
 */
uint32_t flatrecs_full_recsize(const RND_HEAD_TABLE *htable)
{
   return htable->thead.rec_size + sizeof(rec_prefix);
}

/**
 * Calculates number of fixed records that can fit in the data payload of the referenced block.
 *
 * @param htable   pointer to table that defines the record size
 * @param hblock   specific block for which to calculate the record capacity
 *
 * @return number of records that can fit in the data payload
 */
uint32_t flatrecs_get_record_capacity(const RND_HEAD_TABLE *htable, const INFO_BLOCK *hblock)
{
   uint32_t bytes_capacity = hblock->block_size - hblock->bytes_to_data;
   uint32_t recsize = flatrecs_full_recsize(htable);

   if (recsize > 0)
      return bytes_capacity / recsize;
   else
      return -1;
}

/**
 * Get file offset to requested record number, extending the file, if necessary, to
 * accommodate a record past the current end of file.
 *
 * @param htable           pointer to the header information of the table
 * @param recno            record number to which the offset should point
 * @param offset_to_record [out] the offset to which a record can be written
 *
 * @return RND_SUCCESS for success, other RND_ERROR enum value for errors.
 */
RND_ERROR flatrecs_make_offset_to_recno(RNDH *handle,
                                        const BLOCK_LOC *bloc,
                                        RND_HEAD_TABLE *htable,
                                        uint32_t recno,
                                        off_t *offset_to_record)
{
   // Start by invalidating the address
   *offset_to_record = -1;
   RND_ERROR rval = RND_FAIL;

   uint32_t recsize = flatrecs_full_recsize(htable);
   uint32_t start_rec = 1;
   uint32_t rec_capacity, local_index;
   INFO_BLOCK *iblock = (INFO_BLOCK*)htable;
   off_t iblock_offset = bloc->offset;
   
   INFO_BLOCK newblock;

   while (1)
   {
      rec_capacity = flatrecs_get_record_capacity(htable, iblock);
      local_index = recno - start_rec;

      // Considering the current block...
      if (local_index < rec_capacity)
      {
         // If the record is in the block, prepare for successful return:
         *offset_to_record =
            bloc->offset
            + iblock->bytes_to_data
            + (local_index * recsize);

         rval = RND_SUCCESS;
         goto abandon_function;
      }
      else
      {
         // If not in current block, check or create blocks until the top
         // of this loop finds a block that contains the requested record.
         
         // The first record of the next block starts where the current block leaves off:
         start_rec += rec_capacity;

         // Try to use an existing block
         if (!(rval = blocks_get_next_block_head(handle, iblock, &newblock, &iblock_offset)))
         {
           iblock = &newblock;
         }
         else
         {
            if ((rval == RND_REACHED_END_OF_BLOCK_CHAIN))
            {
               // Needing a new block, let's make it as large as necessary to
               // contain the requested recno, even if it's far past last record.
               uint32_t new_bytes_needed = recsize * (recno - start_rec);
               uint32_t chunk_size = handle->head_file.fhead.chunk_size;

               int chunks_to_add = (new_bytes_needed / chunk_size)
                  + (new_bytes_needed % chunk_size ? 1 : 0);

               RND_BLOCK_DEF bdef = { RBT_DATA, chunks_to_add * chunk_size };

               if (!(rval = blocks_extend_chain(handle, iblock_offset, &bdef)))
               {
                  rval = blocks_read_block_head(handle,
                                                bdef.new_block.offset,
                                                &newblock,
                                                sizeof(newblock));

                  if (rval)
                     goto abandon_function;

                  iblock = &newblock;
                  iblock_offset = bdef.new_block.offset;
               }
            }
            else
               // Unexpected error (not RND_REACHED_END_OF_BLOCK_CHAIN), abort:
               goto abandon_function;
         }
      }
   }

  abandon_function:
   return rval;
}


/**
 * Callback for `rnd_lock_area` that is called by `flatrecs_get_next_offset`
 *
 * @param handle         open recno database handle
 * @param bhandle        offset/size of locked memory area
 * @param locked_buffer  contents of locked area
 * @param closure        data passed from calling functions
 *
 * @return  0 (FALSE) to simply release the lock, non-zero (TRUE)
 *          to, before releasing the lock, write the (presumably
 *          changed) contents of `locked_buffer` to the file
 *          location defined by `bhandle`.
 */
bool flatrecs_get_next_offset_lock_callback(RNDH *handle,
                                            BLOCK_LOC *bloc,
                                            void *locked_buffer,
                                            void *closure)
{
   FGN_CLO *clo = (FGN_CLO*)closure;
   RND_HEAD_TABLE *htable = (RND_HEAD_TABLE*)locked_buffer;

   printf("The first record in this block is %lu.\n", htable->bhead.first_recno);
   printf("The last record in this table is %u.\n", htable->thead.last_recno);

   uint32_t new_recno = htable->thead.last_recno + 1;
   off_t new_record_offset = 0;

   RND_ERROR rval = flatrecs_make_offset_to_recno(handle,
                                                  bloc,
                                                  htable,
                                                  new_recno,
                                                  &new_record_offset);

   if (rval)
   {
      // return system error somehow....gotta rethink this.
      return 0;
   }
   else
      return (*clo->user)(handle,
                          (RND_HEAD_TABLE*)locked_buffer,
                          new_record_offset,
                          clo->caller_closure);
}

/**
 * Find a file offset to which a record can be written.
 *
 * @param handle       handle to an open recno database.
 * @param table_head   offset to the block_head of the root of
 *                     a table.  Use 0 for default file table.
 * @param user         Callback function the new record offset is sent.
 * @param closure      Optional memory to be made available to the `user` callback function.
 */
RND_ERROR flatrecs_get_next_offset(RNDH *handle,
                                   off_t table_head,
                                   flatrecs_use_new_record user,
                                   void *closure)
{
   assert(handle && handle->file);
   prime_handle(handle);

   BLOCK_LOC bl = { table_head, sizeof(RND_HEAD_TABLE) };
   FGN_CLO clo = { closure, user };

   return rnd_lock_area(handle, &bl, 1, flatrecs_get_next_offset_lock_callback, &clo);
}

