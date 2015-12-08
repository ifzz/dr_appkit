// Public domain. See "unlicense" statement at the end of this file.

#ifndef ak_config_h
#define ak_config_h

#include "ak_layout.h"
#include <easy_fs/easy_vfs.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ak_config ak_config;
struct ak_config
{
    /// The name of the initial layout to apply.
    char currentLayoutName[256];

    /// The root layout item. This is anonymous and is only used for hierarchy management.
    ak_layout* pRootLayout;
};

/// Parses a config script from a file.
bool ak_parse_config_from_file(ak_config* pConfig, easyvfs_file* pFile);

/// Parses a config script from a string.
bool ak_parse_config_from_string(ak_config* pConfig, const char* configString);


/// Initializes the given config object. This does not allocate the config object.
bool ak_init_config(ak_config* pConfig);

/// Uninitializes the given config object. This does not free the config object.
void ak_uninit_config(ak_config* pConfig);


/// Finds the first occurance of a root level layout with the given name.
ak_layout* ak_config_find_root_layout_by_name(ak_config* pConfig, const char* layoutName);


#ifdef __cplusplus
}
#endif

#endif


/*
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org/>
*/