// Public domain. See "unlicense" statement at the end of this file.

#include "../include/easy_appkit/ak_menu.h"
#include "../include/easy_appkit/ak_window.h"
#include <assert.h>

typedef struct ak_menu ak_menu;

struct ak_menu
{
    /// The first menu item.
    ak_menu_item* pFirstItem;

    /// The last menu item.
    ak_menu_item* pLastItem;


    /// The border to mask, if any.
    ak_menu_border borderMask;

    /// The offset of the masked section of the border.
    float borderMaskOffset;

    /// The length of the masked section of the border.
    float borderMaskLength;

    /// The color of the menu's border.
    easygui_color borderColor;

    /// The background color of the menu.
    easygui_color backgroundColor;

    /// The background color of hovered menu items.
    easygui_color backgroundColorHovered;


    /// The function to call when an item is picked.
    ak_mi_on_picked_proc onItemPicked;

    /// The function to call when an item needs to be measured.
    ak_mi_on_measure_proc onItemMeasure;

    /// The function to call when an item needs to be painted.
    ak_mi_on_paint_proc onItemPaint;


    /// The function to call when the menu is shown.
    ak_menu_on_show_proc onShow;

    /// The user data to pass to onShow().
    void* pOnShowData;

    /// The function to call when the menu is hidden.
    ak_menu_on_hide_proc onHide;

    /// The user data to pass to onHide().
    void* pOnHideData;


    /// The size of the extra data.
    size_t extraDataSize;

    /// A pointer to the extra data.
    char pExtraData[1];
};

struct ak_menu_item
{
    /// The menu that owns this item.
    ak_window* pMenuWindow;


    /// The next item in the list.
    ak_menu_item* pNextItem;

    /// The previous item in the list.
    ak_menu_item* pPrevItem;


    /// The size of the extra data.
    size_t extraDataSize;

    /// A pointer to the extra data.
    char pExtraData[1];
};


///////////////////////////////////////////////////////////////////////////////
//
// Menu
//
///////////////////////////////////////////////////////////////////////////////

ak_window* ak_create_menu(ak_application* pApplication, ak_window* pParent, size_t extraDataSize, const void* pExtraData)
{
    ak_window* pMenuWindow = ak_create_window(pApplication, ak_window_type_popup, pParent, sizeof(ak_menu) - sizeof(char) + extraDataSize, NULL);
    if (pMenuWindow == NULL) {
        return NULL;
    }

    ak_menu* pMenu = ak_get_window_extra_data(pMenuWindow);
    assert(pMenu != NULL);

    pMenu->pFirstItem             = NULL;
    pMenu->pLastItem              = NULL;
    pMenu->borderMask             = ak_menu_border_none;
    pMenu->borderMaskOffset       = 0;
    pMenu->borderMaskLength       = 0;
    pMenu->borderColor            = easygui_rgb(96, 96, 96);
    pMenu->backgroundColor        = easygui_rgb(48, 48, 48);
    pMenu->backgroundColorHovered = easygui_rgb(96, 96, 96);
    pMenu->onItemPicked           = NULL;
    pMenu->onItemMeasure          = NULL;
    pMenu->onItemPaint            = NULL;
    pMenu->onShow                 = NULL;
    pMenu->pOnShowData            = NULL;
    pMenu->onHide                 = NULL;
    pMenu->pOnHideData            = NULL;

    pMenu->extraDataSize = extraDataSize;
    if (pExtraData != NULL) {
        memcpy(pMenu->pExtraData, pExtraData, extraDataSize);
    }


    // Window event handlers.
    ak_window_set_on_hide(pMenuWindow, ak_menu_on_hide);
    ak_window_set_on_show(pMenuWindow, ak_menu_on_show);

    // GUI event handlers.
    easygui_set_on_paint(ak_get_window_panel(pMenuWindow), ak_menu_on_paint);


    return pMenuWindow;
}

void ak_delete_menu(ak_window* pMenuWindow)
{
    ak_menu* pMenu = ak_get_window_extra_data(pMenuWindow);
    if (pMenu == NULL) {
        return;
    }

    // Delete every child item first.
    while (pMenu->pLastItem != NULL)
    {
        ak_delete_menu_item(pMenu->pLastItem);
    }

    // Delete the window last.
    ak_delete_window(pMenuWindow);
}


