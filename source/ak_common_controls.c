
#include "../include/easy_appkit/ak_common_controls.h"
#include <easy_gui/easy_gui.h>
#include <easy_util/easy_util.h>
#include <assert.h>

#include <stdio.h>      // For debugging. Delete me.

///////////////////////////////////////////////////////////////////////////////
//
// Tree-View Control
//
///////////////////////////////////////////////////////////////////////////////

static const char* g_TreeView_ArrowFacingRightString = "\xE2\x96\xB6";       // U+25B6
//static const char* g_TreeView_ArrowFacingDownString  = "\xE2\x96\xBC";       // U+25BC. Straight down.
static const char* g_TreeView_ArrowFacingDownString  = "\xE2\x97\xA2";       // U+25E2. Diagonal (Windows' style)

static unsigned int g_TreeView_ArrowFacingRightUTF32 = 0x25B6;
static unsigned int g_TreeView_ArrowFacingDownUTF32  = 0x25E2;       // U+25E2. Diagonal (Windows' style)

/// Detaches the given tree view item from the tree.
PRIVATE void ak_detach_tree_view_item(ak_tree_view_item* pItem);

/// Refreshes the layout and schedules a redraw of the given tree view control.
PRIVATE void ak_refresh_and_redraw_tree_view(easygui_element* pTV);


typedef struct ak_tree_view ak_tree_view;
struct ak_tree_view
{
    /// A pointer to the root item.
    ak_tree_view_item* pRootItem;

    /// A pointer to the item that the mouse is current hovered over.
    ak_tree_view_item* pHoveredItem;

    /// Whether or not the mouse is sitting over the top of the arrow pHoveredItem.
    bool arrowHovered;


    /// The offset to apply to every item on the x axis. Used for scrolling.
    float offsetX;

    /// The offset to apply to every item on the y axis. Used for scrolling.
    float offsetY;


    /// The font to use when drawing tree-view item text.
    easygui_font* pTextFont;

    /// The color to use when drawing text.
    easygui_color textColor;

    /// The padding to apply to each item.
    float textPadding;


    /// The font to use for the arrow glyphs.
    easygui_font* pArrowFont;

    /// The color to use when drawing the arrow.
    easygui_color arrowColor;

    /// The font metrics of the arrow glyph.
    easygui_font_metrics arrowFontMetrics;

    /// The glyph metrics of the arrow glyph.
    easygui_glyph_metrics arrowMetrics;


    /// Event handlers.
    ak_on_tree_view_item_picked_proc onItemPicked;


    /// The size of the extra data.
    size_t extraDataSize;

    /// A pointer to the extra data buffer.
    char pExtraData[1];
};

struct ak_tree_view_item
{
    /// A pointer to the main tree-view object.
    easygui_element* pTV;


    /// The tree view item text.
    char text[AK_MAX_TREE_VIEW_ITEM_TEXT_LENGTH];


    /// A pointer to the parent item.
    ak_tree_view_item* pParent;

    /// A pointer to the first child.
    ak_tree_view_item* pFirstChild;

    /// A pointer to the last child.
    ak_tree_view_item* pLastChild;

    /// A pointer to the next sibling.
    ak_tree_view_item* pNextSibling;

    /// A pointer to the prev sibling.
    ak_tree_view_item* pPrevSibling;


    /// Whether or not the item is select.
    bool isSelected;

    /// Whether or not the item is expanded.
    bool isExpanded;


    /// The size of the extra data, in bytes.
    size_t extraDataSize;

    /// A pointer to the extra data.
    char pExtraData[1];
};

typedef struct
{
    /// The position of the item relative to the left of the tree-view.
    float posX;

    /// The position of the item relative to the top of the tree-view.
    float posY;

} ak_tree_view_item_metrics;

typedef struct
{
    /// A pointer to the tree-view item that is hit.
    ak_tree_view_item* pItem;

    /// The metrics of the item.
    ak_tree_view_item_metrics itemMetrics;

    /// Whether or not the point is over the arrow.
    bool hitArrow;

} ak_tree_view_hit_test_result;

/// Helper for finding the height of an item.
PRIVATE float ak_get_tree_view_item_height(easygui_element* pTV)
{
    ak_tree_view* pTVData = easygui_get_extra_data(pTV);
    assert(pTVData != NULL);

    easygui_font_metrics fontMetrics;
    easygui_get_font_metrics(pTVData->pTextFont, &fontMetrics);

    return fontMetrics.lineHeight + (pTVData->textPadding*2);
}

