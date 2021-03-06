#include <assert.h>

#include <sys/types.h>   // for stat()
#include <sys/stat.h>    // for stat()

#include <errno.h>

#include "recnodb.h"
#include "chains.h"

#include "blocks.c"
#include "chains.c"
#include "extra.c"

#define MODE_NEW_OR_TRUNCATE "w+b"
#define MODE_OPEN_EXISTING "r+b"

#pragma pack(push)
#pragma pack(1)
typedef struct local_date {
   int year;
   short month;
   short day;
} Date;

typedef struct person_record {
   char  first_name[20];
   char  middle_initial;
   char  last_name[20];
   Date  birthday;
} Person;
#pragma pack(pop)

/**
 * Set RND_HEAD_FILE members to valid state for testing situations.
 */
void make_bogus_head_file(RND_HEAD_FILE *head_file, int blocksize)
{
   // INFO_FILE information
   head_file->fhead.chunk_size = get_blocksize();
   memcpy(head_file->fhead.magic, "RNDB", 4);

   // INFO_BLOCK confirmation
   head_file->bhead.block_type = RBT_FILE;
   head_file->bhead.bytes_to_data = sizeof(RND_HEAD_FILE);
   head_file->bhead.block_size = blocksize;
}

/**
 * Use system `stat` function to get file size.
 */
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
   // Create nonexistent file or open and truncate existing file:
   FILE *f = fopen(filename, MODE_NEW_OR_TRUNCATE);
   if (f)
   {
      printf("`test_basic_block` opened %s.\n", filename);

      RND_ERROR err;

      RNDH handle = {f};
      // Make sure handle is valid to pass assertions:
      make_bogus_head_file(&handle.head_file, blocksize);
      
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
   FILE *f = fopen(filename, MODE_NEW_OR_TRUNCATE);
   if (f)
   {
      printf("`test_generic_block` opened %s.\n", filename);

      RND_ERROR err;
      int blocksize = 4096;

      RNDH handle = {f};
      // Make sure handle is valid to pass assertions:
      make_bogus_head_file(&handle.head_file, blocksize);

      RND_BLOCK_DEF rbd = { RBT_GENERIC, blocksize };

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
   else
      fprintf(stderr, "`test_generic_block` failed to opened %s (%s).\n", filename, strerror(errno));

   return rval;
}

/**
 * Creates a new, fully-operational recnodb file.
 *
 * @param filename   path of file to be created
 * @return 0 (FALSE) on failure, 1 (TRUE) on success.
 */
bool test_newfile_block(const char *filename)
{
   bool rval = 0;
   FILE *f = fopen(filename, MODE_NEW_OR_TRUNCATE);
   if (f)
   {
      printf("`test_newfile_block` opened %s.\n", filename);

      RND_ERROR err;
      int blocksize = 4096;

      RNDH handle = {f};
      // Make sure handle is valid to pass assertions:
      make_bogus_head_file(&handle.head_file, blocksize);

      RND_BLOCK_DEF rbd = { RBT_FILE, blocksize, sizeof(Person), blocksize };

      err = blocks_append_block(&handle, &rbd);
      if (err)
      {
         fprintf(stderr,
                 "test_newfile_block() error with blocks_append_block (%s)\n",
                 rnd_strerror(err, &handle));
      }
      else
         rval = 1;

      fclose(f);
   }
   else
      fprintf(stderr, "`test_newfile_block` failed to opened %s (%s).\n", filename, strerror(errno));

   return rval;
}

/** */
bool test_blocks_file_open(const char *filename)
{
   RNDH handle;

   RND_ERROR err = blocks_file_open(filename, 0, 0, 0, &handle);
   if (err)
   {
      fprintf(stderr,
              "test_blocks_file_open() error with blocks_file_open (%s)\n",
              rnd_strerror(err, NULL));
      return 0;
   }
   else
   {
      blocks_file_close(&handle);
      return 1;
   }
}

bool test_blocks_file_use(const char *filename, rnd_user user)
{
   RND_ERROR err = blocks_file_use(filename, 0, 0, 0, user, NULL);
   if (err)
   {
      fprintf(stderr,
              "test_block_file_use failed calling block_file_use (%s).\n",
              rnd_strerror(err, NULL));
      return 0;
   }
   else
      return 1;
}
   
void simple_file_user(RNDH *handle, void *closure)
{
   printf("Callback `%s' callback for `test_blocks_file_user.`\n", __func__);
}

void add_block_file_user(RNDH *handle, void *closure)
{
   printf("Callback `%s' callback for `test_blocks_file_user.`\n", __func__);

   RND_BLOCK_DEF bdef = { RBT_DATA, 4096 };
   RND_ERROR rval = blocks_append_block(handle, &bdef);
   if (rval)
   {
      fprintf(stderr, "Failed blocks_append_block (%s).\n", rnd_strerror(rval, handle));
      goto abandon_function;
   }

   RND_HEAD_FILE ifile;
   off_t offset_to_ifile;
   rval = chains_last_link(handle, 0, (INFO_BLOCK*)&ifile, &offset_to_ifile);
   if (rval)
   {
      fprintf(stderr,
              "Failed to find last link of main chain (%s).\n",
              rnd_strerror(rval, handle));
      
      goto abandon_function;
   }

   chains_add_link(handle, offset_to_ifile, &bdef.new_block);
      
  abandon_function:
      return;
}

bool walk_block_user_callback(INFO_BLOCK *ib, off_t offset_to_ip, void *closure)
{
   printf("Callback `%s' callback for `test_blocks_file_user.`\n", __func__);

   printf("Block type is %u\n"
          "Block head size is %u\n"
          "Block size is %u\n"
          "Next block is at %ld for %lu\n",
          (unsigned)ib->block_type,
          (unsigned)ib->bytes_to_data,
          ib->block_size,
          ib->next_block.offset,
          ib->next_block.size);

   return 1;
}

void walk_block_user(RNDH *handle, void *closure)
{
   printf("Callback `%s' callback for `test_blocks_file_user.`\n", __func__);

   chains_walk(handle, 0, walk_block_user_callback, closure);
}

void display_handle_alignment(void)
{
   printf("Since some structs are mapped directly to the file structure,\n"
          "it makes sense to align structs for efficiency.  This function\n"
          "displays some member sizes to reveal where and how much padding\n"
          "is required.\n\n");
   
   printf("Displaying sizes of RNDH members as they pertain to byte-alignment.\n"
          "struct recnodb_handle is %lu bytes\n"
          "                FILE* is %lu bytes\n"
          "struct rnd_head_file* is %lu bytes\n"
          "                  int is %lu bytes\n"
          "              padding is %lu bytes\n"
          "\n",
          sizeof(struct recnodb_handle),
          sizeof(FILE*),
          sizeof(struct rnd_head_file*),
          sizeof(int),
          sizeof(uint32_t));
}


int main(int argc, const char **argv)
{
   display_handle_alignment();

   const char *reusefile = "basic3.db";

   if (test_basic_block("basic.db", 4096)
       && test_generic_block("basic2.db")
       && test_newfile_block(reusefile)                        // this will overwrite existing file
       && test_blocks_file_open(reusefile)                     // reuse file
       && printf("\n[32;1mbegin series of calls to 'test_blocks_file_use`.[m\n")
       && test_blocks_file_use(reusefile, simple_file_user)    // reuse file to test reopening
       && printf("\n[32;1mbegin series of calls, each adding a block to %s.[m\n", reusefile)
       && test_blocks_file_use(reusefile, add_block_file_user) // reuse file to add block
       && test_blocks_file_use(reusefile, add_block_file_user) // reuse file to add block
       && test_blocks_file_use(reusefile, add_block_file_user) // reuse file to add block
       && test_blocks_file_use(reusefile, add_block_file_user) // reuse file to add block
       && printf("\n[32;1mWalking collection of blocks in %s.[m\n", reusefile)
       && test_blocks_file_use(reusefile, walk_block_user)     // reuse file to confirm block info
      )
   {
      printf("All tests were successful.\n");
      return 1;
   }

   return 0;
}

