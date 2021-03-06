#ifndef RECNODB_EXTRA_H
#define RECNODB_EXTRA_H

#include "recnodb.h"

void prime_handle(RNDH *handle);

uint32_t get_blocksize(void);
const char *rnd_strerror(RND_ERROR err, RNDH *handle);



#endif
