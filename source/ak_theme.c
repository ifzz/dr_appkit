// Public domain. See "unlicense" statement at the end of this file.

#include "../include/easy_appkit/ak_theme.h"
#include "../include/easy_appkit/ak_application.h"
#include "../include/easy_appkit/ak_build_config.h"
#include <assert.h>

ak_text_theme ak_init_text_theme(ak_application* pApplication, const char* family, unsigned int size, easygui_font_weight weight, easygui_font_slant slant, easygui_color textColor, easygui_color backgroundColor)
{
    ak_text_theme result;
    result.textColor = textColor;
    result.backgroundColor = backgroundColor;

    result.pFont = easygui_create_font(ak_get_application_gui(pApplication), family, size, weight, slant, 0);
    if (result.pFont == NULL) {
        ak_logf(pApplication, "[ERROR] Failed to load font \"%s\"", family);
    }

    if (!easygui_get_font_metrics(result.pFont, &result.fontMetrics)) {
        ak_logf(pApplication, "[ERROR] Failed to retrieve font metrics for \"%s\"", family);
    }

    return result;
}


void ak_theme_load_defaults(ak_application* pApplication, ak_theme* pTheme)
{
    assert(pApplication != NULL);
    assert(pTheme       != NULL);

    //easy2d_context* pDrawingContext = ak_get_application_drawing_context(pApplication);
    //assert(pDrawingContext != NULL);

#ifdef AK_USE_WIN32
    const char*  defaultUIFontFamily = "Segoe UI";
    unsigned int defaultUIFontSize   = 12;

    const char*  defaultMonospaceFontFamily = "Consolas";
    unsigned int defaultMonospaceFontSize   = 13;
#endif
#ifdef AK_USE_GTK
    const char*  defaultUIFontFamily = "Deja Vu Sans";          // TODO: Use GTK theme.
    unsigned int defaultUIFontSize   = 13;                      // TODO: Use GTK theme.

    const char*  defaultMonospaceFontFamily = "Monospace";      // TODO: Use GTK theme.
    unsigned int defaultMonospaceFontSize   = 15;               // TODO: Use GTK theme.
#endif


    //// Colors ////
    pTheme->baseColor = easygui_rgb(64, 64, 64);


    //// Tabs ////
    //pTheme->tabColor         = easygui_rgb(64, 64, 64);
    pTheme->tabColor         = easygui_rgb(52, 52, 52);
    pTheme->tabHoveredColor  = easygui_rgb(0, 128, 255);
    //pTheme->tabActiveColor   = easygui_rgb(0, 101, 202);
    pTheme->tabActiveColor   = easygui_rgb(80, 80, 80);
    pTheme->tabPaddingLeft   = 4;
    pTheme->tabPaddingTop    = 4;
    pTheme->tabPaddingRight  = 4;
    pTheme->tabPaddingBottom = 4;

    
    //// Menus ////
    pTheme->menuBarHeight = 22;
    pTheme->menuBarItemPaddingX = 8;



    //// Fonts ////
    pTheme->pUIFont = easygui_create_font(ak_get_application_gui(pApplication), defaultUIFontFamily, defaultUIFontSize, easy2d_weight_normal, easy2d_slant_none, 0);
    pTheme->uiFontColor = easygui_rgb(240, 240, 240);
    easygui_get_font_metrics(pTheme->pUIFont, &pTheme->uiFontMetrics);
    easygui_get_glyph_metrics(pTheme->pUIFont, 'X', &pTheme->uiCrossMetrics);



    //// Default Text Editor ///

    // Default text editor.
    pTheme->defaultText = ak_init_text_theme(pApplication, defaultMonospaceFontFamily, defaultMonospaceFontSize, easy2d_weight_default, easy2d_slant_none, easygui_rgb(192, 192, 192), easygui_rgb(80, 80, 80));
}

void ak_theme_load_from_file(ak_application* pApplication, ak_theme* pTheme, const char* absolutePath)
{
    assert(pApplication != NULL);
    assert(pTheme       != NULL);

    (void)absolutePath;
    ak_theme_load_defaults(pApplication, pTheme);
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