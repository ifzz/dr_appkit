// Public domain. See "unlicense" statement at the end of this file.

//
// QUICK NOTES
//
// Panels and Tools
// - Every window is associated with a single top-level GUI element.
// - Each top-level element is made up of a collection of panels, with each panel able to be split a
//   maximum of 1 time, either horizontally or vertically.
// - Each unsplit panel can have any number of "tools" attached to it.
// - Each panel can be assigned a type which is defined as a string, and is optional. The type is mainly
//   used to allow an application to make use of special panels for serving a specific type of task.
// - A tool is a GUI element that has 1 particular task. Examples could include text editors, log output
//   windows, project explorer's, etc.
// - When multiple tools are attached to a single panel, the panel uses tabs to allow the user to switch
//   between them.
// - Tools are unaware of the panel they are attached to and are designed such that they can be used
//   independantly.
// - Both panels and tools are just GUI elements with associated user-data. There is no "ak_panel" or
//   "ak_tool" object - they are just easygui_element objects.
// - Tools and panels should be deleted with easygui_delete_element(), however they should be first
//   disassociated with the relevant parts of the application.
// 

#ifndef ak_panel_h
#define ak_panel_h

#include <stdint.h>
#include <stdbool.h>
#include <easy_gui/wip/easygui_tab_bar.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct easygui_element easygui_element;
typedef struct ak_application ak_application;

#define AK_PANEL_OPTION_SHOW_TOOL_TABS             1
#define AK_PANEL_OPTION_SHOW_CLOSE_BUTTON_ON_TABS  2
#define AK_PANEL_OPTION_ALLOW_TAB_PINNING          4
#define AK_PANEL_OPTION_ALLOW_TAB_MOVE             8
#define AK_PANEL_OPTION_EXPANDABLE                 16

typedef enum
{
    ak_panel_split_axis_none,
    ak_panel_split_axis_horizontal,
    ak_panel_split_axis_vertical
} ak_panel_split_axis;


/// Creates an empty panel.
///
/// @remarks
///     An empty panel contains no children and no tools.
easygui_element* ak_create_panel(ak_application* pApplication, easygui_element* pParent, size_t extraDataSize, const void* pExtraData);


/// Retrieves a pointer to the application that owns the given panel.
ak_application* ak_get_panel_application(easygui_element* pPanel);


/// Retrieves the size of the panel's extra data.
size_t ak_panel_get_extra_data_size(easygui_element* pPanel);

/// Retrieves a pointer to the panel's extra data.
///
/// @remarks
///     Note that this is different from easygui_get_extra_data() and is of the size specified when the panel was created with create_panel().
void* ak_panel_get_extra_data(easygui_element* pPanel);


/// Sets the name of the panel.
void ak_panel_set_type(easygui_element* pPanel, const char* type);

/// Retrieves the type of the panel.
///
/// @remarks
///     If the panel does not have a type, an empty string will be returned. Null will be returned on error.
const char* ak_panel_get_type(easygui_element* pPanel);

/// Recursively searches for a panel with the given type, including <pPanel>.
easygui_element* ak_panel_find_by_type_recursive(easygui_element* pPanel, const char* type);

/// Determines if the given panel is of the given type.
///
/// @remarks
///     This returns true for sub-types. For example, if the type of a panel is "My.Panel.Type", true will be returned
///     if this function is called with a type of "My.Panel".
bool ak_panel_is_of_type(easygui_element* pPanel, const char* type);


/// Splits the given panel along the given axis at the given position.
///
/// @remarks
///     This will fail if the panel has any tools attached.
bool ak_panel_split(easygui_element* pPanel, ak_panel_split_axis splitAxis, float splitPos);

/// Unsplits the given panel.
///
/// @remarks
///     This will delete any child elements, so ensure everything has been cleaned up appropriately beforehand.
void ak_panel_unsplit(easygui_element* pPanel);

/// Retrieves the axis that the given panel is split along.
ak_panel_split_axis ak_panel_get_split_axis(easygui_element* pPanel);

/// Retrieves the first child panel of the given split panel.
///
/// @remarks
///     This will fail if the panel is not split.
///     @par
///     The first split panel is the left one in the case of a horizontal split, and the top one for vertical splits.
easygui_element* ak_panel_get_split_panel_1(easygui_element* pPanel);

/// Retrieves the second child panel of the given split panel.
///
/// @remarks
///     This will fail if the panel is not split.
///     @par
///     The second split panel is the right one in the case of a horizontal split, and the bottom one for vertical splits.
easygui_element* ak_panel_get_split_panel_2(easygui_element* pPanel);


/// Attaches a tool to the given panel.
///
/// @remarks
///     This will fail if the given panel is a split panel.
///     @par
///     If the tool is already attached to another tool it will be detached first.
bool ak_panel_attach_tool(easygui_element* pPanel, easygui_element* pTool);

/// Detaches the given tool from the given panel.
///
/// @remarks
///     This will fail if the given panel is a split panel.
///     @par
///     This does not delete the tool, it simply orphans it. You will typically want to either attach it to another panel or delete it after detaching
///     it from this one.
void ak_panel_detach_tool(easygui_element* pPanel, easygui_element* pTool);


/// Activates the given tool by deactivating the previously active tool and then activating this one.
///
/// @remarks
///     Activating a tab involves activating the tab and showing the tool.
///     @par
///     This will fail is <pTool> is not attached to <pPanel>.
bool ak_panel_activate_tool(easygui_element* pPanel, easygui_element* pTool);

/// Retrieves a pointer to the active tool in the given panel, if any.
easygui_element* ak_panel_get_active_tool(easygui_element* pPanel);


/// Retrieves a pointer to the first tool that's attached to the given panel, if any.
easygui_element* ak_panel_get_first_tool(easygui_element* pPanel);

/// Retrieves a pointer to the next tool that's attached to the given panel.
easygui_element* ak_panel_get_next_tool(easygui_element* pPanel, easygui_element* pTool);


/// Sets the option flags to use for the tab bar.
///
/// @remarks
///     This will refresh the tab bar. Usually you will want to only set this once.
void ak_panel_set_tab_options(easygui_element* pPanel, unsigned int options);


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