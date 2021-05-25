#include "recnodb.h"
#include "locks.h"
#include "blocks.h"
#include "extra.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

typedef void (*file_user)(RNDH *handle, void *closure);

void open_lock_file(const char *name, int file_length, file_user user, void *closure)
{
   const char *open_mode = file_length ? "w+" : "r+";
   printf("About to open %s in %s mode.\n", name, open_mode);
   FILE *f = fopen(name, open_mode);
   if (f)
   {
      if (file_length)
      {
         
         if (fseek(f, file_length-1, SEEK_SET))
         {
            printf("fseek failed (%s)\n", strerror(errno));
            goto abandon_function;
         }

         fwrite("X", 1, 1, f);
         fflush(f);
      }

      RNDH handle;
      rnd_init(&handle);

      (*user)(&handle, closure);

      fclose(f);
   }
   else
      printf("Failed to open\n");

  abandon_function:
   return;
}

/* Writing lock test, next two funtions */
bool test_writing_block_lock_user(RNDH *handle, BLOCK_LOC *bhandle, void *locked_buffer, void *closure)
{
   printf("writing to the buffer.\n");
   memcpy(locked_buffer, "0123456789", 10);

   // Return 1 (!=0), notification to rn_lock_area() to write-back the buffer
   return 1;
}

void test_block_lock_writing(RNDH *handle, void *closure)
{
   BLOCK_LOC bhandle = {16, 10};
   RND_ERROR err;
   err = rnd_lock_area(handle, &bhandle, 1, test_writing_block_lock_user, closure);
   if (err)
      printf("failed writing to lock area: %s.\n", rnd_strerror(err, handle));
}

/* Non-writing lock test, next two functions */
bool test_nonwriting_block_lock_user(RNDH *handle, BLOCK_LOC *bhandle, void *locked_buffer, void *closure)
{
   printf("reading from the buffer.\n");
   char *ptr = (char*)locked_buffer;
   char *end = ptr + bhandle->size;

   while (ptr < end)
   {
      printf("%2x ", *ptr);
      ++ptr;
   }

   // Return 0 to abstain from writing
   return 0;
}

void test_block_lock_nonwriting(RNDH *handle, void *closure)
{
   BLOCK_LOC bhandle = {16, 10};
   RND_ERROR err;
   err = rnd_lock_area(handle, &bhandle, 1, test_nonwriting_block_lock_user, closure);
   if (err)
      printf("failed nonwriting lock area: %s.\n", rnd_strerror(err, handle));
}



int main(int argc, const char **argv)
{
   open_lock_file("lock_test.dat", 4096, test_block_lock_writing, NULL);
   open_lock_file("lock_test.dat", 0, test_block_lock_nonwriting, NULL);
   return 0;
}
