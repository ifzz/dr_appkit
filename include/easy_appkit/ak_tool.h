// Public domain. See "unlicense" statement at the end of this file.

//
// QUICK NOTES
//
// - A type of a tool is defined by a string, which can be in a format such as "Editor.Text.CPP". The function
//   ak_is_tool_of_type() can be used to determine whether or not the given tool is of a particular type. With
//   the example above, true will be returned for a call such as ak_is_tool_of_type(pMyTool, "Editor.Text").
//

#ifndef ak_tool_h
#define ak_tool_h

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct drgui_element drgui_element;
typedef struct ak_application ak_application;
typedef struct drgui_tab drgui_tab;

typedef void (* ak_tool_on_handle_action_proc)(drgui_element* pTool, const char* pActionName);

/// Creates a tool.
///
/// @remarks
///     Specific types of tools will call this inside their own creation function.
drgui_element* ak_create_tool(ak_application* pApplication, drgui_element* pParent, const char* type, size_t extraDataSize, const void* pExtraData);

/// Deletes the given tool.
///
/// @remarks
///     This is equivalent to drgui_delete_element(), but is included for consistency with ak_delete_tool().
void ak_delete_tool(drgui_element* pTool);

/// Retrieves a pointer to the application that owns the given tool.
ak_application* ak_get_tool_application(drgui_element* pTool);

/// Retrieves the type string of the tool.
const char* ak_get_tool_type(drgui_element* pTool);

/// Retrieves the size of the extra data.
size_t ak_get_tool_extra_data_size(drgui_element* pTool);

/// Retrieves a pointer to the extra data.
void* ak_get_tool_extra_data(drgui_element* pTool);


/// Determines whether the given tool type is of the given type.
bool ak_is_of_tool_type(const char* pToolType, const char* pBaseToolType);

/// Determines whether or not the tool is of the given type.
///
/// @remarks
///     This returns true for sub-types. For example, if the type of a tool is "Editor.Text.CPP", true will be returned
///     if this function is called with a type of "Editor.Text".
bool ak_is_tool_of_type(drgui_element* pTool, const char* type);


/// Retrieves the tab associated with the given tool.
drgui_tab* ak_get_tool_tab(drgui_element* pTool);

/// Retrieves the panel the tool is attached to, if any.
drgui_element* ak_get_tool_panel(drgui_element* pTool);


/// Sets the title of the tool.
///
/// @remarks
///     This is what will be shown on the tab.
void ak_set_tool_title(drgui_element* pTool, const char* title);

/// Retrieves the title of the tool.
const char* ak_get_tool_title(drgui_element* pTool);


/// Allows the tool to handle the given action.
void ak_tool_handle_action(drgui_element* pTool, const char* pActionName);

/// Sets the function to call when an action needs to be handled.
void ak_tool_set_on_handle_action(drgui_element* pTool, ak_tool_on_handle_action_proc proc);


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