#include "recnodb.h"
#include "recnodb_int.h"

#include <string.h>

EXPORT RND_ERROR rnd_init(RNDH *handle)
{
   memset(handle, 0, sizeof(RNDH));
   handle->page_size = get_blocksize();
   return RND_SUCCESS;
}

EXPORT RND_ERROR rnd_open(RNDH *handle, const char *path, int reclen, RND_FLAGS flags)
{
   return RND_SUCCESS;
}

EXPORT RND_ERROR rnd_put(RNDH *handle, RND_RECNO *recno, RND_DATA *data)
{
   return RND_SUCCESS;
}

EXPORT RND_ERROR rnd_get(RNDH *handle, RND_RECNO recno, RND_DATA *data)
{
   return RND_SUCCESS;
}

EXPORT RND_ERROR rnd_delete(RNDH *handle, RND_RECNO recno)
{
   return RND_SUCCESS;
}

