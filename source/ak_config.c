// Public domain. See "unlicense" statement at the end of this file.

#include "ak_config.h"
#include <stdio.h>
#include <assert.h>

/// Determines whether or not the given string is a layout item tag.
PRIVATE bool ak_is_layout_item_tag(const char* tag)
{
    return
        strcmp(tag, AK_LAYOUT_TYPE_LAYOUT) == 0 ||
        strcmp(tag, AK_LAYOUT_TYPE_WINDOW) == 0 ||
        strcmp(tag, AK_LAYOUT_TYPE_PANEL)  == 0 ||
        strcmp(tag, AK_LAYOUT_TYPE_TOOL)   == 0;
}

typedef struct
{
    /// A pointer to the file to read from. Null if the file is not being used for parsing.
    easyvfs_file* pFile;

    /// The string to read the script from. Not used if the script is being read from the file.
    const char* configString;

    /// The size of the config string. Not used if script is being read from a file instead of the string.
    size_t configStringLength;


    /// A pointer to the config object to load the data into.
    ak_config* pConfig;

    /// A pointer to the layout object that's currently getting parsed.
    ak_layout* pCurrentLayout;

    /// Tracks whether or not an error has been found.
    bool foundError;

    /// The function to call when an error occurs.
    ak_on_config_error_proc onError;

    /// A pointer to the user data to pass to the error callback.
    void* pOnErrorUserData;

} ak_config_parse_context;

static size_t ak_config_on_read_from_file_proc(void* pUserData, void* pDataOut, size_t bytesToRead)
{
    ak_config_parse_context* pContext = pUserData;
    assert(pContext != NULL);

    unsigned int bytesRead;
    if (easyvfs_read(pContext->pFile, pDataOut, (unsigned int)bytesToRead, &bytesRead)) {
        return bytesRead;
    }

    return 0;
}

static size_t ak_config_on_read_from_string_proc(void* pUserData, void* pDataOut, size_t bytesToRead)
{
    ak_config_parse_context* pContext = pUserData;
    assert(pContext != NULL);

    if (bytesToRead > pContext->configStringLength) {
        bytesToRead = pContext->configStringLength;
    }

    if (bytesToRead > 0)
    {
        memcpy(pDataOut, pContext->configString, bytesToRead);

        pContext->configString       += bytesToRead;
        pContext->configStringLength -= bytesToRead;
    }

    return bytesToRead;
}

static void ak_config_on_pair(void* pUserData, const char* key, const char* value)
{
    printf("PAIR: [%s] [%s]\n", key, value);

    ak_config_parse_context* pContext = pUserData;
    assert(pContext != NULL);

    // If we've previously found an error, just skip everything.
    if (pContext->foundError) {
        return;
    }


    if (strcmp(key, "InitialLayout") == 0)
    {
        strcpy_s(pContext->pConfig->currentLayoutName, sizeof(pContext->pConfig->currentLayoutName), value);
        return;
    }

    if (ak_is_layout_item_tag(key))
    {
        // We're starting a new layout item.
        ak_layout* pNewLayout = ak_create_layout(key, value, pContext->pCurrentLayout);
        if (pNewLayout == NULL)
        {
            if (pContext->onError) {
                pContext->onError(pContext->pOnErrorUserData, "Failed to allocate memory for layout object.");
            }

            pContext->foundError = true;
            return;
        }

        pContext->pCurrentLayout = pNewLayout;

        return;
    }

    if (key[0] == '/')
    {
        // We're ending a layout item. We want to do a validation step here. If it's a mismatched tag we need to fail.
        if (ak_is_layout_item_tag(key + 1))
        {
            if (pContext->pCurrentLayout != NULL)
            {
                // Validation.
                if (strcmp(pContext->pCurrentLayout->type, key + 1) != 0)
                {
                    // Tag mismatch.
                    if (pContext->onError)
                    {
                        char msg[4096];
                        snprintf(msg, sizeof(msg), "Tag mismatch. Expecting /%s but got %s", pContext->pCurrentLayout->type, key);

                        pContext->onError(pContext->pOnErrorUserData, msg);
                    }

                    pContext->foundError = true;
                    return;
                }


                // If we get here it should be valid. We just set the current layout item to the parent layout.
                pContext->pCurrentLayout = pContext->pCurrentLayout->pParent;
            }
        }

        return;
    }
}

