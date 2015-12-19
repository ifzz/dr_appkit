// Public domain. See "unlicense" statement at the end of this file.

#ifndef ak_menu_bar_h
#define ak_menu_bar_h

#include "ak_menu.h"

#ifndef AK_MAX_MENU_BAR_ITEM_TEXT_LENGTH
#define AK_MAX_MENU_BAR_ITEM_TEXT_LENGTH    64
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ak_menu_bar_item ak_menu_bar_item;

//typedef void (* ak_mbi_on_activate_proc)  (ak_menu_bar_item* pMBI);
//typedef void (* ak_mbi_on_deactivate_proc)(ak_menu_bar_item* pMBI);
typedef void (* ak_mbi_on_measure_proc)   (ak_menu_bar_item* pMBI, float* pWidthOut, float* pHeightOut);
typedef void (* ak_mbi_on_paint_proc)     (easygui_element* pMBElement, ak_menu_bar_item* pMBI, easygui_rect clippingRect, float offsetX, float offsetY, float width, float height, void* pPaintData);


///////////////////////////////////////////////////////////////////////////////
//
// Menu Bar
//
///////////////////////////////////////////////////////////////////////////////

/// Creates a menu bar.
///
/// @remarks
///     The font to use for the text of menu bar items should be set with ak_mb_set_font() before creating any new items.
easygui_element* ak_create_menu_bar(easygui_context* pContext, easygui_element* pParent, size_t extraDataSize, const void* pExtraData);

/// Deletes the given menu bar.
///
/// @remarks
///     This deletes every menu bar item that's attached to the menu bar, but does not delete the menu's themselves.
void ak_delete_menu_bar(easygui_element* pMBElement);


/// Retrieves the size fo the extra data of the given menu bar.
size_t ak_mb_get_extra_data_size(easygui_element* pMBElement);

/// Retrieves a pointer to the extra data associated with the given menu bar.
void* ak_mb_get_extra_data(easygui_element* pMBElement);


/// Sets the font to use for the text of menu bar items.
void ak_mb_set_font(easygui_element* pMBElement, easygui_font* pFont);

/// Retrieves the font to use for the text of menu bar items.
easygui_font* ak_mb_get_font(easygui_element* pMBElement);

/// Sets the color of the text of menu bar items.
void ak_mb_set_text_color(easygui_element* pMBElement, easygui_color color);

/// Retrieves the color of the text of menu bar items.
easygui_color ak_mb_get_text_color(easygui_element* pMBElement);

/// Sets the background color of the menu bar.
void ak_mb_set_default_background_color(easygui_element* pMBElement, easygui_color color);

/// Retrieves the background color of the menu bar.
easygui_color ak_mb_get_default_background_color(easygui_element* pMBElement);

/// Sets the background color of hovered menu bar items.
void ak_mb_set_hovered_background_color(easygui_element* pMBElement, easygui_color color);

/// Retrieves the background color of hovered menu bar items.
easygui_color ak_mb_get_hovered_background_color(easygui_element* pMBElement);

/// Sets the background color of expanded menu bar items.
void ak_mb_set_expanded_background_color(easygui_element* pMBElement, easygui_color color);

/// Retrieves the background color of expanded menu bar items.
easygui_color ak_mb_get_expanded_background_color(easygui_element* pMBElement);

/// Sets the color of the border of expanded tab bar items.
void ak_mb_set_expanded_border_color(easygui_element* pMBElement, easygui_color color);

/// Retrieves the color of the border of expanded tab bar items.
easygui_color ak_mb_get_expanded_border_color(easygui_element* pMBElement);

/// Sets the amount of padding to apply to the left and right side of each menu bar item.
void ak_mb_set_item_padding_x(easygui_element* pMBElement, float padding);

/// Retrieves the amount of padding to apply to the left and right side of each menu bar item.
float ak_mb_get_item_padding_x(easygui_element* pMBElement);


/// Show the menu of the given item.
///
/// @remarks
///     If another menu is already being shown, that was is hidden first.
void ak_mb_show_item_menu(easygui_element* pMBElement, ak_menu_bar_item* pMBI);

/// Hides the currently visible menu item, if any.
void ak_mb_hide_item_menu(easygui_element* pMBElement);


/// Sets the function to call when a menu bar item is activated.
//void ak_mb_set_on_mbi_activate(easygui_element* pMBElement, ak_mbi_on_activate_proc proc);

/// Sets the function to call when a menu bar item is deactivated.
//void ak_mb_set_on_mbi_deactivate(easygui_element* pMBElement, ak_mbi_on_deactivate_proc proc);

/// Sets the function to call when a menu bar item is measure.
void ak_mb_set_on_mbi_measure(easygui_element* pMBElement, ak_mbi_on_measure_proc proc);

/// Sets the function to call when a menu bar item needs to be painted.
void ak_mb_set_on_mbi_paint(easygui_element* pMBElement, ak_mbi_on_paint_proc proc);


/// Called when the mouse leave event needs to be handled.
void ak_mb_on_mouse_leave(easygui_element* pMBElement);

/// Called when the mouse move event needs to be handled.
void ak_mb_on_mouse_move(easygui_element* pMBElement, int relativeMousePosX, int relativeMousePosY);

/// Called when the mouse button down event needs to be handled.
void ak_mb_on_mouse_button_down(easygui_element* pMBElement, int mouseButton, int relativeMousePosX, int relativeMousePosY);

/// Called when the paint event needs to be handled.
void ak_mb_on_paint(easygui_element* pMBElement, easygui_rect relativeClippingRect, void* pPaintData);


///////////////////////////////////////////////////////////////////////////////
//
// Menu Bar Item
//
///////////////////////////////////////////////////////////////////////////////

/// Creates a menu bar item.
///
/// @remarks
///     This takes over the on_show and on_hide events of the given menu.
ak_menu_bar_item* ak_create_menu_bar_item(easygui_element* pMBElement, ak_window* pMenu, size_t extraDataSize, const void* pExtraData);

/// Deletes the given menu bar item.
///
/// @remarks
///     This does not delete the associated menu.
void ak_delete_menu_bar_item(ak_menu_bar_item* pMBI);


/// Retrieves a pointer to the menu bar element that owns the given menu bar item.
easygui_element* ak_mbi_get_menu_bar(ak_menu_bar_item* pMBI);

/// Retrieves a pointer to the menu that's associated with the given menu bar item.
ak_window* ak_mbi_get_menu(ak_menu_bar_item* pMBI);

/// Retrieves the size of the extra data associated with the given menu bar item.
size_t ak_mbi_get_extra_data_size(ak_menu_bar_item* pMBI);

/// Retrieves a pointer to the extra data associated with the given menu bar item.
void* ak_mbi_get_extra_data(ak_menu_bar_item* pMBI);


/// Sets the text of the given menu bar item.
void ak_mbi_set_text(ak_menu_bar_item* pMBI, const char* text);

/// Retrieves the text of the given menu bar item.
const char* ak_mbi_get_text(ak_menu_bar_item* pMBI);


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