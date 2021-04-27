#ifndef RECNO_INLINES_H
#define RECNO_INLINES_H

static inline uint16_t get_block_type(RND_BHEAD *bh)                  { return bh->block_type; }
static inline void set_block_type(RND_BHEAD *bh, uint16_t block_type) { bh->block_type = block_type; }

static inline uint16_t get_head_size(RND_BHEAD *bh)                   { return bh->head_size; }
static inline void set_head_size(RND_BHEAD *bh, uint16_t head_size)   { bh->head_size = head_size; }

static inline uint32_t get_block_size(RND_BHEAD *bh)                  { return bh->block_size; }
static inline void set_block_size(RND_BHEAD *bh, uint32_t block_size) { bh->block_size = block_size; }

static inline off_t get_offset_next_block(RND_BHEAD *bh)              { return bh->next_block; }
static inline void set_offset_next_block(RND_BHEAD *bh, off_t offset) { bh->next_block = offset; }

static inline uint32_t get_page_size(RND_FHEAD *fh)                   { return fh->page_size; }
static inline void set_page_size(RND_FHEAD *fh, uint32_t page_size)   { fh->page_size = page_size; }

static inline uint32_t get_rec_size(RND_THEAD *th)                    { return th->rec_size; }
static inline void set_rec_size(RND_THEAD *th, uint32_t rec_size)     { th->rec_size = rec_size; }

static inline uint32_t get_last_recno(RND_THEAD *th)                  { return th->last_recno; }
static inline void set_last_recno(RND_THEAD *th, uint32_t last_recno) { th->last_recno = last_recno; }

#endif