static void ak_config_on_error(void* pUserData, const char* message, unsigned int line)
{
    ak_config_parse_context* pContext = pUserData;
    assert(pContext != NULL);

    if (pContext->onError)
    {
        char msg[4096];
        snprintf(msg, sizeof(msg), "(Line %d) %s", line, message);

        pContext->onError(pContext->pOnErrorUserData, msg);
    }
}


bool ak_parse_config_from_file(ak_config* pConfig, easyvfs_file* pFile, ak_on_config_error_proc onError, void* pOnErrorUserData)
{
    if (pConfig == NULL || pFile == NULL) {
        return false;
    }

    if (!ak_init_config(pConfig)) {
        return false;
    }


    ak_config_parse_context context;
    memset(&context, 0, sizeof(context));
    context.pFile            = pFile;
    context.pConfig          = pConfig;
    context.pCurrentLayout   = pConfig->pRootLayout;
    context.onError          = onError;
    context.pOnErrorUserData = pOnErrorUserData;
    easyutil_parse_key_value_pairs(ak_config_on_read_from_file_proc, ak_config_on_pair, NULL, &context);

    if (context.foundError) {
        ak_uninit_config(pConfig);
        return false;
    }

    return true;
}

bool ak_parse_config_from_string(ak_config* pConfig, const char* configString, ak_on_config_error_proc onError, void* pOnErrorUserData)
{
    if (pConfig == NULL || configString == NULL) {
        return false;
    }

    if (!ak_init_config(pConfig)) {
        return false;
    }
    

    ak_config_parse_context context;
    memset(&context, 0, sizeof(context));
    context.configString       = configString;
    context.configStringLength = (unsigned int)strlen(configString);
    context.pConfig            = pConfig;
    context.pCurrentLayout     = pConfig->pRootLayout;
    context.onError            = onError;
    context.pOnErrorUserData   = pOnErrorUserData;
    easyutil_parse_key_value_pairs(ak_config_on_read_from_string_proc, ak_config_on_pair, NULL, &context);

    if (context.foundError) {
        ak_uninit_config(pConfig);
        return false;
    }

    return true;
}


bool ak_init_config(ak_config* pConfig)
{
    if (pConfig == NULL) {
        return false;
    }

    memset(pConfig, 0, sizeof(*pConfig));

    pConfig->pRootLayout = ak_create_layout(NULL, NULL, NULL);
    if (pConfig->pRootLayout == NULL) {
        return false;
    }

    return true;
}

void ak_uninit_config(ak_config* pConfig)
{
    if (pConfig == NULL) {
        return;
    }

    // The layout object needs to be deleted recursively.
    ak_delete_layout(pConfig->pRootLayout);

    // Clear the config to 0.
    memset(pConfig, 0, sizeof(*pConfig));
}


ak_layout* ak_config_find_root_layout_by_name(ak_config* pConfig, const char* layoutName)
{
    if (pConfig != NULL || layoutName == NULL) {
        return NULL;
    }

    if (pConfig->pRootLayout == NULL) {
        return NULL;
    }

    // Only searching root level layouts. The name of a root level layout object is defined by the attribute.
    for (ak_layout* pLayout = pConfig->pRootLayout->pFirstChild; pLayout != NULL; pLayout = pLayout->pNextSibling)
    {
        if (strcmp(pLayout->type, "Layout") == 0 && strcmp(pLayout->attributes, layoutName) == 0) {
            return pLayout;
        }
    }

    return NULL;
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