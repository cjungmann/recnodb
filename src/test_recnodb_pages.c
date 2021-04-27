#include "recnodb.h"

#include <alloca.h>

#include <fcntl.h>     // for fileno()
#include <sys/stat.h>  // for fstat()

#include "recnodb_pages.c"
#include "recnodb_extra.c"
#include "recnodb_locks.c"
#include "recnodb.c"

void report_file_stats(RNDH *handle)
{
   int fno = fileno(handle->file);
   if (fno < 0)
   {
      printf("fileno failed, for some unfathomable reason.\n");
      goto abandon_function;
   }

   struct stat mystat;
   fstat(fno, &mystat);

   printf("Size, in bytes, of file: %ld.\n"
          "Size, in bytes, of a single block: %ld.\n"
          "Size, in number of 512B blocks: %ld.\n",
          mystat.st_size,
          mystat.st_blksize,
          mystat.st_blocks);

     abandon_function:
      return;
}

/**
 * Open/create a file, create two blocks
 */
void simple_create_file_blocks(const char *name)
{
   RND_ERROR error;
   RNDH handle;
   rnd_init(&handle);

   FILE *f;
   f = fopen(name, "w+b");
   if (f)
   {
      handle.file = f;
      handle.page_size = get_blocksize();

      report_file_stats(&handle);

      RND_BHANDLE new_block, second_block;

      error = rnd_add_block(&handle, &new_block, 1, BT_UNDEFINED, NULL, NULL);
      if (error)
         printf("There was an error with the first block: %d.\n", error);
      else
      {

         printf("\nSuccess with first block .\n");
         report_file_stats(&handle);
         error = rnd_add_block(&handle, &second_block, 1, BT_UNDEFINED, NULL, &new_block);
         if (error)
            printf("There was an error with the second block: %d.\n", error);
         else
         {
            printf("\nSuccess with second block .\n");
            report_file_stats(&handle);
         }
      }

      fclose(f);
   }
}

void create_valid_file(const char *name)
{
   RND_ERROR error;
   
   RNDH handle;
   rnd_init(&handle);

   error = rnd_open(&handle, name, 0, RND_CREATE);
   if (!error)
      printf("Failed to open %s.\n", name);
   else
   {



      rnd_close(&handle);
   }
      
}

int main(int argc, const char **argv)
{
   simple_create_file_and_blocks("bogus.db");
   create_valid_file("ersatz.db");

   return 0;
}
