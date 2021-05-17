#include "recnodb.h"
#include "blocks.h"
#include "chains.h"

#include <errno.h>
#include <string.h>

RND_ERROR chains_add_link(RNDH *handle, off_t parent, BLOCK_LOC *new_link)
{
   RND_ERROR rval = RND_SUCCESS;
      
   // Read current info from parent link
   INFO_BLOCK ib_parent;
   if (fseek(handle->file, parent, SEEK_SET)
       || !fread(&ib_parent, sizeof(INFO_BLOCK), 1, handle->file))
   {
      rval = RND_SYSTEM_ERROR;
      handle->sys_errno = errno;
      goto abandon_function;
   }

   // Abort process if parent link is already in use
   if (ib_parent.next_block.offset != 0)
   {
      rval = RND_ATTEMPT_TO_ORPHAN_BLOCK;
      goto abandon_function;
   }

   // update parent link with new block information
   memcpy(&ib_parent.next_block, new_link, sizeof(BLOCK_LOC));

   // Write back
   if (fseek(handle->file, parent, SEEK_SET)
       || !fwrite(&ib_parent, sizeof(INFO_BLOCK), 1, handle->file))
   {
      rval = RND_SYSTEM_ERROR;
      handle->sys_errno = errno;
      goto abandon_function;
   }

  abandon_function:
   return rval;
}

RND_ERROR chains_walk(RNDH *handle, off_t block, block_walk_view viewer, void *closure)
{
   RND_ERROR rval = RND_SUCCESS;
   FILE *file = handle->file;

   char buffer[sizeof(RND_HEAD_FILE)];

   INFO_BLOCK *curblock = (INFO_BLOCK*)buffer;
   off_t      off_block = block;

   if (fseek(file, off_block, SEEK_SET)
       || !fread(curblock, sizeof(RND_HEAD_FILE), 1, file))
   {
      rval = RND_SYSTEM_ERROR;
      handle->sys_errno = errno;
      goto abandon_function;
   }

   while (1)
   {
      if (!(*viewer)((INFO_BLOCK*)curblock, off_block, closure))
         break;

      // Save the next address because curblock will be overwritten:
      off_block = curblock->next_block.offset;

      if (!off_block)
         break;

      if (fseek(file, off_block, SEEK_SET)
          || !fread(curblock, sizeof(RND_HEAD_FILE), 1, file))
      {
         rval = RND_SYSTEM_ERROR;
         handle->sys_errno = errno;
         goto abandon_function;
      }
   }

  abandon_function:
   return rval;
   
}

struct chains_last_link_closure {
   INFO_BLOCK *ib;
   off_t      *offset_to_ib;
};

bool chains_last_link_viewer(INFO_BLOCK *ib, off_t offset_to_ib, void *closure)
{
   if (ib && ib->next_block.offset == 0)
   {
      struct chains_last_link_closure *cllc = (struct chains_last_link_closure*)closure;

      // Save view info to closure members
      memcpy(cllc->ib, ib, ib->bytes_to_data);
      *cllc->offset_to_ib = offset_to_ib;
      
      return 0;
   }
   else
      return 1;
}

RND_ERROR chains_last_link(RNDH *handle, off_t link, INFO_BLOCK *ib, off_t *offset_to_ib)
{
   struct chains_last_link_closure cllc = { ib, offset_to_ib };
   
   return chains_walk(handle, link, chains_last_link_viewer, &cllc);
}