/// Helper for finding the width of the arrow.
PRIVATE float ak_get_tree_view_arrow_width(easygui_element* pTV)
{
    ak_tree_view* pTVData = easygui_get_extra_data(pTV);
    assert(pTVData != NULL);

    return (float)pTVData->arrowMetrics.width + (pTVData->textPadding*2);
}

/// Helper for finding the height of the arrow.
PRIVATE float ak_get_tree_view_arrow_height(easygui_element* pTV)
{
    ak_tree_view* pTVData = easygui_get_extra_data(pTV);
    assert(pTVData != NULL);

    return (float)pTVData->arrowMetrics.height + (pTVData->textPadding*2);
}

/// relativePosX and relativePosY are relative to the main tree-view control.
PRIVATE ak_tree_view_item* ak_find_tree_view_item_under_point_recursive(easygui_element* pTV, ak_tree_view_item* pItem, unsigned int relativePosX, unsigned int relativePosY, float runningPosX, float runningPosY, ak_tree_view_item_metrics* pMetricsOut)
{
    assert(pItem  != NULL);

    ak_tree_view* pTVData = easygui_get_extra_data(pTV);
    assert(pTVData != NULL);

    float offsetPosX = relativePosX + pTVData->offsetX;
    float offsetPosY = relativePosY + pTVData->offsetY;

    if (pItem != pTVData->pRootItem)        // <-- Never check the root item.
    {
        easygui_rect childRect;
        childRect.left   = 0;
        childRect.right  = easygui_get_width(pTV);
        childRect.top    = runningPosY;
        childRect.bottom = childRect.top + ak_get_tree_view_item_height(pTV);

        if (easygui_rect_contains_point(childRect, offsetPosX, offsetPosY))
        {
            // This child contains the point.
            if (pMetricsOut != NULL)
            {
                pMetricsOut->posX = runningPosX;
                pMetricsOut->posY = runningPosY;
            }

            return pItem;
        }
    }


    // At this point we know that the input item does not contain the point. Check it's children.
    if (ak_is_tree_view_item_expanded(pItem))
    {
        if (pItem != pTVData->pRootItem) {
            runningPosX += 16;
        }

        for (ak_tree_view_item* pChild = pItem->pFirstChild; pChild != NULL; pChild = pChild->pNextSibling)
        {
            if (pItem != pTVData->pRootItem) {
                runningPosY += ak_get_tree_view_item_height(pTV);
            }
        
            ak_tree_view_item* pItemUnderPoint = ak_find_tree_view_item_under_point_recursive(pTV, pChild, relativePosX, relativePosY, runningPosX, runningPosY, pMetricsOut);
            if (pItemUnderPoint != NULL) {
                return pItemUnderPoint;
            }
        }
    }

    

    // If we make it here it means we were unable to find a child under the given point.
    return NULL;
}

/// Finds the tree-view item that's sitting under the given point, in relative coordinates.
PRIVATE ak_tree_view_item* ak_find_tree_view_item_under_point(easygui_element* pTV, unsigned int relativePosX, unsigned int relativePosY, ak_tree_view_item_metrics* pMetricsOut)
{
    ak_tree_view* pTVData = easygui_get_extra_data(pTV);
    assert(pTVData != NULL);

    return ak_find_tree_view_item_under_point_recursive(pTV, pTVData->pRootItem, relativePosX, relativePosY, 0, 0, pMetricsOut);
}

/// Performs a hit test against the given point.
PRIVATE bool ak_do_tree_view_hit_test(easygui_element* pTV, unsigned int relativePosX, unsigned int relativePosY, ak_tree_view_hit_test_result* pResult)
{
    ak_tree_view* pTVData = easygui_get_extra_data(pTV);
    assert(pTVData != NULL);

    ak_tree_view_hit_test_result result;
    result.pItem = ak_find_tree_view_item_under_point(pTV, relativePosX, relativePosY, &result.itemMetrics);
    result.hitArrow = false;

    if (pResult != NULL)
    {
        if (result.pItem != NULL && ak_does_tree_view_item_have_children(result.pItem))
        {
            // Now check if the point is over the arrow.
            float offsetPosX = relativePosX + pTVData->offsetX;

            float arrowLeft  = result.itemMetrics.posX;
            float arrowRight = arrowLeft + ak_get_tree_view_arrow_width(pTV);

            result.hitArrow = offsetPosX >= arrowLeft && offsetPosX <= arrowRight;
        }

        *pResult = result;
    }

    return result.pItem != NULL;
}


