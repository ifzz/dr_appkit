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


ak_layout* ak_create_layout(const char* name, const char* attributes, ak_layout* pParent)
{
    ak_layout* pLayout = malloc(sizeof(*pLayout));
    if (pLayout != NULL)
    {
        if (name != NULL) {
            strcpy_s(pLayout->name, sizeof(pLayout->name), name);
        } else {
            pLayout->name[0] = '\0';
        }

        if (attributes != NULL) {
            strcpy_s(pLayout->attributes, sizeof(pLayout->attributes), name);
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