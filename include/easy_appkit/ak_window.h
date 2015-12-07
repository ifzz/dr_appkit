// Public domain. See "unlicense" statement at the end of this file.

#ifndef ak_window_h
#define ak_window_h

#include <stdint.h>

typedef struct ak_window ak_window;
typedef struct ak_application ak_application;
typedef struct easygui_element easygui_element;

typedef enum
{
    ak_window_type_primary,
    ak_window_type_child,
    ak_window_type_popup

} ak_window_type;


/// Creates a window of the given type.
///
/// @remarks
///     This does not show the window. Use ak_show_window() to show the window. It is recommended to set the appropriate
///     properties such as size and position before showing the window in order to present the user with a clean window
///     initialization.
ak_window* ak_create_window(ak_application* pApplication, ak_window_type type, ak_window* pParent, size_t extraDataSize, const void* pExtraData);

/// Deletes the given window.
void ak_delete_window(ak_window* pWindow);


/// Retrieves a pointer to the application that owns the given window.
ak_application* ak_get_window_application(ak_window* pWindow);

/// Retrieves the type of the given window.
ak_window_type ak_get_window_type(ak_window* pWindow);

/// Retrieves a pointer to the top-level GUI element associated with the given window.
easygui_element* ak_get_window_gui_element(ak_window* pWindow);


/// Sets the title of the given window.
void ak_set_window_title(ak_window* pWindow, const char* pTitle);

/// Retrieves the title of the given window.
const char* ak_get_window_title(ak_window* pWindow);


/// Sets the size of the given window.
void ak_set_window_size(ak_window* pWindow, unsigned int width, unsigned int height);

/// Retrieves the size of the given window.
void ak_get_window_size(ak_window* pWindow, unsigned int* pWidthOut, unsigned int* pHeightOut);


/// Sets the position of the given window.
void ak_set_window_position(ak_window* pWindow, int posX, int posY);

/// Retrieves the position of the given window.
void ak_get_window_position(ak_window* pWindow, int* pPosXOut, int* pPosYOut);


/// Shows the given window.
void ak_show_window(ak_window* pWindow);

/// Hides the given window.
void ak_hide_window(ak_window* pWindow);


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
