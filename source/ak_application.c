// Public domain. See "unlicense" statement at the end of this file.

#include "../include/easy_appkit/ak_application.h"
#include "../include/easy_appkit/ak_build_config.h"
#include "../include/easy_appkit/ak_theme.h"
#include "ak_config.h"
#include <easy_util/easy_util.h>
#include <easy_gui/easy_gui.h>
#include <easy_fs/easy_vfs.h>
#include <easy_path/easy_path.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

struct ak_application
{
    /// The name of the application.
    char name[AK_MAX_APPLICATION_NAME_LENGTH];

    /// The virtual file system context. This is mainly used for opening log, theme and configuration files.
    easyvfs_context* pVFS;

    /// The log file.
    easyvfs_file* pLogFile;

    /// The log callback.
    ak_log_proc onLog;
    

    /// A pointer to the GUI context.
    easygui_context* pGUI;

    /// The drawing context.
    easy2d_context* pDrawingContext;

    /// The application's theme.
    ak_theme theme;


    /// A pointer to the function to call when the default layout config is required. This will be called when
    /// layout config file could not be found.
    ak_layout_config_proc onGetDefaultConfig;


    /// The size of the extra data, in bytes.
    size_t extraDataSize;

    /// A pointer to the extra data associated with the application.
    char pExtraData[1];
};


/// Enter's into the main application loop.
PRIVATE int ak_main_loop(ak_application* pApplication);

/// Loads and apply's the main config.
PRIVATE bool ak_load_and_apply_config(ak_application* pApplication);

/// Applies the given config to the given application object.
PRIVATE bool ak_apply_config(ak_application* pApplication, ak_config* pConfig);


ak_application* ak_create_application(const char* pName, size_t extraDataSize, const void* pExtraData)
{
    ak_application* pApplication = malloc(sizeof(ak_application) + extraDataSize - sizeof(pApplication->pExtraData));
    if (pApplication != NULL)
    {
        // Name.
        if (pName != NULL) {
            strcpy_s(pApplication->name, sizeof(pApplication->name), pName);
        } else {
            strcpy_s(pApplication->name, sizeof(pApplication->name), "easy_appkit");
        }


        // File system.
        pApplication->pVFS = easyvfs_create_context();
        if (pApplication->pVFS == NULL) {
            free(pApplication);
            return NULL;
        }


        // Logging
        pApplication->onLog = NULL;
        
        char logDirPath[EASYVFS_MAX_PATH];
        if (ak_get_log_file_folder_path(pApplication, logDirPath, sizeof(logDirPath)))
        {
            const unsigned int maxAttempts = 10;

            char path[EASYVFS_MAX_PATH];
            for (unsigned int iAttempt = 0; iAttempt < maxAttempts; ++iAttempt)
            {
                char istr[16];
                snprintf(istr, 16, "%d", iAttempt);

                strcpy_s(path, sizeof(path), logDirPath);
                easypath_append(path, sizeof(path), easypath_file_name(pApplication->name));
                strcat_s(path, sizeof(path), istr);
                strcat_s(path, sizeof(path), ".log");

                pApplication->pLogFile = easyvfs_open(pApplication->pVFS, path, EASYVFS_WRITE, 0);
                if (pApplication->pLogFile != NULL)
                {
                    // We were able to open the log file, so break here.
                    break;
                }
            }
        }
        else
        {
            pApplication->pLogFile = NULL;
        }



        // GUI.
#ifdef AK_USE_WIN32
        pApplication->pDrawingContext = easy2d_create_context_gdi();
#endif
#ifdef AK_USE_GTK
        pApplication->pDrawingContext = easy2d_create_context_cairo();
#endif
        if (pApplication->pDrawingContext == NULL) {
            easygui_delete_context(pApplication->pGUI);
            free(pApplication);
            return NULL;
        }

        pApplication->pGUI = easygui_create_context_easy_draw();
        if (pApplication->pGUI == NULL) {
            free(pApplication);
            return NULL;
        }




        // Configs.
        pApplication->onGetDefaultConfig = NULL;



        // Extra data.
        pApplication->extraDataSize = extraDataSize;

        if (extraDataSize > 0 && pExtraData != NULL) {
            memcpy(pApplication->pExtraData, pExtraData, extraDataSize);
        }
    }

    return pApplication;
}

