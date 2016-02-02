// Public domain. See "unlicense" statement at the end of this file.

#include "../include/easy_appkit/ak_menu.h"
#include "../include/easy_appkit/ak_window.h"
#include <dr_libs/dr_util.h>
#include <math.h>
#include <assert.h>

#ifndef PRIVATE
#define PRIVATE static
#endif

typedef struct ak_menu ak_menu;

struct ak_menu
{
    /// The first menu item.
    ak_menu_item* pFirstItem;

    /// The last menu item.
    ak_menu_item* pLastItem;


    /// A pointer to the hovered item.
    ak_menu_item* pHoveredItem;


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

    /// The default font to use for menu item text.
    easygui_font* pFont;

    /// The default color to use for menu item text.
    easygui_color textColor;

    /// The color to use for the text of disabled menu items.
    easygui_color textColorDisabled;

    /// The color of the separator.
    easygui_color separatorColor;

    /// The thickness of the separator.
    float separatorWidth;

    /// The width of the border.
    float borderWidth;

    /// The padding between the top of the menu and the first menu item, and the bottom of the menu and the last menu item.
    float paddingY;

    /// The amount of padding to apply to each side of a menu item.
    float itemPadding;

    /// The amount of padding to apply to the left of the main text of menu items.
    float textPaddingLeft;

    /// The amount of padding to apply to the left of the shortcut text.
    float shortcutTextPaddingLeft;

    /// The amount of padding to apply to the left of the sub-menu arrow.
    float arrowPaddingLeft;

    /// The size of icons, assuming they're square.
    float iconSize;

    /// The size of the sub-menu arrow, assuming it's square.
    float arrowSize;

    


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


    /// The position to draw the icon on the x axis.
    float iconDrawPosX;

    /// The position to draw the text on the x axis.
    float textDrawPosX;

    /// The position to draw the shortcut text on the x axis.
    float shortcutTextDrawPosX;

    /// The position to draw the sub-menu arrow, on the x axis.
    float arrowDrawPosX;


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


    /// The image to use as the menu item's icon.
    easygui_image* pIcon;

    /// The tint color of the icon.
    easygui_color iconTintColor;

    /// The main text of the item.
    char text[AK_MAX_MENU_ITEM_TEXT_LENGTH];

    /// The shortcut text of the item.
    char shortcutText[AK_MAX_MENU_ITEM_TEXT_LENGTH];

    /// Whether or not the item is a separator.
    bool isSeparator;

    
    /// Whether or not the item is disabled.
    bool isDisabled;


