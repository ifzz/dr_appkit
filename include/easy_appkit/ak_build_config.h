// Public domain. See "unlicense" statement at the end of this file.

#ifndef ak_build_config_h
#define ak_build_config_h

#ifndef AK_MAX_APPLICATION_NAME_LENGTH
#define AK_MAX_APPLICATION_NAME_LENGTH  63
#endif

#ifndef AK_DEFAULT_APPLICATION_NAME
#define AK_DEFAULT_APPLICATION_NAME     "easy_appkit"
#endif


//////////////////////////////////////////////////
// Non-Configurable Section
//
// This section should not be configured by hand, unless it's for the development of the library itself.

#if defined(_WIN32)
#define AK_USE_WIN32
#define AK_USE_WIN32_THREADS
#elif defined(__APPLE__) && defined(__MACH__)
#define AK_USE_OSX
#define AK_USE_POSIX_THREADS
#else
#define AK_USE_GTK
#define AK_USE_POSIX_THREADS
#endif

#endif  //!ak_build_config_h

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

