// Public domain. See "unlicense" statement at the end of this file.

typedef struct
{
    /// A pointer to the main application.
    ak_application* pApplication;


    /// The axis the two child panels are split along, if any. When this is not ak_panel_split_axis_none, it is assumed
    /// the panel has two child elements, both of which should be panels.
    ak_panel_split_axis splitAxis;

    /// The position of the split.
    float splitPos;

    /// The split ratio. Only used if maintainSplitRatio is true.
    float splitRatio;

    /// Whether or not the split position should be based on splitRatio.
    bool maintainSplitRatio;


    /// The tab bar orientation.
    drgui_tabbar_orientation tabBarOrientation;

    /// The tab bar for tool tabs.
    drgui_element* pTabBar;

    /// The image to use for the close button the tab bar.
    drgui_image* pTabBarCloseImage;

    /// The container for tools.
    drgui_element* pToolContainer;


    /// Flags for tracking basic settings for the panel.
    unsigned int optionFlags;


    /// Keeps track of whether or not the mouse is over the panel.
    bool isMouseOver;

    /// Keeps track of the relative position of the mouse on the X axis.
    float relativeMousePosX;

    /// Keeps track of the relative position of the mouse on the X axis.
    float relativeMousePosY;

    /// A pointer to the tool whose tab is being hovered over.
    drgui_element* pHoveredTool;

    /// A pointer to the tool whose tab is active.
    drgui_element* pActiveTool;


    /// The size of the panel's extra data, in bytes.
    size_t extraDataSize;

    /// A pointer to the panel's extra data.
    char pExtraData[1];

} ak_panel_data;


////////////////////////////////////////////////
// Private API

/// Refreshes the alignment of the child panels of the given panel.
static void ak_panel_refresh_child_alignments(drgui_element* pPanel)
{
    ak_panel_data* pPanelData = drgui_get_extra_data(pPanel);
    assert(pPanelData != NULL);
    assert(pPanelData->splitAxis != ak_panel_split_axis_none);

    assert(pPanel->pFirstChild != NULL);
    assert(pPanel->pFirstChild->pNextSibling != NULL);

    float innerScaleX;
    float innerScaleY;
    drgui_get_inner_scale(pPanel, &innerScaleX, &innerScaleY);

    drgui_element* pChildPanel1 = pPanel->pFirstChild;
    drgui_element* pChildPanel2 = pPanel->pFirstChild->pNextSibling;

    float borderWidth = 0;
    float splitPos = pPanelData->splitPos;

    if (pPanelData->splitAxis == ak_panel_split_axis_horizontal || pPanelData->splitAxis == ak_panel_split_axis_horizontal_bottom)
    {
        // Horizontal.
        if (pPanelData->maintainSplitRatio) {
            splitPos = pPanel->height * pPanelData->splitRatio;
        }

        if (pPanelData->splitAxis == ak_panel_split_axis_horizontal_bottom) {
            splitPos = pPanel->height - splitPos;
        }

        drgui_set_relative_position(pChildPanel1, borderWidth, borderWidth);
        drgui_set_size(pChildPanel1, (pPanel->width / innerScaleX) - (borderWidth*2), splitPos - borderWidth);

        drgui_set_relative_position(pChildPanel2, borderWidth, splitPos);
        drgui_set_size(pChildPanel2, (pPanel->width / innerScaleX) - (borderWidth*2), (pPanel->height / innerScaleY) - splitPos - borderWidth);
    }
    else
    {
        // Vertical.
        if (pPanelData->maintainSplitRatio) {
            splitPos = pPanel->width * pPanelData->splitRatio;
        }

        if (pPanelData->splitAxis == ak_panel_split_axis_vertical_right) {
            splitPos = pPanel->width - splitPos;
        }

        drgui_set_relative_position(pChildPanel1, borderWidth, borderWidth);
        drgui_set_size(pChildPanel1, (splitPos - borderWidth) / innerScaleX, (pPanel->height - (borderWidth*2)) / innerScaleY);

        drgui_set_relative_position(pChildPanel2, splitPos, borderWidth);
        drgui_set_size(pChildPanel2, (pPanel->width - splitPos - borderWidth) / innerScaleX, (pPanel->height - (borderWidth*2)) / innerScaleY);
    }
}

