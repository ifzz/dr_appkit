// Public domain. See "unlicense" statement at the end of this file.

#ifndef ak_application_h
#define ak_application_h

#include <stdint.h>

typedef struct ak_application ak_application;


/// Creates a new application object.
///
/// @remarks
///     <pName> is used for determining where to read and write user configuration files, log files, etc.
///     <pName> cannot be longer than AK_MAX_APPLICATION_NAME_LENGTH which is defined in ak_build_config.h.
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
