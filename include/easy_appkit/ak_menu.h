// Public domain. See "unlicense" statement at the end of this file.

//
// QUICK NOTES
//
// General
// - A menu is seperate from the menu bar.
// - A menu is a popup window.
// - A menu is made up of a series of menu items. Each menu item can be assigned a sub-menu.
// - Menu items cannot be re-parented after creation.
//

#ifndef ak_menu_h
#define ak_menu_h

#include <easy_gui/easy_gui.h>

#ifndef AK_MAX_MENU_ITEM_TEXT_LENGTH
#define AK_MAX_MENU_ITEM_TEXT_LENGTH    64
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    ak_menu_border_none,
    ak_menu_border_left,
    ak_menu_border_right,
    ak_menu_border_top,
    ak_menu_border_bottom
} ak_menu_border;

typedef struct ak_window ak_window;
typedef struct ak_application ak_application;
typedef struct ak_menu_item ak_menu_item;

typedef void (* ak_mi_on_picked_proc)  (ak_menu_item* pMI);
typedef void (* ak_mi_on_measure_proc) (ak_menu_item* pMI, float* pWidthOut, float* pHeightOut);
typedef void (* ak_mi_on_paint_proc)   (easygui_element* pMenuElement, ak_menu_item* pMI, easygui_rect relativeClippingRect, float posX, float posY, float width, float height, void* pPaintData);
typedef void (* ak_menu_on_show_proc)  (ak_window* pMenuWindow, void* pUserData);
typedef void (* ak_menu_on_hide_proc)  (ak_window* pMenuWindow, unsigned int flags, void* pUserData);

///////////////////////////////////////////////////////////////////////////////
//
// Menu
//
///////////////////////////////////////////////////////////////////////////////

/// Creates an empty menu.
///
/// @remarks
///     The menu is initially hidden.
ak_window* ak_create_menu(ak_application* pApplication, ak_window* pParent, size_t extraDataSize, const void* pExtraData);

/// Deletes the given menu.
///
/// @remarks
///     Deleting a menu will delete every attached item.
void ak_delete_menu(ak_window* pMenuWindow);


/// Retrieves the size of the extra data associated with the menu.
size_t ak_menu_get_extra_data_size(ak_window* pMenuWindow);

/// Retrieves a pointer to the extra data associated with the given menu.
void* ak_menu_get_extra_data(ak_window* pMenuWindow);

/// Retrieves the root GUI element of the menu.
easygui_element* ak_menu_get_gui_element(ak_window* pMenuWindow);


/// Shows the given menu.
void ak_menu_show(ak_window* pMenuWindow);

/// Hides the given menu.
void ak_menu_hide(ak_window* pMenuWindow);

/// Sets the position of the menu.
void ak_menu_set_position(ak_window* pMenuWindow, int posX, int posY);

/// Sets the size of the menu.
void ak_menu_set_size(ak_window* pMenuWindow, unsigned int width, unsigned int height);


/// Sets the region of the border to leave undrawn.
///
/// @remarks
///     With menu bars in particular, it is often desirable to leave part of the border of a menu undrawn in order
///     to create a seamless border effect around the menu and the button. This function allow you to specify the
///     section of the border to leave undrawn.
void ak_menu_set_border_mask(ak_window* pMenuWindow, ak_menu_border border, float offset, float length);

/// Sets the color of the border of the menu.
void ak_menu_set_border_color(ak_window* pMenuWindow, easygui_color color);

/// Retrieves the color of the border of the menu.
easygui_color ak_menu_get_border_color(ak_window* pMenuWindow);

/// Sets the background color of the menu.
void ak_menu_set_background_color(ak_window* pMenuWindow, easygui_color color);

/// Retrieves the background color of the menu.
easygui_color ak_menu_get_background_color(ak_window* pMenuWindow);

/// Sets the background color of highlighted menu items.
void ak_menu_set_hovered_background_color(ak_window* pMenuWindow, easygui_color color);

/// Retrieves the background color of highlighted menu items.
easygui_color ak_menu_get_hovered_background_color(ak_window* pMenuWindow);

/// Sets the default font to use for menu items.
void ak_menu_set_font(ak_window* pMenuWindow, easygui_font* pFont);

/// Retrieves the default font to use for menu items.
easygui_font* ak_menu_get_font(ak_window* pMenuWindow);

/// Sets the default text color to use for menu items.
void ak_menu_set_text_color(ak_window* pMenuWindow, easygui_color color);