void ak_delete_application(ak_application* pApplication)
{
    if (pApplication == NULL) {
        return;
    }

    // GUI.
    easygui_delete_context(pApplication->pGUI);
    easy2d_delete_context(pApplication->pDrawingContext);

    // Logs.
    easyvfs_close(pApplication->pLogFile);

    // File system.
    easyvfs_delete_context(pApplication->pVFS);


    // Free the application object last.
    free(pApplication);
}


int ak_run_application(ak_application* pApplication)
{
    if (pApplication == NULL) {
        return -1;
    }

    // The first thing to do when running the application is to load the default config and apply it. If a config
    // file cannot be found, a default config will be requested. If the default config fails, the config will fail
    // and an error code will be returned.
    if (!ak_load_and_apply_config(pApplication)) {
        return -2;
    }

    // At this point the config should be loaded. Now we just enter the main loop.
    return ak_main_loop(pApplication);
}


const char* ak_get_application_name(ak_application* pApplication)
{
    if (pApplication == NULL) {
        return NULL;
    }

    return pApplication->name;
}

easyvfs_context* ak_get_application_vfs(ak_application* pApplication)
{
    if (pApplication == NULL) {
        return NULL;
    }

    return pApplication->pVFS;
}

size_t ak_get_application_extra_data_size(ak_application* pApplication)
{
    if (pApplication == NULL) {
        return 0;
    }

    return pApplication->extraDataSize;
}

void* ak_get_application_extra_data(ak_application* pApplication)
{
    if (pApplication == NULL) {
        return NULL;
    }

    return pApplication->pExtraData;
}

easygui_context* ak_get_application_gui(ak_application* pApplication)
{
    if (pApplication == NULL) {
        return NULL;
    }

    return pApplication->pGUI;
}

easy2d_context* ak_get_application_drawing_context(ak_application* pApplication)
{
    if (pApplication == NULL) {
        return NULL;
    }

    return pApplication->pDrawingContext;
}

ak_theme* ak_get_application_theme(ak_application* pApplication)
{
    if (pApplication == NULL) {
        return NULL;
    }

    return &pApplication->theme;
}


void ak_log(ak_application* pApplication, const char* message)
{
    if (pApplication == NULL) {
        return;
    }


    // Log file.
    if (pApplication->pLogFile != NULL) {
        char dateTime[64];
        easyutil_datetime_short(easyutil_now(), dateTime, sizeof(dateTime));

        easyvfs_write_string(pApplication->pLogFile, "[");
        easyvfs_write_string(pApplication->pLogFile, dateTime);
        easyvfs_write_string(pApplication->pLogFile, "]");
        easyvfs_write_line  (pApplication->pLogFile, message);
        easyvfs_flush(pApplication->pLogFile);
    }


    // Log callback.
    if (pApplication->onLog) {
        pApplication->onLog(pApplication, message);
    }
}

void ak_logf(ak_application* pApplication, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    {
        char msg[4096];
        vsnprintf(msg, sizeof(msg), format, args);
        
        ak_log(pApplication, msg);
    }
    va_end(args);
}

void ak_warning(ak_application* pApplication, const char* message)
{
    ak_logf(pApplication, "[WARNING] %s", message);
}

void ak_warningf(ak_application* pApplication, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    {
        char msg[4096];
        vsnprintf(msg, sizeof(msg), format, args);
        
        ak_warning(pApplication, msg);
    }
    va_end(args);
}

void ak_error(ak_application* pApplication, const char* message)
{
    ak_logf(pApplication, "[ERROR] %s", message);
}

void ak_errorf(ak_application* pApplication, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    {
        char msg[4096];
        vsnprintf(msg, sizeof(msg), format, args);
        
        ak_error(pApplication, msg);
    }
    va_end(args);
}

