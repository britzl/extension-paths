#include <dmsdk/sdk.h>
#include "paths.h"
#import <Foundation/Foundation.h>

int Platform_ApplicationPath(char* path_out, uint32_t path_len)
{
	assert(path_len > 0);
	NSBundle* mainBundle = [NSBundle mainBundle];
	if (mainBundle == NULL)
	{
		return 0;
	}
	const char *bundle_path = [[mainBundle bundlePath] UTF8String];
	if (dmStrlCpy(path_out, bundle_path, path_len) >= path_len)
	{
		path_out[0] = 0;
		return 0;
	}
	return 1;
}