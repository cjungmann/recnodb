#include "flatrecs.h"
#include "extra.h"
#include "locks.h"

#include <assert.h>

#include "recnodb.c"
#include "blocks.c"
#include "chains.c"
#include "extra.c"
#include "locks.c"

#include "flatrecs.h"
#include "flatrecs.c"

#include <sys/types.h>   // for stat() in get_file_size()
#include <sys/stat.h>    // for stat() in get_file_size()

char default_file_path[] = "basic3.db";
const char *g_filepath = default_file_path;


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


void test_get_record_capacity(RNDH *handle, RND_HEAD_TABLE *headtable)
{
   uint32_t reccap = flatrecs_get_record_capacity(headtable, (RND_HEAD_BLOCK*)headtable);
   printf("The first block of this table can hold %u records.\n", reccap);

   uint32_t bytes_payload = blocks_block_payload_size((INFO_BLOCK*)headtable);
   uint32_t bytes_leftover = bytes_payload % flatrecs_full_recsize(headtable);
   
   printf("The full block payload size is %u.\n", bytes_payload);
   printf("That will be %u (blocks) * %u (bytes) with %u bytes left over.\n",
          reccap, flatrecs_full_recsize(headtable), bytes_leftover);
}

/**
 */
bool fake_file_table(RNDH           *handle,
                     RND_HEAD_TABLE *head_table,
                     uint32_t       recno,
                     void           *closure)
{
   // Infer BLOCK_LOC value from fact that head_table starts at offset 0:
   BLOCK_LOC bloc = { 0, head_table->bhead.block_size };

   off_t new_offset;
   RND_ERROR rval = flatrecs_make_offset_to_recno(handle, &bloc, head_table, recno, &new_offset);
   if (rval)
   {
      fprintf(stderr, "fake_file_table failed with error: %s.\n", rnd_strerror(rval, handle));
      return 0;
   }
   else
   {
      printf("flatrecs_make_offset_to_recno returned offset %#lx for recno %u.\n", new_offset, recno);
      printf("Virtual distance to the record is %u.\n", flatrecs_full_recsize(head_table) * recno);
      printf("%s filesize is now at %lu.\n", g_filepath, get_file_size(g_filepath));
      return 1;
   }
}

/**
 * Implementation of `flatrecs_use_new_record` function pointer type for `test_get_next_offset`
 * 
 * Upon entering this callback function, we have a handle to a table (and the
 * file, BTW).  It's here that we can start testing various flatrecs functions.
 */
bool callback_for_test_get_next_offset(RNDH           *handle,
                                       RND_HEAD_TABLE *head_table,
                                       off_t          offset_new_record,
                                       void           *closure)
{
   // printf("We got a locked header at %#lx.\n", offset_new_record);
   // printf("The record size is %u.\n", flatrecs_full_recsize(head_table));
   printf("The offset to the next record is %lu.\n", offset_new_record);

   fake_file_table(handle, head_table, 20, closure);
   fake_file_table(handle, head_table, 80, closure);
   fake_file_table(handle, head_table, 1000, closure);
   fake_file_table(handle, head_table, 1500, closure);

   // test_get_record_capacity(handle, head_table);
   return 0;
}


void test_get_next_offset(RNDH *handle, void *closure)
{
   off_t offset_to_table = 0;
   flatrecs_use_new_record callback = callback_for_test_get_next_offset;

   RND_ERROR err = flatrecs_get_next_offset(handle, offset_to_table, callback, NULL);
   if (err)
      fprintf(stderr, "Error with flatrecs_get_next_offset: %s\n", rnd_strerror(err, handle));
}


/**
 * The library calls this callback function with an opened recnodb file.
 * 
 * This is where tests will begin.  Although these test pertain to undeveloped
 * (at the time I'm writing this code), these tests are targeting access to
 * records while adding, reading, etc.
 */
void run_test_user(RNDH *handle, void *closure)
{
   test_get_next_offset(handle, closure);
}

void run_info_test(void)
{
   printf("Size of RND_HEAD_FILE is  %lu.\n"
          "Size of RND_HEAD_TABLE is %lu.\n"
          "Size of RND_HEAD_BLOCK is %lu.\n",
          sizeof(RND_HEAD_FILE),
          sizeof(RND_HEAD_TABLE),
          sizeof(RND_HEAD_BLOCK));
}

int main(int argc, const char **argv)
{
   printf("In `test_flatrecs` application.\n");
   if (argc > 1)
      g_filepath = argv[1];

   run_info_test();

   rnd_open(g_filepath, 0, 0, run_test_user, NULL);

   return 0;
}
