#ifndef RECNODB_LOCKS_H
#define RECNODB_LOCKS_H

#include "recnodb.h"
#include "pages.h"

// Return TRUE (!=0) to write back contents of locked_buffer.
typedef bool (*lock_callback)(RNDH *handle,
                              RND_BHANDLE *bhandle,
                              void *locked_buffer,
                              void *closure);

RND_ERROR rnd_lock_area(RNDH *handle,
                        RND_BHANDLE *bhandle,
                        bool retrieve_data,
                        lock_callback callback,
                        void *closure);




RND_ERROR rnd_lock_add_block(int tries);
void rnd_unlock_add_block(void);
RND_ERROR rnd_lock_add_record(int tries);
void rnd_unlock_add_record(int tries);


#endif
