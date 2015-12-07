// Public domain. See "unlicense" statement at the end of this file.

#ifndef ak_layout_h
#define ak_layout_h

#include <easy_util/easy_util.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    ak_layout_type_unknown,
    ak_layout_type_root,
    ak_layout_type_application_window,
    ak_layout_type_split_panel_horz,
    ak_layout_type_split_panel_vert,
    ak_layout_type_panel,
    ak_layout_type_tool,

} ak_layout_type;

typedef struct ak_layout ak_layout;
struct ak_layout
{
    /// The type of the layout item.
    char name[256];

    /// The attributes of the layout item as a string. The format of this string depends on the item type.
    char attributes[256];


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
ak_layout* ak_create_layout(const char* name, const char* attributes, ak_layout* pParent);

/// Deletes the given layout object.
void ak_delete_layout(ak_layout* pLayout);



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