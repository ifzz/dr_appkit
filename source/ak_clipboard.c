// Public domain. See "unlicense" statement at the end of this file.

#include "../include/dr_appkit/ak_clipboard.h"
#include "../include/dr_appkit/ak_build_config.h"

#ifdef AK_USE_WIN32
#include <windows.h>

bool ak_clipboard_set_text(const char* text, size_t textLength)
{
    if (textLength == (size_t)-1) {
        textLength = strlen(text);
    }

    // We must ensure line endlings are normalized to \r\n. If we don't do this pasting won't work
    // correctly in things like Notepad.
    //
    // We allocate a buffer x2 the size of the original string to guarantee there will be enough room
    // for the extra \r character's we'll be adding.
    HGLOBAL hTextMem = GlobalAlloc(GMEM_SHARE | GMEM_MOVEABLE, ((textLength*2 + 1) * sizeof(char)));
    if (hTextMem == NULL) {
        return false;
    }

    char* textRN = GlobalLock(hTextMem);
    if (textRN == NULL) {
        GlobalFree(hTextMem);
        return false;
    }

    if (!OpenClipboard(NULL)) {
        GlobalFree(hTextMem);
        return false;
    }

    if (!EmptyClipboard()) {
        GlobalFree(hTextMem);
        CloseClipboard();
        return false;
    }

    while (*text != '\0' && textLength > 0)
    {
        if (text[0] == '\r' && textLength > 1 && text[1] == '\n')
        {
            *textRN++ = '\r';
            *textRN++ = '\n';

            text += 2;
            textLength -= 2;
        }
        else
        {
            if (*text == '\n') {
                *textRN++ = '\r';
            }

            *textRN++ = *text++;

            textLength -= 1;
        }
    }

    *textRN = '\0';


    GlobalUnlock(hTextMem);


    if (SetClipboardData(CF_TEXT, hTextMem) == NULL) {
        GlobalFree(hTextMem);
        CloseClipboard();
        return false;
    }

    CloseClipboard();

    return true;
}

char* ak_clipboard_get_text()
{
    if (!IsClipboardFormatAvailable(CF_TEXT)) {
        return 0;
    }

    if (!OpenClipboard(NULL)) {
        return 0;
    }

    HGLOBAL hTextMem = GetClipboardData(CF_TEXT);
    if (hTextMem == NULL) {
        CloseClipboard();
        return 0;
    }

    char* textRN = GlobalLock(hTextMem);
    if (textRN == NULL) {
        CloseClipboard();
        return false;
    }

    size_t textRNLength = strlen(textRN);
    char* result = malloc(textRNLength + 1);
    strcpy_s(result, textRNLength + 1, textRN);

    GlobalUnlock(hTextMem);
    CloseClipboard();

    return result;
}

void ak_clipboard_free_text(char* text)
{
    free(text);
}
#endif


#ifdef AK_USE_GTK
#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include <string.h>

bool ak_clipboard_set_text(const char* text, size_t textLength)
{
    if (textLength == (size_t)-1) {
        textLength = strlen(text);
    }

    gtk_clipboard_set_text(gtk_clipboard_get_default(gdk_display_get_default()), text, textLength);
    return true;
}

char* ak_clipboard_get_text()
{
    return gtk_clipboard_wait_for_text(gtk_clipboard_get_default(gdk_display_get_default()));
}

void ak_clipboard_free_text(char* text)
{
    g_free(text);
}
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
