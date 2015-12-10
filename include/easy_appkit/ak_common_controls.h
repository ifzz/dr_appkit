// Public domain. See "unlicense" statement at the end of this file.

//
// QUICK NOTES
//
// General
// - This file (and ak_common_controls.c) depends only on easy_gui. The reason for this is that this will likely be
//   merged into easy_gui.
//

#ifndef ak_common_controls_h
#define ak_common_controls_h

#include <easy_gui/easy_gui.h>

#ifdef __cplusplus
extern "C" {
#endif

#define AK_MAX_TREE_VIEW_ITEM_TEXT_LENGTH   256


///////////////////////////////////////////////////////////////////////////////
//
// Tree-View Control
//
///////////////////////////////////////////////////////////////////////////////

typedef struct ak_tree_view_item ak_tree_view_item;


/// Creates a tree-view control.
easygui_element* ak_create_tree_view(easygui_context* pContext, easygui_element* pParent, easygui_font* pFont, easygui_color textColor);

/// Deletes the given tree-view control and all of it's child items.
void ak_delete_tree_view(easygui_element* pTV);


/// Creates a tree view item.
///
/// @remarks
///     When pParent is non-null, the tree-view control must match that of the tree-view control that owns the
///     parent item.
ak_tree_view_item* ak_create_tree_view_item(easygui_element* pTV, ak_tree_view_item* pParent, const char* text, size_t extraDataSize, const void* pExtraData);

/// Recursively deletes a tree view item.
void ak_delete_tree_view_item(ak_tree_view_item* pItem);


/// Appends a tree view item as a child of the given parent item.
void ak_append_tree_view_item(ak_tree_view_item* pItem, ak_tree_view_item* pParent);

/// Prepends a tree view item as a child of the given parent item.
void ak_prepend_tree_view_item(ak_tree_view_item* pItem, ak_tree_view_item* pParent);

/// Appends the given tree view item to the given sibling.
void ak_append_sibling_tree_view_item(ak_tree_view_item* pItemToAppend, ak_tree_view_item* pItemToAppendTo);

/// Prepends the given tree view item to the given sibling.
void ak_prepend_sibling_tree_view_item(ak_tree_view_item* pItemToPrepend, ak_tree_view_item* pItemToPrependTo);



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