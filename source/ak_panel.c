// Public domain. See "unlicense" statement at the end of this file.

#include "../include/easy_appkit/ak_panel.h"
#include "../include/easy_appkit/ak_application.h"
#include "../include/easy_appkit/ak_theme.h"
#include "../include/easy_appkit/ak_tool.h"
#include "../include/easy_appkit/ak_build_config.h"
#include <easy_gui/easy_gui.h>
#include <easy_util/easy_util.h>
#include <assert.h>

typedef struct 
{
    /// A pointer to the main application.
    ak_application* pApplication;


    /// The name of the panel.
    char name[AK_MAX_PANEL_NAME_LENGTH];


    /// The axis the two child panels are split along, if any. When this is not ak_panel_split_axis_none, it is assumed
    /// the panel has two child elements, both of which should be panels.
    ak_panel_split_axis splitAxis;

    /// The position of the split.
    float splitPos;


    /// The container for tools.
    easygui_element* pToolContainer;


    /// Flags for tracking basic settings for the panel.
    unsigned int optionFlags;

    /// The orientation of the tabs.
    ak_panel_tab_orientation tabOrientation;

    /// The size of the tab bar. For horizontal tab bars, this is the height. For vertical tab bars it's the width.
    float tabBarSize;


    /// Keeps track of whether or not the mouse is over the panel.
    bool isMouseOver;

    /// Keeps track of the relative position of the mouse on the X axis.
    float relativeMousePosX;

    /// Keeps track of the relative position of the mouse on the X axis.
    float relativeMousePosY;

    /// A pointer to the tool whose tab is being hovered over.
    easygui_element* pHoveredTool;

    /// A pointer to the tool whose tab is active.
    easygui_element* pActiveTool;


    /// The size of the panel's extra data, in bytes.
    size_t extraDataSize;

    /// A pointer to the panel's extra data.
    char pExtraData[1];

} ak_panel_data;

typedef struct
{
    /// A pointer to the relevant tool.
    easygui_element* pTool;

    /// The tab's rectangle.
    easygui_rect rect;

    /// The rectangle of the close button.
    easygui_rect closeButtonRect;

    /// The text rectangle.
    easygui_rect textRect;

    /// Whether or not the tab is marked as hovered at the time of iteration.
    bool isHovered;

    /// Whether or not the tab is active at the time of iteration.
    bool isActive;

} ak_panel_tab_info;

typedef struct
{
    /// A pointer to the paint data to pass to the easygui paint functions.
    void* pPaintData;

    /// The rectangle region of the drawn tabs. This is used to determine that non-tabbed area that needs redrawing.
    easygui_rect tabsRect;

} ak_panel_paint_context;

typedef struct
{
    /// The relative position of the point on the x axis.
    float relativePosX;

    /// The relative position of the point on the y axis.
    float relativePosY;

    /// A pointer to the tool that the point is over.
    easygui_element* pToolTab;

    /// When the point is hovered over a tab's cross button, this points to the associated tool. This will be null if
    /// the point is not hovered over a cross.
    easygui_element* pToolTabCross;

} ak_panel_hit_test_result;


/// The tab iteration callback function.
typedef bool (* ak_panel_tab_iterator_proc)(easygui_element* pPanel, ak_panel_tab_info* pInfo, void* pUserData);


////////////////////////////////////////////////
// Private API

/// Refreshes the alignment of the child panels of the given panel.
PRIVATE void ak_panel_refresh_child_alignments(easygui_element* pPanel)
{
    ak_panel_data* pPanelData = easygui_get_extra_data(pPanel);
    assert(pPanelData != NULL);
    assert(pPanelData->splitAxis != ak_panel_split_axis_none);
    
    assert(pPanel->pFirstChild != NULL);
    assert(pPanel->pFirstChild->pNextSibling != NULL);

    float innerScaleX;
    float innerScaleY;
    easygui_get_inner_scale(pPanel, &innerScaleX, &innerScaleY);

    easygui_element* pChildPanel1 = pPanel->pFirstChild;
    easygui_element* pChildPanel2 = pPanel->pFirstChild->pNextSibling;

    float borderWidth = 0;

    if (pPanelData->splitAxis == ak_panel_split_axis_horizontal)
    {
        // Horizontal.
        easygui_set_relative_position(pChildPanel1, borderWidth, borderWidth);
        easygui_set_size(pChildPanel1, (pPanelData->splitPos - borderWidth) / innerScaleX, (pPanel->height - (borderWidth*2)) / innerScaleY);

        easygui_set_relative_position(pChildPanel2, pPanelData->splitPos, borderWidth);
        easygui_set_size(pChildPanel2, (pPanel->width - pPanelData->splitPos - borderWidth) / innerScaleX, (pPanel->height - (borderWidth*2)) / innerScaleY);
    }
    else
    {
        // Vertical.
        easygui_set_relative_position(pChildPanel1, borderWidth, borderWidth);
        easygui_set_size(pChildPanel1, (pPanel->width / innerScaleX) - (borderWidth*2), pPanelData->splitPos - borderWidth);

        easygui_set_relative_position(pChildPanel2, borderWidth, pPanelData->splitPos);
        easygui_set_size(pChildPanel2, (pPanel->width / innerScaleX) - (borderWidth*2), (pPanel->height / innerScaleY) - pPanelData->splitPos - borderWidth);
    }
}

