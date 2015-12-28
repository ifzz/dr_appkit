// Public domain. See "unlicense" statement at the end of this file.

//
// QUICK NOTES
//
// General
// - There are several types of windows: application, child, popup and dialogs.
// - Application windows are the main windows with a title bar, minimize/maximize/close buttons, resizability, etc.
// - Child windows are borderless windows that are placed within the client area of a parent window. They would
//   most commonly be used for things like viewports or whatnot.
// - Dialog windows are used for dialogs like an about window, confirmation dialogs, etc.
// - Popup windows are used for things like menus, tooltips, list-box drop-downs, etc.
// - Every window is associated with a single panel GUI element. This panel is always resized such that it's the same
//   size as the window that owns it. The panel is always a top-level element and should never be re-parented.
//

#ifndef ak_window_h
#define ak_window_h

#include <stdint.h>
#include <stdbool.h>

// This flag is posted on on_hide events when a popup window is automatically hidden as a result of the user clicking
// outside of it's region.
#define AK_AUTO_HIDE_FROM_OUTSIDE_CLICK     (1 << 0)

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ak_window ak_window;
typedef struct ak_application ak_application;
typedef struct easygui_element easygui_element;
typedef struct easy2d_surface easy2d_surface;

typedef enum
{
    ak_cursor_type_none,
    ak_cursor_type_default,

    ak_cursor_type_arrow = ak_cursor_type_default,
    ak_cursor_type_ibeam

} ak_cursor_type;

typedef enum
{
    ak_window_type_unknown,         // <-- ak_create_window() will fail if this is used.
    ak_window_type_application,
    ak_window_type_child,
    ak_window_type_dialog,
    ak_window_type_popup

} ak_window_type;

typedef bool (* ak_window_on_hide_proc)        (ak_window* pWindow, unsigned int flags);
typedef bool (* ak_window_on_show_proc)        (ak_window* pWindow);
typedef void (* ak_window_on_activate_proc)    (ak_window* pWindow);
typedef void (* ak_window_on_deactivate_proc)  (ak_window* pWindow);
typedef void (* ak_window_on_mouse_enter_proc) (ak_window* pWindow);
typedef void (* ak_window_on_mouse_leave_proc) (ak_window* pWindow);
typedef void (* ak_window_on_mouse_button_proc)(ak_window* pWindow, int mouseButton, int relativeMousePosX, int relativeMousePosY);
typedef void (* ak_window_on_mouse_wheel_proc) (ak_window* pWindow, int delta, int relativeMousePosX, int relativeMousePosY);


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

/// Retrieves a pointer to the parent window, if any.
ak_window* ak_get_parent_window(ak_window* pWindow);

/// Retrieves the size of the extra data associated with the given window.
size_t ak_get_window_extra_data_size(ak_window* pWindow);

/// Retrieves a pointer to the extra data associated with the given window.
void* ak_get_window_extra_data(ak_window* pWindow);

/// Retrieves a pointer to the top-level GUI element associated with the given window.
easygui_element* ak_get_window_panel(ak_window* pWindow);

/// Retrieves a pointer to the easy_draw surface the window will be drawing to.
easy2d_surface* ak_get_window_surface(ak_window* pWindow);


/// Sets the name of the window.
///
/// @remarks
///     The name of the window acts as an identifier which can be used to retrieve a pointer to the window.
bool ak_set_window_name(ak_window* pWindow, const char* pName);

/// Retrieves the name of the given window.
const char* ak_get_window_name(ak_window* pWindow);


/// Sets the title of the given window.
void ak_set_window_title(ak_window* pWindow, const char* pTitle);

/// Retrieves the title of the given window.
void ak_get_window_title(ak_window* pWindow, char* pTitleOut, size_t titleOutSize);


/// Sets the size of the given window.
///
/// @remarks
///     On Windows, this sets the size of the client area. This is done for consistency with GTK.
void ak_set_window_size(ak_window* pWindow, unsigned int width, unsigned int height);

/// Retrieves the size of the given window.
///
/// @remarks
///     On Windows, this retrieves the size of the client area of the window.
void ak_get_window_size(ak_window* pWindow, unsigned int* pWidthOut, unsigned int* pHeightOut);


