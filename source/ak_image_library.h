// Public domain. See "unlicense" statement at the end of dr_appkit.h.

typedef struct ak_image_library ak_image_library;

// Creates an image library object.
ak_image_library* ak_create_image_library(drgui_context* pGUI);

// Deletes the given image library.
void ak_delete_image_library(ak_image_library* pImageLibrary);


// Retrieves a pointer to the standard cross image for the given scale.
drgui_image* ak_image_library_get_cross(ak_image_library* pImageLibrary, float scale);