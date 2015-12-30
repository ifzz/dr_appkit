// Public domain. See "unlicense" statement at the end of this file.

#ifndef ak_tool_h
#define ak_tool_h

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct easygui_element easygui_element;
typedef struct ak_application ak_application;
typedef struct easygui_tab easygui_tab;

/// Creates a tool.
///
/// @remarks
///     Specific types of tools will call this inside their own creation function.
easygui_element* ak_create_tool(ak_application* pApplication, easygui_element* pParent, const char* type, size_t extraDataSize, const void* pExtraData);

/// Retrieves a pointer to the application that owns the given tool.
ak_application* ak_get_tool_application(easygui_element* pTool);

/// Retrieves the type string of the tool.
const char* ak_get_tool_type(easygui_element* pTool);

/// Retrieves the size of the extra data.
size_t ak_get_tool_extra_data_size(easygui_element* pTool);

/// Retrieves a pointer to the extra data.
void* ak_get_tool_extra_data(easygui_element* pTool);


/// Sets the tab to associate with the given tool.
void ak_set_tool_tab(easygui_element* pTool, easygui_tab* pTab);

/// Retrieves the tab associated with the given tool.
easygui_tab* ak_get_tool_tab(easygui_element* pTool);


/// Sets the title of the tool.
///
/// @remarks
///     This is what will be shown on the tab.
void ak_set_tool_title(easygui_element* pTool, const char* title);

/// Retrieves the title of the tool.
const char* ak_get_tool_title(easygui_element* pTool);



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