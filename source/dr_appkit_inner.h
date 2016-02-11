// Public domain. See "unlicense" statement at the end of this file.

#include "ak_build_config.h"
#include "ak_application.h"
#include "ak_window.h"
#include "ak_platform_layer.h"
#include "ak_clipboard.h"
#include "ak_image.h"
#include "ak_panel.h"
#include "ak_layout.h"
#include "ak_config.h"
#include "ak_menu.h"
#include "ak_menu_bar.h"
#include "ak_theme.h"
#include "ak_tool.h"

#ifdef DR_APPKIT_IMPLEMENTATION
#include "ak_application_private.h"
#include "ak_tool_private.h"
#include "ak_window_private.h"

#include "ak_application.c"
#include "ak_window.c"
#include "ak_platform_layer.c"
#include "ak_clipboard.c"
#include "ak_image.c"
#include "ak_panel.c"
#include "ak_layout.c"
#include "ak_config.c"
#include "ak_menu.c"
#include "ak_menu_bar.c"
#include "ak_theme.c"
#include "ak_tool.c"
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