PRIVATE easygui_rect ak_panel_get_tab_bar_rect(easygui_element* pPanel)
{
    ak_panel_data* pPanelData = easygui_get_extra_data(pPanel);
    assert(pPanelData != NULL);

    easygui_rect rect;
    rect.left   = 0;
    rect.top    = 0;
    rect.right  = 0;
    rect.bottom = 0;
    
    if (pPanelData->tabBarSize > 0)
    {
        switch (pPanelData->tabOrientation)
        {
            case ak_panel_tab_orientation_top:
            {
                rect.bottom = pPanelData->tabBarSize;
                rect.right  = easygui_get_width(pPanel);
                break;
            }

            case ak_panel_tab_orientation_bottom:
            {
                rect.bottom = easygui_get_height(pPanel);
                rect.right  = easygui_get_width(pPanel);
                rect.top    = rect.bottom - pPanelData->tabBarSize;
                break;
            }

            case ak_panel_tab_orientation_left:
            {
                rect.right  = pPanelData->tabBarSize;
                rect.bottom = easygui_get_height(pPanel);
                break;
            }

            case ak_panel_tab_orientation_right:
            {
                rect.right  = easygui_get_width(pPanel);
                rect.bottom = easygui_get_height(pPanel);
                rect.left   = rect.right - pPanelData->tabBarSize;
                break;
            }

            default: break;
        }
    }

    return rect;
}

PRIVATE void ak_panel_refresh_tool_container_layout(easygui_element* pPanel, OUT bool* pDidLayoutChangeOut)
{
    ak_panel_data* pPanelData = easygui_get_extra_data(pPanel);
    assert(pPanelData != NULL);

    bool didLayoutChange = false;

    float width  = easygui_get_width(pPanel);
    float height = easygui_get_height(pPanel);

    float posX = 0;
    float posY = 0;

    switch (pPanelData->tabOrientation)
    {
        case ak_panel_tab_orientation_top:
        {
            posY = pPanelData->tabBarSize;
            height -= pPanelData->tabBarSize;
            break;
        }

        case ak_panel_tab_orientation_bottom:
        {
            height -= pPanelData->tabBarSize;
            break;
        }

        case ak_panel_tab_orientation_left:
        {
            posX = pPanelData->tabBarSize;
            width -= pPanelData->tabBarSize;
            break;
        }

        case ak_panel_tab_orientation_right:
        {
            width -= pPanelData->tabBarSize;
            break;
        }

        default: break;
    }

    float innerScaleX;
    float innerScaleY;
    easygui_get_inner_scale(pPanel, &innerScaleX, &innerScaleY);


    if (easygui_get_relative_position_x(pPanelData->pToolContainer) != posX || easygui_get_relative_position_y(pPanelData->pToolContainer) != posY) {
        didLayoutChange = true;
        easygui_set_relative_position(pPanelData->pToolContainer, posX / innerScaleX, posY / innerScaleY);
    }
    
    if (easygui_get_width(pPanelData->pToolContainer) != width || easygui_get_height(pPanelData->pToolContainer) != height) {
        didLayoutChange = true;
        easygui_set_size(pPanelData->pToolContainer, width / innerScaleX, height / innerScaleY);
    }
    
    if (pDidLayoutChangeOut != NULL) {
        *pDidLayoutChangeOut = didLayoutChange;
    }
}


