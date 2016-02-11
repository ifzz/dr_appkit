// Public domain. See "unlicense" statement at the end of dr_appkit.h.

// An ak_image is basically just a container around multiple drgui_image sub-images which are of different scales.

typedef struct ak_image ak_image;

// ak_create_image().
ak_image* ak_create_image();

// ak_delete_image()
void ak_delete_image(ak_image* pImage);


// ak_image_create_and_add_sub_image()
drgui_image* ak_image_create_and_add_sub_image(ak_image* pImage, drgui_context* pGUI, float associatedScale, unsigned int width, unsigned int height, unsigned int stride, const void* pData);

// ak_image_select_sub_image()
drgui_image* ak_image_select_sub_image(ak_image* pImage, float scale);
