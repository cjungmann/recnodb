#ifndef RECNODB_H
#define RECNODB_H

#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>   // for off_t typedef

#define EXPORT __attribute__((visibility("default")))

typedef uint32_t RND_RECLEN;
typedef uint32_t RND_RECNO;

typedef enum {
   RND_SUCCESS = 0,
   RND_FAIL,
   RND_FILE_ALREADY_OPEN,
   RND_FILE_NOT_OPEN,
   RND_EXTINCT_RECORD,
   RND_ATTEMPT_TO_ORPHAN_BLOCK,
   RND_LOCK_FAILED,
   RND_UNLOCK_FAILED,
   RND_SYSTEM_ERROR
} RND_ERROR;

typedef enum {
   RND_CREATE,
   RND_READONLY
} RND_FLAGS;

typedef struct recnodb_data {
   void *data;
   int64_t  size;
} RND_DATA;

typedef struct recnodb_handle {
   FILE        *file;
   uint32_t    page_size;
   int         sys_errno;
} RNDH;

#include "pages.h"
#include "locks.h"

RND_ERROR rnd_init(RNDH *handle);
RND_ERROR rnd_open(RNDH *handle, const char *path, int reclen, RND_FLAGS flags);
RND_ERROR rnd_close(RNDH *handle);
RND_ERROR rnd_put(RNDH *handle, RND_RECNO *recno, RND_DATA *data);
RND_ERROR rnd_get(RNDH *handle, RND_RECNO recno, RND_DATA *data);
RND_ERROR rnd_delete(RNDH *handle, RND_RECNO recno);


#endif
