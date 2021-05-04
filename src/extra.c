#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>

/* #include "recnodb.h" */
#include "extra.h"

/**
 * Prepare the handle for a new operation.
 */
void prime_handle(RNDH *handle)
{
   handle->sys_errno = 0;
}

uint32_t get_blocksize(void)
{
   struct stat mystat;
   int result = stat(".", &mystat);
   if (!result)
      return mystat.st_blksize;
   else
      return 0;
}

const char *rnd_error_strings[] = {
   "No error",
   "Generic Failure",
   "System Error",
   "Missing FHEAD Parameter",
   "Attempted to Open Already Opened Database",
   "File Not Open",
   "Extinct Record",
   "Attempt to Orphan Block",
   "Lock Failed",
   "Unlock Failed",
   "Lock Read Failed",
   "Unlock Write Failed",
   "Incomplete Read",
   "Incomplete Write"
};

const char *rnd_strerror(RND_ERROR err, RNDH *handle)
{
   if (err == RND_SYSTEM_ERROR)
   {
      if (handle)
         return strerror(handle->sys_errno);
      else if (errno)
         return strerror(errno);
   }
   else if (err < RND_ERROR_LIMIT)
      return rnd_error_strings[err];

   return "Out-of-range Error Number";
}