/// Sets the position of the given window.
void ak_set_window_position(ak_window* pWindow, int posX, int posY);

/// Retrieves the position of the given window.
void ak_get_window_position(ak_window* pWindow, int* pPosXOut, int* pPosYOut);


/// Shows the given window.
void ak_show_window(ak_window* pWindow);

/// Shows the window maximized.
void ak_show_window_maximized(ak_window* pWindow);

/// Shows a window in a non-maximized state with an initial size.
void show_window_sized(ak_window* pWindow, unsigned int width, unsigned int height);

/// Hides the given window.
///
/// @remarks
///     The value of <flags> will be passed on the on_hide event handler.
///     @par
///     When a popup window is automatically hidden as a result of the user clicking outside of it's region, this will
///     be set to AK_AUTO_HIDE_FROM_OUTSIDE_CLICK.
void ak_hide_window(ak_window* pWindow, unsigned int flags);


/// Determines if the window is a descendant of another window.
bool ak_is_window_descendant(ak_window* pDescendant, ak_window* pAncestor);

/// Determines if the window is an ancestor of another window.
bool ak_is_window_ancestor(ak_window* pAncestor, ak_window* pDescendant);


/// Retrieves a pointer to the window that is associated with the given panel.
///
/// @remarks
///     It is assumed the given panel is a top-level element and is associated with a window.
///     @par
///     This will return null if the panel is not a top-level element.
ak_window* ak_get_panel_window(easygui_element* pPanel);


/// Sets the cursor to use with the given window.
void ak_set_window_cursor(ak_window* pWindow, ak_cursor_type cursor);

/// Determines whether or not the cursor is over the given window.
bool ak_is_cursor_over_window(ak_window* pWindow);


/// Retrieves the DPI of the given window.
///
/// @remarks
///     If multi-monitor DPI awareness is supported, it will use that, otherwise it will use
///     the system-wide DPI setting.
void ak_get_window_dpi(ak_window* pWindow, int* pDPIXOut, int* pDPIYOut);

/// Retrieves the DPI of the given window as scale.
void ak_get_window_dpi_scale(ak_window* pWindow, float* pDPIScaleXOut, float* pDPIScaleYOut);


/// Sets the function to call when the on_hide event is received.
void ak_window_set_on_hide(ak_window* pWindow, ak_window_on_hide_proc proc);

/// Sets the function to call when the on_show event is received.
void ak_window_set_on_show(ak_window* pWindow, ak_window_on_show_proc proc);


/// Calls the on_hide event handler for the given window.
bool ak_window_on_hide(ak_window* pWindow, unsigned int flags);

/// Calls the on_show event handler for the given window.
bool ak_window_on_show(ak_window* pWindow);

/// Calls the on_activate event handler for the given window.
void ak_window_on_activate(ak_window* pWindow);

/// Calls the on_deactivate event handler for the given window.
void ak_window_on_deactivate(ak_window* pWindow);

/// Calls the on_mouse_enter event handler for the given window.
void ak_window_on_mouse_enter(ak_window* pWindow);

/// Calls the on_mouse_leave event handler for the given window.
void ak_window_on_mouse_leave(ak_window* pWindow);

/// Calls the on_mouse_button_down event handler for the given window.
void ak_window_on_mouse_button_down(ak_window* pWindow, int mouseButton, int relativeMousePosX, int relativeMousePosY);

/// Calls the on_mouse_button_up event handler for the given window.
void ak_window_on_mouse_button_up(ak_window* pWindow, int mouseButton, int relativeMousePosX, int relativeMousePosY);

/// Calls the on_mouse_button_dblclick event handler for the given window.
void ak_window_on_mouse_button_dblclick(ak_window* pWindow, int mouseButton, int relativeMousePosX, int relativeMousePosY);

/// Calls the on_mouse_button_wheel event handler for the given window.
void ak_window_on_mouse_wheel(ak_window* pWindow, int delta, int relativeMousePosX, int relativeMousePosY);


#ifdef __cplusplus
}
#endif

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
