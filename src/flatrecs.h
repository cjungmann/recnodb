#ifndef RECNODB_FLATRECS_H
#define RECNODB_FLATRECS_H

#include "recnodb.h"

RND_ERROR flatrecs_make_offset_to_recno(RNDH            *handle,
                                        const BLOCK_LOC *bloc,
                                        RND_HEAD_TABLE  *htable,
                                        uint32_t        recno,
                                        off_t           *offset_to_record);

/**
 * Function type called by `flatrecs_get_next_offset`
 *
 * @param handle             handle to open recno database
 * @param head_table         pointer to fresh, locked table header
 * @param offset_new_record  file offset to record for writing
 * @param closure            optional pointer to data from calling function
 *
 * @return    Return non-zero (TRUE) if there are changes to the *head_table* param
 *            that must be written back before releasing the lock.  0 (FALSE) assumes
 *            that no changes were made and the lock will be released without writing.
 */
typedef bool (*flatrecs_use_new_record)(RNDH           *handle,
                                        RND_HEAD_TABLE *head_table,
                                        off_t          offset_new_record,
                                        void           *closure);

RND_ERROR flatrecs_get_next_offset(RNDH *handle,
                                   off_t table_head,
                                   flatrecs_use_new_record user,
                                   void *closure);

#endif
