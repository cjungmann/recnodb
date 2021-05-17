#ifndef RECNODB_CHAINS_H
#define RECNODB_CHAINS_H

typedef bool (*block_walk_view)(INFO_BLOCK *ib, off_t offset_to_ib, void *closure);

RND_ERROR chains_add_link(RNDH *handle, off_t parent, BLOCK_LOC *new_link);
RND_ERROR chains_walk(RNDH *handle, off_t block, block_walk_view viewer, void *closure);

RND_ERROR chains_last_link(RNDH *handle, off_t link, INFO_BLOCK *ib, off_t *offset_to_ib);

#endif