PRIVATE void ak_panel_iterate_tool_tabs(easygui_element* pPanel, ak_panel_tab_iterator_proc callback, void* pUserData)
{
    ak_panel_data* pPanelData = easygui_get_extra_data(pPanel);
    assert(pPanelData != NULL);

    ak_application* pApplication = ak_get_panel_application(pPanel);
    assert(pApplication != NULL);

    ak_theme* pTheme = ak_get_application_theme(pApplication);
    assert(pTheme != NULL);

    float innerScaleX;
    float innerScaleY;
    easygui_get_inner_scale(pPanel, &innerScaleX, &innerScaleY);


    easygui_font* pFont = pTheme->pUIFont;
    
    easygui_font_metrics fontMetrics;
    easygui_get_font_metrics(pFont, 1, 1, &fontMetrics);

    float paddingLeft   = pTheme->tabPaddingLeft;
    float paddingTop    = pTheme->tabPaddingTop;
    float paddingRight  = pTheme->tabPaddingRight;
    float paddingBottom = pTheme->tabPaddingBottom;

    float penPosX = 0;
    float penPosY = 0;
    for (easygui_element* pTool = pPanelData->pToolContainer->pFirstChild; pTool != NULL; pTool = pTool->pNextSibling)
    {
        const char*  text       = ak_get_tool_title(pTool);
        unsigned int textLength = (unsigned int)strlen(text);

        float titleWidth  = 0;
        float titleHeight = (float)fontMetrics.lineHeight;
        easygui_measure_string(pFont, text, textLength, innerScaleX, innerScaleY, &titleWidth, NULL);   // NULL as last argument (pHeightOut) is intentional - want to use the line height instead.

        
        ak_panel_tab_info info;
        info.pTool                  = pTool;
        info.rect.left              = penPosX;
        info.rect.top               = penPosY;
        info.rect.right             = info.rect.left + titleWidth  + paddingLeft + paddingRight;
        info.rect.bottom            = info.rect.top  + titleHeight + paddingTop + paddingBottom;
        info.closeButtonRect.left   = 0;
        info.closeButtonRect.top    = 0;
        info.closeButtonRect.right  = 0;
        info.closeButtonRect.bottom = 0;
        info.textRect.left          = info.rect.left + paddingLeft;
        info.textRect.top           = info.rect.top  + paddingTop;
        info.textRect.right         = info.textRect.left + titleWidth;
        info.textRect.bottom        = info.textRect.top  + titleHeight;
        info.isActive               = pPanelData->pActiveTool  == pTool;
        info.isHovered              = pPanelData->pHoveredTool == pTool;

        if (!callback(pPanel, &info, pUserData)) {
            return;
        }

        // Move the pen forward in preparation for the next tab.
        penPosX = info.rect.right;
    }
}


PRIVATE bool ak_panel_is_point_over_tab_bar(easygui_element* pPanel, float relativePosX, float relativePosY)
{
    ak_panel_data* pPanelData = easygui_get_extra_data(pPanel);
    assert(pPanelData != NULL);

    return easygui_rect_contains_point(ak_panel_get_tab_bar_rect(pPanel), relativePosX, relativePosY);
}


PRIVATE bool ak_panel_tab_bar_hit_test_callback(easygui_element* pPanel, ak_panel_tab_info* pTabInfo, void* pUserData)
{
    ak_panel_data* pPanelData = easygui_get_extra_data(pPanel);
    assert(pPanelData != NULL);

    ak_panel_hit_test_result* pResultOut = pUserData;
    assert(pResultOut != NULL);

    float relativePosX = pResultOut->relativePosX;
    float relativePosY = pResultOut->relativePosY;

    if (ak_panel_is_point_over_tab_bar(pPanel, relativePosX, relativePosY) && easygui_rect_contains_point(pTabInfo->rect, relativePosX, relativePosY))
    {
        pResultOut->pToolTab = pTabInfo->pTool;

        pResultOut->pToolTabCross = NULL;
        if ((pPanelData->optionFlags & AK_PANEL_OPTION_SHOW_CLOSE_BUTTON_ON_TABS) != 0)
        {
            
        }
    }

    return true;
}

PRIVATE bool ak_panel_do_tab_bar_hit_test(easygui_element* pPanel, float relativePosX, float relativePosY, ak_panel_hit_test_result* pResultOut)
{
    ak_panel_data* pPanelData = easygui_get_extra_data(pPanel);
    assert(pPanelData != NULL);

    if (pResultOut != NULL) {
        pResultOut->pToolTab      = NULL;
        pResultOut->pToolTabCross = NULL;
        pResultOut->relativePosX  = relativePosX;
        pResultOut->relativePosY  = relativePosY;
    }

    if (!pPanelData->isMouseOver) {
        return false;
    }


    if ((pPanelData->optionFlags & AK_PANEL_OPTION_SHOW_TOOL_TABS) != 0 && pPanelData->pToolContainer != NULL)
    {
        if (ak_panel_is_point_over_tab_bar(pPanel, relativePosX, relativePosY))
        {
            if (pResultOut != NULL)
            {
                ak_panel_iterate_tool_tabs(pPanel, ak_panel_tab_bar_hit_test_callback, pResultOut);
            }
            
            return true;
        }
    }

    return false;
}


