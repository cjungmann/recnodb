#ifndef RECNODB_LOCKS_H
#define RECNODB_LOCKS_H

#include "recnodb.h"
#include "pages.h"

typedef void (*lock_callback)(RNDH *handle, void *closure);

RND_ERROR rnd_lock_area(RNDH *handle,
                        RND_BLOCK_OFFSET offset,
                        RND_BLOCK_LENGTH length,
                        lock_callback callback,
                        void *closure);

RND_ERROR rnd_lock_add_block(int tries);
void rnd_unlock_add_block(void);
RND_ERROR rnd_lock_add_record(int tries);
void rnd_unlock_add_record(int tries);


#endif
