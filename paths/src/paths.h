#ifndef PATHS_H
#define PATHS_H

#include <dmsdk/sdk.h>

int Platform_BundlePath(char* bundle_path_out, uint32_t path_len);

extern size_t dmStrlCpy(char *dst, const char *src, size_t siz);

#endif

