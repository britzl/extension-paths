#ifndef PATHS_H
#define PATHS_H

#include <dmsdk/sdk.h>

int Platform_ApplicationPath(char* path_out, uint32_t path_len);

extern size_t dmStrlCpy(char *dst, const char *src, size_t siz);

#endif

