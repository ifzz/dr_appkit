// Public domain. See "unlicense" statement at the end of this file.

#ifndef dr_appkit_h
#define dr_appkit_h

// These #defines enable us to load large files on Linux platforms. They need to be placed before including any headers.
#ifndef _WIN32
#ifndef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE
#endif

#ifndef _FILE_OFFSET_BITS
#define _FILE_OFFSET_BITS 64
#endif
#endif

#define DR_GUI_INCLUDE_WIP

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

#ifdef DR_APPKIT_IMPLEMENTATION
#ifdef _WIN32
#include <windows.h>
#endif

#ifdef __linux__
#include <gdk/gdk.h>
#include <gtk/gtk.h>
#endif
#endif

#include "../dr_libs/dr_util.h"
#include "../dr_libs/dr_path.h"
#include "../dr_libs/dr_vfs.h"
#include "../dr_libs/dr_gui.h"
#include "../dr_libs/dr_2d.h"

#include "source/dr_appkit_inner.h"

#ifdef DR_APPKIT_IMPLEMENTATION
#define DR_UTIL_IMPLEMENTATION
#define DR_PATH_IMPLEMENTATION
#define DR_VFS_IMPLEMENTATION
#define DR_GUI_IMPLEMENTATION
#define DR_2D_IMPLEMENTATION

#include "../dr_libs/dr_util.h"
#include "../dr_libs/dr_path.h"
#include "../dr_libs/dr_vfs.h"
#include "../dr_libs/dr_gui.h"
#include "../dr_libs/dr_2d.h"
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