PRIVATE void ak_panel_mark_tab_bar_as_dirty(easygui_element* pPanel)
{
    easygui_dirty(pPanel, ak_panel_get_tab_bar_rect(pPanel));
}

PRIVATE bool ak_panel_tab_paint_callback(easygui_element* pPanel, ak_panel_tab_info* pTabInfo, void* pUserData)
{
    ak_panel_data* pPanelData = easygui_get_extra_data(pPanel);
    assert(pPanelData != NULL);

    ak_panel_paint_context* pPaintContext = pUserData;
    assert(pPaintContext != NULL);

    ak_theme* pTheme = ak_get_application_theme(ak_get_panel_application(pPanel));
    assert(pTheme != NULL);


    void* pPaintData = pPaintContext->pPaintData;
    easygui_font* pFont     = pTheme->pUIFont;
    easygui_color textColor = pTheme->uiFontColor;

    easygui_color backgroundColor;
    if (pTabInfo->isActive) {
        backgroundColor = pTheme->tabActiveColor;
    } else if (pTabInfo->isHovered) {
        backgroundColor = pTheme->tabHoveredColor;
    } else {
        backgroundColor = pTheme->tabColor;
    }

    // Text.
    const char*  toolTitle       = ak_get_tool_title(pTabInfo->pTool);
    unsigned int toolTitleLength = (unsigned int)strlen(toolTitle);
    easygui_draw_text(pPanel, pFont, toolTitle, toolTitleLength, pTabInfo->textRect.left, pTabInfo->textRect.top, textColor, backgroundColor, pPaintData);

    // Background. Part of the background was drawn with the text which we do in order to avoid overdraw. Thus, the background is
    // drawn in four parts - one for each side around the text.
    easygui_draw_rect(pPanel, easygui_make_rect(pTabInfo->rect.left,      pTabInfo->rect.top,        pTabInfo->textRect.left,  pTabInfo->rect.bottom),  backgroundColor, pPaintData); // Left
    easygui_draw_rect(pPanel, easygui_make_rect(pTabInfo->textRect.right, pTabInfo->rect.top,        pTabInfo->rect.right,     pTabInfo->rect.bottom),  backgroundColor, pPaintData); // Right
    easygui_draw_rect(pPanel, easygui_make_rect(pTabInfo->textRect.left,  pTabInfo->rect.top,        pTabInfo->textRect.right, pTabInfo->textRect.top), backgroundColor, pPaintData); // Top
    easygui_draw_rect(pPanel, easygui_make_rect(pTabInfo->textRect.left,  pTabInfo->textRect.bottom, pTabInfo->textRect.right, pTabInfo->rect.bottom),  backgroundColor, pPaintData); // Bottom

    pPaintContext->tabsRect.right  = pTabInfo->rect.right;
    pPaintContext->tabsRect.bottom = pTabInfo->rect.bottom;

    return true;
}

PRIVATE void ak_panel_paint_tab_bar(easygui_element* pPanel, void* pPaintData)
{
    ak_panel_data* pPanelData = easygui_get_extra_data(pPanel);
    assert(pPanelData != NULL);

    ak_theme* pTheme = ak_get_application_theme(ak_get_panel_application(pPanel));
    assert(pTheme != NULL);


    // We need to draw a tab for each tool, if applicable.
    if ((pPanelData->optionFlags & AK_PANEL_OPTION_SHOW_TOOL_TABS) != 0)
    {
        ak_panel_paint_context context;
        context.pPaintData = pPaintData;
        context.tabsRect   = easygui_make_rect(0, 0, 0, 0);

        ak_panel_iterate_tool_tabs(pPanel, ak_panel_tab_paint_callback, &context);

        // After drawing the tabs we need to draw the background region that is not covered by tabs. We can determine this region from
        // the data that was set by panel_iterate_tool_tabs().
        easygui_rect undrawnRect = ak_panel_get_tab_bar_rect(pPanel);

        if (pPanelData->tabOrientation == ak_panel_tab_orientation_top || pPanelData->tabOrientation == ak_panel_tab_orientation_bottom) {
            undrawnRect.left = context.tabsRect.right;      // Top/Bottom alignment.
        } else {
            undrawnRect.top  = context.tabsRect.bottom;     // Left/Right alignment.
        }
        

        easygui_draw_rect(pPanel, undrawnRect, pTheme->tabColor, pPaintData);
    }
}

