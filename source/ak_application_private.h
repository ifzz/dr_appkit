// Public domain. See "unlicense" statement at the end of this file.

#ifndef ak_application_private_h
#define ak_application_private_h

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ak_application ak_application;
typedef struct ak_window ak_window;


/// Retrieves the first window in the main linked list.
ak_window* ak_get_application_first_window(ak_application* pApplication);

/// Retrieves the next window of every created window.
ak_window* ak_get_application_next_window(ak_application* pApplication, ak_window* pWindow);


/// Called by the main application loop when a window want's to close.
void ak_application_on_window_wants_to_close(ak_window* pWindow);

/// Called when a window is being deleted.
///
/// @remarks
///     This will delete every tool and panel tied to the window.
void ak_application_on_delete_window(ak_window* pWindow);

/// Called when a window is about to be hidden.
///
/// @remarks
///     The window can be prevented from being hidden by returning 0.
bool ak_application_on_hide_window(ak_window* pWindow, unsigned int flags);

/// Called when a window is about to be shown.
///
/// @remarks
///     The window can be prevented from being shown by returning 0.
bool ak_application_on_show_window(ak_window* pWindow);

/// Called when a window is activated.
void ak_application_on_activate_window(ak_window* pWindow);

/// Called when a window is deactivated.
void ak_application_on_deactivate_window(ak_window* pWindow);

/// Called when the mouse enters the given window.
void ak_application_on_mouse_enter(ak_window* pWindow);

/// Called when the mouse leaves the given window.
void ak_application_on_mouse_leave(ak_window* pWindow);

/// Called when a mouse button is pressed.
void ak_application_on_mouse_button_down(ak_window* pWindow, int mouseButton, int relativeMousePosX, int relativeMousePosY);

/// Called when a mouse button is released.
void ak_application_on_mouse_button_up(ak_window* pWindow, int mouseButton, int relativeMousePosX, int relativeMousePosY);

/// Called when a mouse button is double clicked.
void ak_application_on_mouse_button_dblclick(ak_window* pWindow, int mouseButton, int relativeMousePosX, int relativeMousePosY);

/// Called when the mouse wheel is turned.
void ak_application_on_mouse_wheel(ak_window* pWindow, int delta, int relativeMousePosX, int relativeMousePosY);

/// Called the on_key_down event handler for the given window.
void ak_application_on_key_down(ak_window* pWindow, easygui_key key, int stateFlags);

/// Called the on_key_up event handlers for the given window.
void ak_application_on_key_up(ak_window* pWindow, easygui_key key, int stateFlags);

/// Called the on_printable_key_down for the given window.
void ak_application_on_printable_key_down(ak_window* pWindow, unsigned int character, int stateFlags);


/// Tracks the given window.
///
/// @remarks
///     This is called by the create_*_window() family of functions.
void ak_application_track_top_level_window(ak_window* pWindow);

/// Untracks the given window.
///
/// @remarks
///     This is called by delete_window().
void ak_application_untrack_top_level_window(ak_window* pWindow);


/// Hides every popup window that is not an ancestor of the given window.
void ak_application_hide_non_ancestor_popups(ak_window* pWindow);


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