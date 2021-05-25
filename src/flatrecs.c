#include "flatrecs.h"
#include "extra.h"
#include "locks.h"

#include <assert.h>


typedef struct flatrecs_get_next_offset_locks_closure {
   void                    *caller_closure;
   flatrecs_use_new_record user;
} FGN_CLO;


bool flatrecs_get_next_offset_lock_callback(RNDH *handle,
                                            BLOCK_LOC *bhandle,
                                            void *locked_buffer,
                                            void *closure)
{
   FGN_CLO *clo = (FGN_CLO*)closure;

   // Calculate this currently made-up number:
   off_t   new_record_offset = 0;

   return (*clo->user)(handle, (RND_HEAD_TABLE*)locked_buffer, new_record_offset, clo->caller_closure);
}

RND_ERROR flatrecs_get_next_offset(RNDH *handle,
                                   off_t table_head,
                                   flatrecs_use_new_record user,
                                   void *closure)
{
   assert(handle && table_head);
   prime_handle(handle);

   BLOCK_LOC bl = { table_head, sizeof(RND_HEAD_TABLE) };
   FGN_CLO clo = { closure, user };

   return rnd_lock_area(handle, &bl, 1, flatrecs_get_next_offset_lock_callback, &clo);
}

