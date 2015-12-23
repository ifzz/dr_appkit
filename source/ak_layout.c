// Public domain. See "unlicense" statement at the end of this file.

#include "ak_layout.h"
#include <stdlib.h>

void ak_detach_layout(ak_layout* pLayout)
{
    if (pLayout == NULL) {
        return;
    }

    if (pLayout->pParent != NULL)
    {
        if (pLayout->pParent->pFirstChild == pLayout) {
            pLayout->pParent->pFirstChild = pLayout->pNextSibling;
        }

        if (pLayout->pParent->pLastChild == pLayout) {
            pLayout->pParent->pLastChild = pLayout->pPrevSibling;
        }


        if (pLayout->pPrevSibling != NULL) {
            pLayout->pPrevSibling->pNextSibling = pLayout->pNextSibling;
        }

        if (pLayout->pNextSibling != NULL) {
            pLayout->pNextSibling->pPrevSibling = pLayout->pPrevSibling;
        }
    }

    pLayout->pParent      = NULL;
    pLayout->pPrevSibling = NULL;
    pLayout->pNextSibling = NULL;
}

void ak_append_layout(ak_layout* pChild, ak_layout* pParent)
{
    if (pChild == NULL || pParent == NULL) {
        return;
    }

    // Detach the child from it's current parent first.
    ak_detach_layout(pChild);

    pChild->pParent = pParent;
    if (pChild->pParent != NULL)
    {
        if (pChild->pParent->pLastChild != NULL) {
            pChild->pPrevSibling = pChild->pParent->pLastChild;
            pChild->pPrevSibling->pNextSibling = pChild;
        }

        if (pChild->pParent->pFirstChild == NULL) {
            pChild->pParent->pFirstChild = pChild;
        }

        pChild->pParent->pLastChild = pChild;
    }
}

void ak_prepend_layout(ak_layout* pChild, ak_layout* pParent)
{
    if (pChild == NULL || pParent == NULL) {
        return;
    }

    // Detach the child from it's current parent first.
    ak_detach_layout(pChild);

    pChild->pParent = pParent;
    if (pChild->pParent != NULL)
    {
        if (pChild->pParent->pFirstChild != NULL) {
            pChild->pNextSibling = pChild->pParent->pFirstChild;
            pChild->pNextSibling->pPrevSibling = pChild;
        }

        if (pChild->pParent->pLastChild == NULL) {
            pChild->pParent->pLastChild = pChild;
        }

        pChild->pParent->pFirstChild = pChild;
    }
}


ak_layout* ak_create_layout(const char* type, const char* attributes, ak_layout* pParent)
{
    ak_layout* pLayout = malloc(sizeof(*pLayout));
    if (pLayout != NULL)
    {
        if (type != NULL) {
            strcpy_s(pLayout->type, sizeof(pLayout->type), type);
        } else {
            pLayout->type[0] = '\0';
        }

        if (attributes != NULL) {
            strcpy_s(pLayout->attributes, sizeof(pLayout->attributes), attributes);
        } else {
            pLayout->attributes[0] = '\0';
        }

        pLayout->pParent      = NULL;
        pLayout->pFirstChild  = NULL;
        pLayout->pLastChild   = NULL;
        pLayout->pNextSibling = NULL;
        pLayout->pPrevSibling = NULL;

        if (pParent != NULL) {
            ak_append_layout(pLayout, pParent);
        }
    }

    return pLayout;
}

void ak_delete_layout(ak_layout* pLayout)
{
    if (pLayout == NULL) {
        return;
    }

    // Every child needs to be deleted first.
    while (pLayout->pFirstChild != NULL)
    {
        ak_delete_layout(pLayout->pFirstChild);
    }

    // Detach the layout object before freeing the object.
    ak_detach_layout(pLayout);

    // Free the object last.
    free(pLayout);
}