size_t ak_menu_get_extra_data_size(ak_window* pMenuWindow)
{
    ak_menu* pMenu = ak_get_window_extra_data(pMenuWindow);
    if (pMenu == NULL) {
        return 0;
    }

    return pMenu->extraDataSize;
}

void* ak_menu_get_extra_data(ak_window* pMenuWindow)
{
    ak_menu* pMenu = ak_get_window_extra_data(pMenuWindow);
    if (pMenu == NULL) {
        return NULL;
    }

    return pMenu->pExtraData;
}

easygui_element* ak_menu_get_gui_element(ak_window* pMenuWindow)
{
    return ak_get_window_panel(pMenuWindow);
}


void ak_menu_show(ak_window* pMenuWindow)
{
    ak_show_window(pMenuWindow);
}

void ak_menu_hide(ak_window* pMenuWindow)
{
    ak_hide_window(pMenuWindow, 0);
}

void ak_menu_set_position(ak_window* pMenuWindow, int posX, int posY)
{
    ak_set_window_position(pMenuWindow, posX, posY);
}

void ak_menu_set_size(ak_window* pMenuWindow, unsigned int width, unsigned int height)
{
    ak_set_window_size(pMenuWindow, width, height);
}


void ak_menu_set_border_mask(ak_window* pMenuWindow, ak_menu_border border, float offset, float length)
{
    ak_menu* pMenu = ak_get_window_extra_data(pMenuWindow);
    if (pMenu == NULL) {
        return;
    }

    pMenu->borderMask = border;
    pMenu->borderMaskOffset = offset;
    pMenu->borderMaskLength = length;
}

void ak_menu_set_border_color(ak_window* pMenuWindow, easygui_color color)
{
    ak_menu* pMenu = ak_get_window_extra_data(pMenuWindow);
    if (pMenu == NULL) {
        return;
    }

    pMenu->borderColor = color;
}

easygui_color ak_menu_get_border_color(ak_window* pMenuWindow)
{
    ak_menu* pMenu = ak_get_window_extra_data(pMenuWindow);
    if (pMenu == NULL) {
        return easygui_rgb(0, 0, 0);
    }

    return pMenu->borderColor;
}

void ak_menu_set_background_color(ak_window* pMenuWindow, easygui_color color)
{
    ak_menu* pMenu = ak_get_window_extra_data(pMenuWindow);
    if (pMenu == NULL) {
        return;
    }

    pMenu->backgroundColor = color;
}

easygui_color ak_menu_get_background_color(ak_window* pMenuWindow)
{
    ak_menu* pMenu = ak_get_window_extra_data(pMenuWindow);
    if (pMenu == NULL) {
        return easygui_rgb(0, 0, 0);
    }

    return pMenu->backgroundColor;
}

void ak_menu_set_hovered_background_color(ak_window* pMenuWindow, easygui_color color)
{
    ak_menu* pMenu = ak_get_window_extra_data(pMenuWindow);
    if (pMenu == NULL) {
        return;
    }

    pMenu->backgroundColorHovered = color;
}

easygui_color ak_menu_get_hovered_background_color(ak_window* pMenuWindow)
{
    ak_menu* pMenu = ak_get_window_extra_data(pMenuWindow);
    if (pMenu == NULL) {
        return easygui_rgb(0, 0, 0);
    }

    return pMenu->backgroundColorHovered;
}


void ak_menu_set_on_item_picked(ak_window* pMenuWindow, ak_mi_on_picked_proc proc)
{
    ak_menu* pMenu = ak_get_window_extra_data(pMenuWindow);
    if (pMenu == NULL) {
        return;
    }

    pMenu->onItemPicked = proc;
}

void ak_menu_set_on_item_measure(ak_window* pMenuWindow, ak_mi_on_measure_proc proc)
{
    ak_menu* pMenu = ak_get_window_extra_data(pMenuWindow);
    if (pMenu == NULL) {
        return;
    }

    pMenu->onItemMeasure = proc;
}

void ak_menu_set_on_item_paint(ak_window* pMenuWindow, ak_mi_on_paint_proc proc)
{
    ak_menu* pMenu = ak_get_window_extra_data(pMenuWindow);
    if (pMenu == NULL) {
        return;
    }

    pMenu->onItemPaint = proc;
}