/// Retrieves the default text color to use for menu items.
easygui_color ak_menu_get_text_color(ak_window* pMenuWindow);

/// Sets the color and thickness of the separator.
void ak_menu_set_separator_style(ak_window* pMenuWindow, easygui_color color, float thickness);

/// Retrieves the color of separator lines.
easygui_color ak_menu_get_separator_color(ak_window* pMenuWindow);

/// Retrieves the thickness of separator lines.
float ak_menu_get_separator_thickness(ak_window* pMenuWindow);


/// Sets the function to call when an item needs to be measured.
void ak_menu_set_on_item_measure(ak_window* pMenuWindow, ak_mi_on_measure_proc proc);

/// Sets the function to call when an item needs to be painted.
void ak_menu_set_on_item_paint(ak_window* pMenuWindow, ak_mi_on_paint_proc proc);

/// Sets the function to call when a menu window is shown.
void ak_menu_set_on_show(ak_window* pMenuWindow, ak_menu_on_show_proc proc, void* pUserData);

/// Sets the function to call when a menu window is hidden.
void ak_menu_set_on_hide(ak_window* pMenuWindow, ak_menu_on_hide_proc proc, void* pUserData);


// For the functions below, note how it's the menu's GUI element that's passed as the first agument.

/// Called when the mouse leave event needs to be processed for the given menu.
void ak_menu_on_mouse_leave(easygui_element* pMenuElement);

/// Called when the mouse move event needs to be processed for the given tree-view control.
void ak_menu_on_mouse_move(easygui_element* pMenuElement, int relativeMousePosX, int relativeMousePosY);

/// Called when the mouse button down event needs to be processed for the given tree-view control.
void ak_menu_on_mouse_button_up(easygui_element* pMenuElement, int mouseButton, int relativeMousePosX, int relativeMousePosY);

/// Called when the paint event needs to be processed for the given tree-view control.
void ak_menu_on_paint(easygui_element* pMenuElement, easygui_rect relativeClippingRect, void* pPaintData);

/// Called when the show event needs to be handled for the given menu.
bool ak_menu_on_show(ak_window* pMenuWindow);

/// Called when the hide event needs to be handled for the given menu.
bool ak_menu_on_hide(ak_window* pMenuWinodw, unsigned int flags);



///////////////////////////////////////////////////////////////////////////////
//
// Menu Item
//
///////////////////////////////////////////////////////////////////////////////

/// Creates a new menu item.
ak_menu_item* ak_create_menu_item(ak_window* pMenuWindow, size_t extraDataSize, const void* pExtraData);

/// Creates a separator menu item.
ak_menu_item* ak_create_separator_menu_item(ak_window* pMenuItem, size_t extraDataSize, const void* pExtraData);

/// Deletes the given menu item.
void ak_delete_menu_item(ak_menu_item* pMI);


/// Retrieves the size of the extra data associated with the given menu item.
size_t ak_mi_get_extra_data_size(ak_menu_item* pMI);

/// Retrieves a pointer to the extra data assocaited with the menu item.
void* ak_mi_get_extra_data(ak_menu_item* pMI);


/// Retrieves a pointer to the menu that owns the given item.
ak_window* ak_mi_get_menu(ak_menu_item* pMI);

/// Retrieves a pointer to the next item in the list.
ak_menu_item* ak_mi_get_next_item(ak_menu_item* pMI);

/// Retrieves a pointer to the previous item in the list.
ak_menu_item* ak_mi_get_prev_item(ak_menu_item* pMI);


/// Determines whether or not the given menu item is a separator.
bool ak_mi_is_separator(ak_menu_item* pMI);


/// Sets the text of the given menu item.
void ak_mi_set_text(ak_menu_item* pMI, const char* text);

/// Retrieves the text of the given menu item.
const char* ak_mi_get_text(ak_menu_item* pMI);

/// Sets the shortcut text of the given menu item.
void ak_mi_set_shortcut_text(ak_menu_item* pMI, const char* text);

/// Retrieves the shortcut text of the given menu item.
const char* ak_mi_get_shortcut_text(ak_menu_item* pMI);


/// Sets the function to call when the given menu item is picked.
void ak_mi_set_on_picked(ak_menu_item* pMI, ak_mi_on_picked_proc proc);

/// Calls the picked event handler for the given menu item.
void ak_mi_on_picked(ak_menu_item* pMI);



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