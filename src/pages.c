#include "recnodb.h"
#include "pages.h"

#include <stdio.h>

#include <errno.h>
#include <string.h>   // for memset()
#include <errno.h>

RND_ERROR rnd_get_new_block(RNDH *handle,
                            RND_BHANDLE *new_block,
                            size_t bytes_needed,

bool add_block_callback(RNDH *handle,
                        RND_BHANDLE *bhandle,
                        void *locked_buffer,
                        void *closure)
{
   RND_BLOCK_REQ *bref = (RND_BLOCK_REQ*)closure;
   RND_BHEAD *bhead_chain_head = (RND_BHEAD*)locked_buffer;

   

}

RND_ERROR rnd_append_block_link(RNDH *handle, RND_BLOCK_REQ *breq)
{
   prime_handle(handle);
   RND_BHANDLE *chain_end = &breq->chain_end;

   RND_ERROR err = rnd_lock_area(handle, chain_end, 1, add_block_callback, breq);

   return err ? err : breq->result;
}

/**
 * Add a block to a chain of blocks
 */
RND_ERROR rnd_add_block(RNDH *handle,
                        RND_BHANDLE *new_block,
                        int pages_needed,
                        BLOCK_TYPE type,
                        RND_DATA *extra_header,
                        RND_BHANDLE *link)
{
   prime_handle(handle);

   RND_ERROR rval = RND_FAIL;
   int items_read;

   memset(new_block, 0, sizeof(RND_BHANDLE));

   // Skip everything if you can't get a lock:
   if ((rval = rnd_lock_add_block(5)))
      goto abandon_function;
   
   // Prevent replacing link::next_block if it's already pointing elsewhere
   if (link && link->size)
   {
      RND_BLOCK_OFFSET link_offset = 0;

      fseek(handle->file, link->offset, SEEK_SET);
      items_read = fread((void*)&link_offset, sizeof(link_offset), 1, handle->file);
      if (items_read == 0)
      {
         handle->sys_errno = errno;
         rval = RND_SYSTEM_ERROR;
         goto abandon_new_block_lock;
      }
      else if (link_offset != 0)
      {
         rval = RND_ATTEMPT_TO_ORPHAN_BLOCK;
         goto abandon_new_block_lock;
      }
   }

   // Add the new block
   int bytes_to_add = (pages_needed * handle_chunk_size(handle));

   if (fseek(handle->file, 0, SEEK_END) == -1)
   {
      handle->sys_errno = errno;
      rval = RND_SYSTEM_ERROR;
      goto abandon_new_block_lock;
   }

   RND_BLOCK_OFFSET offset = (RND_BLOCK_OFFSET)ftell(handle->file);

   // Prepare and write out block header, and extra_header, if included
   RND_BHEAD bhead;
   bhead.block_type = type;
   bhead.block_size = bytes_to_add;
   bhead.next_block = 0;
   bhead.next_block_size = bytes_to_add;

   if (!fwrite(&bhead, sizeof(bhead), 1, handle->file))
   {
      rval = RND_INCOMPLETE_WRITE;
      goto abandon_new_block_lock;
   }

   if (extra_header)
   {
      if (!fwrite(extra_header->data, extra_header->size, 1, handle->file))
      {
         rval = RND_INCOMPLETE_WRITE;
         goto abandon_new_block_lock;
      }
   }


   // write out last byte of the block to establish the full block size in the file:
   fseek(handle->file, offset + bytes_to_add - 1, SEEK_SET);
   /* fputc('\0', handle->file); */
   fputc('X', handle->file);

   fflush(handle->file);

   // Set values for return parameter
   new_block->offset = offset;
   new_block->size = bytes_to_add;

   // Update link::next_block if link was provided:
   if (link && link->size)
   {
      fseek(handle->file, link->offset, SEEK_SET);
      if (!fwrite((void*)&offset, sizeof(offset), 1, handle->file))
      {
         handle->sys_errno = errno;
         rval = RND_SYSTEM_ERROR;
         goto abandon_new_block_lock;
      }
   }

  abandon_new_block_lock:
   rnd_unlock_add_block();
   
  abandon_function:
   return rval;
}

RND_ERROR rnd_add_table_block(RNDH *handle,
                              RND_BHANDLE *new_block,
                              int rec_size,
                              int pages_needed,
                              RND_BHANDLE *link)
{
   RND_THEAD thead;
   memset(&thead, 0, sizeof(thead));
   thead.tinfo.rec_size = rec_size;

   RND_DATA data;
   memset(&data, 0, sizeof(data));
   data.data = &thead;
   data.size = sizeof(thead);

   return rnd_add_block(handle, new_block, pages_needed, BT_TABLE, &data, link);
}

RND_ERROR rnd_add_data_block(RNDH *handle,
                             RND_BHANDLE *new_block,
                             int pages_needed,
                             RND_BHANDLE *link)
{
   return rnd_add_block(handle, new_block, pages_needed, BT_DATA, NULL, link);
}

RND_ERROR rnd_read_block(RNDH *handle, RND_BHANDLE *bhandle, void *buffer, int buffer_size)
{
   RND_ERROR rval = RND_FAIL;
   prime_handle(handle);
   if (!handle->file)
   {
      rval = RND_FILE_NOT_OPEN;
      goto abandon_function;
   }

   if (fseek(handle->file, bhandle->offset, SEEK_SET))
   {
      handle->sys_errno = errno;
      rval = RND_SYSTEM_ERROR;
      goto abandon_function;
   }

   size_t bytes_to_read = buffer_size < bhandle->size ? buffer_size : bhandle->size;
   size_t items_read;
   items_read = fread(buffer, bytes_to_read, 1, handle->file);

   if (items_read == 1)
      rval = RND_SUCCESS;
   else
      rval = RND_INCOMPLETE_READ;

  abandon_function:
   return rval;
}

RND_ERROR rnd_prepare_new_file(RNDH *handle, uint32_t chunk_size, uint16_t record_size)
{
   prime_handle(handle);

   RND_ERROR rval = RND_FAIL;

   RND_FILE_EXTRA file_extra;
   file_extra.tinfo.rec_size = record_size;
   file_extra.tinfo.last_recno = 0;

   memset(&file_extra, 0, sizeof(file_extra));
   memcpy(&file_extra.finfo.magic, "RNDH", 4);
   file_extra.finfo.chunk_size = chunk_size;
   
   RND_BHANDLE bhandle;
   memset(&bhandle, 0, sizeof(RND_BHANDLE));

   RND_DATA fextra;
   fextra.data = &fextra;
   fextra.size = sizeof(fextra);

   rval = rnd_add_block(handle, (RND_BHANDLE*)&bhandle, BT_FILE, 1, &fextra, NULL);
   if (rval)
      goto abandon_function;

   /* // Set file header and its constituent structs: */
   /* RND_FHEAD *fhead = handle->fhead; */
   /* memset(&fhead, 0, sizeof(RND_FHEAD)); */

   /* // block head settings */
   /* fhead->binfo.block_type = BT_FILE; */
   /* fhead->binfo.head_size = sizeof(RND_FHEAD); */
   /* fhead->binfo.block_size = chunk_size;       // initially at least, file block is a single chunk */
   /* fhead->binfo.next_block = 0; */
   /* fhead->binfo.next_block_size = 0; */

   /* // table head settings */
   /* fhead->thead.rec_size = record_size; */
   /* fhead->thead.last_recno = 0; */

   /* // file head settings */
   /* memcpy(&fhead->magic, "RNDB", 4); */
   /* fhead->page_size = page_size; */

   /* // Write the file head struct to the file: */
   /* fseek(handle->file, 0, SEEK_SET); */
   /* fwrite(&fhead, sizeof(RND_FHEAD), 1, handle->file); */

   rval = RND_SUCCESS;

  abandon_function:
   return rval;
}

RND_ERROR rnd_prepare_handle_from_file(RNDH *handle)
{
   prime_handle(handle);

   RND_ERROR rval = RND_FAIL;

   int bytes_xfer;
   fseek(handle->file, 0, SEEK_SET);
   bytes_xfer = fread(handle->fhead, 1, sizeof(RND_FHEAD), handle->file);
   if (bytes_xfer == sizeof(RND_FHEAD))
   {
      rval = RND_SUCCESS;
   }
   else
   {
      rval = RND_SYSTEM_ERROR;
      handle->sys_errno = errno;
   }

   return rval;
}


