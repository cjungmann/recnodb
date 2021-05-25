#include "recnodb.h"
#include "extra.h"

#include <stdio.h>
#include <errno.h>
#include <string.h>

void display_head_table(const RND_HEAD_TABLE *thead)
{
   printf("    bytes to data: %u\n"
          "       block size: %u\n"
          "next block offset: %#lx\n"
          "  next block size: %#lx\n"
          "  CHAIN PARAMETERS\n"
          "     chain offset: %#lx\n"
          "block_penultimate: %#lx\n"
          "       block_last: %#lx\n"
          "  TABLE PARAMETERS\n"
          "         rec size: %u\n"
          "       last recno: %u\n",
          thead->bhead.bytes_to_data,
          thead->bhead.block_size,
          thead->bhead.next_block.offset,
          thead->bhead.next_block.size,

          thead->chead.chain_offset,
          thead->chead.block_penultimate,
          thead->chead.block_last,

          thead->thead.rec_size,
          thead->thead.last_recno);
}

void usage(void)
{
   printf("Test recnodb library interface.\n"
          "\n"
          "testl_recnodb.l [database filename]\n"
          "\n"
          "Using \"bogus.db\" if no database filename specified.\n"
          "\n");
}

void user_open_handle(RNDH *handle, void *closure)
{
   printf("We got a handle!\n");
   display_head_table((RND_HEAD_TABLE*)&handle->head_file);
}

void test_open_library(const char *filename)
{
   RND_ERROR result;
   
   printf("About to test opening database %s.\n", filename);
      
   result = rnd_open(filename, 0, 0, user_open_handle, NULL);
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