PRIVATE void ak_panel_refresh_tabs(easygui_element* pPanel)
{
    assert(pPanel != NULL);

    ak_panel_data* pPanelData = easygui_get_extra_data(pPanel);
    assert(pPanelData != NULL);

    ak_theme* pTheme = ak_get_application_theme(ak_get_panel_application(pPanel));
    assert(pTheme != NULL);


    // The layout of the tool container needs to be updated first.
    bool didTabBarChange = false;
    ak_panel_refresh_tool_container_layout(pPanel, &didTabBarChange);

    easygui_font_metrics fontMetrics;
    easygui_get_font_metrics(pTheme->pUIFont, 1, 1, &fontMetrics);


    // If we are showing the tab bar we will need to redo the hit-test and redraw it.
    if ((pPanelData->optionFlags & AK_PANEL_OPTION_SHOW_TOOL_TABS) != 0)
    {
        pPanelData->tabBarSize = pTheme->tabPaddingTop + pTheme->tabPaddingBottom + fontMetrics.lineHeight;

        // The tab-bar hit test needs to be refreshed so things like mouse-over styling can be updated.
        easygui_element* pPrevHoveredElement = pPanelData->pHoveredTool;

        ak_panel_hit_test_result hitTest;
        ak_panel_do_tab_bar_hit_test(pPanel, pPanelData->relativeMousePosX, pPanelData->relativeMousePosY, &hitTest);

        pPanelData->pHoveredTool = hitTest.pToolTab;
        

        if (pPrevHoveredElement != pPanelData->pHoveredTool) {
            didTabBarChange = true;
        }

        // Redraw the tab bar, if required.
        if (didTabBarChange) {
            ak_panel_mark_tab_bar_as_dirty(pPanel);
        }
    }
}


PRIVATE void ak_panel_on_paint(easygui_element* pPanel, easygui_rect relativeRect, void* pPaintData)
{
    ak_panel_data* pPanelData = easygui_get_extra_data(pPanel);
    assert(pPanelData != NULL);

    ak_theme* pTheme = ak_get_application_theme(ak_get_panel_application(pPanel));
    if (pTheme == NULL) {
        return;
    }
    

    // Only draw the background if we have no children.
    if (pPanel->pFirstChild == NULL || (pPanelData->pToolContainer != NULL && pPanelData->pToolContainer->pFirstChild == NULL))
    {
        easygui_draw_rect(pPanel, relativeRect, pTheme->baseColor, pPaintData);
    }

    ak_panel_paint_tab_bar(pPanel, pPaintData);
}

PRIVATE void ak_panel_on_size(easygui_element* pElement, float newWidth, float newHeight)
{
    (void)newWidth;
    (void)newHeight;

    ak_panel_data* pPanelData = easygui_get_extra_data(pElement);
    assert(pPanelData != NULL);

    if (pPanelData->splitAxis == ak_panel_split_axis_none)
    {
        // It's not a split panel. We need to resize the tool container, and then each tool.
        if (pPanelData->pToolContainer != NULL) {
            ak_panel_refresh_tool_container_layout(pElement, NULL);
        }
    }
    else
    {
        // It's a split panel.
        ak_panel_refresh_child_alignments(pElement);
    }
}

PRIVATE void ak_panel_on_mouse_enter(easygui_element* pElement)
{
    ak_panel_data* pPanelData = easygui_get_extra_data(pElement);
    assert(pPanelData != NULL);

    pPanelData->isMouseOver = true;
}

PRIVATE void ak_panel_on_mouse_leave(easygui_element* pElement)
{
    ak_panel_data* pPanelData = easygui_get_extra_data(pElement);
    assert(pPanelData != NULL);

    pPanelData->isMouseOver = false;

    ak_panel_refresh_tabs(pElement);
}

