// Public domain. See "unlicense" statement at the end of this file.

//
// QUICK NOTES
//
// - The GUI image manager is intended for managing images that will be only used with GUI's. It is not
//   general purpose.
//

#ifndef ak_gui_image_manager_h
#define ak_gui_image_manager_h

#ifdef __cplusplus
extern "C" {
#endif

typedef struct easyvfs_context easyvfs_context;
typedef struct easygui_context easygui_context;
typedef struct easygui_image easygui_image;
typedef struct ak_gui_image_manager ak_gui_image_manager;

/// Creates a new GUI image manager object.
ak_gui_image_manager* ak_create_gui_image_manager(easyvfs_context* pVFS, easygui_context* pGUI);

/// Deletes the given image manager.
void ak_delete_gui_image_manager(ak_gui_image_manager* pIM);


/// Loads a vector image.
easygui_image* ak_load_vector_image_from_file(ak_gui_image_manager* pIM, const char* fileName, unsigned int width, unsigned int height);

/// Deletes the given GUI image.
void ak_unload_image(ak_gui_image_manager* pIM, easygui_image* image);


/// Retrieves a pointer to the right-facing arrow for use with sub-menus and tree-view controls.
easygui_image* ak_get_arrow_right_image(ak_gui_image_manager* pIM);

/// Retrieves a pointer to the right-down facing arrow for use with tree-view controls.
easygui_image* ak_get_arrow_right_down_image(ak_gui_image_manager* pIM);

/// Retrieves a pointer to the red cross image for close buttons.
easygui_image* ak_get_red_cross_image(ak_gui_image_manager* pIM);


#ifdef __cplusplus
}
#endif

#endif // !ak_gui_image_manager_h


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