static void ak_panel_refresh_tool_container_layout(drgui_element* pPanel)
{
    ak_panel_data* pPanelData = drgui_get_extra_data(pPanel);
    assert(pPanelData != NULL);

    float width  = drgui_get_width(pPanel);
    float height = drgui_get_height(pPanel);

    float posX = 0;
    float posY = 0;

    // The layout of the container is based on the the layout of the tab bar and the tab bar's orientation.
    if (drgui_is_visible(pPanelData->pTabBar))
    {
        if (pPanelData->tabBarOrientation == drgui_tabbar_orientation_top)
        {
            posY    = drgui_get_height(pPanelData->pTabBar);
            height -= drgui_get_height(pPanelData->pTabBar);
        }
        else if (pPanelData->tabBarOrientation == drgui_tabbar_orientation_bottom)
        {
            posY    = drgui_get_height(pPanel) - drgui_get_height(pPanelData->pTabBar);
            height -= drgui_get_height(pPanelData->pTabBar);
        }
        else if (pPanelData->tabBarOrientation == drgui_tabbar_orientation_left)
        {
            posX   = drgui_get_width(pPanelData->pTabBar);
            width -= drgui_get_width(pPanelData->pTabBar);
        }
        else if (pPanelData->tabBarOrientation == drgui_tabbar_orientation_right)
        {
            posX   = drgui_get_width(pPanel) - drgui_get_width(pPanelData->pTabBar);
            width -= drgui_get_width(pPanelData->pTabBar);
        }
    }


    float innerScaleX;
    float innerScaleY;
    drgui_get_inner_scale(pPanel, &innerScaleX, &innerScaleY);

    if (drgui_get_relative_position_x(pPanelData->pToolContainer) != posX || drgui_get_relative_position_y(pPanelData->pToolContainer) != posY) {
        drgui_set_relative_position(pPanelData->pToolContainer, posX / innerScaleX, posY / innerScaleY);
    }

    if (drgui_get_width(pPanelData->pToolContainer) != width || drgui_get_height(pPanelData->pToolContainer) != height) {
        drgui_set_size(pPanelData->pToolContainer, width / innerScaleX, height / innerScaleY);
    }
}

static void ak_panel_refresh_tabs(drgui_element* pPanel)
{
    assert(pPanel != NULL);

    ak_panel_data* pPanelData = drgui_get_extra_data(pPanel);
    assert(pPanelData != NULL);

    ak_theme* pTheme = ak_get_application_theme(ak_get_panel_application(pPanel));
    assert(pTheme != NULL);


    // The layout of the tab bar needs to be refreshed. We only adjust the width OR height, which depends on the orientation.
    float panelWidth   = drgui_get_width(pPanel);
    float panelHeight  = drgui_get_height(pPanel);
    float tabbarWidth  = drgui_get_width(pPanelData->pTabBar);
    float tabbarHeight = drgui_get_height(pPanelData->pTabBar);
    float tabbarPosX   = 0;
    float tabbarPosY   = 0;

    if (drgui_is_visible(pPanelData->pTabBar))
    {
        if (pPanelData->tabBarOrientation == drgui_tabbar_orientation_top)
        {
            tabbarWidth = panelWidth;
        }
        else if (pPanelData->tabBarOrientation == drgui_tabbar_orientation_bottom)
        {
            tabbarWidth = panelWidth;
            tabbarPosY  = panelHeight - tabbarHeight;
        }
        else if (pPanelData->tabBarOrientation == drgui_tabbar_orientation_left)
        {
            tabbarHeight = panelHeight;
        }
        else if (pPanelData->tabBarOrientation == drgui_tabbar_orientation_right)
        {
            tabbarHeight = panelHeight;
            tabbarPosX   = panelWidth - tabbarWidth;
        }
    }

    drgui_set_size(pPanelData->pTabBar, tabbarWidth, tabbarHeight);
    drgui_set_relative_position(pPanelData->pTabBar, tabbarPosX, tabbarPosY);



    if ((pPanelData->optionFlags & AK_PANEL_OPTION_SHOW_TOOL_TABS) == 0) {
        drgui_hide(pPanelData->pTabBar);
    } else {
        drgui_show(pPanelData->pTabBar);
    }

    if ((pPanelData->optionFlags & AK_PANEL_OPTION_SHOW_CLOSE_BUTTON_ON_TABS) == 0) {
        drgui_tabbar_hide_close_buttons(pPanelData->pTabBar);
        drgui_tabbar_disable_close_on_middle_click(pPanelData->pTabBar);
    } else {
        drgui_tabbar_show_close_buttons(pPanelData->pTabBar);
        drgui_tabbar_enable_close_on_middle_click(pPanelData->pTabBar);
    }


    // The layout of the tool container needs to be updated first.
    ak_panel_refresh_tool_container_layout(pPanel);
}


