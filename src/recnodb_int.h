#ifndef RECNODB_INT_H
#define RECNODB_INT_H

typedef struct rnd_page_header {
   off_t    next_page;
   uint16_t header_size;
   uint16_t page_size;
} RND_PHEAD;

typedef struct rnd_file_header {
   RND_PHEAD phead;
   uint32_t magic;
   uint8_t  magic_zero;
   uint16_t rec_size;
   uint32_t next_recno;
} RND_FHEAD;

unsigned get_blocksize(void);


#endif