void ak_menu_set_on_show(ak_window* pMenuWindow, ak_menu_on_show_proc proc, void* pUserData)
{
    ak_menu* pMenu = ak_get_window_extra_data(pMenuWindow);
    if (pMenu == NULL) {
        return;
    }

    pMenu->onShow = proc;
    pMenu->pOnShowData = pUserData;
}

void ak_menu_set_on_hide(ak_window* pMenuWindow, ak_menu_on_hide_proc proc, void* pUserData)
{
    ak_menu* pMenu = ak_get_window_extra_data(pMenuWindow);
    if (pMenu == NULL) {
        return;
    }

    pMenu->onHide = proc;
    pMenu->pOnHideData = pUserData;
}


void ak_menu_on_mouse_leave(easygui_element* pMenuElement)
{
    ak_menu* pMenu = ak_get_window_extra_data(ak_get_panel_window(pMenuElement));
    if (pMenu == NULL) {
        return;
    }

    // TODO: Implement Me.
}

void ak_menu_on_mouse_move(easygui_element* pMenuElement, int relativeMousePosX, int relativeMousePosY)
{
    ak_menu* pMenu = ak_get_window_extra_data(ak_get_panel_window(pMenuElement));
    if (pMenu == NULL) {
        return;
    }

    // TODO: Implement Me.
}

void ak_menu_on_mouse_button_down(easygui_element* pMenuElement, int mouseButton, int relativeMousePosX, int relativeMousePosY)
{
    ak_menu* pMenu = ak_get_window_extra_data(ak_get_panel_window(pMenuElement));
    if (pMenu == NULL) {
        return;
    }

    // TODO: Implement Me.
}