/// Deselects every item recursively, including the given element. This does not mark the element as dirty - that must be done at a higher level.
PRIVATE void ak_deselect_all_tree_view_items_recursive(ak_tree_view_item* pItem)
{
    pItem->isSelected = false;

    for (ak_tree_view_item* pChild = pItem->pFirstChild; pChild != NULL; pChild = pChild->pNextSibling)
    {
        ak_deselect_all_tree_view_items_recursive(pChild);
    }
}



// Recursively draws a tree view item. Returns false if the item is not within the clipping rectangle.
PRIVATE bool ak_draw_tree_view_item(easygui_element* pTV, ak_tree_view_item* pItem, float penPosX, float penPosY, easygui_rect relativeClippingRect, void* pPaintData)
{
    // TODO: Check against clipping rectangle. If completely hidden, return false.

    ak_tree_view* pTVData = easygui_get_extra_data(pTV);
    assert(pTVData != NULL);

    easygui_font_metrics fontMetrics;
    if (!easygui_get_font_metrics(pTVData->pTextFont, &fontMetrics)) {
        assert(false);
    }


    // Background. TODO: Only draw the background region that falls outside of the text rectangle in order to prevent overdraw.
    easygui_color bgcolor = easygui_rgb(96, 96, 96);
    if (pTVData->pHoveredItem == pItem) {
        bgcolor = easygui_rgb(112, 112, 112);
    }

    if (ak_is_tree_view_item_selected(pItem)) {
        bgcolor = easygui_rgb(140, 140, 140);
    }


    easygui_draw_rect(pTV, easygui_make_rect(0, penPosY, easygui_get_width(pTV), penPosY + ak_get_tree_view_item_height(pTV)), bgcolor, pPaintData);


    // Arrow. Only draw this if we have children. TODO: Only draw the background region that falls outside of the arrow's rectangle.
    if (pItem->pFirstChild != NULL)
    {
        float arrowPosX = penPosX + pTVData->textPadding;
        float arrowPosY = penPosY + ((ak_get_tree_view_item_height(pTV) - pTVData->arrowMetrics.height) / 2) + (pTVData->arrowMetrics.originY - pTVData->arrowFontMetrics.ascent);

        easygui_color arrowColor = pTVData->arrowColor;
        if (pTVData->pHoveredItem == pItem && pTVData->arrowHovered) {
            arrowColor = easygui_rgb(255, 255, 255);
        }

        // TODO: Branch here depending on whether or not the item is expanded.
        if (ak_is_tree_view_item_expanded(pItem)) {
            easygui_draw_text(pTV, pTVData->pArrowFont, g_TreeView_ArrowFacingDownString, strlen(g_TreeView_ArrowFacingDownString), arrowPosX, arrowPosY, arrowColor, bgcolor, pPaintData);
        } else {
            easygui_draw_text(pTV, pTVData->pArrowFont, g_TreeView_ArrowFacingRightString, strlen(g_TreeView_ArrowFacingRightString), arrowPosX, arrowPosY, arrowColor, bgcolor, pPaintData);
        }
        
    }


    // Text.
    float textPosX = penPosX + ak_get_tree_view_arrow_width(pTV) + pTVData->textPadding;
    float textPosY = penPosY + pTVData->textPadding;
    easygui_draw_text(pTV, pTVData->pTextFont, pItem->text, strlen(pItem->text), textPosX, textPosY, pTVData->textColor, bgcolor, pPaintData);


    // Now we need to draw children if the item is expanded.
    if (ak_is_tree_view_item_expanded(pItem))
    {
        if (pItem != pTVData->pRootItem) {
            penPosX += 16;
        }
    
        for (ak_tree_view_item* pChild = pItem->pFirstChild; pChild != NULL; pChild = pChild->pNextSibling)
        {
            if (pItem != pTVData->pRootItem) {
                penPosY += ak_get_tree_view_item_height(pTV);
            }
        

            // False will be returned if the child is not visible. In this case we can know that the next children will also be invisible
            // so we just terminate from the loop early.
            if (!ak_draw_tree_view_item(pTV, pChild, penPosX, penPosY, relativeClippingRect, pPaintData)) {
                break;
            }
        }
    }

    return true;
}


