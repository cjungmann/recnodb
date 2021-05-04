#ifndef RECNODB_H
#define RECNODB_H

#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>   // for off_t typedef

#define EXPORT __attribute__((visibility("default")))

typedef uint32_t RND_REC_SIZE;
typedef uint32_t RND_RECNO;
typedef int boolean, bool;

/* typedef uint64_t RND_REC_SIZE; */
/* typedef uint64_t RND_RECNO; */

typedef struct recnodb_handle RNDH;

typedef enum {
   RND_SUCCESS = 0,
   RND_FAIL,
   RND_SYSTEM_ERROR,
   RND_MISSING_FHEAD,
   RND_FILE_ALREADY_OPEN,
   RND_FILE_NOT_OPEN,
   RND_EXTINCT_RECORD,
   RND_ATTEMPT_TO_ORPHAN_BLOCK,
   RND_LOCK_FAILED,
   RND_UNLOCK_FAILED,
   RND_LOCK_READ_FAILED,
   RND_UNLOCK_WRITE_FAILED,
   RND_INCOMPLETE_READ,
   RND_INCOMPLETE_WRITE,
   RND_ERROR_LIMIT
} RND_ERROR;

typedef enum {
   RND_CREATE = 1,
   RND_READONLY = 2
} RND_FLAGS;

typedef struct recnodb_data {
   void         *data;
   RND_REC_SIZE size;
   char         pad[4];
} RND_DATA;

struct rnd_file_head;

struct recnodb_handle {
   FILE        *file;
   int         sys_errno;
   
   struct rnd_file_head   *fhead;
};

#include "pages.h"
#include "locks.h"

typedef void (*rnd_user)(RNDH *handle);

RND_ERROR rnd_open(const char *path, int reclen, RND_FLAGS flags, rnd_user user);

RND_ERROR rnd_init(RNDH *handle);
RND_ERROR rnd_open_raw(RNDH *handle, const char *path, int reclen, RND_FLAGS flags);
RND_ERROR rnd_close_raw(RNDH *handle);

RND_ERROR rnd_put(RNDH *handle, RND_RECNO *recno, RND_DATA *data);
RND_ERROR rnd_get(RNDH *handle, RND_RECNO recno, RND_DATA *data);
RND_ERROR rnd_delete(RNDH *handle, RND_RECNO recno);


#endif