PRIVATE void ak_panel_on_mouse_move(easygui_element* pElement, int relativeMousePosX, int relativeMousePosY)
{
    ak_panel_data* pPanelData = easygui_get_extra_data(pElement);
    assert(pPanelData != NULL);

    pPanelData->isMouseOver = true;
    pPanelData->relativeMousePosX = (float)relativeMousePosX;
    pPanelData->relativeMousePosY = (float)relativeMousePosY;

    if (ak_panel_is_point_over_tab_bar(pElement, (float)relativeMousePosX, (float)relativeMousePosY) || pPanelData->pHoveredTool != NULL) {
        ak_panel_refresh_tabs(pElement);
    }
}

PRIVATE void ak_panel_on_mouse_button_down(easygui_element* pElement, int button, int relativeMousePosX, int relativeMousePosY)
{
    ak_panel_data* pPanelData = easygui_get_extra_data(pElement);
    assert(pPanelData != NULL);

    pPanelData->isMouseOver = true;
    pPanelData->relativeMousePosX = (float)relativeMousePosX;
    pPanelData->relativeMousePosY = (float)relativeMousePosY;

    if (ak_panel_is_point_over_tab_bar(pElement, (float)relativeMousePosX, (float)relativeMousePosY))
    {
        switch (button)
        {
            case EASYGUI_MOUSE_BUTTON_LEFT:
            {
                if (pPanelData->pHoveredTool != NULL) {
                    ak_panel_activate_tool(pElement, pPanelData->pHoveredTool);
                }
                
                break;
            }

            default: break;
        }
    }
}




easygui_element* ak_create_panel(ak_application* pApplication, easygui_element* pParent, size_t extraDataSize, const void* pExtraData)
{
    easygui_element* pElement = easygui_create_element(ak_get_application_gui(pApplication), pParent, sizeof(ak_panel_data) - sizeof(char) + extraDataSize);
    if (pElement != NULL)
    {
        ak_panel_data* pPanelData = easygui_get_extra_data(pElement);
        assert(pPanelData != NULL);

        pPanelData->pApplication       = pApplication;
        pPanelData->name[0]            = '\0';
        pPanelData->splitAxis          = ak_panel_split_axis_none;
        pPanelData->splitPos           = 0;
        pPanelData->pToolContainer     = NULL;
        pPanelData->optionFlags        = 0;
        pPanelData->tabOrientation     = ak_panel_tab_orientation_top;
        pPanelData->tabBarSize         = 0;
        pPanelData->isMouseOver        = false;
        pPanelData->relativeMousePosX  = 0;
        pPanelData->relativeMousePosY  = 0;
        pPanelData->pActiveTool        = NULL;
        pPanelData->pHoveredTool       = NULL;
        pPanelData->extraDataSize      = extraDataSize;
        if (pExtraData != NULL) {
            memcpy(pPanelData->pExtraData, pExtraData, extraDataSize);
        }

        easygui_set_on_paint(pElement, ak_panel_on_paint);
        easygui_set_on_size(pElement, ak_panel_on_size);
        easygui_set_on_mouse_enter(pElement, ak_panel_on_mouse_enter);
        easygui_set_on_mouse_leave(pElement, ak_panel_on_mouse_leave);
        easygui_set_on_mouse_move(pElement, ak_panel_on_mouse_move);
        easygui_set_on_mouse_button_down(pElement, ak_panel_on_mouse_button_down);
    }

    return pElement;
}

ak_application* ak_get_panel_application(easygui_element* pPanel)
{
    ak_panel_data* pPanelData = easygui_get_extra_data(pPanel);
    if (pPanelData == NULL) {
        return NULL;
    }

    return pPanelData->pApplication;
}

size_t ak_panel_get_extra_data_size(easygui_element* pPanel)
{
    ak_panel_data* pPanelData = easygui_get_extra_data(pPanel);
    if (pPanelData == NULL) {
        return 0;
    }

    return pPanelData->extraDataSize;
}

void* ak_panel_get_extra_data(easygui_element* pPanel)
{
    ak_panel_data* pPanelData = easygui_get_extra_data(pPanel);
    if (pPanelData == NULL) {
        return NULL;
    }

    return pPanelData->pExtraData;
}


void ak_panel_set_name(easygui_element* pPanel, const char* name)
{
    ak_panel_data* pPanelData = easygui_get_extra_data(pPanel);
    if (pPanelData == NULL) {
        return;
    }

    if (name == NULL) {
        pPanelData->name[0] = '\0';
    } else {
        strcpy_s(pPanelData->name, sizeof(pPanelData->name), name);
    }
}

const char* ak_panel_get_name(easygui_element* pPanel)
{
    ak_panel_data* pPanelData = easygui_get_extra_data(pPanel);
    if (pPanelData == NULL) {
        return NULL;
    }

    return pPanelData->name;
}

