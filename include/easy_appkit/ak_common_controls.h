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

typedef void (* ak_on_tree_view_item_picked_proc)(ak_tree_view_item* pItem);


/// Creates a tree-view control.
easygui_element* ak_create_tree_view(easygui_context* pContext, easygui_element* pParent, easygui_font* pFont, easygui_color textColor, size_t extraDataSize, const void* pExtraData);

/// Deletes the given tree-view control and all of it's child items.
void ak_delete_tree_view(easygui_element* pTV);

/// Retrieves the size of the extra data associated with the given tree-view control.
size_t ak_get_tree_view_extra_data_size(easygui_element* pTV);

/// Retrieves a pointer to the buffer containing the given tree-view's extra data.
void* ak_get_tree_view_extra_data(easygui_element* pTV);


/// Creates a tree view item.
///
/// @remarks
///     When pParent is non-null, the tree-view control must match that of the tree-view control that owns the
///     parent item.
ak_tree_view_item* ak_create_tree_view_item(easygui_element* pTV, ak_tree_view_item* pParent, const char* text, size_t extraDataSize, const void* pExtraData);

/// Recursively deletes a tree view item.
void ak_delete_tree_view_item(ak_tree_view_item* pItem);

/// Deselects every item in the given tree-view control.
void ak_deselect_all_tree_view_items(easygui_element* pTV);

/// Sets the function to call when an leaf item (an item with no children) is picked (double-clicked).
void ak_set_on_tree_view_item_picked(easygui_element* pTV, ak_on_tree_view_item_picked_proc proc);

/// Retrieves the function to call when a leaf item is picked.
ak_on_tree_view_item_picked_proc ak_get_on_tree_view_item_picked(easygui_element* pTV);



/// Retrieves the tree-view control that owns the given item.
easygui_element* ak_get_tree_view_from_item(ak_tree_view_item* pItem);

/// Retrieves the parent tree-view item.
ak_tree_view_item* ak_get_tree_view_item_parent(ak_tree_view_item* pItem);

/// Retrieves the text of the given tree view item.
const char* ak_get_tree_view_item_text(ak_tree_view_item* pItem);

/// Retrieves the size of the extra data associated with the given tree-view item.
size_t ak_get_tree_view_item_extra_data_size(ak_tree_view_item* pItem);

/// Retrieves a pointer to the extra data associated with the given tree-view item.
void* ak_get_tree_view_item_extra_data(ak_tree_view_item* pItem);

/// Appends a tree view item as a child of the given parent item.
void ak_append_tree_view_item(ak_tree_view_item* pItem, ak_tree_view_item* pParent);

/// Prepends a tree view item as a child of the given parent item.
void ak_prepend_tree_view_item(ak_tree_view_item* pItem, ak_tree_view_item* pParent);

/// Appends the given tree view item to the given sibling.
void ak_append_sibling_tree_view_item(ak_tree_view_item* pItemToAppend, ak_tree_view_item* pItemToAppendTo);

/// Prepends the given tree view item to the given sibling.
void ak_prepend_sibling_tree_view_item(ak_tree_view_item* pItemToPrepend, ak_tree_view_item* pItemToPrependTo);

/// Determines whether or not the given item has any children.
bool ak_does_tree_view_item_have_children(ak_tree_view_item* pItem);

/// Selects the given item.
void ak_select_tree_view_item(ak_tree_view_item* pItem);

/// Deselects the given item.
void ak_deselect_tree_view_item(ak_tree_view_item* pItem);

/// Determines whether or not the given tree view item is selected.
bool ak_is_tree_view_item_selected(ak_tree_view_item* pItem);

/// Expands the given item.
void ak_expand_tree_view_item(ak_tree_view_item* pItem);

/// Collapses the given item.
void ak_collapse_tree_view_item(ak_tree_view_item* pItem);

/// Determines whether or not the given item is expanded.
bool ak_is_tree_view_item_expanded(ak_tree_view_item* pItem);



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