void ak_menu_on_paint(easygui_element* pMenuElement, easygui_rect relativeClippingRect, void* pPaintData)
{
    ak_menu* pMenu = ak_get_window_extra_data(ak_get_panel_window(pMenuElement));
    if (pMenu == NULL) {
        return;
    }

    float borderWidth = 1;



    // TODO: Implement Me.

    // TEMP: For now, just draw a outlined rectangle around the element.
    easygui_draw_rect(pMenuElement, easygui_grow_rect(easygui_get_local_rect(pMenuElement), -borderWidth), pMenu->backgroundColor, pPaintData);


    // Border.
    float menuWidth;
    float menuHeight;
    easygui_get_size(pMenuElement, &menuWidth, &menuHeight);
    
    // Top.
    {
        easygui_rect borderRect = easygui_make_rect(0, 0, menuWidth, borderWidth);
        if (pMenu->borderMask == ak_menu_border_top && pMenu->borderMaskLength > 0)
        {
            if (pMenu->borderMaskOffset > 0) {
                easygui_draw_rect(pMenuElement, easygui_make_rect(borderRect.left, borderRect.top, borderRect.left + pMenu->borderMaskOffset, borderRect.bottom), pMenu->borderColor, pPaintData);
            }
            
            easygui_draw_rect(pMenuElement, easygui_make_rect(borderRect.left + pMenu->borderMaskOffset, borderRect.top, borderRect.left + pMenu->borderMaskOffset + pMenu->borderMaskLength, borderRect.bottom), pMenu->backgroundColor, pPaintData);
            easygui_draw_rect(pMenuElement, easygui_make_rect(borderRect.left + pMenu->borderMaskOffset + pMenu->borderMaskLength, borderRect.top, borderRect.right, borderRect.bottom), pMenu->borderColor, pPaintData);
        }
        else
        {
            easygui_draw_rect(pMenuElement, borderRect, pMenu->borderColor, pPaintData);
        }
    }
    
    // Bottom
    {
        easygui_rect borderRect = easygui_make_rect(0, menuHeight - borderWidth, menuWidth, menuHeight);
        if (pMenu->borderMask == ak_menu_border_bottom && pMenu->borderMaskLength > 0)
        {
            if (pMenu->borderMaskOffset > 0) {
                easygui_draw_rect(pMenuElement, easygui_make_rect(borderRect.left, borderRect.top, borderRect.left + pMenu->borderMaskOffset, borderRect.bottom), pMenu->borderColor, pPaintData);
            }

            easygui_draw_rect(pMenuElement, easygui_make_rect(borderRect.left + pMenu->borderMaskOffset, borderRect.top, borderRect.left + pMenu->borderMaskOffset + pMenu->borderMaskLength, borderRect.bottom), pMenu->backgroundColor, pPaintData);
            easygui_draw_rect(pMenuElement, easygui_make_rect(borderRect.left + pMenu->borderMaskOffset + pMenu->borderMaskLength, borderRect.top, borderRect.right, borderRect.bottom), pMenu->borderColor, pPaintData);
        }
        else
        {
            easygui_draw_rect(pMenuElement, borderRect, pMenu->borderColor, pPaintData);
        }
    }

    // Left
    {
        easygui_rect borderRect = easygui_make_rect(0, borderWidth, borderWidth, menuHeight - borderWidth);
        if (pMenu->borderMask == ak_menu_border_left && pMenu->borderMaskLength > 0)
        {
            if (pMenu->borderMaskOffset > 0) {
                easygui_draw_rect(pMenuElement, easygui_make_rect(borderRect.left, borderRect.top, borderRect.right, borderRect.top + pMenu->borderMaskOffset), pMenu->borderColor, pPaintData);
            }

            easygui_draw_rect(pMenuElement, easygui_make_rect(borderRect.left, borderRect.top + pMenu->borderMaskOffset, borderRect.right, borderRect.top + pMenu->borderMaskOffset + pMenu->borderMaskLength), pMenu->backgroundColor, pPaintData);
            easygui_draw_rect(pMenuElement, easygui_make_rect(borderRect.left, borderRect.top + pMenu->borderMaskOffset + pMenu->borderMaskLength, borderRect.right, borderRect.bottom), pMenu->borderColor, pPaintData);
        }
        else
        {
            easygui_draw_rect(pMenuElement, borderRect, pMenu->borderColor, pPaintData);
        }
    }

    // Right
    {
        easygui_rect borderRect = easygui_make_rect(menuWidth - borderWidth, borderWidth, menuWidth, menuHeight - borderWidth);
        if (pMenu->borderMask == ak_menu_border_right && pMenu->borderMaskLength > 0)
        {
            if (pMenu->borderMaskOffset > 0) {
                easygui_draw_rect(pMenuElement, easygui_make_rect(borderRect.left, borderRect.top, borderRect.right, borderRect.top + pMenu->borderMaskOffset), pMenu->borderColor, pPaintData);
            }

            easygui_draw_rect(pMenuElement, easygui_make_rect(borderRect.left, borderRect.top + pMenu->borderMaskOffset, borderRect.right, borderRect.top + pMenu->borderMaskOffset + pMenu->borderMaskLength), pMenu->backgroundColor, pPaintData);
            easygui_draw_rect(pMenuElement, easygui_make_rect(borderRect.left, borderRect.top + pMenu->borderMaskOffset + pMenu->borderMaskLength, borderRect.right, borderRect.bottom), pMenu->borderColor, pPaintData);
        }
        else
        {
            easygui_draw_rect(pMenuElement, borderRect, pMenu->borderColor, pPaintData);
        }
    }
}

bool ak_menu_on_show(ak_window* pMenuWindow)
{
    ak_menu* pMenu = ak_get_window_extra_data(pMenuWindow);
    if (pMenu == NULL) {
        return false;;
    }

    if (pMenu->onShow) {
        pMenu->onShow(pMenuWindow, pMenu->pOnShowData);
    }

    return true;
}

bool ak_menu_on_hide(ak_window* pMenuWindow, unsigned int flags)
{
    ak_menu* pMenu = ak_get_window_extra_data(pMenuWindow);
    if (pMenu == NULL) {
        return false;
    }

    if (pMenu->onHide) {
        pMenu->onHide(pMenuWindow, flags, pMenu->pOnHideData);
    }

    return true;
}



///////////////////////////////////////////////////////////////////////////////
//
// Menu Item
//
///////////////////////////////////////////////////////////////////////////////

/// Appends the given menu item to it's parent menu.
PRIVATE void ak_mi_append(ak_menu_item* pMI, ak_window* pMenuWindow);

/// Detaches the given menu item from it's parent menu.
PRIVATE void ak_mi_detach(ak_menu_item* pMI);