PRIVATE void ak_measure_tree_view_items_recursive(ak_tree_view_item* pItem, float runningPosX, float runningPosY, float* pWidthOut, float* pHeightOut)
{
    assert(pItem != NULL);

    ak_tree_view* pTVData = easygui_get_extra_data(pItem->pTV);
    assert(pTVData != NULL);


    if (pWidthOut != NULL)
    {
        float textWidth;
        easygui_measure_string(pTVData->pTextFont, pItem->text, strlen(pItem->text), &textWidth, NULL);

        float itemRight  = runningPosX + ak_get_tree_view_arrow_width(pItem->pTV) + textWidth + pTVData->textPadding;
            
        if (itemRight > *pWidthOut) {
            *pWidthOut = itemRight;
        }
    }

    if (pHeightOut != NULL)
    {
        float itemBottom = runningPosY + ak_get_tree_view_item_height(pItem->pTV);

        *pHeightOut = itemBottom;
    }



    if (ak_is_tree_view_item_expanded(pItem))
    {
        if (pItem != pTVData->pRootItem) {
            runningPosX += 16;
        }

        for (ak_tree_view_item* pChild = pItem->pFirstChild; pChild != NULL; pChild = pChild->pNextSibling)
        {
            if (pItem != pTVData->pRootItem) {
                runningPosY += ak_get_tree_view_item_height(pItem->pTV);
            }

            ak_measure_tree_view_items_recursive(pChild, runningPosX, runningPosY, pWidthOut, pHeightOut);
        }
    }
}

PRIVATE void ak_measure_tree_view_items(easygui_element* pTV, float* pWidthOut, float* pHeightOut)
{
    ak_tree_view* pTVData = easygui_get_extra_data(pTV);
    assert(pTVData != NULL);

    ak_measure_tree_view_items_recursive(pTVData->pRootItem, 0, 0, pWidthOut, pHeightOut);
}


static void ak_tree_view_on_paint(easygui_element* pTV, easygui_rect relativeClippingRect, void* pPaintData)
{
    (void)relativeClippingRect;

    ak_tree_view* pTVData = easygui_get_extra_data(pTV);
    if (pTVData == NULL) {
        return;
    }

    // Each visible item needs to be drawn recursively. We start from top to bottom, starting from the first visible item and ending
    // at the last visible item.

    // TEMP: For now, just draw every item to get the basics working.
    ak_draw_tree_view_item(pTV, pTVData->pRootItem, 0, 0, relativeClippingRect, pPaintData);


    // Draw the part of the background that's not covered by items.
    float itemsBottom;
    ak_measure_tree_view_items(pTV, NULL, &itemsBottom);
    itemsBottom += pTVData->offsetX;

    easygui_draw_rect(pTV, easygui_make_rect(0, itemsBottom, easygui_get_width(pTV), easygui_get_height(pTV)), easygui_rgb(96, 96, 96), pPaintData);
}

static void ak_tree_view_on_mouse_leave(easygui_element* pTV)
{
    ak_tree_view* pTVData = easygui_get_extra_data(pTV);
    if (pTVData == NULL) {
        return;
    }

    if (pTVData->pHoveredItem != NULL || pTVData->arrowHovered)
    {
        pTVData->pHoveredItem = NULL;
        pTVData->arrowHovered = false;
        
        // For now just redraw the entire control, but should optimize this to only redraw the regions of the new and old hovered items.
        easygui_dirty(pTV, easygui_get_local_rect(pTV));
    }
}

static void ak_tree_view_on_mouse_move(easygui_element* pTV, int relativeMousePosX, int relativeMousePosY)
{
    ak_tree_view* pTVData = easygui_get_extra_data(pTV);
    if (pTVData == NULL) {
        return;
    }

    ak_tree_view_hit_test_result hit;
    ak_do_tree_view_hit_test(pTV, relativeMousePosX, relativeMousePosY, &hit);

    if (hit.pItem != pTVData->pHoveredItem || hit.hitArrow != pTVData->arrowHovered)
    {
        // The tree-view needs to now about the new hovered item so it can know to draw it with the hovered style.
        pTVData->pHoveredItem = hit.pItem;
        pTVData->arrowHovered = hit.hitArrow;

        // For now just redraw the entire control, but should optimize this to only redraw the regions of the new and old hovered items.
        easygui_dirty(pTV, easygui_get_local_rect(pTV));
    }
}

static void ak_tree_view_on_mouse_button_down(easygui_element* pTV, int mouseButton, int relativeMousePosX, int relativeMousePosY)
{
    ak_tree_view* pTVData = easygui_get_extra_data(pTV);
    if (pTVData == NULL) {
        return;
    }

    ak_tree_view_hit_test_result hit;
    ak_do_tree_view_hit_test(pTV, relativeMousePosX, relativeMousePosY, &hit);

    if (mouseButton == EASYGUI_MOUSE_BUTTON_LEFT)
    {
        if (hit.pItem != NULL)
        {
            // TODO: If the CTRL key is down, don't deselect. If shift is down, select the range.
            ak_deselect_all_tree_view_items(pTV);
            ak_select_tree_view_item(hit.pItem);

            // If we hit the arrow, expand it.
            if (hit.hitArrow) {
                if (ak_is_tree_view_item_expanded(hit.pItem)) {
                    ak_collapse_tree_view_item(hit.pItem);
                } else {
                    ak_expand_tree_view_item(hit.pItem);
                }
            }
        }
    }
}

