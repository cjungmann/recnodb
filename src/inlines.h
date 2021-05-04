#ifndef RECNO_INLINES_H
#define RECNO_INLINES_H

#define INLINE static inline

/* INLINE RND_BLOCK_TYPE get_block_type(const RND_BHEAD *bh)               { return bh->block_type; } */
/* INLINE void set_block_type(RND_BHEAD *bh, RND_BLOCK_TYPE block_type)    { bh->block_type = block_type; } */

/* INLINE void *get_block_data(RND_BHEAD *bh)      { return (void*)(((char*)bh) + bh->bytes_to_data); } */

/* INLINE RND_BLOCK_SIZE get_block_size(const RND_BHEAD *bh)               { return bh->block_size; } */
/* INLINE void set_block_size(RND_BHEAD *bh, RND_BLOCK_SIZE block_size)    { bh->block_size = block_size; } */

/* INLINE RND_BLOCK_OFFSET get_offset_next_block(const RND_BHEAD *bh)      { return bh->next_block; } */
/* INLINE void set_offset_next_block(RND_BHEAD *bh, RND_BLOCK_OFFSET offset) { bh->next_block = offset; } */

/* INLINE RND_PAGE_SIZE get_page_size(const RND_FHEAD *fh)                 { return fh->page_size; } */
/* INLINE void set_page_size(RND_FHEAD *fh, RND_PAGE_SIZE page_size)       { fh->page_size = page_size; } */

/* INLINE RND_REC_SIZE get_rec_size(const RND_THEAD *th)                   { return th->rec_size; } */
/* INLINE void set_rec_size(RND_THEAD *th, RND_REC_SIZE rec_size)          { th->rec_size = rec_size; } */

INLINE uint32_t thead_last_recno(const RND_THEAD *th)          { return th->tinfo.last_recno; }
INLINE void set_last_recno(RND_THEAD *th, uint32_t last_recno) { th->tinfo.last_recno = last_recno; }




INLINE uint32_t fhead_chunk_size(const RND_FHEAD *fhead) { return fhead->finfo.chunk_size; }
INLINE uint32_t handle_chunk_size(const RNDH *handle)    { return fhead_chunk_size(handle->fhead); }



#endif
