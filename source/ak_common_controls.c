
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

/// Helper for finding the height of an item.
PRIVATE float ak_get_tree_view_item_height(easygui_element* pTV)
{
    ak_tree_view* pTVData = easygui_get_extra_data(pTV);
    assert(pTVData != NULL);

    easygui_font_metrics fontMetrics;
    easygui_get_font_metrics(pTVData->pTextFont, &fontMetrics);

    return fontMetrics.lineHeight + (pTVData->textPadding*2);
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
    easygui_color bgcolor = easygui_rgb(128, 128, 128);
    if (pTVData->pHoveredItem == pItem) {
        bgcolor = easygui_rgb(160, 160, 160);
    }

    easygui_draw_rect(pTV, easygui_make_rect(0, penPosY, easygui_get_width(pTV), penPosY + ak_get_tree_view_item_height(pTV)), bgcolor, pPaintData);


    // Text.
    easygui_draw_text(pTV, pTVData->pTextFont, pItem->text, strlen(pItem->text), penPosX + pTVData->textPadding, penPosY + pTVData->textPadding, pTVData->textColor, bgcolor, pPaintData);


    // Now we need to draw children.
    if (pItem != pTVData->pRootItem) {
        penPosX += 16 + pTVData->textPadding;
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

    return true;
}

static void ak_tree_view_on_paint(easygui_element* pTV, easygui_rect relativeClippingRect, void* pPaintData)
{
    (void)relativeClippingRect;

    ak_tree_view* pTVData = easygui_get_extra_data(pTV);
    if (pTVData == NULL) {
        return;
    }

    easygui_draw_rect(pTV, easygui_get_local_rect(pTV), easygui_rgb(128, 128, 128), pPaintData);

    // Each visible item needs to be drawn recursively. We start from top to bottom, starting from the first visible item and ending
    // at the last visible item.

    // TEMP: For now, just draw every item to get the basics working.
    ak_draw_tree_view_item(pTV, pTVData->pRootItem, 0, 0, relativeClippingRect, pPaintData);

    
}

static void ak_tree_view_on_mouse_leave(easygui_element* pTV)
{
    ak_tree_view* pTVData = easygui_get_extra_data(pTV);
    if (pTVData == NULL) {
        return;
    }

    if (pTVData->pHoveredItem != NULL)
    {
        pTVData->pHoveredItem = NULL;
        
        // For now just redraw the entire control, but should optimize this to only redraw the regions of the new and old hovered items.
        easygui_dirty(pTV, easygui_get_local_rect(pTV));
    }
}

static void ak_tree_view_on_mouse_move(easygui_element* pTV, int relativeMousePosX, int relativeMousePosY)
{
    (void)pTV;
    (void)relativeMousePosX;
    (void)relativeMousePosY;

    ak_tree_view* pTVData = easygui_get_extra_data(pTV);
    if (pTVData == NULL) {
        return;
    }

    ak_tree_view_item_metrics metrics;
    ak_tree_view_item* pHoveredItem = ak_find_tree_view_item_under_point(pTV, relativeMousePosX, relativeMousePosY, &metrics);

    if (pHoveredItem != pTVData->pHoveredItem)
    {
        // The tree-view needs to now about the new hovered item so it can know to draw it with the hovered style.
        pTVData->pHoveredItem = pHoveredItem;

        // For now just redraw the entire control, but should optimize this to only redraw the regions of the new and old hovered items.
        easygui_dirty(pTV, easygui_get_local_rect(pTV));
    }
}

static void ak_tree_view_on_mouse_button_down(easygui_element* pTV, int mouseButton, int relativeMousePosX, int relativeMousePosY)
{
    (void)pTV;
    (void)mouseButton;
    (void)relativeMousePosX;
    (void)relativeMousePosY;
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
    (void)pTV;
    (void)mouseButton;
    (void)relativeMousePosX;
    (void)relativeMousePosY;
}

static void ak_tree_view_on_mouse_wheel(easygui_element* pTV, int delta, int relativeMousePosX, int relativeMousePosY)
{
    (void)pTV;
    (void)delta;
    (void)relativeMousePosX;
    (void)relativeMousePosY;
}


easygui_element* ak_create_tree_view(easygui_context* pContext, easygui_element* pParent, easygui_font* pFont, easygui_color textColor)
{
    easygui_element* pTV = easygui_create_element(pContext, pParent, sizeof(ak_tree_view));
    if (pTV == NULL) {
        return NULL;
    }

    ak_tree_view* pTVData = easygui_get_extra_data(pTV);
    if (pTVData == NULL) {
        return NULL;
    }

    pTVData->pHoveredItem = NULL;
    pTVData->offsetX = 0;
    pTVData->offsetY = 0;


    // Theme.
    pTVData->pTextFont   = pFont;
    pTVData->textColor   = textColor;
    pTVData->textPadding = 2;


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

    return pTV;
}

void ak_delete_tree_view(easygui_element* pTV)
{
    if (pTV == NULL) {
        return;
    }

    ak_tree_view* pTVData = easygui_get_extra_data(pTV);
    if (pTVData == NULL) {
        return;
    }

    // Recursively delete the tree view items.
    ak_delete_tree_view_item(pTVData->pRootItem);

    // Delete the element last.
    easygui_delete_element(pTV);
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
