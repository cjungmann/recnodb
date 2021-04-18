#include "recnodb_int.c"

int main(int argc, const char **argv)
{
   unsigned bs = get_blocksize();
   printf("Block size is %u.\n", bs);
   return 0;
}

