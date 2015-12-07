// Public domain. See "unlicense" statement at the end of this file.

#include "../include/easy_appkit/ak_window.h"
#include "../include/easy_appkit/ak_application.h"
#include "../include/easy_appkit/ak_build_config.h"
#include <easy_gui/easy_gui.h>

#ifdef AK_USE_WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

struct ak_window
{
    /// A pointer to the application that owns this window.
    ak_application* pApplication;

    /// The Win32 window handle.
    HWND hWnd;

    /// The top-level GUI element to draw to this window.
    easygui_element* pGUIElement;
};



#endif

#ifdef AK_USE_GTK
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