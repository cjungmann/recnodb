#include "recnodb.h"

#include <alloca.h>

#include <fcntl.h>     // for fileno()
#include <sys/stat.h>  // for fstat()

#include <unistd.h>    // for access()

#include "pages.c"
#include "extra.c"
#include "locks.c"
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

void show_block_info(const RND_BHEAD* bhead)
{
   const char *types[] = { "Undefined", "Generic", "File", "Table", "Data" };

   printf("   Block type: %s\n"
          "Bytes to data: %u\n"
          "   Block size: %u\n",
          types[bhead->block_type],
          bhead->bytes_to_data,
          bhead->block_size);
}

void pre_delete_file(const char *filename)
{
   int aval = access(filename, F_OK);
   if (aval == 0)
   {
      if (unlink(filename))
         printf("Failed to delete file %s (%s).\n", filename, strerror(errno));
      else
         printf("Deleted file %s.\n", filename);
   }
}


/**
 * Open/create a file, create two blocks
 */
void simple_create_file_and_blocks(const char *name)
{
   RND_ERROR error;

   pre_delete_file(name);

   // We have to fake a RNDH handle in order to use rnd_add_block()
   RNDH handle;
   rnd_init(&handle);

   RND_FHEAD fhead;
   memset(&fhead, 0, sizeof(fhead));
   memcpy(&fhead.finfo.magic, "RNDH", 4);
   fhead.finfo.chunk_size = get_blocksize();

   handle.fhead = &fhead;

   FILE *f;
   f = fopen(name, "w+b");
   if (f)
   {
      handle.file = f;

      report_file_stats(&handle);

      RND_BHANDLE new_block, second_block;

      error = rnd_add_block(&handle, &new_block, 1, BT_GENERIC, NULL, NULL);
      if (error)
         printf("There was an error with the first block: %s.\n", rnd_strerror(error, &handle));
      else
      {

         printf("\nSuccess with first block .\n");
         report_file_stats(&handle);
         error = rnd_add_block(&handle, &second_block, 1, BT_GENERIC, NULL, &new_block);
         if (error)
            printf("There was an error with the second block: %s.\n", rnd_strerror(error, &handle));
         else
         {
            printf("\nSuccess with second block .\n");
            report_file_stats(&handle);
         }
      }

      fclose(f);
   }
}

void recurse_read_blocks(RNDH *handle, RND_BHANDLE *bhandle)
{
   char buffer[bhandle->size];
   RND_ERROR err = rnd_read_block(handle, bhandle, buffer, sizeof(buffer));
   if (err)
      printf("Failed to read block: %s.\n", rnd_strerror(err, handle));
   else
   {
      RND_BHEAD *bhead = (RND_BHEAD*)&buffer;
      show_block_info(bhead);

      if (bhead->next_block)
         recurse_read_blocks(handle, (RND_BHANDLE*)&bhead->next_block);
   }
}

void read_simple_file_blocks(const char *name)
{
   FILE *f = fopen(name, "r");
   if (f)
   {
      // Prepare an incomplete handle for reading the file
      RNDH handle;
      rnd_init(&handle);
      handle.file = f;

      // BHANDLE for reading the first block
      RND_BHANDLE bhandle = { 0, 4096 };
      recurse_read_blocks(&handle, &bhandle);

      fclose(f);
   }
   else
      printf("Failed to open %s (%d).\n", name, errno);
}
 
void create_valid_file(const char *name)
{
   RND_ERROR error;

   pre_delete_file(name);
   
   RNDH handle;
   rnd_init(&handle);

   RND_FHEAD fhead;
   handle.fhead = &fhead;

   error = rnd_open_raw(&handle, name, 0, RND_CREATE);
   if (error)
      printf("Failed to open %s (%s).\n", name, rnd_strerror(error, &handle));
   else
   {
      printf("Opened %s.\n", name);
      rnd_close_raw(&handle);
   }
}

int main(int argc, const char **argv)
{
   printf("[32;1msimple_create_file_and_blocks.[m\n");
   simple_create_file_and_blocks("bogus.db");

   printf("\n[32;1mread_simple_file_blocks.[m\n");
   read_simple_file_blocks("bogus.db");

   printf("\n[32;1mcreate_valid_file.[m\n");
   create_valid_file("ersatz.db");

   return 0;
}