static void ak_tree_view_on_mouse_button_up(easygui_element* pTV, int mouseButton, int relativeMousePosX, int relativeMousePosY)
{
    (void)pTV;
    (void)mouseButton;
    (void)relativeMousePosX;
    (void)relativeMousePosY;
}

static void ak_tree_view_on_mouse_button_dblclick(easygui_element* pTV, int mouseButton, int relativeMousePosX, int relativeMousePosY)
{
    ak_tree_view* pTVData = easygui_get_extra_data(pTV);
    if (pTVData == NULL) {
        return;
    }

    ak_tree_view_hit_test_result hit;
    ak_do_tree_view_hit_test(pTV, relativeMousePosX, relativeMousePosY, &hit);

    if (mouseButton == EASYGUI_MOUSE_BUTTON_LEFT)
    {
        if (hit.pItem != NULL && !hit.hitArrow)
        {
            if (ak_does_tree_view_item_have_children(hit.pItem))
            {
                if (ak_is_tree_view_item_expanded(hit.pItem)) {
                    ak_collapse_tree_view_item(hit.pItem);
                } else {
                    ak_expand_tree_view_item(hit.pItem);
                }
            }
            else
            {
                if (pTVData->onItemPicked) {
                    pTVData->onItemPicked(hit.pItem);
                }
            }
        }
    }
}

static void ak_tree_view_on_mouse_wheel(easygui_element* pTV, int delta, int relativeMousePosX, int relativeMousePosY)
{
    (void)pTV;
    (void)delta;
    (void)relativeMousePosX;
    (void)relativeMousePosY;
}


easygui_element* ak_create_tree_view(easygui_context* pContext, easygui_element* pParent, easygui_font* pFont, easygui_color textColor, size_t extraDataSize, const void* pExtraData)
{
    easygui_element* pTV = easygui_create_element(pContext, pParent, sizeof(ak_tree_view) - sizeof(char) + extraDataSize);
    if (pTV == NULL) {
        return NULL;
    }

    ak_tree_view* pTVData = easygui_get_extra_data(pTV);
    if (pTVData == NULL) {
        return NULL;
    }

    pTVData->pHoveredItem = NULL;
    pTVData->arrowHovered = false;
    pTVData->offsetX = 0;
    pTVData->offsetY = 0;


    // Theme.
    pTVData->pTextFont   = pFont;
    pTVData->textColor   = textColor;
    pTVData->textPadding = 2;

    pTVData->pArrowFont = easygui_create_font(pTV->pContext, "Segoe UI Symbol", 9, easygui_weight_normal, easygui_slant_none, 0);   // TODO: Change this font for non-Win32 builds, or allow the application to draw their own arrows.
    pTVData->arrowColor = easygui_rgb(224, 224, 224);
    easygui_get_font_metrics(pTVData->pArrowFont, &pTVData->arrowFontMetrics);
    easygui_get_glyph_metrics(pTVData->pArrowFont, g_TreeView_ArrowFacingRightUTF32, &pTVData->arrowMetrics);


    // Callbacks.
    easygui_register_on_paint(pTV, ak_tree_view_on_paint);
    easygui_register_on_mouse_leave(pTV, ak_tree_view_on_mouse_leave);
    easygui_register_on_mouse_move(pTV, ak_tree_view_on_mouse_move);
    easygui_register_on_mouse_button_down(pTV, ak_tree_view_on_mouse_button_down);
    easygui_register_on_mouse_button_up(pTV, ak_tree_view_on_mouse_button_up);
    easygui_register_on_mouse_button_dblclick(pTV, ak_tree_view_on_mouse_button_dblclick);
    easygui_register_on_mouse_wheel(pTV, ak_tree_view_on_mouse_wheel);


    // Root item.
    pTVData->pRootItem = ak_create_tree_view_item(pTV, NULL, NULL, 0, NULL);
    ak_expand_tree_view_item(pTVData->pRootItem);


    // Event handlers.
    pTVData->onItemPicked = NULL;


    // Extra data.
    pTVData->extraDataSize = extraDataSize;
    if (pExtraData != NULL) {
        memcpy(pTVData->pExtraData, pExtraData, extraDataSize);
    }

    return pTV;
}

