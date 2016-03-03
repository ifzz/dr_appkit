// Public domain. See "unlicense" statement at the end of this file.

typedef struct
{
    /// A pointer to the application that owns this tool.
    ak_application* pApplication;

    /// The tools type.
    char type[AK_MAX_TOOL_TYPE_LENGTH];


    /// The tool's title. This is what will show up on the tool's tab.
    char title[256];

    /// The tool's tab that will be shown on the tab bar.
    drgui_tab* pTab;

    /// The panel the tab is attached to, if any.
    drgui_element* pPanel;


    /// The function to call when the tool needs to handle an action.
    ak_tool_on_handle_action_proc onHandleAction;


    /// The size of the tool's extra data, in bytes.
    size_t extraDataSize;

    /// A pointer to the tool's extra data.
    char pExtraData[1];

} ak_tool_data;


drgui_element* ak_create_tool(ak_application* pApplication, drgui_element* pParent, const char* type, size_t extraDataSize, const void* pExtraData)
{
    drgui_element* pElement = drgui_create_element(ak_get_application_gui(pApplication), pParent, sizeof(ak_tool_data) - sizeof(char) + extraDataSize, NULL);
    if (pElement != NULL)
    {
        drgui_hide(pElement);


        ak_tool_data* pToolData = drgui_get_extra_data(pElement);
        assert(pToolData != NULL);

        pToolData->pApplication   = pApplication;
        pToolData->type[0]        = '\0';
        pToolData->title[0]       = '\0';
        pToolData->pTab           = NULL;
        pToolData->pPanel         = NULL;
        pToolData->onHandleAction = NULL;

        if (type != NULL) {
            strcpy_s(pToolData->type, sizeof(pToolData->type), type);
        }


        pToolData->extraDataSize = extraDataSize;
        if (pExtraData != NULL) {
            memcpy(pToolData->pExtraData, pExtraData, extraDataSize);
        }
    }

    return pElement;
}

void ak_delete_tool(drgui_element* pTool)
{
    drgui_delete_element(pTool);
}

ak_application* ak_get_tool_application(drgui_element* pTool)
{
    ak_tool_data* pToolData = drgui_get_extra_data(pTool);
    if (pToolData == NULL) {
        return NULL;
    }

    return pToolData->pApplication;
}

const char* ak_get_tool_type(drgui_element* pTool)
{
    ak_tool_data* pToolData = drgui_get_extra_data(pTool);
    if (pToolData == NULL) {
        return NULL;
    }

    return pToolData->type;
}


size_t ak_get_tool_extra_data_size(drgui_element* pTool)
{
    ak_tool_data* pToolData = drgui_get_extra_data(pTool);
    if (pToolData == NULL) {
        return 0;
    }

    return pToolData->extraDataSize;
}

void* ak_get_tool_extra_data(drgui_element* pTool)
{
    ak_tool_data* pToolData = drgui_get_extra_data(pTool);
    if (pToolData == NULL) {
        return NULL;
    }

    return pToolData->pExtraData;
}


bool ak_is_of_tool_type(const char* pToolType, const char* pBaseToolType)
{
    if (pToolType == NULL || pBaseToolType == NULL) {
        return false;
    }

    return strncmp(pToolType, pBaseToolType, strlen(pBaseToolType)) == 0;
}

bool ak_is_tool_of_type(drgui_element* pTool, const char* type)
{
    ak_tool_data* pToolData = drgui_get_extra_data(pTool);
    if (pToolData == NULL) {
        return false;
    }

    return strncmp(pToolData->type, type, strlen(type)) == 0;
}


drgui_tab* ak_get_tool_tab(drgui_element* pTool)
{
    ak_tool_data* pToolData = drgui_get_extra_data(pTool);
    if (pToolData == NULL) {
        return NULL;
    }

    return pToolData->pTab;
}

drgui_element* ak_get_tool_panel(drgui_element* pTool)
{
    ak_tool_data* pToolData = drgui_get_extra_data(pTool);
    if (pToolData == NULL) {
        return NULL;
    }

    return pToolData->pPanel;
}


void ak_set_tool_title(drgui_element* pTool, const char* title)
{
    ak_tool_data* pToolData = drgui_get_extra_data(pTool);
    if (pToolData == NULL) {
        return;
    }

    strncpy_s(pToolData->title, sizeof(pToolData->title), (title != NULL) ? title : "", _TRUNCATE);

    drgui_tab_set_text(pToolData->pTab, title);
}

const char* ak_get_tool_title(drgui_element* pTool)
{
    ak_tool_data* pToolData = drgui_get_extra_data(pTool);
    if (pToolData == NULL) {
        return NULL;
    }

    return pToolData->title;
}


void ak_tool_handle_action(drgui_element* pTool, const char* pActionName)
{
    ak_tool_data* pToolData = drgui_get_extra_data(pTool);
    if (pToolData == NULL) {
        return;
    }

    if (pToolData->onHandleAction) {
        pToolData->onHandleAction(pTool, pActionName);
    }
}

void ak_tool_set_on_handle_action(drgui_element* pTool, ak_tool_on_handle_action_proc proc)
{
    ak_tool_data* pToolData = drgui_get_extra_data(pTool);
    if (pToolData == NULL) {
        return;
    }

    pToolData->onHandleAction = proc;
}



void ak_set_tool_tab(drgui_element* pTool, drgui_tab* pTab)
{
    ak_tool_data* pToolData = drgui_get_extra_data(pTool);
    if (pToolData == NULL) {
        return;
    }

    pToolData->pTab = pTab;
}

void ak_set_tool_panel(drgui_element* pTool, drgui_element* pPanel)
{
    ak_tool_data* pToolData = drgui_get_extra_data(pTool);
    if (pToolData == NULL) {
        return;
    }

    pToolData->pPanel = pPanel;
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
