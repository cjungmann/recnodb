#include "recnodb.h"
#include "recnodb_int.h"

#include <sys/types.h>
#include <sys/stat.h>

unsigned get_blocksize(void)
{
   struct stat mystat;
   int result = stat(".", &mystat);
   if (!result)
      return mystat.st_blksize;
   else
      return 0;
}










/* Local Variables: */
/* compile-command: "b=recnodb_int; \*/
/*  cc -Wall -Werror -ggdb        \*/
/*  -std=c99 -pedantic            \*/
/*  -D${b^^}_MAIN -o $b ${b}.c"   \*/
/* End: */

