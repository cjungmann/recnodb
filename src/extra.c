#include <sys/types.h>
#include <sys/stat.h>

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
