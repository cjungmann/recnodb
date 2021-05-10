#include "recnodb.h"
#include "extra.h"

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
EXPORT
RND_ERROR rnd_open_raw(RNDH *handle, const char *path, int reclen, RND_FLAGS flags)
{
   RND_ERROR rval = RND_FAIL;

   if (handle->file != NULL)
   {
      rval = RND_FILE_ALREADY_OPEN;
      goto abandon_function;
   }

   if (handle->fhead == NULL)
   {
      rval = RND_MISSING_FHEAD;
      goto abandon_function;
   }

   const char *open_mode = ((flags & RND_CREATE)!=0 ? "w+b" : "r+b");
   FILE *f = fopen(path, open_mode);

   if (!f)
   {
      handle->sys_errno = errno;
      rval = RND_SYSTEM_ERROR;
      goto abandon_function;
   }

   /* if (rval == RND_SUCCESS) */
      handle->file = f;

   if (flags & RND_CREATE)
      rval = rnd_prepare_new_file(handle, get_blocksize(), reclen);
   else
      rval = rnd_prepare_handle_from_file(handle);

  abandon_function:
   return rval;
}

/*
 * Close and clean up the contents of a RNDH handle.
 */
EXPORT RND_ERROR rnd_close_raw(RNDH *handle)
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

EXPORT
RND_ERROR rnd_open(const char *path, int reclen, RND_FLAGS flags, rnd_user user)
{
   RNDH handle;

   rnd_init(&handle);
   
   RND_ERROR result;
   RND_FHEAD fhead;
   memset(&fhead, 0, sizeof(fhead));

   if (!(result = rnd_open_raw(&handle, path, reclen, flags)))
   {
      (*user)(&handle);

      rnd_close_raw(&handle);
   }

   return result;
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

