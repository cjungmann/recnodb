#include "recnodb.h"

#include <stdio.h>

#include <errno.h>
#include <string.h>   // for memset()
#include <errno.h>

RND_ERROR rnd_add_block(RNDH *handle,
                        RND_BHANDLE *new_block,
                        int pages_needed,
                        BLOCK_TYPE type,
                        RND_DATA *extra_header,
                        RND_BHANDLE *link)
{
   prime_handle(handle);

   RND_ERROR rval = RND_FAIL;
   int bytes_xfer;

   memset(new_block, 0, sizeof(RND_BHANDLE));

   // Skip everything if you can't get a lock:
   if ((rval = rnd_lock_add_block(5)))
      goto abandon_function;
   
   // Prevent replacing link::next_block if it's already pointing elsewhere
   if (link && link->offset)
   {
      RND_BLOCK_OFFSET link_offset = 0;

      fseek(handle->file, link->offset, SEEK_SET);
      bytes_xfer = fread((void*)&link_offset, sizeof(link_offset), 1, handle->file);
      if (bytes_xfer != sizeof(link_offset))
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
   int bytes_to_add = (pages_needed * handle->page_size);

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

   bytes_xfer = fwrite(&bhead, sizeof(bhead), 1, handle->file);

   if (extra_header)
      bytes_xfer += fwrite(extra_header->data, extra_header->size, 1, handle->file);

   // write out last byte of the block to establish the full block size in the file:
   fseek(handle->file, offset + bytes_to_add - 1, SEEK_SET);
   /* fputc('\0', handle->file); */
   fputc('X', handle->file);

   fflush(handle->file);

   // Set values for return parameter
   new_block->offset = offset;
   new_block->length = bytes_to_add;

   // Update link::next_block if link was provided:
   if (link && link->offset)
   {
      fseek(handle->file, link->offset, SEEK_SET);
      bytes_xfer = fwrite((void*)&offset, sizeof(offset), 1, handle->file);
      if (bytes_xfer != sizeof(offset))
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
   thead.rec_size = rec_size;

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


RND_ERROR rnd_prepare_new_file(RNDH *handle, uint16_t page_size, uint16_t record_size)
{
   prime_handle(handle);

   RND_ERROR rval = RND_FAIL;
   
   handle->page_size = page_size;

   RND_BHANDLE bhandle;
   memset(&bhandle, 0, sizeof(RND_BHANDLE));

   rval = rnd_add_block(handle, (RND_BHANDLE*)&bhandle, BT_FILE, 1, NULL, NULL);
   if (rval)
      goto abandon_function;

   // Set file header and its constituent structs:
   RND_FHEAD fhead;
   memset(&fhead, 0, sizeof(RND_FHEAD));

   // block head settings
   ((RND_BHEAD*)&fhead)->block_type = BT_FILE;
   ((RND_BHEAD*)&fhead)->head_size = sizeof(RND_FHEAD);
   ((RND_BHEAD*)&fhead)->block_size = page_size;
   // Not needed:
   /* ((RND_BHEAD*)&fhead)->next_block = NULL; */

   // table head settings
   fhead.thead.rec_size = record_size;
   // Not needed:
   /* fhead.thead.last_recno = 0; */

   // file head settings
   memcpy(&fhead.magic, "RNDB", 4);
   fhead.page_size = page_size;

   // Write the file head struct to the file:
   fseek(handle->file, 0, SEEK_SET);
   fwrite(&fhead, sizeof(fhead), 1, handle->file);

   rval = RND_SUCCESS;

  abandon_function:
   return rval;
}

RND_ERROR rnd_prepare_handle_from_file(RNDH *handle)
{
   prime_handle(handle);

   RND_ERROR rval = RND_FAIL;

   RND_FHEAD fhead;
   int bytes_xfer;
   fseek(handle->file, 0, SEEK_SET);
   bytes_xfer = fread(&fhead, sizeof(fhead), 1, handle->file);
   if (bytes_xfer != sizeof(fhead))
   {
      handle->sys_errno = errno;
      rval = RND_SYSTEM_ERROR;
      goto abandon_function;
   }

   handle->page_size = fhead.page_size;


  abandon_function:
   return rval;
}

