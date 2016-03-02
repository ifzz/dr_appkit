// Public domain. See "unlicense" statement at the end of this file.

typedef struct
{
    // A pointer to the application that owns the text box.
    ak_application* pApplication;

    // The timer for blinking the cursor.
    ak_timer* pTimer;

    // The size of the extra data.
    size_t extraDataSize;

    // The extra data.
    char pExtraData[1];

} ak_textbox;

void ak_textbox__on_timer(ak_timer* pTimer, void* pUserData)
{
    drgui_textbox_step((drgui_element*)pUserData, 100);
}


drgui_element* ak_create_textbox(ak_application* pApplication, drgui_element* pParent, size_t extraDataSize, const void* pExtraData)
{
    if (pApplication == NULL) {
        return NULL;
    }

    drgui_element* pTextBox = drgui_create_textbox(ak_get_application_gui(pApplication), pParent, sizeof(ak_textbox) - sizeof(char) + extraDataSize, NULL);
    if (pTextBox == NULL) {
        return NULL;
    }

    ak_textbox* pAKTextBox = drgui_textbox_get_extra_data(pTextBox);
    assert(pAKTextBox != NULL);
    pAKTextBox->pApplication = pApplication;
    pAKTextBox->pTimer = NULL;
    pAKTextBox->extraDataSize = extraDataSize;
    if (pExtraData != NULL) {
        memcpy(pAKTextBox->pExtraData, pExtraData, extraDataSize);
    }

    drgui_set_on_capture_keyboard(pTextBox, ak_textbox_on_capture_keyboard);
    drgui_set_on_release_keyboard(pTextBox, ak_textbox_on_release_keyboard);

    // Set the default theme.
    ak_theme* pTheme = ak_get_application_theme(pApplication);
    drgui_textbox_set_text_color(pTextBox, pTheme->defaultText.textColor);
    drgui_textbox_set_background_color(pTextBox, pTheme->defaultText.backgroundColor);
    drgui_textbox_set_cursor_color(pTextBox, pTheme->defaultText.textColor);
    drgui_textbox_set_font(pTextBox, pTheme->defaultText.pFont);

    return pTextBox;
}

void ak_delete_textbox(drgui_element* pTextBox)
{
    drgui_delete_textbox(pTextBox);
}

size_t ak_textbox_get_extra_data_size(drgui_element* pTextBox)
{
    ak_textbox* pAKTextBox = drgui_textbox_get_extra_data(pTextBox);
    if (pAKTextBox == NULL) {
        return 0;
    }

    return pAKTextBox->extraDataSize;
}

void* ak_textbox_get_extra_data(drgui_element* pTextBox)
{
    ak_textbox* pAKTextBox = drgui_textbox_get_extra_data(pTextBox);
    if (pAKTextBox == NULL) {
        return 0;
    }

    return pAKTextBox->pExtraData;
}

void ak_textbox_on_capture_keyboard(drgui_element* pTextBox, drgui_element* pPrevCapturedElement)
{
    ak_textbox* pAKTextBox = drgui_textbox_get_extra_data(pTextBox);
    if (pAKTextBox != NULL) {
        if (pAKTextBox->pTimer == NULL) {
            pAKTextBox->pTimer = ak_create_timer(pAKTextBox->pApplication, 100, ak_textbox__on_timer, pTextBox);
        }
    }

    drgui_textbox_on_capture_keyboard(pTextBox, pPrevCapturedElement);
}

void ak_textbox_on_release_keyboard(drgui_element* pTextBox, drgui_element* pNewCapturedElement)
{
    ak_textbox* pAKTextBox = drgui_textbox_get_extra_data(pTextBox);
    if (pAKTextBox != NULL) {
        ak_delete_timer(pAKTextBox->pTimer);
        pAKTextBox->pTimer = NULL;
    }

    drgui_textbox_on_release_keyboard(pTextBox, pNewCapturedElement);
}