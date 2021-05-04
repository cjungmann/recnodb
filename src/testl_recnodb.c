#include "recnodb.h"

#include <stdio.h>
#include <errno.h>
#include <string.h>

void display_fhead(const RND_FHEAD *fhead)
{
   printf("    bytes to data: %u\n"
          "       block size: %u\n"
          "next block offset: %#lx\n"
          "  next block size: %#lx\n"
          "  TABLE PARAMETERS\n"
          "         rec size: %u\n"
          "       last recno: %u\n",
          fhead->binfo.bytes_to_data,
          fhead->binfo.block_size,
          fhead->binfo.next_block,
          fhead->binfo.next_block_size,
          fhead->tinfo.rec_size,
          fhead->tinfo.last_recno);
}

void user_open_handle(RNDH *handle)
{
   printf("We got a handle!\n");
   display_fhead(handle->fhead);
}

void test_open_library(const char *filename)
{
   RND_ERROR result;
   
   printf("About to test opening database %s.\n", filename);
      
   result = rnd_open(filename, 0, 0, user_open_handle);
   if (result)
   {
      printf("Failed to open (%s).\n", rnd_strerror(result, NULL));
   }
}

int main(int argc, const char **argv)
{
   const char *filename = "bogus.db";
   if (argc > 1)
      filename = argv[1];

   test_open_library(filename);

   printf("Hi mom\n");
   return 0;
}