bool ak_parse_window_layout_attributes(const char* attributesStr, ak_window_layout_attributes* pAttributesOut)
{
    if (attributesStr == NULL || pAttributesOut == NULL) {
        return false;
    }


    char typeStr[16];
    attributesStr = easyutil_next_token(attributesStr, typeStr, sizeof(typeStr));
    if (attributesStr == NULL) {
        return false;
    }

    ak_window_type type = ak_window_type_unknown;
    if (strcmp(typeStr, "application") == 0) {
        type = ak_window_type_application;
    } else if (strcmp(typeStr, "child") == 0) {
        type = ak_window_type_child;
    } else if (strcmp(typeStr, "dialog") == 0) {
        type = ak_window_type_dialog;
    } else if (strcmp(typeStr, "popup") == 0) {
        type = ak_window_type_popup;
    } else {
        return false;
    }


    char posXStr[8];
    attributesStr = easyutil_next_token(attributesStr, posXStr, sizeof(posXStr));
    if (attributesStr == NULL) {
        return false;
    }

    char posYStr[8];
    attributesStr = easyutil_next_token(attributesStr, posYStr, sizeof(posYStr));
    if (attributesStr == NULL) {
        return false;
    }

    char widthStr[8];
    attributesStr = easyutil_next_token(attributesStr, widthStr, sizeof(widthStr));
    if (attributesStr == NULL) {
        return false;
    }

    char heightStr[8];
    attributesStr = easyutil_next_token(attributesStr, heightStr, sizeof(heightStr));
    if (attributesStr == NULL) {
        return false;
    }

    char maximizedStr[8];
    attributesStr = easyutil_next_token(attributesStr, maximizedStr, sizeof(maximizedStr));
    if (attributesStr == NULL) {
        return false;
    }


    // The next two properties are optional (title and name).
    attributesStr = easyutil_next_token(attributesStr, pAttributesOut->title, sizeof(pAttributesOut->title));
    if (attributesStr == NULL) {
        strcpy_s(pAttributesOut->title, sizeof(pAttributesOut->title), "");
    }

    attributesStr = easyutil_next_token(attributesStr, pAttributesOut->name, sizeof(pAttributesOut->name));
    if (attributesStr == NULL) {
        strcpy_s(pAttributesOut->name, sizeof(pAttributesOut->name), "");
    }

    pAttributesOut->type   = type;
    pAttributesOut->posX   = atoi(posXStr);
    pAttributesOut->posY   = atoi(posYStr);
    pAttributesOut->width  = atoi(widthStr);
    pAttributesOut->height = atoi(heightStr);

    if (strcmp(maximizedStr, "false") == 0) {
        pAttributesOut->maximized = false;
    } else {
        pAttributesOut->maximized = true;
    }

    return true;
}


bool ak_parse_panel_layout_attributes(const char* attributesStr, ak_panel_layout_attributes* pAttributesOut)
{
    if (attributesStr == NULL || pAttributesOut == NULL) {
        return false;
    }

    char name[AK_MAX_PANEL_NAME_LENGTH];
    name[0] = '\0';

    ak_panel_split_axis splitAxis = ak_panel_split_axis_none;
    float splitPos = 0;


    // The first token is the split axis.
    char splitAxisOrNameStr[AK_MAX_PANEL_NAME_LENGTH];
    attributesStr = easyutil_next_token(attributesStr, splitAxisOrNameStr, sizeof(splitAxisOrNameStr));
    if (attributesStr != NULL)
    {
        if (strcmp(splitAxisOrNameStr, "hsplit") == 0) {
            splitAxis = ak_panel_split_axis_horizontal;
        } else if (strcmp(splitAxisOrNameStr, "vsplit") == 0) {
            splitAxis = ak_panel_split_axis_vertical;
        } else {
            // It's not "hsplit" or "vsplit", so assume it's the name.
            strcpy_s(name, sizeof(name), splitAxisOrNameStr);
        }


        // The second token is the position of the split.
        if (splitAxis != ak_panel_split_axis_none)
        {
            char splitPosStr[8];
            attributesStr = easyutil_next_token(attributesStr, splitPosStr, sizeof(splitPosStr));
            if (attributesStr == NULL) {
                return false;
            }

            splitPos = (float)atof(splitPosStr);


            // The third argument on a split panel will be the name which is optional.
            attributesStr = easyutil_next_token(attributesStr, splitAxisOrNameStr, sizeof(splitAxisOrNameStr));
            if (attributesStr != NULL) {
                strcpy_s(name, sizeof(name), splitAxisOrNameStr);
            }
        }
    }


    strcpy_s(pAttributesOut->name, sizeof(pAttributesOut->name), name);
    pAttributesOut->splitAxis = splitAxis;
    pAttributesOut->splitPos  = splitPos;

    return true;
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