void ak_delete_tree_view(easygui_element* pTV)
{
    ak_tree_view* pTVData = easygui_get_extra_data(pTV);
    if (pTVData == NULL) {
        return;
    }

    // Recursively delete the tree view items.
    ak_delete_tree_view_item(pTVData->pRootItem);

    // Delete the element last.
    easygui_delete_element(pTV);
}


size_t ak_get_tree_view_extra_data_size(easygui_element* pTV)
{
    ak_tree_view* pTVData = easygui_get_extra_data(pTV);
    if (pTVData == NULL) {
        return 0;
    }

    return pTVData->extraDataSize;
}

void* ak_get_tree_view_extra_data(easygui_element* pTV)
{
    ak_tree_view* pTVData = easygui_get_extra_data(pTV);
    if (pTVData == NULL) {
        return NULL;
    }

    return pTVData->pExtraData;
}


void ak_deselect_all_tree_view_items(easygui_element* pTV)
{
    ak_tree_view* pTVData = easygui_get_extra_data(pTV);
    if (pTVData == NULL) {
        return;
    }

    ak_deselect_all_tree_view_items_recursive(pTVData->pRootItem);

    easygui_dirty(pTV, easygui_get_local_rect(pTV));
}

void ak_set_on_tree_view_item_picked(easygui_element* pTV, ak_on_tree_view_item_picked_proc proc)
{
    ak_tree_view* pTVData = easygui_get_extra_data(pTV);
    if (pTVData == NULL) {
        return;
    }

    pTVData->onItemPicked = proc;
}

ak_on_tree_view_item_picked_proc ak_get_on_tree_view_item_picked(easygui_element* pTV)
{
    ak_tree_view* pTVData = easygui_get_extra_data(pTV);
    if (pTVData == NULL) {
        return NULL;
    }

    return pTVData->onItemPicked;
}




ak_tree_view_item* ak_create_tree_view_item(easygui_element* pTV, ak_tree_view_item* pParent, const char* text, size_t extraDataSize, const void* pExtraData)
{
    if (pTV == NULL) {
        return NULL;
    }

    if (pParent != NULL && pParent->pTV != pTV) {
        return NULL;
    }


    ak_tree_view_item* pItem = malloc(sizeof(*pItem) + extraDataSize - sizeof(pItem->pExtraData));
    if (pItem == NULL) {
        return NULL;
    }

    pItem->pTV           = pTV;
    pItem->text[0]       = '\0';
    pItem->pParent       = NULL;
    pItem->pFirstChild   = NULL;
    pItem->pLastChild    = NULL;
    pItem->pNextSibling  = NULL;
    pItem->pPrevSibling  = NULL;
    pItem->isSelected    = false;
    pItem->isExpanded    = false;        // Have items collapsed by default?
    pItem->extraDataSize = extraDataSize;

    if (pExtraData != NULL) {
        memcpy(pItem->pExtraData, pExtraData, sizeof(extraDataSize));
    }

    if (text != NULL) {
        strcpy_s(pItem->text, sizeof(pItem->text), text);
    }


    // Append the item to the end of the parent item.
    ak_append_tree_view_item(pItem, pParent);

    return pItem;
}

void ak_delete_tree_view_item(ak_tree_view_item* pItem)
{
    if (pItem == NULL) {
        return;
    }


    // Children need to be deleted first.
    while (pItem->pFirstChild != NULL) {
        ak_delete_tree_view_item(pItem->pFirstChild);
    }

    // We need to grab a pointer to the main tree-view control so we can refresh and redraw it after we have detached the item.
    easygui_element* pTV = pItem->pTV;

    // The item needs to be completely detached first.
    ak_detach_tree_view_item(pItem);

    // Refresh the layout and redraw the tree-view control.
    ak_refresh_and_redraw_tree_view(pTV);

    // Free the item last for safety.
    free(pItem);
}


easygui_element* ak_get_tree_view_from_item(ak_tree_view_item* pItem)
{
    if (pItem == NULL) {
        return NULL;
    }

    return pItem->pTV;
}

ak_tree_view_item* ak_get_tree_view_item_parent(ak_tree_view_item* pItem)
{
    if (pItem == NULL) {
        return NULL;
    }

    return pItem->pParent;
}

const char* ak_get_tree_view_item_text(ak_tree_view_item* pItem)
{
    if (pItem == NULL) {
        return NULL;
    }

    return pItem->text;
}

size_t ak_get_tree_view_item_extra_data_size(ak_tree_view_item* pItem)
{
    if (pItem == NULL) {
        return 0;
    }

    return pItem->extraDataSize;
}