easygui_element* ak_panel_find_by_name_recursive(easygui_element* pPanel, const char* name)
{
    ak_panel_data* pPanelData = easygui_get_extra_data(pPanel);
    if (pPanelData == NULL) {
        return NULL;
    }

    if (name == NULL) {
        return NULL;
    }


    if (strcmp(pPanelData->name, name) == 0) {
        return pPanel;
    }

    // If it's a split panel we need to check those. If it's not split, we just return NULL;
    if (pPanelData->splitAxis != ak_panel_split_axis_none)
    {
        easygui_element* pResult = NULL;
        
        pResult = ak_panel_find_by_name_recursive(ak_panel_get_split_panel_1(pPanel), name);
        if (pResult != NULL) {
            return pResult;
        }

        pResult = ak_panel_find_by_name_recursive(ak_panel_get_split_panel_2(pPanel), name);
        if (pResult != NULL) {
            return pResult;
        }
    }

    return NULL;
}


bool ak_panel_split(easygui_element* pPanel, ak_panel_split_axis splitAxis, float splitPos)
{
    ak_panel_data* pPanelData = easygui_get_extra_data(pPanel);
    if (pPanelData == NULL) {
        return false;
    }

    // It's an error for a panel to be split while it has tools attached.
    if (pPanelData->pToolContainer != NULL) {
        return false;
    }


    // If children do not already exist we just create them. Otherwise we just reuse the existing ones and re-align them.
    easygui_element* pChildPanel1 = NULL;
    easygui_element* pChildPanel2 = NULL;

    if (pPanelData->splitAxis == ak_panel_split_axis_none)
    {
        pChildPanel1 = ak_create_panel(pPanelData->pApplication, pPanel, 0, NULL);
        pChildPanel2 = ak_create_panel(pPanelData->pApplication, pPanel, 0, NULL);
    }
    else
    {
        // Assume the existing children are panels.
        assert(pPanel->pFirstChild != NULL);
        assert(pPanel->pFirstChild->pNextSibling != NULL);

        pChildPanel1 = pPanel->pFirstChild;
        pChildPanel2 = pPanel->pFirstChild->pNextSibling;
    }

    pPanelData->splitAxis = splitAxis;
    pPanelData->splitPos  = splitPos;
    
    assert(pChildPanel1 != NULL);
    assert(pChildPanel2 != NULL);

    ak_panel_refresh_child_alignments(pPanel);

    return true;
}

void ak_panel_unsplit(easygui_element* pPanel)
{
    ak_panel_data* pPanelData = easygui_get_extra_data(pPanel);
    if (pPanelData == NULL) {
        return;
    }

    if (pPanelData->splitAxis == ak_panel_split_axis_none) {
        return;
    }


    easygui_delete_element(pPanel->pFirstChild->pNextSibling);
    easygui_delete_element(pPanel->pFirstChild);

    pPanelData->splitAxis = ak_panel_split_axis_none;
    pPanelData->splitPos  = 0;
}

ak_panel_split_axis ak_panel_get_split_axis(easygui_element* pPanel)
{
    ak_panel_data* pPanelData = easygui_get_extra_data(pPanel);
    if (pPanelData == NULL) {
        return ak_panel_split_axis_none;
    }

    return pPanelData->splitAxis;
}

easygui_element* ak_panel_get_split_panel_1(easygui_element* pPanel)
{
    ak_panel_data* pPanelData = easygui_get_extra_data(pPanel);
    if (pPanelData == NULL) {
        return NULL;
    }

    if (pPanelData->splitAxis == ak_panel_split_axis_none) {
        return NULL;
    }

    return pPanel->pFirstChild;
}


easygui_element* ak_panel_get_split_panel_2(easygui_element* pPanel)
{
    ak_panel_data* pPanelData = easygui_get_extra_data(pPanel);
    if (pPanelData == NULL) {
        return NULL;
    }

    if (pPanelData->splitAxis == ak_panel_split_axis_none) {
        return NULL;
    }

    return pPanel->pFirstChild->pNextSibling;
}