void ak_set_log_callback(ak_application* pApplication, ak_log_proc proc)
{
    if (pApplication == NULL) {
        return;
    }

    pApplication->onLog = proc;
}

ak_log_proc ak_get_log_callback(ak_application* pApplication)
{
    if (pApplication == NULL) {
        return NULL;
    }

    return pApplication->onLog;
}

bool ak_get_log_file_folder_path(ak_application* pApplication, char* pathOut, size_t pathOutSize)
{
    if (pApplication == NULL) {
        return false;
    }

    if (pathOut == NULL || pathOutSize == 0) {
        return false;
    }


    if (!easyutil_get_log_folder_path(pathOut, pathOutSize)) {
        return false;
    }

    return easypath_append(pathOut, pathOutSize, ak_get_application_name(pApplication));
}

bool ak_get_config_file_folder_path(ak_application* pApplication, char* pathOut, size_t pathOutSize)
{
    if (pApplication == NULL) {
        return false;
    }

    if (pathOut == NULL || pathOutSize == 0) {
        return false;
    }

    if (!easyutil_get_config_folder_path(pathOut, pathOutSize)) {
        return false;
    }

    return easypath_append(pathOut, pathOutSize, ak_get_application_name(pApplication));
}

bool ak_get_config_file_path(ak_application* pApplication, char* pathOut, size_t pathOutSize)
{
    if (ak_get_config_file_folder_path(pApplication, pathOut, pathOutSize))
    {
        return easypath_append(pathOut, pathOutSize, easypath_file_name(ak_get_application_name(pApplication))) && strcat_s(pathOut, pathOutSize, ".cfg") == 0;
    }

    return false;
}


void ak_set_on_default_config(ak_application* pApplication, ak_layout_config_proc proc)
{
    if (pApplication == NULL) {
        return;
    }

    pApplication->onGetDefaultConfig = proc;
}

ak_layout_config_proc ak_get_on_default_config(ak_application* pApplication)
{
    if (pApplication == NULL) {
        return NULL;
    }

    return pApplication->onGetDefaultConfig;
}




///////////////////////////////////////////////////////////////////////////////
//
// Private
//
///////////////////////////////////////////////////////////////////////////////

PRIVATE int ak_main_loop(ak_application* pApplication)
{
#ifdef AK_USE_WIN32
    (void)pApplication;

    MSG msg;
    BOOL bRet;
    while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0)
    {
        if (bRet == -1)
        {
            // Unknown error.
            return -43;
        }

        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    return 0;
#endif

#ifdef AK_USE_GTK
    (void)pApplication;
    return -1;
#endif
}

PRIVATE bool ak_load_and_apply_config(ak_application* pApplication)
{
    assert(pApplication != NULL);

    // We first need to try and open the config file. If we can't find it we need to try and load the default config. If both
    // fail, we need to return false.
    char configPath[EASYVFS_MAX_PATH];
    if (ak_get_config_file_path(pApplication, configPath, sizeof(configPath)))
    {
        easyvfs_file* pConfigFile = easyvfs_open(ak_get_application_vfs(pApplication), configPath, EASYVFS_READ, 0);
        if (pConfigFile != NULL)
        {
            ak_config config;
            if (ak_parse_config_from_file(&config, pConfigFile))
            {
                if (ak_apply_config(pApplication, &config))
                {
                    return true;
                }
            }
        }
    }


    // If we get here we want to try loading the default config.
    if (pApplication->onGetDefaultConfig)
    {
        ak_config config;
        if (ak_parse_config_from_string(&config, pApplication->onGetDefaultConfig(pApplication)))
        {
            return ak_apply_config(pApplication, &config);
        }
    }


    // If we get here, both configs have failed to load.
    return false;
}

PRIVATE bool ak_apply_config(ak_application* pApplication, ak_config* pConfig)
{
    assert(pApplication != NULL);
    assert(pConfig      != NULL);

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