void* ak_get_tree_view_item_extra_data(ak_tree_view_item* pItem)
{
    if (pItem == NULL) {
        return NULL;
    }

    return pItem->pExtraData;
}

void ak_append_tree_view_item(ak_tree_view_item* pItem, ak_tree_view_item* pParent)
{
    if (pItem == NULL) {
        return;
    }

    ak_tree_view* pTVData = easygui_get_extra_data(pItem->pTV);
    if (pTVData == NULL) {
        return;
    }


    // If a parent was not specified, append to the root item.
    if (pParent == NULL)
    {
        assert(pItem->pTV != NULL);
        
        if (pTVData->pRootItem != NULL) {
            ak_append_tree_view_item(pItem, pTVData->pRootItem);
        }
    }
    else
    {
        assert(pItem->pTV == pParent->pTV);

        // Detach the child from it's current parent first.
        ak_detach_tree_view_item(pItem);

        pItem->pParent = pParent;
        assert(pItem->pParent != NULL);

        if (pItem->pParent->pLastChild != NULL) {
            pItem->pPrevSibling = pItem->pParent->pLastChild;
            pItem->pPrevSibling->pNextSibling = pItem;
        }

        if (pItem->pParent->pFirstChild == NULL) {
            pItem->pParent->pFirstChild = pItem;
        }

        pItem->pParent->pLastChild = pItem;


        // Refresh the layout and redraw the tree-view control.
        ak_refresh_and_redraw_tree_view(pItem->pTV);
    }
}

void ak_prepend_tree_view_item(ak_tree_view_item* pItem, ak_tree_view_item* pParent)
{
    if (pItem == NULL) {
        return;
    }

    ak_tree_view* pTVData = easygui_get_extra_data(pItem->pTV);
    if (pTVData == NULL) {
        return;
    }


    // If a parent was not specified, prepend to the root item.
    if (pParent == NULL)
    {
        assert(pItem->pTV != NULL);

        if (pTVData->pRootItem != NULL) {   
            ak_prepend_tree_view_item(pItem, pTVData->pRootItem);
        }
    }
    else
    {
        assert(pItem->pTV == pParent->pTV);

        // Detach the child from it's current parent first.
        ak_detach_tree_view_item(pItem);

        pItem->pParent = pParent;
        assert(pItem->pParent != NULL);

        if (pItem->pParent->pFirstChild != NULL) {
            pItem->pNextSibling = pItem->pParent->pFirstChild;
            pItem->pNextSibling->pPrevSibling = pItem;
        }

        if (pItem->pParent->pLastChild == NULL) {
            pItem->pParent->pLastChild = pItem;
        }

        pItem->pParent->pFirstChild = pItem;


        // Refresh the layout and redraw the tree-view control.
        ak_refresh_and_redraw_tree_view(pItem->pTV);
    }
}

void ak_append_sibling_tree_view_item(ak_tree_view_item* pItemToAppend, ak_tree_view_item* pItemToAppendTo)
{
    if (pItemToAppend == NULL) {
        return;
    }

    ak_tree_view* pTVData = easygui_get_extra_data(pItemToAppend->pTV);
    if (pTVData == NULL) {
        return;
    }


    // If a parent was not specified, append to the root item.
    if (pItemToAppendTo == NULL)
    {
        assert(pItemToAppend->pTV != NULL);

        if (pTVData->pRootItem != NULL) {
            ak_append_tree_view_item(pItemToAppend, pTVData->pRootItem);
        }
    }
    else
    {
        assert(pItemToAppend->pTV == pItemToAppendTo->pTV);

        // Detach the child from it's current parent first.
        ak_detach_tree_view_item(pItemToAppend);


        pItemToAppend->pParent = pItemToAppendTo->pParent;
        assert(pItemToAppend->pParent != NULL);

        pItemToAppend->pNextSibling = pItemToAppendTo->pNextSibling;
        pItemToAppend->pPrevSibling = pItemToAppendTo;

        pItemToAppendTo->pNextSibling->pPrevSibling = pItemToAppend;
        pItemToAppendTo->pNextSibling = pItemToAppend;

        if (pItemToAppend->pParent->pLastChild == pItemToAppendTo) {
            pItemToAppend->pParent->pLastChild = pItemToAppend;
        }


        // Refresh the layout and redraw the tree-view control.
        ak_refresh_and_redraw_tree_view(pItemToAppend->pTV);
    }
}