    /// The function to call when an item is picked.
    ak_mi_on_picked_proc onPicked;


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

/// Default implementation of the item measure event.
PRIVATE void ak_menu_on_measure_item_default(ak_menu_item* pMI, float* pWidthOut, float* pHeightOut);

/// Paints the given menu item.
PRIVATE void ak_menu_on_paint_item_default(easygui_element* pMenuElement, ak_menu_item* pMI, easygui_rect relativeClippingRect, float posX, float posY, float width, float height, void* pPaintData);

/// Updates the layout data for the default menu items.
PRIVATE void ak_menu_update_item_layout_info(ak_window* pMenuWindow);

/// Resizes the menu based on the size of it's menu items.
PRIVATE void ak_menu_resize_by_items(ak_window* pMenuWindow);

/// Finds the item under the given point.
PRIVATE ak_menu_item* ak_menu_find_item_under_point(ak_window* pMenuWindow, float relativePosX, float relativePosY);

ak_window* ak_create_menu(ak_application* pApplication, ak_window* pParent, size_t extraDataSize, const void* pExtraData)
{
    ak_window* pMenuWindow = ak_create_window(pApplication, ak_window_type_popup, pParent, sizeof(ak_menu) - sizeof(char) + extraDataSize, NULL);
    if (pMenuWindow == NULL) {
        return NULL;
    }

    ak_menu* pMenu = ak_get_window_extra_data(pMenuWindow);
    assert(pMenu != NULL);

    pMenu->pFirstItem              = NULL;
    pMenu->pLastItem               = NULL;
    pMenu->pHoveredItem            = NULL;
    pMenu->borderMask              = ak_menu_border_none;
    pMenu->borderMaskOffset        = 0;
    pMenu->borderMaskLength        = 0;
    pMenu->borderColor             = easygui_rgb(64, 64, 64);
    pMenu->backgroundColor         = easygui_rgb(32, 32, 32);
    pMenu->backgroundColorHovered  = easygui_rgb(64, 64, 64);
    pMenu->pFont                   = NULL;
    pMenu->textColor               = easygui_rgb(224, 224, 224);
    pMenu->textColorDisabled       = easygui_rgb(112, 112, 112);
    pMenu->separatorColor          = easygui_rgb(48, 48, 48);
    pMenu->separatorWidth          = 1;
    pMenu->borderWidth             = 1;
    pMenu->paddingY                = 2;

    pMenu->itemPadding             = 2;
    pMenu->textPaddingLeft         = 8;
    pMenu->shortcutTextPaddingLeft = 32;
    pMenu->arrowPaddingLeft        = 4;
    pMenu->iconSize                = 16;
    pMenu->arrowSize               = 8;

    pMenu->onItemMeasure           = ak_menu_on_measure_item_default;
    pMenu->onItemPaint             = ak_menu_on_paint_item_default;
    pMenu->onShow                  = NULL;
    pMenu->pOnShowData             = NULL;
    pMenu->onHide                  = NULL;
    pMenu->pOnHideData             = NULL;

    pMenu->iconDrawPosX            = 0;
    pMenu->textDrawPosX            = 0;
    pMenu->shortcutTextDrawPosX    = 0;
    pMenu->arrowDrawPosX           = 0;

    pMenu->extraDataSize = extraDataSize;
    if (pExtraData != NULL) {
        memcpy(pMenu->pExtraData, pExtraData, extraDataSize);
    }


    // Window event handlers.
    ak_window_set_on_hide(pMenuWindow, ak_menu_on_hide);
    ak_window_set_on_show(pMenuWindow, ak_menu_on_show);

    // GUI event handlers.
    easygui_set_on_mouse_leave(ak_get_window_panel(pMenuWindow), ak_menu_on_mouse_leave);
    easygui_set_on_mouse_move(ak_get_window_panel(pMenuWindow), ak_menu_on_mouse_move);
    easygui_set_on_mouse_button_up(ak_get_window_panel(pMenuWindow), ak_menu_on_mouse_button_up);
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

void ak_menu_set_font(ak_window* pMenuWindow, easygui_font* pFont)
{
    ak_menu* pMenu = ak_get_window_extra_data(pMenuWindow);
    if (pMenu == NULL) {
        return;
    }

    pMenu->pFont = pFont;
}

easygui_font* ak_menu_get_font(ak_window* pMenuWindow)
{
    ak_menu* pMenu = ak_get_window_extra_data(pMenuWindow);
    if (pMenu == NULL) {
        return NULL;
    }

    return pMenu->pFont;
}

void ak_menu_set_text_color(ak_window* pMenuWindow, easygui_color color)
{
    ak_menu* pMenu = ak_get_window_extra_data(pMenuWindow);
    if (pMenu == NULL) {
        return;
    }

    pMenu->textColor = color;
}

easygui_color ak_menu_get_text_color(ak_window* pMenuWindow)
{
    ak_menu* pMenu = ak_get_window_extra_data(pMenuWindow);
    if (pMenu == NULL) {
        return easygui_rgb(0, 0, 0);
    }

    return pMenu->textColor;
}

void ak_menu_set_separator_style(ak_window* pMenuWindow, easygui_color color, float thickness)
{
    ak_menu* pMenu = ak_get_window_extra_data(pMenuWindow);
    if (pMenu == NULL) {
        return;
    }

    pMenu->separatorColor = color;
    pMenu->separatorWidth = thickness;
}

easygui_color ak_menu_get_separator_color(ak_window* pMenuWindow)
{
    ak_menu* pMenu = ak_get_window_extra_data(pMenuWindow);
    if (pMenu == NULL) {
        return easygui_rgb(0, 0, 0);
    }

    return pMenu->separatorColor;
}

float ak_menu_get_separator_thickness(ak_window* pMenuWindow)
{
    ak_menu* pMenu = ak_get_window_extra_data(pMenuWindow);
    if (pMenu == NULL) {
        return 0;
    }

    return pMenu->separatorWidth;
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

    if (pMenu->pHoveredItem != NULL)
    {
        pMenu->pHoveredItem = NULL;
        easygui_dirty(pMenuElement, easygui_get_local_rect(pMenuElement));
    }
}

void ak_menu_on_mouse_move(easygui_element* pMenuElement, int relativeMousePosX, int relativeMousePosY, int stateFlags)
{
    (void)stateFlags;

    ak_menu* pMenu = ak_get_window_extra_data(ak_get_panel_window(pMenuElement));
    if (pMenu == NULL) {
        return;
    }

    ak_menu_item* pOldHoveredItem = pMenu->pHoveredItem;
    ak_menu_item* pNewHoveredItem = ak_menu_find_item_under_point(ak_get_panel_window(pMenuElement), (float)relativeMousePosX, (float)relativeMousePosY);

    if (pOldHoveredItem != pNewHoveredItem)
    {
        pMenu->pHoveredItem = pNewHoveredItem;
        easygui_dirty(pMenuElement, easygui_get_local_rect(pMenuElement));
    }
}

void ak_menu_on_mouse_button_up(easygui_element* pMenuElement, int mouseButton, int relativeMousePosX, int relativeMousePosY, int stateFlags)
{
    (void)stateFlags;

    ak_menu* pMenu = ak_get_window_extra_data(ak_get_panel_window(pMenuElement));
    if (pMenu == NULL) {
        return;
    }

    if (mouseButton == EASYGUI_MOUSE_BUTTON_LEFT)
    {
        if (pMenu->pHoveredItem != NULL && !pMenu->pHoveredItem->isSeparator && ak_mi_is_enabled(pMenu->pHoveredItem)) {
            ak_mi_on_picked(pMenu->pHoveredItem);
        }
    }
}

void ak_menu_on_paint(easygui_element* pMenuElement, easygui_rect relativeClippingRect, void* pPaintData)
{
    ak_menu* pMenu = ak_get_window_extra_data(ak_get_panel_window(pMenuElement));
    if (pMenu == NULL) {
        return;
    }

    // Before painting we need to update the layout information to use when drawing each item. This will calculate things
    // like th eposition of menu text, shortcut text, the icon and the arrow for sub menus.
    ak_menu_update_item_layout_info(ak_get_panel_window(pMenuElement));

    const float borderWidth = pMenu->borderWidth;

    // Draw each item, making sure to only include the region inside the border to avoid overdraw when the border is drawn.
    if (pMenu->onItemMeasure && pMenu->onItemPaint)
    {
        float runningPosX = borderWidth;
        float runningPosY = borderWidth + pMenu->paddingY;
        for (ak_menu_item* pMI = pMenu->pFirstItem; pMI != NULL; pMI = pMI->pNextItem)
        {
            float itemWidth;
            float itemHeight;
            pMenu->onItemMeasure(pMI, &itemWidth, &itemHeight);
            pMenu->onItemPaint(pMenuElement, pMI, relativeClippingRect, runningPosX, runningPosY, itemWidth, itemHeight, pPaintData);

            runningPosY += itemHeight;
        }
    }

    float scaleX;
    float scaleY;
    easygui_get_inner_scale(pMenuElement, &scaleX, &scaleY);

    
    // Top and bottom padding.
    float menuWidth;
    float menuHeight;
    easygui_get_size(pMenuElement, &menuWidth, &menuHeight);

    menuWidth  = menuWidth / scaleX;
    menuHeight = menuHeight / scaleY;

    easygui_draw_rect(pMenuElement, easygui_make_rect(borderWidth, borderWidth, menuWidth - borderWidth, borderWidth + pMenu->paddingY), pMenu->backgroundColor, pPaintData);
    easygui_draw_rect(pMenuElement, easygui_make_rect(borderWidth, menuHeight - borderWidth - pMenu->paddingY, menuWidth - borderWidth, menuHeight - borderWidth), pMenu->backgroundColor, pPaintData);

    

    // Border.

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

PRIVATE void ak_menu_on_measure_item_default(ak_menu_item* pMI, float* pWidthOut, float* pHeightOut)
{
    assert(pMI != NULL);

    ak_menu* pMenu = ak_get_window_extra_data(pMI->pMenuWindow);
    if (pMenu == NULL) {
        return;
    }

    // Separators are treated slightly differently.
    if (pMI->isSeparator)
    {
        if (pWidthOut) {
            *pWidthOut = pMenu->itemPadding*2;
        }
        if (pHeightOut) {
            *pHeightOut = pMenu->itemPadding*2 + pMenu->separatorWidth;
        }
    }
    else
    {
        float innerScaleX;
        float innerScaleY;
        easygui_get_inner_scale(ak_get_window_panel(pMI->pMenuWindow), &innerScaleX, &innerScaleY);

        float textWidth  = 0;
        float textHeight = 0;
        easygui_measure_string(pMenu->pFont, pMI->text, strlen(pMI->text), innerScaleX, innerScaleY, &textWidth, &textHeight);

        float shortcutTextWidth  = 0;
        float shortcutTextHeight = 0;
        easygui_measure_string(pMenu->pFont, pMI->shortcutText, strlen(pMI->shortcutText), innerScaleX, innerScaleY, &shortcutTextWidth, &shortcutTextHeight);


        if (pWidthOut) {
            *pWidthOut = pMenu->itemPadding + pMenu->iconSize + pMenu->textPaddingLeft + textWidth + pMenu->shortcutTextPaddingLeft + shortcutTextWidth + pMenu->arrowPaddingLeft + pMenu->arrowSize + pMenu->itemPadding;
        }
        if (pHeightOut) {
            *pHeightOut = dr_max(textHeight, dr_max(shortcutTextHeight, dr_max(pMenu->iconSize, pMenu->arrowSize))) + (pMenu->itemPadding*2);
        }
    }
}

PRIVATE void ak_menu_on_paint_item_default(easygui_element* pMenuElement, ak_menu_item* pMI, easygui_rect relativeClippingRect, float posX, float posY, float width, float height, void* pPaintData)
{
    assert(pMI != NULL);

    ak_menu* pMenu = ak_get_window_extra_data(pMI->pMenuWindow);
    if (pMenu == NULL) {
        return;
    }

    const float borderWidth = pMenu->borderWidth;
    const float padding = pMenu->itemPadding;

    float scaleX;
    float scaleY;
    easygui_get_inner_scale(pMenuElement, &scaleX, &scaleY);

    float menuWidth;
    float menuHeight;
    easygui_get_size(pMenuElement, &menuWidth, &menuHeight);

    menuWidth  /= scaleX;
    menuHeight /= scaleY;


    easygui_color bgcolor   = pMenu->backgroundColor;
    easygui_color textColor = pMenu->textColor;
    
    // Separators are treated differently to normal items.
    if (pMI->isSeparator)
    {
        // Separator.
        easygui_draw_rect(pMenuElement, easygui_make_rect(posX + padding, posY + padding, posX + menuWidth - borderWidth - padding, posY + padding + pMenu->separatorWidth), pMenu->separatorColor, pPaintData);
    }
    else
    {
        // Normal item.

        if (pMI == pMenu->pHoveredItem) {
            bgcolor = pMenu->backgroundColorHovered;
        }

        if (!ak_mi_is_enabled(pMI)) {
            bgcolor   = pMenu->backgroundColor;
            textColor = pMenu->textColorDisabled;
        }


        // Icon.
        if (pMI->pIcon != NULL)
        {
            unsigned int iconWidth;
            unsigned int iconHeight;
            easygui_get_image_size(pMI->pIcon, &iconWidth, &iconHeight);

            easygui_draw_image_args args;
            args.dstX            = posX + pMenu->iconDrawPosX;
            args.dstY            = posY + pMenu->itemPadding;
            args.dstWidth        = (float)16;
            args.dstHeight       = (float)16;
            args.srcX            = 0;
            args.srcY            = 0;
            args.srcWidth        = (float)iconWidth;
            args.srcHeight       = (float)iconHeight;
            args.dstBoundsX      = args.dstX;
            args.dstBoundsY      = args.dstY;
            args.dstBoundsWidth  = pMenu->iconSize;
            args.dstBoundsHeight = height - (padding*2);
            args.foregroundTint  = pMI->iconTintColor;
            args.backgroundColor = bgcolor;
            args.boundsColor     = bgcolor;
            args.options         = EASYGUI_IMAGE_DRAW_BACKGROUND | EASYGUI_IMAGE_DRAW_BOUNDS | EASYGUI_IMAGE_CLIP_BOUNDS | EASYGUI_IMAGE_ALIGN_CENTER;
            easygui_draw_image(pMenuElement, pMI->pIcon, &args, pPaintData);
        }
        else
        {
            easygui_draw_rect(pMenuElement, easygui_make_rect(posX + pMenu->iconDrawPosX, posY + pMenu->itemPadding, posX + pMenu->iconDrawPosX + pMenu->iconSize, posY + height - pMenu->itemPadding), bgcolor, pPaintData);
        }


        // Space between the icon and the main text.
        easygui_draw_rect(pMenuElement, easygui_make_rect(posX + pMenu->iconDrawPosX + pMenu->iconSize, posY + pMenu->itemPadding, posX + pMenu->textDrawPosX, posY + height - padding), bgcolor, pPaintData);


        // Text.
        float textWidth = 0;
        float textHeight = 0;
        easygui_measure_string(pMenu->pFont, pMI->text, strlen(pMI->text), scaleX, scaleY, &textWidth, &textHeight);

        float textPosX = posX + pMenu->textDrawPosX;
        float textPosY = posY + ((height - textHeight) / 2);
        easygui_draw_text(pMenuElement, pMenu->pFont, pMI->text, (int)strlen(pMI->text), textPosX, textPosY, textColor, bgcolor, pPaintData);

        // The gap between the bottom padding and the text, if any.
        if (textPosY + textHeight < posY + height - padding) {
            easygui_draw_rect(pMenuElement, easygui_make_rect(textPosX, textPosY + textHeight, textPosX + textWidth, posY + height - padding), bgcolor, pPaintData);
        }

        // The gap between the top padding and the text, if any.
        if (textPosY > posY + padding) {
            easygui_draw_rect(pMenuElement, easygui_make_rect(textPosX, posY + padding, textPosX + textWidth, textPosY), bgcolor, pPaintData);
        }


        // Space between the main text and the shortcut.
        easygui_draw_rect(pMenuElement, easygui_make_rect(posX + pMenu->textDrawPosX + textWidth, posY + pMenu->itemPadding, posX + pMenu->shortcutTextDrawPosX, posY + height - padding), bgcolor, pPaintData);
    

        // Shortcut text.
        float shortcutTextWidth = 0;
        float shortcutTextHeight = 0;
        easygui_measure_string(pMenu->pFont, pMI->shortcutText, strlen(pMI->shortcutText), scaleX, scaleY, &shortcutTextWidth, &shortcutTextHeight);

        float shortcutTextPosX = posX + pMenu->shortcutTextDrawPosX;
        float shortcutTextPosY = posY + ((height - shortcutTextHeight) / 2);
        easygui_draw_text(pMenuElement, pMenu->pFont, pMI->shortcutText, (int)strlen(pMI->shortcutText), shortcutTextPosX, shortcutTextPosY, textColor, bgcolor, pPaintData);

        // The gap between the bottom padding and the text, if any.
        if (shortcutTextPosY + shortcutTextHeight < posY + height - padding) {
            easygui_draw_rect(pMenuElement, easygui_make_rect(shortcutTextPosX, shortcutTextPosY + shortcutTextHeight, shortcutTextPosX + shortcutTextWidth, posY + height - padding), bgcolor, pPaintData);
        }

        // The gap between the top padding and the text, if any.
        if (shortcutTextPosY > posY + padding) {
            easygui_draw_rect(pMenuElement, easygui_make_rect(shortcutTextPosX, posY + padding, shortcutTextPosX + shortcutTextWidth, shortcutTextPosY), bgcolor, pPaintData);
        }


        // Space between the shortcut text and the arrow.
        easygui_draw_rect(pMenuElement, easygui_make_rect(posX + pMenu->shortcutTextDrawPosX + shortcutTextWidth, posY + pMenu->itemPadding, posX + pMenu->arrowDrawPosX, posY + height - padding), bgcolor, pPaintData);

        // Arrow.
        //if (ak_mi_get_sub_menu(pMI) != NULL)
        //{
        //}
        //else
        {
            // There is no arrow - just draw a blank rectangle.
            easygui_draw_rect(pMenuElement, easygui_make_rect(posX + pMenu->arrowDrawPosX, posY + pMenu->itemPadding, posX + pMenu->arrowDrawPosX + pMenu->arrowSize, posY + height - padding), bgcolor, pPaintData);
        }
    }


    // Padding.
    easygui_draw_rect_outline(pMenuElement, easygui_make_rect(posX, posY, posX + menuWidth - borderWidth*2, posY + height), bgcolor, padding, pPaintData);
}

PRIVATE void ak_menu_update_item_layout_info(ak_window* pMenuWindow)
{
    ak_menu* pMenu = ak_get_window_extra_data(pMenuWindow);
    if (pMenu == NULL) {
        return;
    }

    float maxTextWidth  = 0;
    float maxShortcutTextWidth = 0;
    if (pMenu->onItemMeasure && pMenu->onItemPaint)
    {
        for (ak_menu_item* pMI = pMenu->pFirstItem; pMI != NULL; pMI = pMI->pNextItem)
        {
            if (!pMI->isSeparator)
            {
                float innerScaleX;
                float innerScaleY;
                easygui_get_inner_scale(ak_get_window_panel(pMI->pMenuWindow), &innerScaleX, &innerScaleY);

                float textWidth  = 0;
                float textHeight = 0;
                easygui_measure_string(pMenu->pFont, pMI->text, strlen(pMI->text), innerScaleX, innerScaleY, &textWidth, &textHeight);

                float shortcutTextWidth  = 0;
                float shortcutTextHeight = 0;
                easygui_measure_string(pMenu->pFont, pMI->shortcutText, strlen(pMI->shortcutText), innerScaleX, innerScaleY, &shortcutTextWidth, &shortcutTextHeight);

                maxTextWidth = dr_max(maxTextWidth, textWidth);
                maxShortcutTextWidth = dr_max(maxShortcutTextWidth, shortcutTextWidth);
            }
        }
    }

    pMenu->iconDrawPosX         = pMenu->itemPadding;
    pMenu->textDrawPosX         = pMenu->iconDrawPosX + pMenu->iconSize + pMenu->textPaddingLeft;
    pMenu->shortcutTextDrawPosX = pMenu->textDrawPosX + maxTextWidth + pMenu->shortcutTextPaddingLeft;
    pMenu->arrowDrawPosX        = pMenu->shortcutTextDrawPosX + maxShortcutTextWidth + pMenu->arrowPaddingLeft;
}

PRIVATE void ak_menu_resize_by_items(ak_window* pMenuWindow)
{
    ak_menu* pMenu = ak_get_window_extra_data(pMenuWindow);
    if (pMenu == NULL) {
        return;
    }

    float scaleX;
    float scaleY;
    easygui_get_inner_scale(ak_get_window_panel(pMenuWindow), &scaleX, &scaleY);


    const float borderWidth = pMenu->borderWidth;

    float menuWidth = 0;
    float menuHeight = 0;

    if (pMenu->onItemMeasure)
    {
        for (ak_menu_item* pMI = pMenu->pFirstItem; pMI != NULL; pMI = pMI->pNextItem)
        {
            float itemWidth = 0;
            float itemHeight = 0;
            pMenu->onItemMeasure(pMI, &itemWidth, &itemHeight);

            menuWidth = dr_max(menuWidth, itemWidth);
            menuHeight += itemHeight;
        }
    }
    
    menuWidth  += borderWidth*2;
    menuHeight += pMenu->paddingY*2 + borderWidth*2;

    ak_menu_set_size(pMenuWindow, (unsigned int)(menuWidth * scaleX), (unsigned int)(menuHeight * scaleY));
}

PRIVATE ak_menu_item* ak_menu_find_item_under_point(ak_window* pMenuWindow, float relativePosX, float relativePosY)
{
    ak_menu* pMenu = ak_get_window_extra_data(pMenuWindow);
    if (pMenu == NULL) {
        return NULL;
    }

    if (pMenu->onItemMeasure == NULL) {
        return NULL;
    }

    int menuWidth = 0;
    int menuHeight = 0;
    ak_get_window_size(pMenuWindow, &menuWidth, &menuHeight);

    float runningPosY = pMenu->borderWidth + pMenu->paddingY;
    for (ak_menu_item* pMI = pMenu->pFirstItem; pMI != NULL; pMI = pMI->pNextItem)
    {
        float itemWidth = 0;
        float itemHeight = 0;
        pMenu->onItemMeasure(pMI, &itemWidth, &itemHeight);

        if (relativePosX >= 0 && relativePosX < (float)menuWidth && relativePosY >= runningPosY && relativePosY < runningPosY + (float)itemHeight)
        {
            return pMI;
        }
        
        runningPosY += itemHeight;
    }

    return NULL;
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

    pMI->pMenuWindow     = NULL;
    pMI->pNextItem       = NULL;
    pMI->pPrevItem       = NULL;
    pMI->pIcon           = NULL;
    pMI->iconTintColor   = easygui_rgb(255, 255, 255);
    pMI->text[0]         = '\0';
    pMI->shortcutText[0] = '\0';
    pMI->isSeparator     = false;
    pMI->isDisabled      = false;
    pMI->onPicked        = NULL;
    
    pMI->extraDataSize = extraDataSize;
    if (pExtraData != NULL) {
        memcpy(pMI->pExtraData, pExtraData, extraDataSize);
    }


    // Append the item to the end of the list.
    ak_mi_append(pMI, pMenuWindow);

    return pMI;
}

ak_menu_item* ak_create_separator_menu_item(ak_window* pMenuWindow, size_t extraDataSize, const void* pExtraData)
{
    ak_menu_item* pMI = ak_create_menu_item(pMenuWindow, extraDataSize, pExtraData);
    if (pMI == NULL) {
        return NULL;
    }

    pMI->isSeparator = true;

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


bool ak_mi_is_separator(ak_menu_item* pMI)
{
    if (pMI == NULL) {
        return false;
    }

    return pMI->isSeparator;
}


void ak_mi_set_icon(ak_menu_item* pMI, easygui_image* pImage)
{
    if (pMI == NULL) {
        return;
    }

    pMI->pIcon = pImage;
}

easygui_image* ak_mi_get_icon(ak_menu_item* pMI)
{
    if (pMI == NULL) {
        return NULL;
    }
    
    return pMI->pIcon;
}

void ak_mi_set_icon_tint(ak_menu_item* pMI, easygui_color tint)
{
    if (pMI == NULL) {
        return;
    }

    pMI->iconTintColor = tint;
}

easygui_color ak_mi_get_icon_tint(ak_menu_item* pMI)
{
    if (pMI == NULL) {
        return easygui_rgb(0, 0, 0);
    }

    return pMI->iconTintColor;
}


void ak_mi_set_text(ak_menu_item* pMI, const char* text)
{
    if (pMI == NULL) {
        return;
    }

    if (text != NULL) {
        strcpy_s(pMI->text, sizeof(pMI->text), text);
    } else {
        pMI->text[0] = '\0';
    }

    ak_menu_resize_by_items(pMI->pMenuWindow);
}

const char* ak_mi_get_text(ak_menu_item* pMI)
{
    if (pMI == NULL) {
        return NULL;
    }

    return pMI->text;
}

void ak_mi_set_shortcut_text(ak_menu_item* pMI, const char* text)
{
    if (pMI == NULL) {
        return;
    }

    if (text != NULL) {
        strcpy_s(pMI->shortcutText, sizeof(pMI->shortcutText), text);
    } else {
        pMI->shortcutText[0] = '\0';
    }

    ak_menu_resize_by_items(pMI->pMenuWindow);
}

const char* ak_mi_get_shortcut_text(ak_menu_item* pMI)
{
    if (pMI == NULL) {
        return NULL;
    }

    return pMI->shortcutText;
}


void ak_mi_disable(ak_menu_item* pMI)
{
    if (pMI == NULL) {
        return;
    }

    if (!pMI->isDisabled)
    {
        pMI->isDisabled = true;
        easygui_dirty(ak_get_window_panel(pMI->pMenuWindow), easygui_get_local_rect(ak_get_window_panel(pMI->pMenuWindow)));
    }
}

void ak_mi_enable(ak_menu_item* pMI)
{
    if (pMI == NULL) {
        return;
    }

    if (pMI->isDisabled)
    {
        pMI->isDisabled = false;
        easygui_dirty(ak_get_window_panel(pMI->pMenuWindow), easygui_get_local_rect(ak_get_window_panel(pMI->pMenuWindow)));
    }
}

bool ak_mi_is_enabled(ak_menu_item* pMI)
{
    if (pMI == NULL) {
        return false;
    }

    return !pMI->isDisabled;
}


void ak_mi_set_on_picked(ak_menu_item* pMI, ak_mi_on_picked_proc proc)
{
    if (pMI == NULL) {
        return;
    }

    pMI->onPicked = proc;
}

void ak_mi_on_picked(ak_menu_item* pMI)
{
    if (pMI == NULL) {
        return;
    }

    if (pMI->onPicked) {
        pMI->onPicked(pMI);
    }
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

    // The window needs to be resized.
    ak_menu_resize_by_items(pMI->pMenuWindow);

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

    // The window needs to be resized.
    ak_menu_resize_by_items(pMI->pMenuWindow);

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