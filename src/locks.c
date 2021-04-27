#include "locks.h"

#include <string.h>   // for memset()
#include <fcntl.h>    // for fcntl()  (setting locks)
#include <unistd.h>   // for getpid()
#include <errno.h>

/**
 * Refer to `man 3 fcntl` and `man 3 fileno`
 */
RND_ERROR rnd_lock_area(RNDH *handle,
                        RND_BLOCK_OFFSET offset,
                        RND_BLOCK_LENGTH length,
                        lock_callback callback,
                        void *closure)
{
   prime_handle(handle);
   
   RND_ERROR rval = RND_FAIL;

   int fd = fileno(handle->file);
   struct flock fl;
   memset(&fl, 0, sizeof(fl));
   fl.l_type = F_SETLK;
   fl.l_whence = SEEK_SET;
   fl.l_start = offset;
   fl.l_len = length;
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

   // Let calling function use the lock:
   (*callback)(handle, closure);

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




