// Public domain. See "unlicense" statement at the end of this file.

#ifndef ak_layout_h
#define ak_layout_h

#include "../include/dr_appkit/ak_build_config.h"
#include "../include/dr_appkit/ak_panel.h"
#include "../include/dr_appkit/ak_tool.h"
#include "../include/dr_appkit/ak_window.h"
#include <dr_libs/dr_util.h>

#ifdef __cplusplus
extern "C" {
#endif

#define AK_LAYOUT_TYPE_LAYOUT   "Layout"
#define AK_LAYOUT_TYPE_WINDOW   "Window"
#define AK_LAYOUT_TYPE_PANEL    "Panel"
#define AK_LAYOUT_TYPE_TOOL     "Tool"

typedef struct ak_layout ak_layout;
struct ak_layout
{
    /// The type of the layout item.
    char type[AK_MAX_LAYOUT_NAME_LENGTH];

    /// The attributes of the layout item as a string. The format of this string depends on the item type.
    char attributes[AK_MAX_LAYOUT_ATTRIB_LENGTH];


    /// A pointer to the parent item.
    ak_layout* pParent;

    /// A pointer to the first child item.
    ak_layout* pFirstChild;

    /// A pointer to the last child item.
    ak_layout* pLastChild;

    /// A pointer to the next sibling item.
    ak_layout* pNextSibling;

    /// A pointer to the previous sibling item.
    ak_layout* pPrevSibling;
};

/// Detaches the given layout item from it's parent and orphans it.
///
/// @remarks
///     This will also detach the item from it's siblings, but will keep it's children.
void ak_detach_layout(ak_layout* pLayout);

/// Appends a layout item to another as a child.
void ak_append_layout(ak_layout* pChild, ak_layout* pParent);

/// Prepends a layout item to another as a child.
void ak_prepend_layout(ak_layout* pChild, ak_layout* pParent);


/// Creates a new layout item.
ak_layout* ak_create_layout(const char* type, const char* attributes, ak_layout* pParent);

/// Deletes the given layout object.
void ak_delete_layout(ak_layout* pLayout);



/// Structure representing the attributes of a window layout object.
typedef struct
{
    /// The window type.
    ak_window_type type;

    /// The position of the window on the x axis.
    int posX;

    /// The position of the window on the y axis.
    int posY;

    /// The width of the window.
    unsigned int width;

    /// The height of the window.
    unsigned int height;

    /// Whether or not the window is maximized.
    bool maximized;

    /// The window title.
    char title[AK_MAX_WINDOW_TITLE_LENGTH];

    /// The name of the window.
    char name[AK_MAX_WINDOW_NAME_LENGTH];

} ak_window_layout_attributes;

/// Parses the attribute string of a window layout type.
bool ak_parse_window_layout_attributes(const char* attributesStr, ak_window_layout_attributes* pAttributesOut);


/// Structure representing the attributes of a panel layout object.
typedef struct
{
    /// The name of the panel.
    char type[AK_MAX_PANEL_TYPE_LENGTH];

    /// The split axis.
    ak_panel_split_axis splitAxis;

    /// The position of the split.
    float splitPos;

} ak_panel_layout_attributes;

/// Parses the attribute string of a panel layout type.
bool ak_parse_panel_layout_attributes(const char* attributesStr, ak_panel_layout_attributes* pAttributesOut);


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
