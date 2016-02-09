// Public domain. See "unlicense" statement at the end of this file.

#ifdef AK_USE_WIN32
unsigned int ak_get_caret_blink_rate()
{
    return GetCaretBlinkTime();
}

const char*  defaultUIFontFamily = "Segoe UI";
    unsigned int defaultUIFontSize   = 12;

void ak_platform_get_default_font(char* familyOut, size_t familyOutSize, float* sizeOut, drgui_font_weight* weightOut, drgui_font_slant* slantOut)
{
    if (familyOut) {
        strcpy_s(familyOut, familyOutSize, "Segoe UI");
    }
    if (sizeOut) {
        *sizeOut = 12;
    }
    if (weightOut) {
        *weightOut = drgui_font_weight_normal;
    }
    if (slantOut) {
        *slantOut = drgui_font_slant_none;
    }
}
#endif

#ifdef AK_USE_GTK
unsigned int ak_get_caret_blink_rate()
{
    gint blinkTime = 1200;
    g_object_get(gtk_settings_get_default(), "gtk-cursor-blink-time", &blinkTime, NULL);

    return (unsigned int)blinkTime / 2;
}

void ak_platform_get_default_font(char* familyOut, size_t familyOutSize, float* sizeOut, drgui_font_weight* weightOut, drgui_font_slant* slantOut)
{
    char family[256] = {'\0'};
    float size = 10;
    drgui_font_weight weight = drgui_font_weight_default;
    drgui_font_slant slant = drgui_font_slant_none;
    char condensed[256] = {'\0'};

    // With GTK, the font name is formatted like "<family><space><weight and slant (optional)><space><size>". We'll use out trusty tokenizer for this.
    char* fontNameAndSize = NULL;
    g_object_get(gtk_settings_get_default(), "gtk-font-name", &fontNameAndSize, NULL);

    const char* prevToken = fontNameAndSize;
    if (prevToken != NULL)
    {
        char token[256];

        const char* nextToken = dr_next_token(prevToken, token, sizeof(token));
        while (nextToken != NULL)
        {
            if (token[0] >= '0' && token[0] <= '9')
            {
                // We found the size.
                size = (float)atof(token);

                // If we haven't set the family yet, we need to do so now. It will be the entirety of fontNameAndSize, up to this token.
                if (family[0] == '\0') {
                    strncpy_s(family, sizeof(family), fontNameAndSize, prevToken - fontNameAndSize);
                }
            }
            else
            {
                // Check for weight or slant.
                bool foundWeightOrSlant = false;
                if (_stricmp(token, "regular") == 0) {
                    foundWeightOrSlant = true;
                    weight = drgui_font_weight_normal;
                } else if (_stricmp(token, "roman") == 0) {
                    foundWeightOrSlant = true;
                    weight = drgui_font_weight_normal;
                } else if (_stricmp(token, "book") == 0) {
                    foundWeightOrSlant = true;
                    weight = drgui_font_weight_normal;
                } else if (_stricmp(token, "semibold") == 0 || _stricmp(token, "semi-bold") == 0) {
                    foundWeightOrSlant = true;
                    weight = drgui_font_weight_semi_bold;
                } else if (_stricmp(token, "bold") == 0) {
                    foundWeightOrSlant = true;
                    weight = drgui_font_weight_bold;
                } else if (_stricmp(token, "extrabold") == 0) {
                    foundWeightOrSlant = true;
                    weight = drgui_font_weight_extra_bold;
                } else if (_stricmp(token, "heavy") == 0) {
                    foundWeightOrSlant = true;
                    weight = drgui_font_weight_heavy;
                } else if (_stricmp(token, "thin") == 0) {
                    foundWeightOrSlant = true;
                    weight = drgui_font_weight_thin;
                } else if (_stricmp(token, "light") == 0) {
                    foundWeightOrSlant = true;
                    weight = drgui_font_weight_light;
                } else if (_stricmp(token, "extralight") == 0) {
                    foundWeightOrSlant = true;
                    weight = drgui_font_weight_extra_light;
                } else if (_stricmp(token, "oblique") == 0) {
                    foundWeightOrSlant = true;
                    slant = drgui_font_slant_oblique;
                } else if (_stricmp(token, "italic") == 0) {
                    foundWeightOrSlant = true;
                    slant = drgui_font_slant_italic;
                } else if (_stricmp(token, "bold-oblique") == 0) {
                    foundWeightOrSlant = true;
                    weight = drgui_font_weight_bold;
                    slant = drgui_font_slant_oblique;
                } else if (_stricmp(token, "condensed") == 0 || _stricmp(token, "semi-condensed") == 0) {
                    strcpy_s(condensed, sizeof(condensed), token);
                }


                // If we haven't set the family yet, we need to do so now. It will be the entirety of fontNameAndSize, up to this token.
                if (foundWeightOrSlant && family[0] == '\0') {
                    strncpy_s(family, sizeof(family), fontNameAndSize, prevToken - fontNameAndSize);
                }
            }

            prevToken = nextToken;
            nextToken = dr_next_token(prevToken, token, sizeof(token));
        }
    }

    if (familyOut) {
        if (family[0] != '\0') {
            strcpy_s(familyOut, familyOutSize, family);
        } else {
            strcpy_s(familyOut, familyOutSize, "Sans");
        }

        if (condensed[0] != '\0') {
            strcat_s(familyOut, familyOutSize, " ");
            strcat_s(familyOut, familyOutSize, condensed);
        }
    }
    if (sizeOut) {
        *sizeOut = size * (96.0/72.0);
    }
    if (weightOut) {
        *weightOut = weight;
    }
    if (slantOut) {
        *slantOut = slant;
    }

    g_free(fontNameAndSize);
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
