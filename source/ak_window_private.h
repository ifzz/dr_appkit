// Public domain. See "unlicense" statement at the end of this file.

#ifndef ak_window_private_h
#define ak_window_private_h

#ifdef __cplusplus
extern "C" {
#endif

typedef struct drgui_context drgui_context;
typedef struct ak_window ak_window;


/// Initializes the underlying platform layer.
void ak_init_platform();

/// Uninitializes the underlying platform layer.
void ak_uninit_platform();


#ifdef AK_USE_WIN32
/// Win32 Only. Posts a quit message to the main loop.
void ak_win32_post_quit_message(int resultCode);
#endif  //!AK_USE_WIN32

#ifdef AK_USE_GTK
/// GTK Only. Exists the main loop.
void ak_gtk_post_quit_message(int resultCode);
#endif  //!AK_USE_GTK


/// Connects the given GUI context to the undering windowing system by registering the appropriate global outboud event handlers.
void ak_connect_gui_to_window_system(drgui_context* pGUI);


/// Retrieves a pointer to the first child window.
ak_window* ak_get_first_child_window(ak_window* pWindow);

/// Retrieves a pointer to the last child window.
ak_window* ak_get_last_child_window(ak_window* pWindow);


/// Sets the next window in the main linked list that's controlled by the application.
void ak_set_next_sibling_window(ak_window* pWindow, ak_window* pNextWindow);

/// Retrieves the next window in the main linked list that's controlled by the application.
ak_window* ak_get_next_sibling_window(ak_window* pWindow);

/// Sets the previous window in the main linked list that's controlled by the application.
void ak_set_prev_sibling_window(ak_window* pWindow, ak_window* pPrevWindow);

/// Retrieves the previous window in the main linked list that's controlled by the application.
ak_window* ak_get_prev_sibling_window(ak_window* pWindow);



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
