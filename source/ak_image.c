// Public domain. See "unlicense" statement at the end of dr_appkit.h.

typedef struct
{
    /// A pointer to the drgui_image object.
    drgui_image* pGUIImage;

    /// The associated scale.
    float associatedScale;

} ak_subimage;

struct ak_image
{
    /// The number of sub-images contained within the image.
    size_t subImageCount;

    /// The list of sub-images.
    ak_subimage* pSubImages;
};


ak_image* ak_create_image()
{
    ak_image* pImage = malloc(sizeof(*pImage));
    if (pImage == NULL) {
        return NULL;
    }

    pImage->subImageCount = 0;
    pImage->pSubImages    = NULL;

    return pImage;
}

void ak_delete_image(ak_image* pImage)
{
    if (pImage == NULL) {
        return;
    }

    for (size_t i = 0; i < pImage->subImageCount; ++i)
    {
        drgui_delete_image(pImage->pSubImages[i].pGUIImage);
    }

    free(pImage->pSubImages);
    free(pImage);
}


drgui_image* ak_image_create_and_add_sub_image(ak_image* pImage, drgui_context* pGUI, float associatedScale, unsigned int width, unsigned int height, unsigned int stride, const void* pData)
{
    if (pImage == NULL || pGUI == NULL) {
        return NULL;
    }

    drgui_image* pSubImage = drgui_create_image(pGUI, width, height, stride, pData);
    if (pSubImage == NULL) {
        return NULL;
    }

    // If there is another image with the same scale it needs to be replaced.
    for (size_t i = 0; i < pImage->subImageCount; ++i) {
        if (pImage->pSubImages[i].associatedScale == associatedScale) {
            drgui_delete_image(pImage->pSubImages[i].pGUIImage);
            pImage->pSubImages[i].pGUIImage = pSubImage;
            return pSubImage;
        }
    }

    // If we get here it means a sub-image of the given scale does not already exist. We just need to add it to the end of the list.
    ak_subimage* pNewSubImages = realloc(pImage->pSubImages, sizeof(*pImage->pSubImages) * (pImage->subImageCount + 1));
    if (pNewSubImages == NULL) {
        drgui_delete_image(pSubImage);
        return NULL;
    }

    pNewSubImages[pImage->subImageCount].pGUIImage = pSubImage;
    pNewSubImages[pImage->subImageCount].associatedScale = associatedScale;

    pImage->pSubImages     = pNewSubImages;
    pImage->subImageCount += 1;

    return pSubImage;
}

drgui_image* ak_image_select_sub_image(ak_image* pImage, float scale)
{
    if (pImage == NULL || pImage->subImageCount == 0) {
        return NULL;
    }

    ak_subimage* pClosestSubImage = pImage->pSubImages;
    for (size_t i = 1; i < pImage->subImageCount; ++i) {
        if (fabs(pImage->pSubImages[i].associatedScale - scale) < fabs(pClosestSubImage->associatedScale - scale)) {
            pClosestSubImage = pImage->pSubImages + i;
        }
    }

    assert(pClosestSubImage != NULL);
    return pClosestSubImage->pGUIImage;
}
