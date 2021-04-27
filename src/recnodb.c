#include "recnodb.h"
#include "recnodb_extra.h"
#include "recnodb_pages.h"

#include <string.h>
#include <errno.h>

/*
 * Prepare an RNDH handle.
 */
EXPORT RND_ERROR rnd_init(RNDH *handle)
{
   memset(handle, 0, sizeof(RNDH));
   return RND_SUCCESS;
}

/*
 * Open a file, creating it if specified in the flags.
 */
EXPORT RND_ERROR rnd_open(RNDH *handle, const char *path, int reclen, RND_FLAGS flags)
{
   RND_ERROR rval = RND_FAIL;

   if (handle->file != NULL)
   {
      rval = RND_FILE_ALREADY_OPEN;
      goto abandon_function;
   }

   FILE *f = fopen(path, "r+b");

   if (!f)
   {
      handle->sys_errno = errno;
      rval = RND_SYSTEM_ERROR;
      goto abandon_function;
   }

   if (flags & RND_CREATE)
      rval = rnd_prepare_new_file(handle, get_blocksize(), reclen);
   else
      rval = rnd_prepare_handle_from_file(handle);

   if (rval == RND_SUCCESS)
      handle->file = f;

  abandon_function:
   return rval;
}

/*
 * Close and clean up the contents of a RNDH handle.
 */
EXPORT RND_ERROR rnd_close(RNDH *handle)
{
   if (handle->file)
   {
      fclose(handle->file);
      handle->file = NULL;
      return RND_SUCCESS;
   }
   else
      return RND_FILE_NOT_OPEN;

}

/*
 * Write some data to the database.
 */
EXPORT RND_ERROR rnd_put(RNDH *handle, RND_RECNO *recno, RND_DATA *data)
{
   return RND_SUCCESS;
}

/*
 * Retrieve data from the database.
 */
EXPORT RND_ERROR rnd_get(RNDH *handle, RND_RECNO recno, RND_DATA *data)
{
   return RND_SUCCESS;
}

/*
 * Delete data from the database.
 */
EXPORT RND_ERROR rnd_delete(RNDH *handle, RND_RECNO recno)
{
   return RND_SUCCESS;
}

