// Public domain. See "unlicense" statement at the end of this file.

// Creates a dr_gui textbox control with some predefined properties provided by dr_appkit.
//
// This will set the on_capture_keyboard and on_release_keyboard event handlers. If you need to handle this
// yourself, you'll need to call ak_textbox_on_capture_keyboard() and ak_textbox_on_release_keyboard() manually
// from within your own event handlers.
drgui_element* ak_create_textbox(ak_application* pApplication, drgui_element* pParent, size_t extraDataSize, const void* pExtraData);

// Deletes the given textbox that was created with ak_delete_textbox().
void ak_delete_textbox(drgui_element* pTextBox);

// Retrieves the size of the extra data.
size_t ak_textbox_get_extra_data_size(drgui_element* pTextBox);

// Retrieves a pointer to the extra data associated with the given textbox.
void* ak_textbox_get_extra_data(drgui_element* pTextBox);

// Handles the on_capture_keyboard event.
void ak_textbox_on_capture_keyboard(drgui_element* pTextBox, drgui_element* pPrevCapturedElement);

// Handles the on_release_keyboard event.
void ak_textbox_on_release_keyboard(drgui_element* pTextBox, drgui_element* pNewCapturedElement);