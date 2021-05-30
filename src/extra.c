#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>

/* #include "recnodb.h" */
#include "extra.h"

/**
 * Clears *handle::sys_errno* to prepare the handle for a new operation.
 *
 * @param handle   handle to an open recnodb file.
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
   "Invalid RecNoDB file",
   "Bad Parameter",
   "Missing FHEAD Parameter",
   "Attempted to Open Already Opened Database",
   "File Not Open",
   "Extinct Record",
   "Attempt to Orphan Block",
   "Reached End of Block Chain",
   "Lock Failed",
   "Unlock Failed",
   "Lock Read Failed",
   "Unlock Write Failed",
   "Incomplete Read",
   "Incomplete Write",
   "Invalid Block Size",
   "Invalid Block Location",
   "Invalid File Head"
};

/**
 * Returns a const char* to an error string from the appropriate source.
 *
 * Except for RND_SYSTEM_ERROR, which returns the string from system function
 * `strerror`, returns a string corresponding to the error value, to be used
 * in more user-friendly error messages.
 *
 * @param err    library error code
 * @param handle handle in use during the error, which may have sys_errno set.
 *
 * @return       const char * to a string describing the error.
 */
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