ak_menu_item* ak_create_menu_item(ak_window* pMenuWindow, size_t extraDataSize, const void* pExtraData)
{
    ak_menu_item* pMI = malloc(sizeof(ak_menu_item) - sizeof(pMI->pExtraData) + extraDataSize);
    if (pMI == NULL) {
        return NULL;
    }

    pMI->pMenuWindow = NULL;
    pMI->pNextItem = NULL;
    pMI->pPrevItem = NULL;
    
    pMI->extraDataSize = extraDataSize;
    if (pExtraData != NULL) {
        memcpy(pMI->pExtraData, pExtraData, extraDataSize);
    }


    // Append the item to the end of the list.
    ak_mi_append(pMI, pMenuWindow);

    return pMI;
}

void ak_delete_menu_item(ak_menu_item* pMI)
{
    if (pMI == NULL) {
        return;
    }

    // Detach the item first.
    ak_mi_detach(pMI);

    // Once detached, free any memory.
    free(pMI);
}


size_t ak_mi_get_extra_data_size(ak_menu_item* pMI)
{
    if (pMI == NULL) {
        return 0;
    }

    return pMI->extraDataSize;
}

void* ak_mi_get_extra_data(ak_menu_item* pMI)
{
    if (pMI == NULL) {
        return NULL;
    }

    return pMI->pExtraData;
}


ak_window* ak_mi_get_menu(ak_menu_item* pMI)
{
    if (pMI == NULL) {
        return NULL;
    }

    return pMI->pMenuWindow;
}

ak_menu_item* ak_mi_get_next_item(ak_menu_item* pMI)
{
    if (pMI == NULL) {
        return NULL;
    }

    return pMI->pNextItem;
}

ak_menu_item* ak_mi_get_prev_item(ak_menu_item* pMI)
{
    if (pMI == NULL) {
        return NULL;
    }

    return pMI->pPrevItem;
}



PRIVATE void ak_mi_append(ak_menu_item* pMI, ak_window* pMenuWindow)
{
    assert(pMI != NULL);
    assert(pMenuWindow != NULL);
    assert(pMI->pMenuWindow == NULL);
    assert(pMI->pNextItem == NULL);
    assert(pMI->pPrevItem == NULL);

    ak_menu* pMenu = ak_get_window_extra_data(pMenuWindow);
    assert(pMenu != NULL);

    pMI->pMenuWindow = pMenuWindow;

    if (pMenu->pFirstItem == NULL)
    {
        assert(pMenu->pLastItem == NULL);

        pMenu->pFirstItem = pMI;
        pMenu->pLastItem  = pMI;
    }
    else
    {
        assert(pMenu->pLastItem != NULL);

        pMI->pPrevItem = pMenu->pLastItem;

        pMenu->pLastItem->pNextItem = pMI;
        pMenu->pLastItem = pMI;
    }


    // The content of the menu has changed so we'll need to schedule a redraw.
    easygui_dirty(ak_menu_get_gui_element(pMenuWindow), easygui_get_local_rect(ak_menu_get_gui_element(pMenuWindow)));
}

PRIVATE void ak_mi_detach(ak_menu_item* pMI)
{
    assert(pMI != NULL);

    ak_menu* pMenu = ak_get_window_extra_data(pMI->pMenuWindow);
    assert(pMenu != NULL);


    if (pMI->pNextItem != NULL) {
        pMI->pNextItem->pPrevItem = pMI->pPrevItem;
    }

    if (pMI->pPrevItem != NULL) {
        pMI->pPrevItem->pNextItem = pMI->pNextItem;
    }


    if (pMI == pMenu->pFirstItem) {
        pMenu->pFirstItem = pMI->pNextItem;
    }

    if (pMI == pMenu->pLastItem) {
        pMenu->pLastItem = pMI->pPrevItem;
    }


    pMI->pNextItem = NULL;
    pMI->pPrevItem = NULL;
    pMI->pMenuWindow = NULL;

    
    // The content of the menu has changed so we'll need to schedule a redraw.
    easygui_dirty(ak_menu_get_gui_element(pMI->pMenuWindow), easygui_get_local_rect(ak_menu_get_gui_element(pMI->pMenuWindow)));
}


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