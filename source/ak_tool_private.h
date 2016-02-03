// Public domain. See "unlicense" statement at the end of this file.

#ifndef ak_tool_private_h
#define ak_tool_private_h

#include "ak_config.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct drgui_element drgui_element;

/// Sets the tab to associate with the given tool.
///
/// @remarks
///     This is only used by panels when the tool is attached to it.
void ak_set_tool_tab(drgui_element* pTool, drgui_tab* pTab);

/// Sets the panel the tool is attached to.
///
/// @remarks
///     This is only used by panels when the tool is attached to it. This is just a basic setter.
void ak_set_tool_panel(drgui_element* pTool, drgui_element* pPanel);


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