// Public domain. See "unlicense" statement at the end of this file.

#include "../include/easy_appkit/ak_panel.h"
#include "../include/easy_appkit/ak_application.h"
#include "../include/easy_appkit/ak_theme.h"
#include "../include/easy_appkit/ak_tool.h"
#include "../include/easy_appkit/ak_gui_image_manager.h"
#include "../include/easy_appkit/ak_build_config.h"
#include "ak_tool_private.h"
#include <easy_gui/easy_gui.h>
#include <easy_util/easy_util.h>
#include <assert.h>

typedef struct 
{
    /// A pointer to the main application.
    ak_application* pApplication;


    /// The type of the panel.
    char type[AK_MAX_PANEL_TYPE_LENGTH];


    /// The axis the two child panels are split along, if any. When this is not ak_panel_split_axis_none, it is assumed
    /// the panel has two child elements, both of which should be panels.
    ak_panel_split_axis splitAxis;

    /// The position of the split.
    float splitPos;

    /// The tab bar orientation.
    tabbar_orientation tabBarOrientation;

    /// The tab bar for tool tabs.
    easygui_element* pTabBar;

    /// The container for tools.
    easygui_element* pToolContainer;


    /// Flags for tracking basic settings for the panel.
    unsigned int optionFlags;


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

PRIVATE void ak_panel_refresh_tool_container_layout(easygui_element* pPanel)
{
    ak_panel_data* pPanelData = easygui_get_extra_data(pPanel);
    assert(pPanelData != NULL);

    float width  = easygui_get_width(pPanel);
    float height = easygui_get_height(pPanel);

    float posX = 0;
    float posY = 0;

    // The layout of the container is based on the the layout of the tab bar and the tab bar's orientation.
    if (easygui_is_visible(pPanelData->pTabBar))
    {
        if (pPanelData->tabBarOrientation == tabbar_orientation_top)
        {
            posY    = easygui_get_height(pPanelData->pTabBar);
            height -= easygui_get_height(pPanelData->pTabBar);
        }
        else if (pPanelData->tabBarOrientation == tabbar_orientation_bottom)
        {
            posY    = easygui_get_height(pPanel) - easygui_get_height(pPanelData->pTabBar);
            height -= easygui_get_height(pPanelData->pTabBar);
        }
        else if (pPanelData->tabBarOrientation == tabbar_orientation_left)
        {
            posX   = easygui_get_width(pPanelData->pTabBar);
            width -= easygui_get_width(pPanelData->pTabBar);
        }
        else if (pPanelData->tabBarOrientation == tabbar_orientation_right)
        {
            posX   = easygui_get_width(pPanel) - easygui_get_width(pPanelData->pTabBar);
            width -= easygui_get_width(pPanelData->pTabBar);
        }
    }


    float innerScaleX;
    float innerScaleY;
    easygui_get_inner_scale(pPanel, &innerScaleX, &innerScaleY);

    if (easygui_get_relative_position_x(pPanelData->pToolContainer) != posX || easygui_get_relative_position_y(pPanelData->pToolContainer) != posY) {
        easygui_set_relative_position(pPanelData->pToolContainer, posX / innerScaleX, posY / innerScaleY);
    }
    
    if (easygui_get_width(pPanelData->pToolContainer) != width || easygui_get_height(pPanelData->pToolContainer) != height) {
        easygui_set_size(pPanelData->pToolContainer, width / innerScaleX, height / innerScaleY);
    }
}

PRIVATE void ak_panel_refresh_tabs(easygui_element* pPanel)
{
    assert(pPanel != NULL);

    ak_panel_data* pPanelData = easygui_get_extra_data(pPanel);
    assert(pPanelData != NULL);

    ak_theme* pTheme = ak_get_application_theme(ak_get_panel_application(pPanel));
    assert(pTheme != NULL);


    // The layout of the tab bar needs to be refreshed. We only adjust the width OR height, which depends on the orientation.
    float panelWidth   = easygui_get_width(pPanel);
    float panelHeight  = easygui_get_height(pPanel);
    float tabbarWidth  = easygui_get_width(pPanelData->pTabBar);
    float tabbarHeight = easygui_get_height(pPanelData->pTabBar);
    float tabbarPosX   = 0;
    float tabbarPosY   = 0;

    if (easygui_is_visible(pPanelData->pTabBar))
    {
        if (pPanelData->tabBarOrientation == tabbar_orientation_top)
        {
            tabbarWidth = panelWidth;
        }
        else if (pPanelData->tabBarOrientation == tabbar_orientation_bottom)
        {
            tabbarWidth = panelWidth;
            tabbarPosY  = panelHeight - tabbarHeight;
        }
        else if (pPanelData->tabBarOrientation == tabbar_orientation_left)
        {
            tabbarHeight = panelHeight;
        }
        else if (pPanelData->tabBarOrientation == tabbar_orientation_right)
        {
            tabbarHeight = panelHeight;
            tabbarPosX   = panelWidth - tabbarWidth;
        }
    }

    easygui_set_size(pPanelData->pTabBar, tabbarWidth, tabbarHeight);
    easygui_set_relative_position(pPanelData->pTabBar, tabbarPosX, tabbarPosY);



    if ((pPanelData->optionFlags & AK_PANEL_OPTION_SHOW_TOOL_TABS) == 0) {
        easygui_hide(pPanelData->pTabBar);
    } else {
        easygui_show(pPanelData->pTabBar);
    }

    if ((pPanelData->optionFlags & AK_PANEL_OPTION_SHOW_CLOSE_BUTTON_ON_TABS) == 0) {
        tabbar_hide_close_buttons(pPanelData->pTabBar);
        tabbar_disable_close_on_middle_click(pPanelData->pTabBar);
    } else {
        tabbar_show_close_buttons(pPanelData->pTabBar);
        tabbar_enable_close_on_middle_click(pPanelData->pTabBar);
    }


    // The layout of the tool container needs to be updated first.
    ak_panel_refresh_tool_container_layout(pPanel);
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
            ak_panel_refresh_tabs(pElement);
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
}

PRIVATE void ak_panel_on_mouse_button_down(easygui_element* pElement, int button, int relativeMousePosX, int relativeMousePosY)
{
    ak_panel_data* pPanelData = easygui_get_extra_data(pElement);
    assert(pPanelData != NULL);

    pPanelData->isMouseOver = true;
    pPanelData->relativeMousePosX = (float)relativeMousePosX;
    pPanelData->relativeMousePosY = (float)relativeMousePosY;
}


PRIVATE void ak_panel_on_tab_bar_size(easygui_element* pTBElement, float newWidth, float newHeight)
{
    easygui_element* pPanel = *(easygui_element**)tabbar_get_extra_data(pTBElement);
    assert(pPanel != NULL);

    // TODO: Make sure the tab bar is pinned in the correct position for right and bottom alignments.

    ak_panel_refresh_tool_container_layout(pPanel);
}

PRIVATE void ak_panel_on_tab_deactivated(easygui_element* pTBElement, easygui_tab* pTab)
{
    easygui_element* pPanel = *(easygui_element**)tabbar_get_extra_data(pTBElement);
    assert(pPanel != NULL);

    easygui_element* pTool = *(easygui_element**)tab_get_extra_data(pTab);
    assert(pTool != NULL);


    // The tool needs to be hidden.
    easygui_hide(pTool);

    ak_panel_data* pPanelData = easygui_get_extra_data(pPanel);
    pPanelData->pActiveTool = NULL;
}

PRIVATE void ak_panel_on_tab_activated(easygui_element* pTBElement, easygui_tab* pTab)
{
    easygui_element* pPanel = *(easygui_element**)tabbar_get_extra_data(pTBElement);
    assert(pPanel != NULL);

    easygui_element* pTool = *(easygui_element**)tab_get_extra_data(pTab);
    assert(pTool != NULL);


    // The tool needs to be shown.
    easygui_show(pTool);


    ak_panel_data* pPanelData = easygui_get_extra_data(pPanel);
    pPanelData->pActiveTool = pTool;
}

PRIVATE void ak_panel_on_tab_close(easygui_element* pTBElement, easygui_tab* pTab)
{
    easygui_element* pPanel = *(easygui_element**)tabbar_get_extra_data(pTBElement);
    assert(pPanel != NULL);

    easygui_element* pTool = *(easygui_element**)tab_get_extra_data(pTab);
    assert(pTool != NULL);

    ak_application_delete_tool(ak_get_panel_application(pPanel), pTool, false);     // "false" means that the tool should not be forced to be deleted - we may want to show a confirmation dialog.
}



easygui_element* ak_create_panel(ak_application* pApplication, easygui_element* pParent, size_t extraDataSize, const void* pExtraData)
{
    easygui_element* pElement = easygui_create_element(ak_get_application_gui(pApplication), pParent, sizeof(ak_panel_data) - sizeof(char) + extraDataSize);
    if (pElement != NULL)
    {
        ak_panel_data* pPanelData = easygui_get_extra_data(pElement);
        assert(pPanelData != NULL);

        pPanelData->pApplication       = pApplication;
        pPanelData->type[0]            = '\0';
        pPanelData->splitAxis          = ak_panel_split_axis_none;
        pPanelData->splitPos           = 0;
        pPanelData->tabBarOrientation  = tabbar_orientation_top;
        pPanelData->pTabBar            = NULL;
        pPanelData->pToolContainer     = NULL;
        pPanelData->optionFlags        = 0;
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


void ak_panel_set_type(easygui_element* pPanel, const char* type)
{
    ak_panel_data* pPanelData = easygui_get_extra_data(pPanel);
    if (pPanelData == NULL) {
        return;
    }

    if (type == NULL) {
        pPanelData->type[0] = '\0';
    } else {
        strcpy_s(pPanelData->type, sizeof(pPanelData->type), type);
    }
}

const char* ak_panel_get_type(easygui_element* pPanel)
{
    ak_panel_data* pPanelData = easygui_get_extra_data(pPanel);
    if (pPanelData == NULL) {
        return NULL;
    }

    return pPanelData->type;
}

easygui_element* ak_panel_find_by_type_recursive(easygui_element* pPanel, const char* type)
{
    ak_panel_data* pPanelData = easygui_get_extra_data(pPanel);
    if (pPanelData == NULL) {
        return NULL;
    }

    if (type == NULL) {
        return NULL;
    }


    if (strcmp(pPanelData->type, type) == 0) {
        return pPanel;
    }

    // If it's a split panel we need to check those. If it's not split, we just return NULL;
    if (pPanelData->splitAxis != ak_panel_split_axis_none)
    {
        easygui_element* pResult = NULL;
        
        pResult = ak_panel_find_by_type_recursive(ak_panel_get_split_panel_1(pPanel), type);
        if (pResult != NULL) {
            return pResult;
        }

        pResult = ak_panel_find_by_type_recursive(ak_panel_get_split_panel_2(pPanel), type);
        if (pResult != NULL) {
            return pResult;
        }
    }

    return NULL;
}

bool ak_panel_is_of_type(easygui_element* pPanel, const char* type)
{
    ak_panel_data* pPanelData = easygui_get_extra_data(pPanel);
    if (pPanelData == NULL) {
        return false;
    }

    return strncmp(pPanelData->type, type, strlen(type)) == 0;
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

    ak_theme* pTheme = ak_get_application_theme(pPanelData->pApplication);
    if (pTheme == NULL) {
        return false;
    }


    if (pPanelData->splitAxis != ak_panel_split_axis_none) {
        return false;
    }

    if (pTool == NULL) {
        return false;
    }


    // If the tool is already attached to a panel it will need to be detached first.
    if (ak_get_tool_panel(pTool) != NULL) {
        ak_panel_detach_tool(ak_get_tool_panel(pTool), pTool);
    }


    float innerScaleX;
    float innerScaleY;
    easygui_get_absolute_inner_scale(pPanel, &innerScaleX, &innerScaleY);


    // We need a tab bar and a tool container if we haven't already got one.
    if (pPanelData->pTabBar == NULL)
    {
        assert(pPanelData->pToolContainer == NULL);     // We should never have a tool container without a tab bar.
        assert(pPanel->pFirstChild == NULL);            // Assume the panel does not have any children.

        // Tab bar first.
        pPanelData->pTabBar = easygui_create_tab_bar(pPanel->pContext, pPanel, pPanelData->tabBarOrientation, sizeof(&pPanel), &pPanel);
        if (pPanelData->pTabBar == NULL) {
            return false;
        }

        // Make sure the size of the tab bar is set such that it extends across the entire panel.
        if (pPanelData->tabBarOrientation == tabbar_orientation_top)
        {
            easygui_set_relative_position(pPanelData->pTabBar, 0, 0);
            easygui_set_size(pPanelData->pTabBar, easygui_get_width(pPanel) / innerScaleX, 0);
        }
        else if (pPanelData->tabBarOrientation == tabbar_orientation_bottom)
        {
            easygui_set_relative_position(pPanelData->pTabBar, 0, easygui_get_height(pPanel) / innerScaleY);
            easygui_set_size(pPanelData->pTabBar, easygui_get_width(pPanel) / innerScaleX, 0);
        }
        else if (pPanelData->tabBarOrientation == tabbar_orientation_left)
        {
            easygui_set_relative_position(pPanelData->pTabBar, 0, 0);
            easygui_set_size(pPanelData->pTabBar, 0, easygui_get_height(pPanel) / innerScaleY);
        }
        else if (pPanelData->tabBarOrientation == tabbar_orientation_right)
        {
            easygui_set_relative_position(pPanelData->pTabBar, easygui_get_width(pPanel) / innerScaleX, 0);
            easygui_set_size(pPanelData->pTabBar, 0, easygui_get_height(pPanel) / innerScaleY);
        }

        tabbar_set_font(pPanelData->pTabBar, pTheme->pUIFont);
        tabbar_enable_auto_size(pPanelData->pTabBar);
        easygui_set_on_size(pPanelData->pTabBar, ak_panel_on_tab_bar_size);
        tabbar_set_close_button_image(pPanelData->pTabBar, ak_get_red_cross_image(ak_get_application_image_manager(pPanelData->pApplication)));
        tabbar_set_on_tab_deactivated(pPanelData->pTabBar, ak_panel_on_tab_deactivated);
        tabbar_set_on_tab_activated(pPanelData->pTabBar, ak_panel_on_tab_activated);
        tabbar_set_on_tab_closed(pPanelData->pTabBar, ak_panel_on_tab_close);

        if ((pPanelData->optionFlags & AK_PANEL_OPTION_SHOW_TOOL_TABS) == 0) {
            easygui_hide(pPanelData->pTabBar);
        }



        // Tool container.
        pPanelData->pToolContainer = easygui_create_element(pPanel->pContext, pPanel, 0);
        if (pPanelData->pToolContainer == NULL) {
            return false;
        }
        easygui_set_size(pPanelData->pToolContainer, easygui_get_width(pPanel), easygui_get_height(pPanel));

        easygui_set_on_size(pPanelData->pToolContainer, easygui_on_size_fit_children_to_parent);
    }



    assert(pPanelData->pToolContainer != NULL);

    easygui_prepend(pTool, pPanelData->pToolContainer);
    ak_set_tool_panel(pTool, pPanel);


    // Initial size and position.
    easygui_set_relative_position(pTool, 0, 0);
    easygui_set_size(pTool, easygui_get_width(pPanelData->pToolContainer) / innerScaleX, easygui_get_height(pPanelData->pToolContainer) / innerScaleY);


    // We need to create and prepend a tab for the tool.
    easygui_tab* pToolTab = tabbar_create_and_prepend_tab(pPanelData->pTabBar, ak_get_tool_title(pTool), sizeof(&pTool), &pTool);
    ak_set_tool_tab(pTool, pToolTab);


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


    // If the tab is the active one we need to switch to another.
    easygui_tab* pTab = ak_get_tool_tab(pTool);
    if (tabbar_get_active_tab(pPanelData->pTabBar) == pTab)
    {
        easygui_tab* pNextTab = tab_get_next_tab(pTab);
        if (pNextTab == NULL) {
            pNextTab = tab_get_prev_tab(pTab);
        }

        tabbar_activate_tab(pPanelData->pTabBar, pNextTab);
    }


    easygui_detach(pTool);
    ak_set_tool_panel(pTool, NULL);

    tab_delete(ak_get_tool_tab(pTool));
    ak_set_tool_tab(pTool, NULL);


    // The tab bar might need to be refreshed.
    ak_panel_refresh_tabs(pPanel);
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


    // To activate a tool we just activate the associated tab on the tab bar control which will in turn post
    // activate and deactivate events which is where the actual swith will occur.
    tabbar_activate_tab(pPanelData->pTabBar, ak_get_tool_tab(pTool));

    return true;
}

easygui_element* ak_panel_get_active_tool(easygui_element* pPanel)
{
    ak_panel_data* pPanelData = easygui_get_extra_data(pPanel);
    if (pPanelData == NULL) {
        return false;
    }

    return pPanelData->pActiveTool;
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
