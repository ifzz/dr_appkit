// Public domain. See "unlicense" statement at the end of this file.

#include "../include/easy_appkit/ak_gui_image_manager.h"
#include "nanosvg.h"
#include "nanosvgrast.h"
#include <easy_fs/easy_vfs.h>
#include <easy_gui/easy_gui.h>
#include <easy_path/easy_path.h>

struct ak_gui_image_manager
{
    /// A pointer to the virtual file system to use to load image files.
    easyvfs_context* pVFS;

    /// A pointer to the GUI context.
    easygui_context* pGUI;
};

ak_gui_image_manager* ak_create_gui_image_manager(easyvfs_context* pVFS, easygui_context* pGUI)
{
    if (pVFS == NULL || pGUI == NULL) {
        return NULL;
    }

    ak_gui_image_manager* pIM = malloc(sizeof(*pIM));
    if (pIM == NULL) {
        return NULL;
    }

    pIM->pVFS = pVFS;
    pIM->pGUI = pGUI;

    return pIM;
}

void ak_delete_gui_image_manager(ak_gui_image_manager* pIM)
{
    if (pIM == NULL) {
        return;
    }

    // TODO: Unload every image.

    free(pIM);
}


easygui_image* ak_load_vector_image_from_file(ak_gui_image_manager* pIM, const char* fileName, unsigned int width, unsigned int height)
{
    if (pIM == NULL || fileName == NULL || width == 0 || height == 0) {
        return NULL;
    }

    // TODO: Find an already-loaded image of the same name and increment reference counter if exists.

    // Currently, only SVG is supported.
    if (!easypath_extension_equal(fileName, "svg")) {
        return NULL;
    }

    char* svg = easyvfs_open_and_read_text_file(pIM->pVFS, fileName, NULL);
    if (svg == NULL) {
        return NULL;
    }

    NSVGimage* pSVGImage = nsvgParse(svg, "px", 96);
    if (pSVGImage == NULL) {
        easyvfs_free(svg);
        return NULL;
    }

    easyvfs_free(svg);
    svg = NULL;

    const float scale = (float)width / pSVGImage->width;
    int svgWidth  = (int)pSVGImage->width;
    int svgHeight = (int)pSVGImage->height;

    // At this point we have loaded the image and now we need to rasterize it.
    void* pImageData = malloc(svgWidth * svgHeight * 4);
    if (pImageData == NULL) {
        nsvgDelete(pSVGImage);
        return NULL;
    }

    NSVGrasterizer* pSVGRast = nsvgCreateRasterizer();
    if (pSVGRast == NULL) {
        nsvgDelete(pSVGImage);
        return NULL;
    }

    nsvgRasterize(pSVGRast, pSVGImage, 0, 0, scale, pImageData, (int)svgWidth, (int)svgHeight, (int)svgWidth*4);

    nsvgDeleteRasterizer(pSVGRast);
    nsvgDelete(pSVGImage);


    // At this point we have the rasterized image and we can create the easy_gui image resource.
    easygui_image* pImage = easygui_create_image(pIM->pGUI, width, height, svgWidth*4, pImageData);
    if (pImage == NULL) {
        free(pImageData);
        return NULL;
    }

    free(pImageData);
    return pImage;
}

void ak_unload_image(ak_gui_image_manager* pIM, easygui_image* pImage)
{
    if (pIM == NULL || pImage == NULL) {
        return;
    }

    // TODO: Reference count.

    easygui_delete_image(pImage);
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
