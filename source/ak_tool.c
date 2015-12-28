// Public domain. See "unlicense" statement at the end of this file.

#include "../include/easy_appkit/ak_panel.h"
#include "../include/easy_appkit/ak_tool.h"
#include "../include/easy_appkit/ak_application.h"
#include "../include/easy_appkit/ak_build_config.h"
#include <easy_gui/easy_gui.h>
#include <assert.h>

typedef struct
{
    /// A pointer to the application that owns this tool.
    ak_application* pApplication;

    /// The tools type.
    char type[AK_MAX_TOOL_TYPE_LENGTH];


    /// The tool's title. This is what will show up on the tool's tab.
    char title[256];


    /// The size of the tool's extra data, in bytes.
    size_t extraDataSize;

    /// A pointer to the tool's extra data.
    char pExtraData[1];

} ak_tool_data;


easygui_element* ak_create_tool(ak_application* pApplication, easygui_element* pParent, const char* type, size_t extraDataSize, const void* pExtraData)
{
    easygui_element* pElement = easygui_create_element(ak_get_application_gui(pApplication), pParent, sizeof(ak_tool_data) - sizeof(char) + extraDataSize);
    if (pElement != NULL)
    {
        easygui_hide(pElement);


        ak_tool_data* pToolData = easygui_get_extra_data(pElement);
        assert(pToolData != NULL);

        pToolData->pApplication = pApplication;
        pToolData->type[0]      = '\0';
        pToolData->title[0]     = '\0';

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

ak_application* ak_get_tool_application(easygui_element* pTool)
{
    ak_tool_data* pToolData = easygui_get_extra_data(pTool);
    if (pToolData == NULL) {
        return NULL;
    }

    return pToolData->pApplication;
}

const char* ak_get_tool_type(easygui_element* pTool)
{
    ak_tool_data* pToolData = easygui_get_extra_data(pTool);
    if (pToolData == NULL) {
        return NULL;
    }

    return pToolData->type;
}


size_t ak_get_tool_extra_data_size(easygui_element* pTool)
{
    ak_tool_data* pToolData = easygui_get_extra_data(pTool);
    if (pToolData == NULL) {
        return 0;
    }

    return pToolData->extraDataSize;
}

void* ak_get_tool_extra_data(easygui_element* pTool)
{
    ak_tool_data* pToolData = easygui_get_extra_data(pTool);
    if (pToolData == NULL) {
        return NULL;
    }

    return pToolData->pExtraData;
}


void ak_set_tool_title(easygui_element* pTool, const char* title)
{
    ak_tool_data* pToolData = easygui_get_extra_data(pTool);
    if (pToolData == NULL) {
        return;
    }

    strcpy_s(pToolData->title, sizeof(pToolData->title), (title != NULL) ? title : "");
}

const char* ak_get_tool_title(easygui_element* pTool)
{
    ak_tool_data* pToolData = easygui_get_extra_data(pTool);
    if (pToolData == NULL) {
        return NULL;
    }

    return pToolData->title;
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