static void ak_panel_on_paint(drgui_element* pPanel, drgui_rect relativeRect, void* pPaintData)
{
    ak_panel_data* pPanelData = drgui_get_extra_data(pPanel);
    assert(pPanelData != NULL);

    ak_theme* pTheme = ak_get_application_theme(ak_get_panel_application(pPanel));
    if (pTheme == NULL) {
        return;
    }


    // Only draw the background if we have no children.
    if (pPanel->pFirstChild == NULL || (pPanelData->pToolContainer != NULL && pPanelData->pToolContainer->pFirstChild == NULL))
    {
        drgui_draw_rect(pPanel, relativeRect, pTheme->baseColor, pPaintData);
    }
}

static void ak_panel_on_size(drgui_element* pElement, float newWidth, float newHeight)
{
    (void)newWidth;
    (void)newHeight;

    ak_panel_data* pPanelData = drgui_get_extra_data(pElement);
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

static void ak_panel_on_mouse_enter(drgui_element* pElement)
{
    ak_panel_data* pPanelData = drgui_get_extra_data(pElement);
    assert(pPanelData != NULL);

    pPanelData->isMouseOver = true;
}

static void ak_panel_on_mouse_leave(drgui_element* pElement)
{
    ak_panel_data* pPanelData = drgui_get_extra_data(pElement);
    assert(pPanelData != NULL);

    pPanelData->isMouseOver = false;

    ak_panel_refresh_tabs(pElement);
}

static void ak_panel_on_mouse_move(drgui_element* pElement, int relativeMousePosX, int relativeMousePosY, int stateFlags)
{
    (void)stateFlags;

    ak_panel_data* pPanelData = drgui_get_extra_data(pElement);
    assert(pPanelData != NULL);

    pPanelData->isMouseOver = true;
    pPanelData->relativeMousePosX = (float)relativeMousePosX;
    pPanelData->relativeMousePosY = (float)relativeMousePosY;
}

static void ak_panel_on_mouse_button_down(drgui_element* pElement, int button, int relativeMousePosX, int relativeMousePosY, int stateFlags)
{
    (void)stateFlags;
    (void)button;

    ak_panel_data* pPanelData = drgui_get_extra_data(pElement);
    assert(pPanelData != NULL);

    pPanelData->isMouseOver = true;
    pPanelData->relativeMousePosX = (float)relativeMousePosX;
    pPanelData->relativeMousePosY = (float)relativeMousePosY;
}


static void ak_panel_on_tab_bar_size(drgui_element* pTBElement, float newWidth, float newHeight)
{
    (void)newWidth;
    (void)newHeight;

    drgui_element* pPanel = *(drgui_element**)drgui_tabbar_get_extra_data(pTBElement);
    assert(pPanel != NULL);

    // TODO: Make sure the tab bar is pinned in the correct position for right and bottom alignments.

    ak_panel_refresh_tool_container_layout(pPanel);
}

static void ak_panel_on_tab_deactivated(drgui_element* pTBElement, drgui_tab* pTab)
{
    drgui_element* pPanel = *(drgui_element**)drgui_tabbar_get_extra_data(pTBElement);
    assert(pPanel != NULL);

    drgui_element* pTool = *(drgui_element**)tab_get_extra_data(pTab);
    assert(pTool != NULL);


    // The tool needs to be hidden.
    drgui_hide(pTool);

    ak_panel_data* pPanelData = drgui_get_extra_data(pPanel);
    pPanelData->pActiveTool = NULL;

    ak_application_on_tool_deactivated(pPanelData->pApplication, pTool);
}

static void ak_panel_on_tab_activated(drgui_element* pTBElement, drgui_tab* pTab)
{
    drgui_element* pPanel = *(drgui_element**)drgui_tabbar_get_extra_data(pTBElement);
    assert(pPanel != NULL);

    drgui_element* pTool = *(drgui_element**)tab_get_extra_data(pTab);
    assert(pTool != NULL);


    // The tool needs to be shown.
    drgui_show(pTool);

    ak_panel_data* pPanelData = drgui_get_extra_data(pPanel);
    pPanelData->pActiveTool = pTool;

    ak_application_on_tool_activated(pPanelData->pApplication, pTool);
}

static void ak_panel_on_tab_close(drgui_element* pTBElement, drgui_tab* pTab)
{
    drgui_element* pPanel = *(drgui_element**)drgui_tabbar_get_extra_data(pTBElement);
    assert(pPanel != NULL);

    drgui_element* pTool = *(drgui_element**)tab_get_extra_data(pTab);
    assert(pTool != NULL);

    ak_application_delete_tool(ak_get_panel_application(pPanel), pTool, false);     // "false" means that the tool should not be forced to be deleted - we may want to show a confirmation dialog.
}



drgui_element* ak_create_panel(ak_application* pApplication, drgui_element* pParent, size_t extraDataSize, const void* pExtraData)
{
    drgui_element* pElement = drgui_create_element(ak_get_application_gui(pApplication), pParent, sizeof(ak_panel_data) - sizeof(char) + extraDataSize, NULL);
    if (pElement != NULL)
    {
        ak_panel_data* pPanelData = drgui_get_extra_data(pElement);
        assert(pPanelData != NULL);

        pPanelData->pApplication       = pApplication;
        pPanelData->splitAxis          = ak_panel_split_axis_none;
        pPanelData->splitPos           = 0;
        pPanelData->splitRatio         = 0;
        pPanelData->maintainSplitRatio = false;
        pPanelData->tabBarOrientation  = drgui_tabbar_orientation_top;
        pPanelData->pTabBar            = NULL;
        pPanelData->pTabBarCloseImage  = NULL;
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

        drgui_set_on_paint(pElement, ak_panel_on_paint);
        drgui_set_on_size(pElement, ak_panel_on_size);
        drgui_set_on_mouse_enter(pElement, ak_panel_on_mouse_enter);
        drgui_set_on_mouse_leave(pElement, ak_panel_on_mouse_leave);
        drgui_set_on_mouse_move(pElement, ak_panel_on_mouse_move);
        drgui_set_on_mouse_button_down(pElement, ak_panel_on_mouse_button_down);
    }

    return pElement;
}

ak_application* ak_get_panel_application(drgui_element* pPanel)
{
    ak_panel_data* pPanelData = drgui_get_extra_data(pPanel);
    if (pPanelData == NULL) {
        return NULL;
    }

    return pPanelData->pApplication;
}

drgui_element* ak_panel_get_parent(drgui_element* pPanel)
{
    return pPanel->pParent;
}

size_t ak_panel_get_extra_data_size(drgui_element* pPanel)
{
    ak_panel_data* pPanelData = drgui_get_extra_data(pPanel);
    if (pPanelData == NULL) {
        return 0;
    }

    return pPanelData->extraDataSize;
}

void* ak_panel_get_extra_data(drgui_element* pPanel)
{
    ak_panel_data* pPanelData = drgui_get_extra_data(pPanel);
    if (pPanelData == NULL) {
        return NULL;
    }

    return pPanelData->pExtraData;
}


void ak_panel_set_type(drgui_element* pPanel, const char* type)
{
    drgui_set_type(pPanel, type);
}

const char* ak_panel_get_type(drgui_element* pPanel)
{
    return drgui_get_type(pPanel);
}

drgui_element* ak_panel_find_by_type_recursive(drgui_element* pPanel, const char* type)
{
    ak_panel_data* pPanelData = drgui_get_extra_data(pPanel);
    if (pPanelData == NULL) {
        return NULL;
    }

    if (type == NULL) {
        return NULL;
    }


    if (strcmp(drgui_get_type(pPanel), type) == 0) {
        return pPanel;
    }

    // If it's a split panel we need to check those. If it's not split, we just return NULL;
    if (pPanelData->splitAxis != ak_panel_split_axis_none)
    {
        drgui_element* pResult = NULL;

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

bool ak_panel_is_of_type(drgui_element* pPanel, const char* type)
{
    return drgui_is_of_type(pPanel, type);
}


bool ak_panel_split(drgui_element* pPanel, ak_panel_split_axis splitAxis, float splitPos)
{
    ak_panel_data* pPanelData = drgui_get_extra_data(pPanel);
    if (pPanelData == NULL) {
        return false;
    }

    // It's an error for a panel to be split while it has tools attached.
    if (pPanelData->pToolContainer != NULL) {
        return false;
    }


    // If children do not already exist we just create them. Otherwise we just reuse the existing ones and re-align them.
    drgui_element* pChildPanel1 = NULL;
    drgui_element* pChildPanel2 = NULL;

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

void ak_panel_unsplit(drgui_element* pPanel)
{
    ak_panel_data* pPanelData = drgui_get_extra_data(pPanel);
    if (pPanelData == NULL) {
        return;
    }

    if (pPanelData->splitAxis == ak_panel_split_axis_none) {
        return;
    }


    drgui_delete_element(pPanel->pFirstChild->pNextSibling);
    drgui_delete_element(pPanel->pFirstChild);

    pPanelData->splitAxis = ak_panel_split_axis_none;
    pPanelData->splitPos  = 0;
}

bool ak_panel_is_split(drgui_element* pPanel)
{
    return ak_panel_get_split_axis(pPanel) != ak_panel_split_axis_none;
}

ak_panel_split_axis ak_panel_get_split_axis(drgui_element* pPanel)
{
    ak_panel_data* pPanelData = drgui_get_extra_data(pPanel);
    if (pPanelData == NULL) {
        return ak_panel_split_axis_none;
    }

    return pPanelData->splitAxis;
}

float ak_panel_get_split_pos(drgui_element* pPanel)
{
    ak_panel_data* pPanelData = drgui_get_extra_data(pPanel);
    if (pPanelData == NULL) {
        return 0;
    }

    return pPanelData->splitPos;
}

drgui_element* ak_panel_get_split_panel_1(drgui_element* pPanel)
{
    ak_panel_data* pPanelData = drgui_get_extra_data(pPanel);
    if (pPanelData == NULL) {
        return NULL;
    }

    if (pPanelData->splitAxis == ak_panel_split_axis_none) {
        return NULL;
    }

    return pPanel->pFirstChild;
}

drgui_element* ak_panel_get_split_panel_2(drgui_element* pPanel)
{
    ak_panel_data* pPanelData = drgui_get_extra_data(pPanel);
    if (pPanelData == NULL) {
        return NULL;
    }

    if (pPanelData->splitAxis == ak_panel_split_axis_none) {
        return NULL;
    }

    return pPanel->pFirstChild->pNextSibling;
}

void ak_panel_enable_ratio_split(drgui_element* pPanel)
{
    ak_panel_data* pPanelData = drgui_get_extra_data(pPanel);
    if (pPanelData == NULL) {
        return;
    }

    pPanelData->maintainSplitRatio = true;

    float panelSize;
    if (pPanelData->splitAxis == ak_panel_split_axis_vertical) {
        panelSize = drgui_get_width(pPanel);
    } else {
        panelSize = drgui_get_height(pPanel);
    }

    if (panelSize == 0) {
        pPanelData->splitRatio = 1;
    } else {
        pPanelData->splitRatio = pPanelData->splitPos / panelSize;
    }
}

void ak_panel_disable_ratio_split(drgui_element* pPanel)
{
    ak_panel_data* pPanelData = drgui_get_extra_data(pPanel);
    if (pPanelData == NULL) {
        return;
    }

    pPanelData->maintainSplitRatio = false;
    pPanelData->splitRatio = 0;
}


bool ak_panel_attach_tool(drgui_element* pPanel, drgui_element* pTool)
{
    ak_panel_data* pPanelData = drgui_get_extra_data(pPanel);
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
    drgui_get_absolute_inner_scale(pPanel, &innerScaleX, &innerScaleY);


    // We need a tab bar and a tool container if we haven't already got one.
    if (pPanelData->pTabBar == NULL)
    {
        assert(pPanelData->pToolContainer == NULL);     // We should never have a tool container without a tab bar.
        assert(pPanel->pFirstChild == NULL);            // Assume the panel does not have any children.

        // Tab bar first.
        pPanelData->pTabBar = drgui_create_tab_bar(pPanel->pContext, pPanel, pPanelData->tabBarOrientation, sizeof(&pPanel), &pPanel);
        if (pPanelData->pTabBar == NULL) {
            return false;
        }

        // Make sure the size of the tab bar is set such that it extends across the entire panel.
        if (pPanelData->tabBarOrientation == drgui_tabbar_orientation_top)
        {
            drgui_set_relative_position(pPanelData->pTabBar, 0, 0);
            drgui_set_size(pPanelData->pTabBar, drgui_get_width(pPanel) / innerScaleX, 0);
        }
        else if (pPanelData->tabBarOrientation == drgui_tabbar_orientation_bottom)
        {
            drgui_set_relative_position(pPanelData->pTabBar, 0, drgui_get_height(pPanel) / innerScaleY);
            drgui_set_size(pPanelData->pTabBar, drgui_get_width(pPanel) / innerScaleX, 0);
        }
        else if (pPanelData->tabBarOrientation == drgui_tabbar_orientation_left)
        {
            drgui_set_relative_position(pPanelData->pTabBar, 0, 0);
            drgui_set_size(pPanelData->pTabBar, 0, drgui_get_height(pPanel) / innerScaleY);
        }
        else if (pPanelData->tabBarOrientation == drgui_tabbar_orientation_right)
        {
            drgui_set_relative_position(pPanelData->pTabBar, drgui_get_width(pPanel) / innerScaleX, 0);
            drgui_set_size(pPanelData->pTabBar, 0, drgui_get_height(pPanel) / innerScaleY);
        }

        drgui_tabbar_set_font(pPanelData->pTabBar, pTheme->pUIFont);
        drgui_tabbar_enable_auto_size(pPanelData->pTabBar);
        drgui_set_on_size(pPanelData->pTabBar, ak_panel_on_tab_bar_size);
        drgui_tabbar_set_on_tab_deactivated(pPanelData->pTabBar, ak_panel_on_tab_deactivated);
        drgui_tabbar_set_on_tab_activated(pPanelData->pTabBar, ak_panel_on_tab_activated);
        drgui_tabbar_set_on_tab_closed(pPanelData->pTabBar, ak_panel_on_tab_close);

        if (pPanelData->pTabBarCloseImage != NULL) {
            drgui_tabbar_set_close_button_image(pPanelData->pTabBar, pPanelData->pTabBarCloseImage);
        }

        if ((pPanelData->optionFlags & AK_PANEL_OPTION_SHOW_TOOL_TABS) == 0) {
            drgui_hide(pPanelData->pTabBar);
        }



        // Tool container.
        pPanelData->pToolContainer = drgui_create_element(pPanel->pContext, pPanel, 0, NULL);
        if (pPanelData->pToolContainer == NULL) {
            return false;
        }
        drgui_set_size(pPanelData->pToolContainer, drgui_get_width(pPanel), drgui_get_height(pPanel));

        drgui_set_on_size(pPanelData->pToolContainer, drgui_on_size_fit_children_to_parent);
    }



    assert(pPanelData->pToolContainer != NULL);

    drgui_prepend(pTool, pPanelData->pToolContainer);
    ak_set_tool_panel(pTool, pPanel);


    // Initial size and position.
    drgui_set_relative_position(pTool, 0, 0);
    drgui_set_size(pTool, drgui_get_width(pPanelData->pToolContainer) / innerScaleX, drgui_get_height(pPanelData->pToolContainer) / innerScaleY);


    // We need to create and prepend a tab for the tool.
    drgui_tab* pToolTab = drgui_tabbar_create_and_prepend_tab(pPanelData->pTabBar, ak_get_tool_title(pTool), sizeof(&pTool), &pTool);
    ak_set_tool_tab(pTool, pToolTab);


    // Activate the new tool.
    ak_panel_activate_tool(pPanel, pTool);

    // The tab bar might need to be refreshed.
    ak_panel_refresh_tabs(pPanel);

    return true;
}

void ak_panel_detach_tool(drgui_element* pPanel, drgui_element* pTool)
{
    ak_panel_data* pPanelData = drgui_get_extra_data(pPanel);
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
    drgui_tab* pTab = ak_get_tool_tab(pTool);
    if (drgui_tabbar_get_active_tab(pPanelData->pTabBar) == pTab)
    {
        drgui_tab* pNextTab = tab_get_next_tab(pTab);
        if (pNextTab == NULL) {
            pNextTab = tab_get_prev_tab(pTab);
        }

        drgui_tabbar_activate_tab(pPanelData->pTabBar, pNextTab);
    }


    drgui_detach(pTool);
    ak_set_tool_panel(pTool, NULL);

    tab_delete(ak_get_tool_tab(pTool));
    ak_set_tool_tab(pTool, NULL);


    // The tab bar might need to be refreshed.
    ak_panel_refresh_tabs(pPanel);
}


bool ak_panel_activate_tool(drgui_element* pPanel, drgui_element* pTool)
{
    ak_panel_data* pPanelData = drgui_get_extra_data(pPanel);
    if (pPanelData == NULL) {
        return false;
    }

    if (!(pTool == NULL || pTool->pParent == pPanelData->pToolContainer)) {
        return false;
    }

    // To activate a tool we just activate the associated tab on the tab bar control which will in turn post
    // activate and deactivate events which is where the actual swith will occur.
    drgui_tabbar_activate_tab(pPanelData->pTabBar, ak_get_tool_tab(pTool));

    return true;
}

drgui_element* ak_panel_get_active_tool(drgui_element* pPanel)
{
    ak_panel_data* pPanelData = drgui_get_extra_data(pPanel);
    if (pPanelData == NULL) {
        return false;
    }

    return pPanelData->pActiveTool;
}


drgui_element* ak_panel_get_first_tool(drgui_element* pPanel)
{
    ak_panel_data* pPanelData = drgui_get_extra_data(pPanel);
    if (pPanelData == NULL) {
        return NULL;
    }

    if (pPanelData->pToolContainer == NULL) {
        return NULL;
    }

    return pPanelData->pToolContainer->pFirstChild;
}

drgui_element* ak_panel_get_next_tool(drgui_element* pPanel, drgui_element* pTool)
{
    ak_panel_data* pPanelData = drgui_get_extra_data(pPanel);
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


void ak_panel_set_tab_options(drgui_element* pPanel, unsigned int options)
{
    ak_panel_data* pPanelData = drgui_get_extra_data(pPanel);
    if (pPanelData == NULL) {
        return;
    }

    pPanelData->optionFlags = options;

    // The tabs need to be refreshed in order to reflect the new options.
    ak_panel_refresh_tabs(pPanel);
}

void ak_panel_set_tab_close_button_image(drgui_element* pPanel, drgui_image* pImage)
{
    ak_panel_data* pPanelData = drgui_get_extra_data(pPanel);
    if (pPanelData == NULL) {
        return;
    }

    pPanelData->pTabBarCloseImage = pImage;

    if (pPanelData->pTabBar != NULL) {
        drgui_tabbar_set_close_button_image(pPanelData->pTabBar, pImage);
    }
}
