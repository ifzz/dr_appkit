// Public domain. See "unlicense" statement at the end of dr_appkit.h.

struct ak_image_library
{
    // The GUI context.
    drgui_context* pGUI;

    // The cross icon.
    ak_image* pCross;
};

ak_image_library* ak_create_image_library(drgui_context* pGUI)
{
    if (pGUI == NULL) {
        return NULL;
    }

    ak_image_library* pImageLibrary = malloc(sizeof(*pImageLibrary));
    if (pImageLibrary == NULL) {
        return pImageLibrary;
    }

    pImageLibrary->pGUI = pGUI;

    pImageLibrary->pCross = ak_create_image();
    if (pImageLibrary->pCross != NULL)
    {
        ak_image_create_and_add_sub_image(pImageLibrary->pCross, pGUI, 1.0f, 16, 16, 16*4, g_AKImage_Cross_16x16);
        ak_image_create_and_add_sub_image(pImageLibrary->pCross, pGUI, 1.5f, 24, 24, 24*4, g_AKImage_Cross_24x24);
        ak_image_create_and_add_sub_image(pImageLibrary->pCross, pGUI, 2.0f, 32, 32, 32*4, g_AKImage_Cross_32x32);
    }

    return pImageLibrary;
}

void ak_delete_image_library(ak_image_library* pImageLibrary)
{
    if (pImageLibrary == NULL) {
        return;
    }

    ak_delete_image(pImageLibrary->pCross);
    free(pImageLibrary);
}


drgui_image* ak_image_library_get_cross(ak_image_library* pImageLibrary, float scale)
{
    if (pImageLibrary == NULL) {
        return NULL;
    }

    return ak_image_select_sub_image(pImageLibrary->pCross, scale);
}
