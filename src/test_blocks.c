#include <assert.h>

#include <sys/types.h>   // for stat()
#include <sys/stat.h>    // for stat()

#include <errno.h>

#include "recnodb.h"
#include "blocks.c"
#include "extra.c"

off_t get_file_size(const char *filename)
{
   off_t rval = 0;
   struct stat mystat;

   if (stat(filename, &mystat))
      fprintf(stderr, "stat failed (%s)\n", strerror(errno));
   else
      rval = mystat.st_size;

   return rval;
}

bool test_basic_block(const char *filename, int blocksize)
{
   FILE *f = fopen(filename, "w+b");
   if (f)
   {
      RND_ERROR err;
      RNDH handle = {f};
      if ((err = blocks_extend_file(&handle, blocksize)))
      {
         fprintf(stderr, "blocks_extend_file failed (%s)\n", rnd_strerror(err, &handle));
         return 0;
      }

      fclose(f);

      if (get_file_size(filename) != blocksize)
         fprintf(stderr, "test_basic_block file size mismatch.\n");
      else
         return 1;
   }
   else
      fprintf(stderr, "fopen failed (%s).\n", strerror(errno));

   return 0;
}

bool test_generic_block(const char *filename)
{
   bool rval = 0;
   FILE *f = fopen(filename, "w+b");
   if (f)
   {
      RND_ERROR err;

      RNDH handle = {f};
      BLOCK_LOC new_block;
      RND_BLOCK_DEF rbd = { &new_block, RBT_GENERIC, 4096 };

      err = blocks_append_block(&handle, &rbd);
      if (err)
      {
         fprintf(stderr,
                 "test_generic_block() error with blocks_append_block (%s)\n",
                 rnd_strerror(err, &handle));
      }
      else
         rval = 1;
      
      fclose(f);
   }

   return rval;
}

bool test_newfile_block(const char *filename)
{
   bool rval = 0;
   FILE *f = fopen(filename, "w+b");
   if (f)
   {
      RND_ERROR err;

      RNDH handle = {f};
      BLOCK_LOC new_block;
      RND_BLOCK_DEF rbd = { &new_block, RBT_FILE, 4096, 0, 4096 };

      err = blocks_append_block(&handle, &rbd);
      if (err)
      {
         fprintf(stderr,
                 "test_generic_block() error with blocks_append_block (%s)\n",
                 rnd_strerror(err, &handle));
      }
      else
         rval = 1;

      fclose(f);
   }

   return rval;
}


int main(int argc, const char **argv)
{
   if (test_basic_block("basic.db", 4096)
       && test_generic_block("basic2.db")
       && test_newfile_block("basic3.db")
      )
   {
      printf("All tests were successful.\n");
      return 1;
   }

   return 0;
}

