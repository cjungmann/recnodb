#include "locks.h"

#include <string.h>   // for memset()
#include <fcntl.h>    // for fcntl()  (setting locks)
#include <unistd.h>   // for getpid()
#include <errno.h>

/**
 * Refer to `man 3 fcntl` and `man 3 fileno`
 */
RND_ERROR rnd_lock_area(RNDH *handle,
                        RND_BHANDLE *bhandle,
                        bool retrieve_data,
                        lock_callback callback,
                        void *closure)
{
   prime_handle(handle);
   
   RND_ERROR rval = RND_FAIL;

   int fd = fileno(handle->file);
   struct flock fl;
   memset(&fl, 0, sizeof(fl));
   fl.l_type = F_WRLCK;
   fl.l_whence = SEEK_SET;
   fl.l_start = bhandle->offset;
   fl.l_len = bhandle->size;
   fl.l_pid = getpid();

   if (fcntl(fd, F_SETLK, &fl) == -1)
   {
      if (errno == EAGAIN || errno == EACCES)
         rval = RND_LOCK_FAILED;
      else
      {
         rval = RND_SYSTEM_ERROR;
         handle->sys_errno = errno;
      }
      
      goto abandon_function;
   }

   if (retrieve_data)
   {
      char buffer[bhandle->size];
      off_t saved_offset = ftell(handle->file);
      if (fseek(handle->file, bhandle->offset, SEEK_SET))
      {
         rval = RND_SYSTEM_ERROR;
         handle->sys_errno = errno;
         goto abandon_lock;
      }

      if (!fread(buffer, sizeof(buffer), 1, handle->file))
      {
         rval = RND_LOCK_READ_FAILED;
         goto restore_file_position;
      }

      // Had back to calling function, writing changed
      // buffer if callback returns TRUE:
      if ((*callback)(handle, bhandle, buffer, closure))
      {
         if (fseek(handle->file, bhandle->offset, SEEK_SET)
             || !fwrite(buffer, sizeof(buffer), 1, handle->file))
         {
            rval = RND_UNLOCK_WRITE_FAILED;
            goto restore_file_position;
         }
      }

      rval = RND_SUCCESS;

     restore_file_position:
      fseek(handle->file, saved_offset, SEEK_SET);
   }
   else
      (*callback)(handle, bhandle, NULL, closure);

  abandon_lock:

   fl.l_type = F_UNLCK;
   if (fcntl(fd, F_SETLK, &fl) == -1)
   {
      rval = RND_SYSTEM_ERROR;
      handle->sys_errno = errno;
   }

  abandon_function:
   return rval;
}

RND_ERROR rnd_lock_add_block(int tries)
{
   return RND_SUCCESS;
}

void rnd_unlock_add_block(void)
{
}

RND_ERROR rnd_lock_add_record(int tries)
{
   return RND_SUCCESS;
}

void rnd_unlock_add_record(int tries)
{
}




