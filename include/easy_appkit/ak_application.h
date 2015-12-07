// Public domain. See "unlicense" statement at the end of this file.

#ifndef ak_application_h
#define ak_application_h

#include <stdint.h>
#include <stdbool.h>

typedef struct ak_application ak_application;
typedef struct ak_window ak_window;
typedef struct ak_theme ak_theme;
typedef struct easygui_context easygui_context;
typedef struct easy2d_context easy2d_context;

typedef void (* ak_log_proc)           (ak_application* pApplication, const char* message);
typedef void (* ak_layout_config_proc) (ak_application* pApplication, const char* layoutName);

/// Creates a new application object.
///
/// @remarks
///     <pName> is used for determining where to read and write user configuration files, log files, etc.
///     <pName> cannot be longer than AK_MAX_APPLICATION_NAME_LENGTH which is defined in ak_build_config.h.
///     <pName> can be a path-style string such as "MyApplication/MySubApplication". In this case, the log
///     theme, config, etc. files will be opened based on this path.
///     @par
///     User-defined data can be associated with an application by specifying the number of extra bytes
///     that are required. Use ak_application_get_extra_data() to retrieve a pointer to a buffer that can
///     be used as the storage for the user-defined data. The user-defined data is initially undefined.
///     @par
///     <pExtraData> is a pointer to the initial user-defined data. This can be null, in which case the
///     extra data is left initially undefined. This will create a copy of the extra data.
ak_application* ak_create_application(const char* pName, size_t extraDataSize, const void* pExtraData);

/// Deletes an application object that was created with ak_create_application().
void ak_delete_application(ak_application* pApplication);


/// Begins running the application.
///
/// @return The result code.
///
/// @remarks
///     This is where the application will load configs, create windows, create GUI elements and enter into
///     the main loop.
///     @par
///     A return value of 0 indicates the application was terminated naturally.
int ak_run_application(ak_application* pApplication);


/// Retrieves the name of the application.
///
/// @remarks
///     If null was passed to ak_create_application(), the returned string will be equal to the value of
///     AK_DEFAULT_APPLICATION_NAME which is defined in ak_build_config.h.
const char* ak_get_application_name(ak_application* pApplication);

/// Retrieves the size of the extra data that is associated with the application.
size_t ak_get_application_extra_data_size(ak_application* pApplication);

/// Retrieves a pointer to the buffer containing the user-defined data.
void* ak_get_application_extra_data(ak_application* pApplication);

/// Retrieves a pointer to the GUI context associated with the application.
easygui_context* ak_get_application_gui(ak_application* pApplication);

/// Retrieves a pointer to the GUI drawing context.
easy2d_context* ak_get_application_drawing_context(ak_application* pApplication);

/// Retrieves a pointer to the object representing the application's theme.
ak_theme* ak_get_application_theme(ak_application* pApplication);


/// Posts a log message.
void ak_log(ak_application* pApplication, const char* message);

/// Posts a formatted log message.
void ak_logf(ak_application* pApplication, const char* format, ...);

/// Posts a warning to the log.
void ak_warning(ak_application* pApplication, const char* message);

/// Posts a formatted warning log message.
void ak_warningf(ak_application* pApplication, const char* format, ...);

/// Posts an error log message.
void ak_error(ak_application* pApplication, const char* message);

/// Posts a formatted error log message.
void ak_errorf(ak_application* pApplication, const char* format, ...);

/// Sets the function to call when a log message is posted.
void ak_set_log_callback(ak_application* pApplication, ak_log_proc proc);

/// Retrieves a pointer to the log callback function, if any.
ak_log_proc ak_get_log_callback(ak_application* pApplication);

/// Retrieves the path of the log file.
///
/// @remarks
///     If a log file is unable to be opened, this will append a numeric value to the file path in case the
///     file already exists.
bool ak_get_log_file_folder_path(ak_application* pApplication, char* pathOut, size_t pathOutSize);


/// Sets the function to call when the default config of a layout is required.
void ak_set_on_default_config(ak_application* pApplication, ak_layout_config_proc proc);

/// Retrieves the function to call when the default config of a layout is required.
ak_layout_config_proc ak_get_on_default_config(ak_application* pApplication);


#endif

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