void ak_prepend_sibling_tree_view_item(ak_tree_view_item* pItemToPrepend, ak_tree_view_item* pItemToPrependTo)
{
    if (pItemToPrepend == NULL) {
        return;
    }

    ak_tree_view* pTVData = easygui_get_extra_data(pItemToPrepend->pTV);
    if (pTVData == NULL) {
        return;
    }


    // If a parent was not specified, prepend to the root item.
    if (pItemToPrependTo == NULL)
    {
        assert(pItemToPrepend->pTV != NULL);

        if (pTVData->pRootItem != NULL) {
            ak_prepend_tree_view_item(pItemToPrepend, pTVData->pRootItem);
        }
    }
    else
    {
        assert(pItemToPrepend->pTV == pItemToPrependTo->pTV);

        // Detach the child from it's current parent first.
        ak_detach_tree_view_item(pItemToPrepend);


        pItemToPrepend->pParent = pItemToPrependTo->pParent;
        assert(pItemToPrepend->pParent != NULL);

        pItemToPrepend->pPrevSibling = pItemToPrependTo->pNextSibling;
        pItemToPrepend->pNextSibling = pItemToPrependTo;

        pItemToPrependTo->pPrevSibling->pNextSibling = pItemToPrepend;
        pItemToPrependTo->pNextSibling = pItemToPrepend;

        if (pItemToPrepend->pParent->pFirstChild == pItemToPrependTo) {
            pItemToPrepend->pParent->pFirstChild = pItemToPrepend;
        }


        // Refresh the layout and redraw the tree-view control.
        ak_refresh_and_redraw_tree_view(pItemToPrepend->pTV);
    }
}

bool ak_does_tree_view_item_have_children(ak_tree_view_item* pItem)
{
    if (pItem == NULL) {
        return false;
    }

    return pItem->pFirstChild != NULL;
}

void ak_select_tree_view_item(ak_tree_view_item* pItem)
{
    if (pItem == NULL) {
        return;
    }

    if (!pItem->isSelected)
    {
        pItem->isSelected = true;
        easygui_dirty(pItem->pTV, easygui_get_local_rect(pItem->pTV));
    }
}

void ak_deselect_tree_view_item(ak_tree_view_item* pItem)
{
    if (pItem == NULL) {
        return;
    }

    if (pItem->isSelected)
    {
        pItem->isSelected = false;
        easygui_dirty(pItem->pTV, easygui_get_local_rect(pItem->pTV));
    }
}

bool ak_is_tree_view_item_selected(ak_tree_view_item* pItem)
{
    if (pItem == NULL) {
        return false;
    }

    return pItem->isSelected;
}

void ak_expand_tree_view_item(ak_tree_view_item* pItem)
{
    if (pItem == NULL) {
        return;
    }

    if (!pItem->isExpanded)
    {
        pItem->isExpanded = true;
        easygui_dirty(pItem->pTV, easygui_get_local_rect(pItem->pTV));
    }
}

void ak_collapse_tree_view_item(ak_tree_view_item* pItem)
{
    if (pItem == NULL) {
        return;
    }

    if (pItem->isExpanded)
    {
        pItem->isExpanded = false;
        easygui_dirty(pItem->pTV, easygui_get_local_rect(pItem->pTV));
    }
}

bool ak_is_tree_view_item_expanded(ak_tree_view_item* pItem)
{
    if (pItem == NULL) {
        return false;
    }

    return pItem->isExpanded;
}


PRIVATE void ak_detach_tree_view_item(ak_tree_view_item* pItem)
{
    assert(pItem != NULL);

    if (pItem->pParent != NULL)
    {
        if (pItem->pParent->pFirstChild == pItem) {
            pItem->pParent->pFirstChild = pItem->pNextSibling;
        }

        if (pItem->pParent->pLastChild == pItem) {
            pItem->pParent->pLastChild = pItem->pPrevSibling;
        }


        if (pItem->pPrevSibling != NULL) {
            pItem->pPrevSibling->pNextSibling = pItem->pNextSibling;
        }

        if (pItem->pNextSibling != NULL) {
            pItem->pNextSibling->pPrevSibling = pItem->pPrevSibling;
        }
    }

    pItem->pParent      = NULL;
    pItem->pPrevSibling = NULL;
    pItem->pNextSibling = NULL;
}


PRIVATE void ak_refresh_and_redraw_tree_view(easygui_element* pTV)
{
    assert(pTV != NULL);

    ak_tree_view* pTVData = easygui_get_extra_data(pTV);
    if (pTVData == NULL) {
        return;
    }





    // For now, just redraw the entire control.
    easygui_dirty(pTV, easygui_get_local_rect(pTV));
}