bool ak_panel_attach_tool(easygui_element* pPanel, easygui_element* pTool)
{
    ak_panel_data* pPanelData = easygui_get_extra_data(pPanel);
    if (pPanelData == NULL) {
        return false;
    }

    if (pPanelData->splitAxis != ak_panel_split_axis_none) {
        return false;
    }

    if (pTool == NULL) {
        return false;
    }


    // We need a tool container first.
    if (pPanelData->pToolContainer == NULL)
    {
        assert(pPanel->pFirstChild == NULL);        // Assume the panel does not have any children.

        pPanelData->pToolContainer = easygui_create_element(pPanel->pContext, pPanel, 0);
        if (pPanelData->pToolContainer == NULL) {
            return false;
        }
        easygui_set_size(pPanelData->pToolContainer, easygui_get_width(pPanel), easygui_get_height(pPanel));

        easygui_set_on_size(pPanelData->pToolContainer, easygui_on_size_fit_children_to_parent);
    }

    assert(pPanelData->pToolContainer != NULL);

    easygui_prepend(pTool, pPanelData->pToolContainer);

    // Initial size and position.
    easygui_set_relative_position(pTool, 0, 0);
    easygui_set_size(pTool, easygui_get_width(pPanelData->pToolContainer), easygui_get_height(pPanelData->pToolContainer));

    // Activate the new tool.
    ak_panel_activate_tool(pPanel, pTool);
    
    // The tab bar might need to be refreshed.
    ak_panel_refresh_tabs(pPanel);

    return true;
}

void ak_panel_detach_tool(easygui_element* pPanel, easygui_element* pTool)
{
    ak_panel_data* pPanelData = easygui_get_extra_data(pPanel);
    if (pPanelData == NULL) {
        return;
    }

    if (pPanelData->splitAxis != ak_panel_split_axis_none) {
        return;
    }
    if (pPanelData->pToolContainer == NULL) {
        return;
    }

    if (pTool == NULL) {
        return;
    }
    if (pTool->pParent != pPanelData->pToolContainer) {
        return;
    }


    easygui_detach(pTool);


    // The tab bar might need to be refreshed.
    ak_panel_refresh_tabs(pPanel);
}

PRIVATE void ak_panel_deactivate_tool_no_redraw(easygui_element* pPanel)
{
    ak_panel_data* pPanelData = easygui_get_extra_data(pPanel);
    assert(pPanelData != NULL);

    easygui_hide(pPanelData->pActiveTool);
    pPanelData->pActiveTool = NULL;
}

bool ak_panel_activate_tool(easygui_element* pPanel, easygui_element* pTool)
{
    ak_panel_data* pPanelData = easygui_get_extra_data(pPanel);
    if (pPanelData == NULL) {
        return false;
    }

    if (!(pTool == NULL || pTool->pParent == pPanelData->pToolContainer)) {
        return false;
    }


    if (pPanelData->pActiveTool != pTool)
    {
        // The previous tool needs to be deactivated.
        ak_panel_deactivate_tool_no_redraw(pPanel);

        
        // The new active tool needs to be shown.
        pPanelData->pActiveTool = pTool;
        easygui_show(pTool);


        // The tab bar needs to be redrawn in order for it to show the newly active tab.
        ak_panel_mark_tab_bar_as_dirty(pPanel);
    }

    return true;
}

void ak_panel_deactivate_tool(easygui_element* pPanel)
{
    ak_panel_data* pPanelData = easygui_get_extra_data(pPanel);
    if (pPanelData == NULL) {
        return;
    }

    if (pPanelData->pActiveTool != NULL)
    {
        ak_panel_deactivate_tool_no_redraw(pPanel);
        ak_panel_mark_tab_bar_as_dirty(pPanel);
    }
}


easygui_element* ak_panel_get_first_tool(easygui_element* pPanel)
{
    ak_panel_data* pPanelData = easygui_get_extra_data(pPanel);
    if (pPanelData == NULL) {
        return NULL;
    }

    if (pPanelData->pToolContainer == NULL) {
        return NULL;
    }

    return pPanelData->pToolContainer->pFirstChild;
}

easygui_element* ak_panel_get_next_tool(easygui_element* pPanel, easygui_element* pTool)
{
    ak_panel_data* pPanelData = easygui_get_extra_data(pPanel);
    if (pPanelData == NULL) {
        return NULL;
    }

    if (pTool == NULL) {
        return NULL;
    }

    if (pTool->pParent != pPanelData->pToolContainer) {
        return NULL;
    }

    return pTool->pNextSibling;
}


void ak_panel_set_tab_options(easygui_element* pPanel, unsigned int options)
{
    ak_panel_data* pPanelData = easygui_get_extra_data(pPanel);
    if (pPanelData == NULL) {
        return;
    }

    pPanelData->optionFlags = options;

    // The tabs need to be refreshed in order to reflect the new options.
    ak_panel_refresh_tabs(pPanel);
}
