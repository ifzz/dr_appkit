// Public domain. See "unlicense" statement at the end of this file.

#include "../include/dr_appkit/ak_window.h"
#include "../include/dr_appkit/ak_application.h"
#include "../include/dr_appkit/ak_build_config.h"
#include "../include/dr_appkit/ak_panel.h"
#include "ak_window_private.h"
#include "ak_application_private.h"
#include <dr_libs/dr_gui.h>
#include <dr_libs/dr_util.h>
#include <assert.h>

#ifndef PRIVATE
#define PRIVATE static
#endif

#ifdef AK_USE_WIN32
#include <windows.h>
#endif

#ifdef AK_USE_GTK
#include <gdk/gdk.h>
#include <gtk/gtk.h>
#endif

struct ak_window
{
#ifdef AK_USE_WIN32
    /// The Win32 window handle.
    HWND hWnd;

    /// The cursor to use with this window.
    HCURSOR hCursor;

    /// The relative position of the window. This is mainly required for popup windows because they require special handling when
    /// setting their position.
    int popupRelativePosX;
    int popupRelativePosY;

    /// The high-surrogate from a WM_CHAR message. This is used in order to build a surrogate pair from a couple of WM_CHAR messages. When
    /// a WM_CHAR message is received when code point is not a high surrogate, this is set to 0.
    unsigned short utf16HighSurrogate;

    /// Keeps track of whether or not the cursor is over this window.
    bool isCursorOver;

    /// Keeps track of whether or not the window is marked as deleted.
    bool isMarkedAsDeleted;
#endif

#ifdef AK_USE_GTK
    /// A pointer to the GTK window.
    GtkWidget* pGTKWindow;

    /// The cursor to use with this window.
    GdkCursor* pGTKCursor;


    /// Keeps track of whether or not the cursor is over this window.
    bool isCursorOver;

    /// Keeps track of whether or not the window is marked as deleted.
    bool isMarkedAsDeleted;
#endif


    /// A pointer to the application that owns this window.
    ak_application* pApplication;

    /// The window type.
    ak_window_type type;

    /// The top-level GUI element to draw to this window.
    drgui_element* pPanel;

    /// The easy_draw surface we'll be drawing to.
    dr2d_surface* pSurface;

    /// The name of the window.
    char name[AK_MAX_WINDOW_NAME_LENGTH];


    /// The flags to pass to the onHide event handler.
    unsigned int onHideFlags;


    /// The function to call when the window is wanting to close.
    ak_window_on_close_proc onClose;

    /// The function to call when the window is about to be hidden. If false is returned the window is prevented from being hidden.
    ak_window_on_hide_proc onHide;

    /// The function to call when the window is about to be shown. If false is returned, the window is prevented from being shown.
    ak_window_on_show_proc onShow;

    /// The function to call when the window has been activated.
    ak_window_on_activate_proc onActivate;

    /// The function to call when the window has been deactivated.
    ak_window_on_deactivate_proc onDeactivate;

    /// Called when the mouse enters the client area of the window.
    ak_window_on_mouse_enter_proc onMouseEnter;

    /// Called when the mouse leaves the client area of the window.
    ak_window_on_mouse_leave_proc onMouseLeave;

    /// Called when a mouse button is pressed.
    ak_window_on_mouse_button_proc onMouseButtonDown;

    /// Called when a mouse button is released.
    ak_window_on_mouse_button_proc onMouseButtonUp;

    /// Called when a mouse button is double clicked.
    ak_window_on_mouse_button_proc onMouseButtonDblClick;

    /// Called when the mouse wheel is turned.
    ak_window_on_mouse_wheel_proc onMouseWheel;

    /// Called when a key is pressed.
    ak_window_on_key_down_proc onKeyDown;

    /// Called when a key is released.
    ak_window_on_key_up_proc onKeyUp;

    /// Called when a printable key is pressed.
    ak_window_on_printable_key_down_proc onPrintableKeyDown;


    /// A pointer to the parent window.
    ak_window* pParent;

    /// The first child window.
    ak_window* pFirstChild;

    /// The last child window.
    ak_window* pLastChild;

    /// [Internal Use Only] The next window in the linked list of windows that were created by this application. This is not necessarily a related window.
    ak_window* pNextSibling;

    /// [Internal Use Only] The previous window in the linked list of windows that were created by this application. This is not necessarily a related window.
    ak_window* pPrevSibling;


    /// The size of the extra data in bytes.
    size_t extraDataSize;

    /// A pointer to the extra data.
    char pExtraData[1];
};

PRIVATE void ak_detach_window(ak_window* pWindow)
{
    if (pWindow->pParent != NULL)
    {
        if (pWindow->pParent->pFirstChild == pWindow) {
            pWindow->pParent->pFirstChild = pWindow->pNextSibling;
        }

        if (pWindow->pParent->pLastChild == pWindow) {
            pWindow->pParent->pLastChild = pWindow->pPrevSibling;
        }


        if (pWindow->pPrevSibling != NULL) {
            pWindow->pPrevSibling->pNextSibling = pWindow->pNextSibling;
        }

        if (pWindow->pNextSibling != NULL) {
            pWindow->pNextSibling->pPrevSibling = pWindow->pPrevSibling;
        }
    }

    pWindow->pParent      = NULL;
    pWindow->pPrevSibling = NULL;
    pWindow->pNextSibling = NULL;
}

PRIVATE void ak_append_window(ak_window* pWindow, ak_window* pParent)
{
    // Detach the child from it's current parent first.
    ak_detach_window(pWindow);

    pWindow->pParent = pParent;
    assert(pWindow->pParent != NULL);

    if (pWindow->pParent->pLastChild != NULL) {
        pWindow->pPrevSibling = pWindow->pParent->pLastChild;
        pWindow->pPrevSibling->pNextSibling = pWindow;
    }

    if (pWindow->pParent->pFirstChild == NULL) {
        pWindow->pParent->pFirstChild = pWindow;
    }

    pWindow->pParent->pLastChild = pWindow;
}



#ifdef AK_USE_WIN32
static const char* g_WindowClass        = "AK_WindowClass";
static const char* g_WindowClass_Dialog = "AK_WindowClass_Dialog";
static const char* g_WindowClass_Popup  = "AK_WindowClass_Popup";

#define GET_X_LPARAM(lp)                        ((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp)                        ((int)(short)HIWORD(lp))


typedef struct
{
    /// A pointer to the window object itself.
    ak_window* pWindow;

    /// The Win32 window handle.
    HWND hWnd;

}element_user_data;



PRIVATE void ak_on_global_capture_mouse(drgui_element* pElement)
{
    drgui_element* pTopLevelElement = drgui_find_top_level_element(pElement);
    assert(pTopLevelElement != NULL);

    element_user_data* pElementData = ak_panel_get_extra_data(pTopLevelElement);
    if (pElementData != NULL) {
        SetCapture(pElementData->hWnd);
    }
}

PRIVATE void ak_on_global_release_mouse(drgui_element* pElement)
{
    drgui_element* pTopLevelElement = drgui_find_top_level_element(pElement);
    assert(pTopLevelElement != NULL);

    element_user_data* pElementData = ak_panel_get_extra_data(pTopLevelElement);
    if (pElementData != NULL) {
        ReleaseCapture();
    }
}

PRIVATE void ak_on_global_capture_keyboard(drgui_element* pElement, drgui_element* pPrevCapturedElement)
{
    (void)pPrevCapturedElement;

    drgui_element* pTopLevelElement = drgui_find_top_level_element(pElement);
    assert(pTopLevelElement != NULL);

    element_user_data* pElementData = ak_panel_get_extra_data(pTopLevelElement);
    if (pElementData != NULL) {
        SetFocus(pElementData->hWnd);
    }
}

PRIVATE void ak_on_global_release_keyboard(drgui_element* pElement, drgui_element* pNewCapturedElement)
{
    (void)pNewCapturedElement;

    drgui_element* pTopLevelElement = drgui_find_top_level_element(pElement);
    assert(pTopLevelElement != NULL);

    element_user_data* pElementData = ak_panel_get_extra_data(pTopLevelElement);
    if (pElementData != NULL) {
        SetFocus(NULL);
    }
}

PRIVATE void ak_on_global_dirty(drgui_element* pElement, drgui_rect relativeRect)
{
    drgui_element* pTopLevelElement = drgui_find_top_level_element(pElement);
    assert(pTopLevelElement != NULL);

    element_user_data* pElementData = ak_panel_get_extra_data(pTopLevelElement);
    if (pElementData != NULL)
    {
        drgui_rect absoluteRect = relativeRect;
        drgui_make_rect_absolute(pElement, &absoluteRect);


        RECT rect;
        rect.left   = (LONG)absoluteRect.left;
        rect.top    = (LONG)absoluteRect.top;
        rect.right  = (LONG)absoluteRect.right;
        rect.bottom = (LONG)absoluteRect.bottom;
        InvalidateRect(pElementData->hWnd, &rect, FALSE);
    }
}


PRIVATE drgui_element* ak_create_window_panel(ak_application* pApplication, ak_window* pWindow, HWND hWnd)
{
    drgui_element* pElement = ak_create_panel(pApplication, NULL, sizeof(element_user_data), NULL);
    if (pElement == NULL) {
        return NULL;
    }

    ak_panel_set_type(pElement, "AK.RootWindowPanel");


    float dpiScaleX;
    float dpiScaleY;
    ak_get_window_dpi_scale(pWindow, &dpiScaleX, &dpiScaleY);

#if 0
    // TESTING. DELETE THIS BLOCK LATER.
    {
        dpiScaleX = 2;
        dpiScaleY = 2;
    }
#endif

    drgui_set_inner_scale(pElement, dpiScaleX, dpiScaleY);

    element_user_data* pUserData = ak_panel_get_extra_data(pElement);
    assert(pUserData != NULL);

    pUserData->hWnd    = hWnd;
    pUserData->pWindow = pWindow;

    return pElement;
}

PRIVATE void ak_delete_window_panel(drgui_element* pTopLevelElement)
{
    drgui_delete_element(pTopLevelElement);
}



ak_window* ak_alloc_and_init_window_win32(ak_application* pApplication, ak_window* pParent, ak_window_type type, HWND hWnd, size_t extraDataSize, const void* pExtraData)
{
    ak_window* pWindow = malloc(sizeof(*pWindow) + extraDataSize - sizeof(pWindow->pExtraData));
    if (pWindow == NULL)
    {
        ak_errorf(pApplication, "Failed to allocate memory for window.");
        return NULL;
    }

    pWindow->pSurface = dr2d_create_surface_gdi_HWND(ak_get_application_drawing_context(pApplication), hWnd);
    if (pWindow->pSurface == NULL)
    {
        ak_errorf(pApplication, "Failed to create drawing surface for window.");

        free(pWindow);
        return NULL;
    }

    pWindow->pPanel = ak_create_window_panel(pApplication, pWindow, hWnd);
    if (pWindow->pPanel == NULL)
    {
        ak_errorf(pApplication, "Failed to create panel element for window.");

        dr2d_delete_surface(pWindow->pSurface);
        free(pWindow);
        return NULL;
    }

    // The GUI panel needs to have it's initial size set.
    int windowWidth;
    int windowHeight;
    ak_get_window_size(pWindow, &windowWidth, &windowHeight);
    drgui_set_size(pWindow->pPanel, (float)windowWidth, (float)windowHeight);

    // The name should be an empty string by default.
    pWindow->hWnd                  = hWnd;
    pWindow->hCursor               = LoadCursor(NULL, IDC_ARROW);
    pWindow->popupRelativePosX     = 0;
    pWindow->popupRelativePosY     = 0;
    pWindow->utf16HighSurrogate    = 0;
    pWindow->isCursorOver          = false;
    pWindow->isMarkedAsDeleted     = false;

    pWindow->pApplication          = pApplication;
    pWindow->type                  = type;
    pWindow->name[0]               = '\0';
    pWindow->onHideFlags           = 0;
    pWindow->onClose               = NULL;
    pWindow->onHide                = NULL;
    pWindow->onShow                = NULL;
    pWindow->onActivate            = NULL;
    pWindow->onDeactivate          = NULL;
    pWindow->onMouseEnter          = NULL;
    pWindow->onMouseLeave          = NULL;
    pWindow->onMouseButtonDown     = NULL;
    pWindow->onMouseButtonUp       = NULL;
    pWindow->onMouseButtonDblClick = NULL;
    pWindow->onMouseWheel          = NULL;
    pWindow->onKeyDown             = NULL;
    pWindow->onKeyUp               = NULL;
    pWindow->onPrintableKeyDown    = NULL;
    pWindow->pParent               = NULL;
    pWindow->pFirstChild           = NULL;
    pWindow->pLastChild            = NULL;
    pWindow->pNextSibling          = NULL;
    pWindow->pPrevSibling          = NULL;
    pWindow->extraDataSize         = extraDataSize;

    if (pExtraData != NULL) {
        memcpy(pWindow->pExtraData, pExtraData, extraDataSize);
    }


    // We need to link our window object to the Win32 window.
    SetWindowLongPtrA(hWnd, 0, (LONG_PTR)pWindow);


    // The application needs to track this window.
    if (pParent == NULL) {
        ak_application_track_top_level_window(pWindow);
    } else {
        ak_append_window(pWindow, pParent);
    }


    return pWindow;
}

void ak_uninit_and_free_window_win32(ak_window* pWindow)
{
    if (pWindow->pParent == NULL) {
        ak_application_untrack_top_level_window(pWindow);
    } else {
        ak_detach_window(pWindow);
    }



    SetWindowLongPtrA(pWindow->hWnd, 0, (LONG_PTR)NULL);


    //ak_application_on_delete_window(pWindow);

    ak_delete_window_panel(pWindow->pPanel);
    pWindow->pPanel = NULL;

    dr2d_delete_surface(pWindow->pSurface);
    pWindow->pSurface = NULL;

    free(pWindow);
}

PRIVATE void ak_refresh_popup_position(ak_window* pPopupWindow)
{
    // This function will place the given window (which is assumed to be a popup window) relative to the client area of it's parent.

    assert(pPopupWindow != NULL);
    assert(pPopupWindow->type == ak_window_type_popup);

    HWND hOwnerWnd = GetWindow(pPopupWindow->hWnd, GW_OWNER);
    if (hOwnerWnd != NULL)
    {
        RECT ownerRect;
        GetWindowRect(hOwnerWnd, &ownerRect);

        POINT p;
        p.x = 0;
        p.y = 0;
        if (ClientToScreen(hOwnerWnd, &p))
        {
            RECT parentRect;
            GetWindowRect(hOwnerWnd, &parentRect);

            int offsetX = p.x - parentRect.left;
            int offsetY = p.y - parentRect.top;

            SetWindowPos(pPopupWindow->hWnd, NULL, ownerRect.left + pPopupWindow->popupRelativePosX + offsetX, ownerRect.top + pPopupWindow->popupRelativePosY + offsetY, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
        }
    }
}

PRIVATE HWND ak_get_top_level_application_HWND(HWND hWnd)
{
    HWND hTopLevelWindow = hWnd;
    do
    {
        ak_window* pWindow = (ak_window*)GetWindowLongPtr(hTopLevelWindow, 0);
        if (pWindow != NULL)
        {
            if (pWindow->type == ak_window_type_application)
            {
                return hTopLevelWindow;
            }
        }

        hTopLevelWindow = GetWindow(hTopLevelWindow, GW_OWNER);

    } while (hTopLevelWindow != 0);

    return hTopLevelWindow;
}


PRIVATE BOOL ak_is_window_owned_by_this_application(HWND hWnd)
{
    // We just use the window class to determine this.
    char className[256];
    GetClassNameA(hWnd, className, sizeof(className));

    return strcmp(className, g_WindowClass) == 0 || strcmp(className, g_WindowClass_Popup) == 0;
}


PRIVATE void ak_win32_track_mouse_leave_event(HWND hWnd)
{
    TRACKMOUSEEVENT tme;
    ZeroMemory(&tme, sizeof(tme));
    tme.cbSize    = sizeof(tme);
    tme.dwFlags   = TME_LEAVE;
    tme.hwndTrack = hWnd;
    TrackMouseEvent(&tme);
}

bool ak_is_win32_mouse_button_key_code(WPARAM wParam)
{
    return wParam == VK_LBUTTON || wParam == VK_RBUTTON || wParam == VK_MBUTTON || wParam == VK_XBUTTON1 || wParam == VK_XBUTTON2;
}

drgui_key ak_win32_to_drgui_key(WPARAM wParam)
{
    switch (wParam)
    {
    case VK_BACK:   return DRGUI_BACKSPACE;
    case VK_SHIFT:  return DRGUI_SHIFT;
    case VK_ESCAPE: return DRGUI_ESCAPE;
    case VK_PRIOR:  return DRGUI_PAGE_UP;
    case VK_NEXT:   return DRGUI_PAGE_DOWN;
    case VK_END:    return DRGUI_END;
    case VK_HOME:   return DRGUI_HOME;
    case VK_LEFT:   return DRGUI_ARROW_LEFT;
    case VK_UP:     return DRGUI_ARROW_UP;
    case VK_RIGHT:  return DRGUI_ARROW_RIGHT;
    case VK_DOWN:   return DRGUI_ARROW_DOWN;
    case VK_DELETE: return DRGUI_DELETE;

    default: break;
    }

    return (drgui_key)wParam;
}

int ak_win32_get_modifier_key_state_flags()
{
    int stateFlags = 0;

    SHORT keyState = GetAsyncKeyState(VK_SHIFT);
    if (keyState & 0x8000) {
        stateFlags |= AK_KEY_STATE_SHIFT_DOWN;
    }

    keyState = GetAsyncKeyState(VK_CONTROL);
    if (keyState & 0x8000) {
        stateFlags |= AK_KEY_STATE_CTRL_DOWN;
    }

    keyState = GetAsyncKeyState(VK_MENU);
    if (keyState & 0x8000) {
        stateFlags |= AK_KEY_STATE_ALT_DOWN;
    }

    return stateFlags;
}

int ak_win32_get_mouse_event_state_flags(WPARAM wParam)
{
    int stateFlags = 0;

    if ((wParam & MK_LBUTTON) != 0) {
        stateFlags |= AK_MOUSE_BUTTON_LEFT_DOWN;
    }

    if ((wParam & MK_RBUTTON) != 0) {
        stateFlags |= AK_MOUSE_BUTTON_RIGHT_DOWN;
    }

    if ((wParam & MK_MBUTTON) != 0) {
        stateFlags |= AK_MOUSE_BUTTON_MIDDLE_DOWN;
    }

    if ((wParam & MK_XBUTTON1) != 0) {
        stateFlags |= AK_MOUSE_BUTTON_4_DOWN;
    }

    if ((wParam & MK_XBUTTON2) != 0) {
        stateFlags |= AK_MOUSE_BUTTON_5_DOWN;
    }


    if ((wParam & MK_CONTROL) != 0) {
        stateFlags |= AK_KEY_STATE_CTRL_DOWN;
    }

    if ((wParam & MK_SHIFT) != 0) {
        stateFlags |= AK_KEY_STATE_SHIFT_DOWN;
    }


    SHORT keyState = GetAsyncKeyState(VK_MENU);
    if (keyState & 0x8000) {
        stateFlags |= AK_KEY_STATE_ALT_DOWN;
    }


    return stateFlags;
}

LRESULT CALLBACK GenericWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    ak_window* pWindow = (ak_window*)GetWindowLongPtrA(hWnd, 0);
    if (pWindow == NULL) {
        return DefWindowProcA(hWnd, msg, wParam, lParam);
    }

    if (pWindow->isMarkedAsDeleted)
    {
        switch (msg)
        {
            case WM_DESTROY:
            {
                ak_uninit_and_free_window_win32(pWindow);
                break;
            }

            default: break;
        }
    }
    else
    {
        switch (msg)
        {
            case WM_CREATE:
            {
                // This allows us to track mouse enter and leave events for the window.
                ak_win32_track_mouse_leave_event(hWnd);
                return 0;
            }

            case WM_DESTROY:
            {
                ak_uninit_and_free_window_win32(pWindow);
                break;
            }

            case WM_ERASEBKGND:
            {
                return 1;       // Never draw the background of the window - always leave that to dr_gui.
            }


            case WM_CLOSE:
            {
                //ak_application_on_window_wants_to_close(pWindow);
                ak_application_on_close_window(pWindow);
                return 0;
            }

            case WM_WINDOWPOSCHANGING:
            {
                // Showing and hiding windows can be cancelled if false is returned by any of the event handlers.
                WINDOWPOS* pWindowPos = (WINDOWPOS*)lParam;
                assert(pWindowPos != NULL);

                if ((pWindowPos->flags & SWP_HIDEWINDOW) != 0)
                {
                    if (!ak_application_on_hide_window(pWindow, pWindow->onHideFlags))
                    {
                        pWindowPos->flags &= ~SWP_HIDEWINDOW;
                    }

                    pWindow->onHideFlags = 0;
                }

                if ((pWindowPos->flags & SWP_SHOWWINDOW) != 0)
                {
                    if (!ak_application_on_show_window(pWindow))
                    {
                        pWindowPos->flags &= ~SWP_SHOWWINDOW;
                    }
                }


                break;
            }


            case WM_MOUSELEAVE:
            {
                pWindow->isCursorOver = false;

                ak_application_on_mouse_leave(pWindow);
                break;
            }

            case WM_MOUSEMOVE:
            {
                // On Win32 we need to explicitly tell the operating system to post a WM_MOUSELEAVE event. The problem is that it needs to be re-issued when the
                // mouse re-enters the window. The easiest way to do this is to just call it in response to every WM_MOUSEMOVE event.
                if (!pWindow->isCursorOver)
                {
                    ak_win32_track_mouse_leave_event(hWnd);
                    pWindow->isCursorOver = true;

                    ak_application_on_mouse_enter(pWindow);
                }

                ak_application_on_mouse_move(pWindow, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), ak_win32_get_mouse_event_state_flags(wParam));
                break;
            }


            case WM_NCLBUTTONDOWN:
            {
                POINT p;
                p.x = GET_X_LPARAM(lParam);
                p.y = GET_Y_LPARAM(lParam);
                ScreenToClient(hWnd, &p);

                ak_application_on_mouse_button_down(pWindow, DRGUI_MOUSE_BUTTON_LEFT, p.x, p.y, ak_win32_get_mouse_event_state_flags(wParam) | AK_MOUSE_BUTTON_LEFT_DOWN);
                break;
            }
            case WM_NCLBUTTONUP:
            {
                POINT p;
                p.x = GET_X_LPARAM(lParam);
                p.y = GET_Y_LPARAM(lParam);
                ScreenToClient(hWnd, &p);

                ak_application_on_mouse_button_up(pWindow, DRGUI_MOUSE_BUTTON_LEFT, p.x, p.y, ak_win32_get_mouse_event_state_flags(wParam));
                break;
            }
            case WM_NCLBUTTONDBLCLK:
            {
                POINT p;
                p.x = GET_X_LPARAM(lParam);
                p.y = GET_Y_LPARAM(lParam);
                ScreenToClient(hWnd, &p);

                ak_application_on_mouse_button_down(pWindow, DRGUI_MOUSE_BUTTON_LEFT, p.x, p.y, ak_win32_get_mouse_event_state_flags(wParam) | AK_MOUSE_BUTTON_LEFT_DOWN);
                ak_application_on_mouse_button_dblclick(pWindow, DRGUI_MOUSE_BUTTON_LEFT, p.x, p.y, ak_win32_get_mouse_event_state_flags(wParam) | AK_MOUSE_BUTTON_LEFT_DOWN);
                break;
            }

            case WM_LBUTTONDOWN:
            {
                ak_application_on_mouse_button_down(pWindow, DRGUI_MOUSE_BUTTON_LEFT, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), ak_win32_get_mouse_event_state_flags(wParam) | AK_MOUSE_BUTTON_LEFT_DOWN);
                break;
            }
            case WM_LBUTTONUP:
            {
                ak_application_on_mouse_button_up(pWindow, DRGUI_MOUSE_BUTTON_LEFT, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), ak_win32_get_mouse_event_state_flags(wParam));
                break;
            }
            case WM_LBUTTONDBLCLK:
            {
                ak_application_on_mouse_button_down(pWindow, DRGUI_MOUSE_BUTTON_LEFT, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), ak_win32_get_mouse_event_state_flags(wParam) | AK_MOUSE_BUTTON_LEFT_DOWN);
                ak_application_on_mouse_button_dblclick(pWindow, DRGUI_MOUSE_BUTTON_LEFT, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), ak_win32_get_mouse_event_state_flags(wParam) | AK_MOUSE_BUTTON_LEFT_DOWN);
                break;
            }


            case WM_NCRBUTTONDOWN:
            {
                POINT p;
                p.x = GET_X_LPARAM(lParam);
                p.y = GET_Y_LPARAM(lParam);
                ScreenToClient(hWnd, &p);

                ak_application_on_mouse_button_down(pWindow, DRGUI_MOUSE_BUTTON_RIGHT, p.x, p.y, ak_win32_get_mouse_event_state_flags(wParam) | AK_MOUSE_BUTTON_RIGHT_DOWN);
                break;
            }
            case WM_NCRBUTTONUP:
            {
                POINT p;
                p.x = GET_X_LPARAM(lParam);
                p.y = GET_Y_LPARAM(lParam);
                ScreenToClient(hWnd, &p);

                ak_application_on_mouse_button_up(pWindow, DRGUI_MOUSE_BUTTON_RIGHT, p.x, p.y, ak_win32_get_mouse_event_state_flags(wParam));
                break;
            }
            case WM_NCRBUTTONDBLCLK:
            {
                POINT p;
                p.x = GET_X_LPARAM(lParam);
                p.y = GET_Y_LPARAM(lParam);
                ScreenToClient(hWnd, &p);

                ak_application_on_mouse_button_down(pWindow, DRGUI_MOUSE_BUTTON_RIGHT, p.x, p.y, ak_win32_get_mouse_event_state_flags(wParam) | AK_MOUSE_BUTTON_RIGHT_DOWN);
                ak_application_on_mouse_button_dblclick(pWindow, DRGUI_MOUSE_BUTTON_RIGHT, p.x, p.y, ak_win32_get_mouse_event_state_flags(wParam) | AK_MOUSE_BUTTON_RIGHT_DOWN);
                break;
            }

            case WM_RBUTTONDOWN:
            {
                ak_application_on_mouse_button_down(pWindow, DRGUI_MOUSE_BUTTON_RIGHT, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), ak_win32_get_mouse_event_state_flags(wParam) | AK_MOUSE_BUTTON_RIGHT_DOWN);
                break;
            }
            case WM_RBUTTONUP:
            {
                ak_application_on_mouse_button_up(pWindow, DRGUI_MOUSE_BUTTON_RIGHT, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), ak_win32_get_mouse_event_state_flags(wParam));
                break;
            }
            case WM_RBUTTONDBLCLK:
            {
                ak_application_on_mouse_button_down(pWindow, DRGUI_MOUSE_BUTTON_RIGHT, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), ak_win32_get_mouse_event_state_flags(wParam) | AK_MOUSE_BUTTON_RIGHT_DOWN);
                ak_application_on_mouse_button_dblclick(pWindow, DRGUI_MOUSE_BUTTON_RIGHT, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), ak_win32_get_mouse_event_state_flags(wParam) | AK_MOUSE_BUTTON_RIGHT_DOWN);
                break;
            }


            case WM_NCMBUTTONDOWN:
            {
                POINT p;
                p.x = GET_X_LPARAM(lParam);
                p.y = GET_Y_LPARAM(lParam);
                ScreenToClient(hWnd, &p);

                ak_application_on_mouse_button_down(pWindow, DRGUI_MOUSE_BUTTON_MIDDLE, p.x, p.y, ak_win32_get_mouse_event_state_flags(wParam) | AK_MOUSE_BUTTON_MIDDLE_DOWN);
                break;
            }
            case WM_NCMBUTTONUP:
            {
                POINT p;
                p.x = GET_X_LPARAM(lParam);
                p.y = GET_Y_LPARAM(lParam);
                ScreenToClient(hWnd, &p);

                ak_application_on_mouse_button_up(pWindow, DRGUI_MOUSE_BUTTON_MIDDLE, p.x, p.y, ak_win32_get_mouse_event_state_flags(wParam));
                break;
            }
            case WM_NCMBUTTONDBLCLK:
            {
                POINT p;
                p.x = GET_X_LPARAM(lParam);
                p.y = GET_Y_LPARAM(lParam);
                ScreenToClient(hWnd, &p);

                ak_application_on_mouse_button_down(pWindow, DRGUI_MOUSE_BUTTON_MIDDLE, p.x, p.y, ak_win32_get_mouse_event_state_flags(wParam) | AK_MOUSE_BUTTON_MIDDLE_DOWN);
                ak_application_on_mouse_button_dblclick(pWindow, DRGUI_MOUSE_BUTTON_MIDDLE, p.x, p.y, ak_win32_get_mouse_event_state_flags(wParam) | AK_MOUSE_BUTTON_MIDDLE_DOWN);
                break;
            }

            case WM_MBUTTONDOWN:
            {
                ak_application_on_mouse_button_down(pWindow, DRGUI_MOUSE_BUTTON_MIDDLE, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), ak_win32_get_mouse_event_state_flags(wParam) | AK_MOUSE_BUTTON_MIDDLE_DOWN);
                break;
            }
            case WM_MBUTTONUP:
            {
                ak_application_on_mouse_button_up(pWindow, DRGUI_MOUSE_BUTTON_MIDDLE, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), ak_win32_get_mouse_event_state_flags(wParam));
                break;
            }
            case WM_MBUTTONDBLCLK:
            {
                ak_application_on_mouse_button_down(pWindow, DRGUI_MOUSE_BUTTON_MIDDLE, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), ak_win32_get_mouse_event_state_flags(wParam) | AK_MOUSE_BUTTON_MIDDLE_DOWN);
                ak_application_on_mouse_button_dblclick(pWindow, DRGUI_MOUSE_BUTTON_MIDDLE, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), ak_win32_get_mouse_event_state_flags(wParam) | AK_MOUSE_BUTTON_MIDDLE_DOWN);
                break;
            }

            case WM_MOUSEWHEEL:
            {
                int delta = GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;

                POINT p;
                p.x = GET_X_LPARAM(lParam);
                p.y = GET_Y_LPARAM(lParam);
                ScreenToClient(hWnd, &p);

                ak_application_on_mouse_wheel(pWindow, delta, p.x, p.y, ak_win32_get_mouse_event_state_flags(wParam));
                break;
            }


            case WM_KEYDOWN:
                {
                    if (!ak_is_win32_mouse_button_key_code(wParam))
                    {
                        int stateFlags = ak_win32_get_modifier_key_state_flags();
                        if ((lParam & (1 << 30)) != 0) {
                            stateFlags |= AK_KEY_STATE_AUTO_REPEATED;
                        }

                        ak_application_on_key_down(pWindow, ak_win32_to_drgui_key(wParam), stateFlags);
                    }

                    break;
                }

            case WM_KEYUP:
                {
                    if (!ak_is_win32_mouse_button_key_code(wParam))
                    {
                        int stateFlags = ak_win32_get_modifier_key_state_flags();
                        ak_application_on_key_up(pWindow, ak_win32_to_drgui_key(wParam), stateFlags);
                    }

                    break;
                }

                // NOTE: WM_UNICHAR is not posted by Windows itself, but rather intended to be posted by applications. Thus, we need to use WM_CHAR. WM_CHAR
                //       posts events as UTF-16 code points. When the code point is a surrogate pair, we need to store it and wait for the next WM_CHAR event
                //       which will contain the other half of the pair.
            case WM_CHAR:
                {
                    // Windows will post WM_CHAR events for keys we don't particularly want. We'll filter them out here (they will be processed by WM_KEYDOWN).
                    if (wParam < 32 || wParam == 127)       // 127 = ASCII DEL (will be triggered by CTRL+Backspace)
                    {
                        if (wParam != VK_TAB  &&
                            wParam != VK_RETURN)    // VK_RETURN = Enter Key.
                        {
                            break;
                        }
                    }


                    if ((lParam & (1U << 31)) == 0)     // Bit 31 will be 1 if the key was pressed, 0 if it was released.
                    {
                        if (IS_HIGH_SURROGATE(wParam))
                        {
                            assert(pWindow->utf16HighSurrogate == 0);
                            pWindow->utf16HighSurrogate = (unsigned short)wParam;
                        }
                        else
                        {
                            unsigned int character = (unsigned int)wParam;
                            if (IS_LOW_SURROGATE(wParam))
                            {
                                assert(IS_HIGH_SURROGATE(pWindow->utf16HighSurrogate) != 0);
                                character = dr_utf16pair_to_utf32_ch(pWindow->utf16HighSurrogate, (unsigned short)wParam);
                            }

                            pWindow->utf16HighSurrogate = 0;


                            int repeatCount = lParam & 0x0000FFFF;
                            for (int i = 0; i < repeatCount; ++i)
                            {
                                int stateFlags = ak_win32_get_modifier_key_state_flags();
                                if ((lParam & (1 << 30)) != 0) {
                                    stateFlags |= AK_KEY_STATE_AUTO_REPEATED;
                                }

                                ak_application_on_printable_key_down(pWindow, character, stateFlags);
                            }
                        }
                    }

                    break;
                }


            case WM_MOVE:
            {
                break;
            }

            case WM_SIZE:
            {
                drgui_set_size(pWindow->pPanel, LOWORD(lParam), HIWORD(lParam));
                break;
            }

            case WM_PAINT:
            {
                RECT rect;
                if (GetUpdateRect(hWnd, &rect, FALSE)) {
                    drgui_draw(pWindow->pPanel, drgui_make_rect((float)rect.left, (float)rect.top, (float)rect.right, (float)rect.bottom), pWindow->pSurface);
                }

                break;
            }


            case WM_NCACTIVATE:
            {
                BOOL keepActive = (BOOL)wParam;
                BOOL syncOthers = TRUE;

                ak_application* pApplication = pWindow->pApplication;
                for (ak_window* pTrackedWindow = ak_get_application_first_window(pApplication); pTrackedWindow != NULL; pTrackedWindow = ak_get_application_next_window(pApplication, pTrackedWindow))
                {
                    if (pTrackedWindow->hWnd == (HWND)lParam)
                    {
                        keepActive = TRUE;
                        syncOthers = FALSE;

                        break;
                    }
                }

                if (lParam == -1)
                {
                    return DefWindowProc(hWnd, msg, keepActive, 0);
                }

                if (syncOthers)
                {
                    for (ak_window* pTrackedWindow = ak_get_application_first_window(pApplication); pTrackedWindow != NULL; pTrackedWindow = ak_get_application_next_window(pApplication, pTrackedWindow))
                    {
                        if (hWnd != pTrackedWindow->hWnd && hWnd != (HWND)lParam)
                        {
                            SendMessage(pTrackedWindow->hWnd, msg, keepActive, -1);
                        }
                    }
                }

                return DefWindowProc(hWnd, msg, keepActive, lParam);
            }

            case WM_ACTIVATE:
            {
                HWND hActivatedWnd   = NULL;
                HWND hDeactivatedWnd = NULL;

                if (LOWORD(wParam) != WA_INACTIVE)
                {
                    // Activated.
                    hActivatedWnd   = hWnd;
                    hDeactivatedWnd = (HWND)lParam;
                }
                else
                {
                    // Deactivated.
                    hActivatedWnd   = (HWND)lParam;
                    hDeactivatedWnd = hWnd;
                }


                BOOL isActivatedWindowOwnedByThis   = ak_is_window_owned_by_this_application(hActivatedWnd);
                BOOL isDeactivatedWindowOwnedByThis = ak_is_window_owned_by_this_application(hDeactivatedWnd);

                if (isActivatedWindowOwnedByThis && isDeactivatedWindowOwnedByThis)
                {
                    // Both windows are owned the by application.

                    if (LOWORD(wParam) != WA_INACTIVE)
                    {
                        hActivatedWnd   = ak_get_top_level_application_HWND(hActivatedWnd);
                        hDeactivatedWnd = ak_get_top_level_application_HWND(hDeactivatedWnd);

                        if (hActivatedWnd != hDeactivatedWnd)
                        {
                            if (hDeactivatedWnd != NULL)
                            {
                                ak_application_on_deactivate_window((ak_window*)GetWindowLongPtrA(hDeactivatedWnd, 0));
                            }

                            if (hActivatedWnd != NULL)
                            {
                                ak_application_on_activate_window((ak_window*)GetWindowLongPtrA(hActivatedWnd, 0));
                            }
                        }
                    }
                }
                else
                {
                    // The windows are not both owned by this manager.

                    if (isDeactivatedWindowOwnedByThis)
                    {
                        hDeactivatedWnd = ak_get_top_level_application_HWND(hDeactivatedWnd);
                        if (hDeactivatedWnd != NULL)
                        {
                            ak_application_on_deactivate_window((ak_window*)GetWindowLongPtrA(hDeactivatedWnd, 0));
                        }
                    }

                    if (isActivatedWindowOwnedByThis)
                    {
                        hActivatedWnd = ak_get_top_level_application_HWND(hActivatedWnd);
                        if (hActivatedWnd != NULL)
                        {
                            ak_application_on_activate_window((ak_window*)GetWindowLongPtrA(hActivatedWnd, 0));
                        }
                    }
                }



                break;
            }

            case WM_SETCURSOR:
            {
                if (LOWORD(lParam) == HTCLIENT)
                {
                    SetCursor(pWindow->hCursor);
                    return TRUE;
                }

                break;
            }

            case WM_SETFOCUS:
            {
                ak_application_on_focus_window(pWindow);
                break;
            }

            case WM_KILLFOCUS:
            {
                ak_application_on_unfocus_window(pWindow);
                break;
            }


        default: break;
        }
    }

    return DefWindowProcA(hWnd, msg, wParam, lParam);
}


PRIVATE ak_window* ak_create_application_window(ak_application* pApplication, ak_window* pParent, size_t extraDataSize, const void* pExtraData)
{
    assert(pApplication != NULL);

    // You will note here that we do not actually assign a parent window to the new window in CreateWindowEx(). The reason for this is that
    // we want the window to show up on the task bar as a sort of stand alone window. If we set the parent in CreateWindowEx() we won't get
    // this behaviour. As a result, pParent is actually completely ignored, however we assume all child application windows are, conceptually,
    // children of the primary window.
    (void)pParent;


    // Create a window.
    DWORD dwExStyle = 0;
    DWORD dwStyle   = WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_OVERLAPPEDWINDOW;
    HWND hWnd = CreateWindowExA(dwExStyle, g_WindowClass, "", dwStyle, 0, 0, 1280, 720, NULL, NULL, NULL, NULL);
    if (hWnd == NULL)
    {
        ak_errorf(pApplication, "Failed to create Win32 application window.");
        return NULL;
    }


    // This is deleted in the WM_DESTROY event. The reason for this is that deleting a window should be recursive and we can just
    // let Windows tell us when a child window is deleted.
    ak_window* pWindow = ak_alloc_and_init_window_win32(pApplication, pParent, ak_window_type_application, hWnd, extraDataSize, pExtraData);
    if (pWindow == NULL)
    {
        ak_errorf(pApplication, "Failed to allocate and initialize application window.");

        DestroyWindow(hWnd);
        return NULL;
    }



    return pWindow;
}

PRIVATE ak_window* ak_create_child_window(ak_application* pApplication, ak_window* pParent, size_t extraDataSize, const void* pExtraData)
{
    assert(pApplication  != NULL);

    // A child window must always have a parent.
    if (pParent == NULL)
    {
        ak_errorf(pApplication, "Attempting to create a child window without a parent.");
        return NULL;
    }


    // Create a window.
    DWORD dwExStyle = 0;
    DWORD dwStyle   = WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_CHILDWINDOW;
    HWND hWnd = CreateWindowExA(dwExStyle, g_WindowClass, NULL, dwStyle, 0, 0, 1, 1, pParent->hWnd, NULL, NULL, NULL);
    if (hWnd == NULL)
    {
        ak_errorf(pApplication, "Failed to create Win32 child window.");
        return NULL;
    }


    ak_window* pWindow = ak_alloc_and_init_window_win32(pApplication, pParent, ak_window_type_child, hWnd, extraDataSize, pExtraData);
    if (pWindow == NULL)
    {
        ak_errorf(pApplication, "Failed to allocate and initialize child window.");

        DestroyWindow(hWnd);
        return NULL;
    }


    return pWindow;
}

PRIVATE ak_window* ak_create_dialog_window(ak_application* pApplication, ak_window* pParent, size_t extraDataSize, const void* pExtraData)
{
    assert(pApplication  != NULL);

    // A dialog window must always have a parent.
    if (pParent == NULL)
    {
        ak_errorf(pApplication, "Attempting to create a dialog window without a parent.");
        return NULL;
    }


    // Create a window.
    DWORD dwExStyle = WS_EX_DLGMODALFRAME;
    DWORD dwStyle   = WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
    HWND hWnd = CreateWindowExA(dwExStyle, g_WindowClass_Dialog, "", dwStyle, 0, 0, 1, 1, pParent->hWnd, NULL, NULL, NULL);
    if (hWnd == NULL)
    {
        ak_errorf(pApplication, "Failed to create Win32 dialog window.");
        return NULL;
    }


    ak_window* pWindow = ak_alloc_and_init_window_win32(pApplication, pParent, ak_window_type_dialog, hWnd, extraDataSize, pExtraData);
    if (pWindow == NULL)
    {
        ak_errorf(pApplication, "Failed to allocate and initialize dialog window.");

        DestroyWindow(hWnd);
        return NULL;
    }


    return pWindow;
}

PRIVATE ak_window* ak_create_popup_window(ak_application* pApplication, ak_window* pParent, size_t extraDataSize, const void* pExtraData)
{
    assert(pApplication != NULL);

    // A popup window must always have a parent.
    if (pParent == NULL)
    {
        ak_errorf(pApplication, "Attempting to create a popup window without a parent.");
        return NULL;
    }


    // Create a window.
    DWORD dwExStyle = 0;
    DWORD dwStyle   = WS_POPUP;
    HWND hWnd = CreateWindowExA(dwExStyle, g_WindowClass_Popup, NULL, dwStyle, 0, 0, 1, 1, pParent->hWnd, NULL, NULL, NULL);
    if (hWnd == NULL)
    {
        ak_errorf(pApplication, "Failed to create Win32 popup window.");
        return NULL;
    }


    ak_window* pWindow = ak_alloc_and_init_window_win32(pApplication, pParent, ak_window_type_popup, hWnd, extraDataSize, pExtraData);
    if (pWindow == NULL)
    {
        ak_errorf(pApplication, "Failed to allocate and initialize popup window.");

        DestroyWindow(hWnd);
        return NULL;
    }

    pWindow->popupRelativePosX = 0;
    pWindow->popupRelativePosY = 0;

    ak_refresh_popup_position(pWindow);


    return pWindow;
}


ak_window* ak_create_window(ak_application* pApplication, ak_window_type type, ak_window* pParent, size_t extraDataSize, const void* pExtraData)
{
    if (pApplication == NULL || type == ak_window_type_unknown) {
        return NULL;
    }

    switch (type)
    {
        case ak_window_type_application:
        {
            return ak_create_application_window(pApplication, pParent, extraDataSize, pExtraData);
        }

        case ak_window_type_child:
        {
            return ak_create_child_window(pApplication, pParent, extraDataSize, pExtraData);
        }

        case ak_window_type_dialog:
        {
            return ak_create_dialog_window(pApplication, pParent, extraDataSize, pExtraData);
        }

        case ak_window_type_popup:
        {
            return ak_create_popup_window(pApplication, pParent, extraDataSize, pExtraData);
        }

        case ak_window_type_unknown:
        default:
        {
            return NULL;
        }
    }
}

void ak_delete_window(ak_window* pWindow)
{
    if (pWindow == NULL) {
        return;
    }

    assert(pWindow->isMarkedAsDeleted == false);        // <-- If you've hit this assert it means you're trying to delete a window multiple times.
    pWindow->isMarkedAsDeleted = true;

    // We just destroy the HWND. This will post a WM_DESTROY message which is where we delete our own window data structure.
    DestroyWindow(pWindow->hWnd);
}





void ak_set_window_title(ak_window* pWindow, const char* pTitle)
{
    if (pWindow == NULL) {
        return;
    }

    SetWindowTextA(pWindow->hWnd, pTitle);
}

void ak_get_window_title(ak_window* pWindow, char* pTitleOut, size_t titleOutSize)
{
    if (pTitleOut == NULL || titleOutSize == 0) {
        return;
    }

    if (pWindow == NULL) {
        pTitleOut[0] = '\0';
        return;
    }

    // For safety we always want the returned string to be null terminated. MSDN is a bit vague on whether or not it places a
    // null terminator on error, so we'll do it explicitly.
    int length = GetWindowTextA(pWindow->hWnd, pTitleOut, (int)titleOutSize);
    pTitleOut[length] = '\0';
}


void ak_set_window_size(ak_window* pWindow, int width, int height)
{
    if (pWindow == NULL) {
        return;
    }

    RECT windowRect;
    RECT clientRect;
    GetWindowRect(pWindow->hWnd, &windowRect);
    GetClientRect(pWindow->hWnd, &clientRect);

    int windowFrameX = (windowRect.right - windowRect.left) - (clientRect.right - clientRect.left);
    int windowFrameY = (windowRect.bottom - windowRect.top) - (clientRect.bottom - clientRect.top);

    assert(windowFrameX >= 0);
    assert(windowFrameY >= 0);

    int scaledWidth  = width  + windowFrameX;
    int scaledHeight = height + windowFrameY;
    SetWindowPos(pWindow->hWnd, NULL, 0, 0, scaledWidth, scaledHeight, SWP_NOZORDER | SWP_NOMOVE);
}

void ak_get_window_size(ak_window* pWindow, int* pWidthOut, int* pHeightOut)
{
    RECT rect;
    if (pWindow == NULL)
    {
        rect.left   = 0;
        rect.top    = 0;
        rect.right  = 0;
        rect.bottom = 0;
    }
    else
    {
        GetClientRect(pWindow->hWnd, &rect);
    }


    if (pWidthOut != NULL) {
        *pWidthOut = rect.right - rect.left;
    }

    if (pHeightOut != NULL) {
        *pHeightOut = rect.bottom - rect.top;
    }
}


void ak_set_window_position(ak_window* pWindow, int posX, int posY)
{
    if (pWindow == NULL) {
        return;
    }


    // Popup windows require special handling.
    if (pWindow->type != ak_window_type_popup)
    {
        SetWindowPos(pWindow->hWnd, NULL, posX, posY, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
    }
    else
    {
        pWindow->popupRelativePosX = posX;
        pWindow->popupRelativePosY = posY;
        ak_refresh_popup_position(pWindow);
    }
}

void ak_get_window_position(ak_window* pWindow, int* pPosXOut, int* pPosYOut)
{
    RECT rect;
    if (pWindow == NULL)
    {
        rect.left   = 0;
        rect.top    = 0;
        rect.right  = 0;
        rect.bottom = 0;
    }
    else
    {
        GetWindowRect(pWindow->hWnd, &rect);
        MapWindowPoints(HWND_DESKTOP, GetParent(pWindow->hWnd), (LPPOINT)&rect, 2);
    }


    if (pPosXOut != NULL) {
        *pPosXOut = rect.left;
    }

    if (pPosYOut != NULL) {
        *pPosYOut = rect.top;
    }
}

void ak_center_window(ak_window* pWindow)
{
    int parentPosX = 0;
    int parentPosY = 0;
    int parentWidth  = 0;
    int parentHeight = 0;

    int windowWidth;
    int windowHeight;
    ak_get_window_size(pWindow, &windowWidth, &windowHeight);

    if (pWindow->pParent != NULL)
    {
        // Center on the parent.
        ak_get_window_size(pWindow->pParent, &parentWidth, &parentHeight);

        RECT rect;
        GetWindowRect(pWindow->pParent->hWnd, &rect);
        parentPosX = rect.left;
        parentPosY = rect.top;
    }
    else
    {
        // Center on the monitor.
        MONITORINFO mi;
        ZeroMemory(&mi, sizeof(mi));
        mi.cbSize = sizeof(MONITORINFO);
        if (GetMonitorInfoA(MonitorFromWindow(pWindow->hWnd, 0), &mi))
        {
            parentWidth  = mi.rcMonitor.right - mi.rcMonitor.left;
            parentHeight = mi.rcMonitor.bottom - mi.rcMonitor.top;
        }
    }

    int windowPosX = ((parentWidth  - windowWidth)  / 2) + parentPosX;
    int windowPosY = ((parentHeight - windowHeight) / 2) + parentPosY;
    SetWindowPos(pWindow->hWnd, NULL, windowPosX, windowPosY, windowWidth, windowHeight, SWP_NOZORDER | SWP_NOSIZE);
}


void ak_show_window(ak_window* pWindow)
{
    if (pWindow == NULL) {
        return;
    }

    ShowWindow(pWindow->hWnd, SW_SHOW);

    // Popup windows should be activated as soon as it's being shown.
    if (pWindow->type == ak_window_type_popup) {
        SetActiveWindow(pWindow->hWnd);
    }
}

void ak_show_window_maximized(ak_window* pWindow)
{
    if (pWindow == NULL) {
        return;
    }

    ShowWindow(pWindow->hWnd, SW_SHOWMAXIMIZED);
}

void show_window_sized(ak_window* pWindow, int width, int height)
{
    if (pWindow == NULL) {
        return;
    }

    // Set the size first.
    ak_set_window_size(pWindow, width, height);

    // Now show the window in it's default state.
    ak_show_window(pWindow);
}

void ak_hide_window(ak_window* pWindow, unsigned int flags)
{
    if (pWindow == NULL) {
        return;
    }

    pWindow->onHideFlags = flags;
    ShowWindow(pWindow->hWnd, SW_HIDE);
}


bool ak_is_window_descendant(ak_window* pDescendant, ak_window* pAncestor)
{
    if (pDescendant == NULL || pAncestor == NULL) {
        return false;
    }

    return IsChild(pAncestor->hWnd, pDescendant->hWnd);
}

bool ak_is_window_ancestor(ak_window* pAncestor, ak_window* pDescendant)
{
    if (pAncestor == NULL || pDescendant == NULL) {
        return false;
    }

    ak_window* pParent = ak_get_parent_window(pDescendant);
    if (pParent != NULL)
    {
        if (pParent == pAncestor) {
            return true;
        } else {
            return ak_is_window_ancestor(pAncestor, pParent);
        }
    }

    return false;
}


ak_window* ak_get_panel_window(drgui_element* pPanel)
{
    drgui_element* pTopLevelPanel = drgui_find_top_level_element(pPanel);
    if (pTopLevelPanel == NULL) {
        return NULL;
    }

    if (!ak_panel_is_of_type(pTopLevelPanel, "AK.RootWindowPanel")) {
        return NULL;
    }

    element_user_data* pWindowData = ak_panel_get_extra_data(pTopLevelPanel);
    assert(pWindowData != NULL);

    assert(ak_panel_get_extra_data_size(pTopLevelPanel) == sizeof(element_user_data));      // A loose check to help ensure we're working with the right kind of panel.
    return pWindowData->pWindow;
}


void ak_set_window_cursor(ak_window* pWindow, ak_cursor_type cursor)
{
    assert(pWindow != NULL);

    switch (cursor)
    {
        case ak_cursor_type_ibeam:
        {
            pWindow->hCursor = LoadCursor(NULL, IDC_IBEAM);
            break;
        }


        case ak_cursor_type_none:
        {
            pWindow->hCursor = NULL;
            break;
        }

        //case cursor_type_arrow:
        case ak_cursor_type_default:
        default:
        {
            pWindow->hCursor = LoadCursor(NULL, IDC_ARROW);
            break;
        }
    }

    // If the cursor is currently inside the window it needs to be changed right now.
    if (ak_is_cursor_over_window(pWindow))
    {
        SetCursor(pWindow->hCursor);
    }
}

bool ak_is_cursor_over_window(ak_window* pWindow)
{
    assert(pWindow != NULL);
    return pWindow->isCursorOver;
}


typedef enum MONITOR_DPI_TYPE {
    MDT_EFFECTIVE_DPI = 0,
    MDT_ANGULAR_DPI = 1,
    MDT_RAW_DPI = 2,
    MDT_DEFAULT = MDT_EFFECTIVE_DPI
} MONITOR_DPI_TYPE;

typedef HRESULT (__stdcall * PFN_GetDpiForMonitor) (HMONITOR hmonitor, MONITOR_DPI_TYPE dpiType, UINT *dpiX, UINT *dpiY);

void ak_get_window_dpi(ak_window* pWindow, int* pDPIXOut, int* pDPIYOut)
{
    // If multi-monitor DPI awareness is not supported we will need to fall back to system DPI.
    HMODULE hSHCoreDLL = LoadLibraryW(L"shcore.dll");
    if (hSHCoreDLL == NULL)
    {
        dr_win32_get_system_dpi(pDPIXOut, pDPIYOut);
        return;
    }

    PFN_GetDpiForMonitor _GetDpiForMonitor = (PFN_GetDpiForMonitor)GetProcAddress(hSHCoreDLL, "GetDpiForMonitor");
    if (_GetDpiForMonitor == NULL)
    {
        dr_win32_get_system_dpi(pDPIXOut, pDPIYOut);
        FreeLibrary(hSHCoreDLL);
        return;
    }


    UINT dpiX;
    UINT dpiY;
    if (_GetDpiForMonitor(MonitorFromWindow(pWindow->hWnd, MONITOR_DEFAULTTOPRIMARY), MDT_EFFECTIVE_DPI, &dpiX, &dpiY) == S_OK)
    {
        if (pDPIXOut) {
            *pDPIXOut = (int)dpiX;
        }
        if (pDPIYOut) {
            *pDPIYOut = (int)dpiY;
        }
    }
    else
    {
        dr_win32_get_system_dpi(pDPIXOut, pDPIYOut);
    }

    FreeLibrary(hSHCoreDLL);
}

void ak_get_window_dpi_scale(ak_window* pWindow, float* pDPIScaleXOut, float* pDPIScaleYOut)
{
    float scaleX = 1;
    float scaleY = 1;

    if (pWindow != NULL)
    {
#if defined(AK_USE_WIN32)
        int baseDPIX;
        int baseDPIY;
        dr_win32_get_base_dpi(&baseDPIX, &baseDPIY);

        int windowDPIX = baseDPIX;
        int windowDPIY = baseDPIY;
        ak_get_window_dpi(pWindow, &windowDPIX, &windowDPIY);

        scaleX = (float)windowDPIX / baseDPIX;
        scaleY = (float)windowDPIY / baseDPIY;
#endif


#if defined(AK_USE_GTK)
        // TODO: Add support for scaling to GTK.
        scaleX = 1;
        scaleY = 1;
#endif
    }

    if (pDPIScaleXOut) {
        *pDPIScaleXOut = scaleX;
    }
    if (pDPIScaleYOut) {
        *pDPIScaleYOut = scaleY;
    }
}







///////////////////////////////////////////////////////////////////////////////
//
// Private APIs
//
///////////////////////////////////////////////////////////////////////////////

unsigned int g_Win32ClassRegCounter = 0;

void ak_init_platform()
{
    if (g_Win32ClassRegCounter > 0) {
        g_Win32ClassRegCounter += 1;
    }


    // Standard windows.
    WNDCLASSEXA wc;
    ZeroMemory(&wc, sizeof(wc));
    wc.cbSize        = sizeof(wc);
    wc.cbWndExtra    = sizeof(ak_window*);
    wc.lpfnWndProc   = (WNDPROC)GenericWindowProc;
    wc.lpszClassName = g_WindowClass;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon         = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(101));
    wc.style         = CS_DBLCLKS;
    if (!RegisterClassExA(&wc)) {
        return;
    }

    // Dialog windows.
    WNDCLASSEXA wcDialog;
    ZeroMemory(&wcDialog, sizeof(wcDialog));
    wcDialog.cbSize        = sizeof(wcDialog);
    wcDialog.cbWndExtra    = sizeof(ak_window*);
    wcDialog.lpfnWndProc   = (WNDPROC)GenericWindowProc;
    wcDialog.lpszClassName = g_WindowClass_Dialog;
    wcDialog.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wcDialog.style         = CS_DBLCLKS;
    if (!RegisterClassExA(&wcDialog)) {
        UnregisterClassA(g_WindowClass, NULL);
        return;
    }

    // Popup windows.
    WNDCLASSEXA wcPopup;
    ZeroMemory(&wcPopup, sizeof(wcPopup));
    wcPopup.cbSize        = sizeof(wcPopup);
    wcPopup.cbWndExtra    = sizeof(ak_window*);
    wcPopup.lpfnWndProc   = (WNDPROC)GenericWindowProc;
    wcPopup.lpszClassName = g_WindowClass_Popup;
    wcPopup.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wcPopup.style         = CS_DBLCLKS | CS_DROPSHADOW;
    if (!RegisterClassExA(&wcPopup)) {
        UnregisterClassA(g_WindowClass, NULL);
        UnregisterClassA(g_WindowClass_Popup, NULL);
        return;
    }


    g_Win32ClassRegCounter += 1;
}

void ak_uninit_platform()
{
    if (g_Win32ClassRegCounter > 0)
    {
        g_Win32ClassRegCounter -= 1;

        if (g_Win32ClassRegCounter == 0)
        {
            UnregisterClassA(g_WindowClass,        NULL);
            UnregisterClassA(g_WindowClass_Dialog, NULL);
            UnregisterClassA(g_WindowClass_Popup,  NULL);
        }
    }
}


void ak_win32_post_quit_message(int exitCode)
{
    PostQuitMessage(exitCode);
}

#endif  //!AK_USE_WIN32


#ifdef AK_USE_GTK
static int g_GTKResultCode = 0;
static int g_GTKInitCounter = 0;

static GdkCursor* g_GTKCursor_Default = NULL;
static GdkCursor* g_GTKCursor_IBeam   = NULL;

typedef struct
{
    /// A pointer to the window object itself.
    ak_window* pWindow;

} ak_element_user_data_gtk;

void ak_uninit_and_free_window_gtk(ak_window* pWindow);


void ak_init_platform()
{
    if (g_GTKInitCounter == 0) {
        gtk_init(NULL, NULL);

        g_GTKCursor_Default = gdk_cursor_new_for_display(gdk_display_get_default(), GDK_LEFT_PTR);
        g_GTKCursor_IBeam   = gdk_cursor_new_for_display(gdk_display_get_default(), GDK_XTERM);
    }

    g_GTKInitCounter += 1;
}

void ak_uninit_platform()
{
    if (g_GTKInitCounter > 0) {
        g_GTKInitCounter -= 1;
    }
}



PRIVATE drgui_element* ak_create_window_panel(ak_application* pApplication, ak_window* pWindow)
{
    drgui_element* pElement = ak_create_panel(pApplication, NULL, sizeof(ak_element_user_data_gtk), NULL);
    if (pElement == NULL) {
        return NULL;
    }

    ak_element_user_data_gtk* pUserData = ak_panel_get_extra_data(pElement);
    assert(pUserData != NULL);

    pUserData->pWindow = pWindow;

    drgui_set_type(pElement, "AK.RootWindowPanel");


    float dpiScaleX;
    float dpiScaleY;
    ak_get_window_dpi_scale(pWindow, &dpiScaleX, &dpiScaleY);

    drgui_set_inner_scale(pElement, dpiScaleX, dpiScaleY);

    return pElement;
}

PRIVATE void ak_delete_window_panel(drgui_element* pTopLevelElement)
{
    drgui_delete_element(pTopLevelElement);
}

static drgui_key ak_gtk_to_drgui_key(guint keyval)
{
    switch (keyval)
    {
    case GDK_KEY_BackSpace: return DRGUI_BACKSPACE;
    case GDK_KEY_Shift_L:   return DRGUI_SHIFT;
    case GDK_KEY_Shift_R:   return DRGUI_SHIFT;
    case GDK_KEY_Escape:    return DRGUI_ESCAPE;
    case GDK_KEY_Page_Up:   return DRGUI_PAGE_UP;
    case GDK_KEY_Page_Down: return DRGUI_PAGE_DOWN;
    case GDK_KEY_End:       return DRGUI_END;
    case GDK_KEY_Begin:     return DRGUI_HOME;
    case GDK_KEY_Left:      return DRGUI_ARROW_LEFT;
    case GDK_KEY_Up:        return DRGUI_ARROW_UP;
    case GDK_KEY_Right:     return DRGUI_ARROW_RIGHT;
    case GDK_KEY_Down:      return DRGUI_ARROW_DOWN;
    case GDK_KEY_Delete:    return DRGUI_DELETE;

    default: break;
    }

    return (drgui_key)keyval;
}

static int ak_gtk_get_modifier_state_flags(guint stateFromGTK)
{
    int result = 0;

    if ((stateFromGTK & GDK_SHIFT_MASK) != 0) {
        result |= AK_KEY_STATE_SHIFT_DOWN;
    }
    if ((stateFromGTK & GDK_CONTROL_MASK) != 0) {
        result |= AK_KEY_STATE_CTRL_DOWN;
    }
    if ((stateFromGTK & GDK_MOD1_MASK) != 0) {
        result |= AK_KEY_STATE_ALT_DOWN;
    }

    if ((stateFromGTK & GDK_BUTTON1_MASK) != 0) {
        result |= AK_MOUSE_BUTTON_LEFT_DOWN;
    }
    if ((stateFromGTK & GDK_BUTTON2_MASK) != 0) {
        result |= AK_MOUSE_BUTTON_MIDDLE_DOWN;
    }
    if ((stateFromGTK & GDK_BUTTON3_MASK) != 0) {
        result |= AK_MOUSE_BUTTON_RIGHT_DOWN;
    }
    if ((stateFromGTK & GDK_BUTTON4_MASK) != 0) {
        result |= AK_MOUSE_BUTTON_4_DOWN;
    }
    if ((stateFromGTK & GDK_BUTTON5_MASK) != 0) {
        result |= AK_MOUSE_BUTTON_5_DOWN;
    }

    return result;
}


static int ak_from_gtk_mouse_button(guint buttonGTK)
{
    switch (buttonGTK) {
        case 1: return DRGUI_MOUSE_BUTTON_LEFT;
        case 2: return DRGUI_MOUSE_BUTTON_MIDDLE;
        case 3: return DRGUI_MOUSE_BUTTON_RIGHT;
        default: return (int)buttonGTK;
    }
}


static void ak_gtk_on_show(GtkWidget* pGTKWindow, gpointer pUserData)
{
    (void)pGTKWindow;

    ak_window* pWindow = pUserData;
    if (pWindow == NULL) {
        return;
    }

    if (!ak_application_on_show_window(pWindow)) {
        ak_hide_window(pWindow, AK_HIDE_BLOCKED);    // The event handler returned false, so prevent the window from being shown.
    }
}

static void ak_gtk_on_hide(GtkWidget* pGTKWindow, gpointer pUserData)
{
    (void)pGTKWindow;

    ak_window* pWindow = pUserData;
    if (pWindow == NULL) {
        return;
    }

    if ((pWindow->onHideFlags & AK_HIDE_BLOCKED) != 0) {
        pWindow->onHideFlags &= ~AK_HIDE_BLOCKED;
        return;
    }

    if (!ak_application_on_hide_window(pWindow, pWindow->onHideFlags)) {
        ak_show_window(pWindow);    // The event handler returned false, so prevent the window from being hidden.
    }
}

static void ak_gtk_on_paint(GtkWidget* pGTKWindow, cairo_t* pCairoContext, gpointer pUserData)
{
    (void)pGTKWindow;

    ak_window* pWindow = pUserData;
    if (pWindow == NULL) {
        return;
    }

    // NOTE: Because we are using dr_2d to draw the GUI, the last argument to drgui_draw() must be a pointer
    //       to the relevant dr2d_surface object.

    double clipLeft;
    double clipTop;
    double clipRight;
    double clipBottom;
    cairo_clip_extents(pCairoContext, &clipLeft, &clipTop, &clipRight, &clipBottom);

    drgui_rect drawRect;
    drawRect.left   = (float)clipLeft;
    drawRect.top    = (float)clipTop;
    drawRect.right  = (float)clipRight;
    drawRect.bottom = (float)clipBottom;
    drgui_draw(pWindow->pPanel, drawRect, pWindow->pSurface);

    // At this point the GUI has been drawn, however nothing has been drawn to the window yet. To do this we will
    // use cairo directly with a cairo_set_source_surface() / cairo_paint() pair. We can get a pointer to dr_2d's
    // internal cairo_surface_t object as shown below.
    cairo_surface_t* pCairoSurface = dr2d_get_cairo_surface_t(pWindow->pSurface);
    if (pCairoSurface != NULL)
    {
        cairo_set_source_surface(pCairoContext, pCairoSurface, 0, 0);
        cairo_paint(pCairoContext);
    }
}

static void ak_gtk_on_onfigure(GtkWidget* pGTKWindow, GdkEventConfigure* pEvent, gpointer pUserData)
{
    ak_window* pWindow = pUserData;
    if (pWindow == NULL) {
        return;
    }

    // If the window's size has changed, it's panel and surface need to be resized, and then redrawn.
    if (pEvent->width != dr2d_get_surface_width(pWindow->pSurface) || pEvent->height != dr2d_get_surface_height(pWindow->pSurface))
    {
        // Size has changed.

        // dr_2d does not support dynamic resizing of surfaces. Thus, we need to delete and recreate it.
        if (pWindow->pSurface != NULL) {
            dr2d_delete_surface(pWindow->pSurface);
        }
        pWindow->pSurface = dr2d_create_surface(ak_get_application_drawing_context(pWindow->pApplication), (float)pEvent->width, (float)pEvent->height);

        // We'll also want to resize the root GUI element so that it's the same size as the parent window.
        drgui_set_size(pWindow->pPanel, (float)pEvent->width, (float)pEvent->height);


        // Invalidate the window to force a redraw.
        gtk_widget_queue_draw(pGTKWindow);
    }
}

static gboolean ak_gtk_on_mouse_enter(GtkWidget* pGTKWindow, GdkEventCrossing* pEvent, gpointer pUserData)
{
    (void)pGTKWindow;
    (void)pEvent;

    ak_window* pWindow = pUserData;
    if (pWindow == NULL) {
        return true;
    }

    pWindow->isCursorOver = true;

    ak_application_on_mouse_enter(pWindow);
    return true;
}

static gboolean ak_gtk_on_mouse_leave(GtkWidget* pGTKWindow, GdkEventCrossing* pEvent, gpointer pUserData)
{
    (void)pGTKWindow;
    (void)pEvent;

    ak_window* pWindow = pUserData;
    if (pWindow == NULL) {
        return true;
    }

    pWindow->isCursorOver = false;

    ak_application_on_mouse_leave(pWindow);
    return true;
}

static gboolean ak_gtk_on_mouse_move(GtkWidget* pGTKWindow, GdkEventMotion* pEvent, gpointer pUserData)
{
    (void)pGTKWindow;

    ak_window* pWindow = pUserData;
    if (pWindow == NULL) {
        return true;
    }

    ak_application_on_mouse_move(pWindow, pEvent->x, pEvent->y, ak_gtk_get_modifier_state_flags(pEvent->state));
    return false;
}

static gboolean ak_gtk_on_mouse_button_down(GtkWidget* pGTKWindow, GdkEventButton* pEvent, gpointer pUserData)
{
    (void)pGTKWindow;

    ak_window* pWindow = pUserData;
    if (pWindow == NULL) {
        return true;
    }

    if (pEvent->type == GDK_BUTTON_PRESS) {
        ak_application_on_mouse_button_down(pWindow, ak_from_gtk_mouse_button(pEvent->button), pEvent->x, pEvent->y, ak_gtk_get_modifier_state_flags(pEvent->state));
    } else if (pEvent->type == GDK_2BUTTON_PRESS) {
        ak_application_on_mouse_button_dblclick(pWindow, ak_from_gtk_mouse_button(pEvent->button), pEvent->x, pEvent->y, ak_gtk_get_modifier_state_flags(pEvent->state));
    }

    return true;
}

static gboolean ak_gtk_on_mouse_button_up(GtkWidget* pGTKWindow, GdkEventButton* pEvent, gpointer pUserData)
{
    (void)pGTKWindow;

    ak_window* pWindow = pUserData;
    if (pWindow == NULL) {
        return true;
    }

    ak_application_on_mouse_button_up(pWindow, ak_from_gtk_mouse_button(pEvent->button), pEvent->x, pEvent->y, ak_gtk_get_modifier_state_flags(pEvent->state));
    return true;
}

static gboolean ak_gtk_on_mouse_wheel(GtkWidget* pGTKWindow, GdkEventScroll* pEvent, gpointer pUserData)
{
    (void)pGTKWindow;

    ak_window* pWindow = pUserData;
    if (pWindow == NULL) {
        return true;
    }

    gdouble delta_y = 0;
    if (pEvent->direction == GDK_SCROLL_UP) {
        delta_y = -1;
    } else if (pEvent->direction == GDK_SCROLL_DOWN) {
        delta_y = 1;
    }

    ak_application_on_mouse_wheel(pWindow, (int)-delta_y, pEvent->x, pEvent->y, ak_gtk_get_modifier_state_flags(pEvent->state));

    return true;
}

static gboolean ak_gtk_on_key_down(GtkWidget* pGTKWindow, GdkEventKey* pEvent, gpointer pUserData)
{
    (void)pGTKWindow;

    ak_window* pWindow = pUserData;
    if (pWindow == NULL) {
        return true;
    }

    int stateFlags = ak_gtk_get_modifier_state_flags(pEvent->state);
    // TODO: Check here if key is auto-repeated.

    ak_application_on_key_down(pWindow, ak_gtk_to_drgui_key(pEvent->keyval), stateFlags);

    guint32 utf32 = gdk_keyval_to_unicode(pEvent->keyval);
    if (utf32 == 0) {
        if (pEvent->keyval == GDK_KEY_KP_Enter) {
            utf32 = '\r';
        }
    }

    if (utf32 != 0 && (stateFlags & AK_KEY_STATE_CTRL_DOWN) == 0 && (stateFlags & AK_KEY_STATE_ALT_DOWN) == 0) {
        if (!(utf32 < 32 || utf32 == 127) || utf32 == '\t' || utf32 == '\r') {
            ak_application_on_printable_key_down(pWindow, (unsigned int)utf32, stateFlags);
        }
    }

    return true;
}

static gboolean ak_gtk_on_key_up(GtkWidget* pGTKWindow, GdkEventKey* pEvent, gpointer pUserData)
{
    (void)pGTKWindow;

    ak_window* pWindow = pUserData;
    if (pWindow == NULL) {
        return true;
    }

    ak_application_on_key_up(pWindow, ak_gtk_to_drgui_key(pEvent->keyval), ak_gtk_get_modifier_state_flags(pEvent->state));
    return true;
}

static gboolean ak_gtk_on_close(GtkWidget* pGTKWindow, GdkEvent* pEvent, gpointer pUserData)
{
    (void)pGTKWindow;
    (void)pEvent;

    ak_window* pWindow = pUserData;
    if (pWindow == NULL) {
        return true;
    }

    ak_application_on_close_window(pWindow);
    return true;
}

static void ak_gtk_on_destroy(GtkWidget* pGTKWindow, gpointer pUserData)
{
    (void)pGTKWindow;

    ak_window* pWindow = pUserData;
    if (pWindow == NULL) {
        return;
    }

    ak_uninit_and_free_window_gtk(pWindow);
    return;
}

static gboolean ak_gtk_on_receive_focus(GtkWidget* pGTKWindow, GdkEventFocus* pEvent, gpointer pUserData)
{
    (void)pGTKWindow;
    (void)pEvent;

    ak_window* pWindow = pUserData;
    if (pWindow == NULL) {
        return true;
    }

    // TODO: Do something.
    printf("Receive Focus\n");

    return true;
}

static gboolean ak_gtk_on_lose_focus(GtkWidget* pGTKWindow, GdkEventFocus* pEvent, gpointer pUserData)
{
    (void)pGTKWindow;
    (void)pEvent;

    ak_window* pWindow = pUserData;
    if (pWindow == NULL) {
        return true;
    }

    // TODO: Do something.
    printf("Lose Focus\n");

    return true;
}



ak_window* ak_alloc_and_init_window_gtk(ak_application* pApplication, ak_window* pParent, ak_window_type type, GtkWidget* pGTKWindow, size_t extraDataSize, const void* pExtraData)
{
    ak_window* pWindow = malloc(sizeof(*pWindow) + extraDataSize - sizeof(pWindow->pExtraData));
    if (pWindow == NULL)
    {
        ak_errorf(pApplication, "Failed to allocate memory for window.");
        return NULL;
    }

    pWindow->pGTKWindow            = pGTKWindow;
    pWindow->pGTKCursor            = g_GTKCursor_Default;
    pWindow->isCursorOver          = false;
    pWindow->isMarkedAsDeleted     = false;
    pWindow->pApplication          = pApplication;
    pWindow->type                  = type;
    pWindow->pSurface              = NULL;
    pWindow->name[0]               = '\0';
    pWindow->onHideFlags           = 0;
    pWindow->onClose               = NULL;
    pWindow->onHide                = NULL;
    pWindow->onShow                = NULL;
    pWindow->onActivate            = NULL;
    pWindow->onDeactivate          = NULL;
    pWindow->onMouseEnter          = NULL;
    pWindow->onMouseLeave          = NULL;
    pWindow->onMouseButtonDown     = NULL;
    pWindow->onMouseButtonUp       = NULL;
    pWindow->onMouseButtonDblClick = NULL;
    pWindow->onMouseWheel          = NULL;
    pWindow->onKeyDown             = NULL;
    pWindow->onKeyUp               = NULL;
    pWindow->onPrintableKeyDown    = NULL;
    pWindow->pParent               = NULL;
    pWindow->pFirstChild           = NULL;
    pWindow->pLastChild            = NULL;
    pWindow->pNextSibling          = NULL;
    pWindow->pPrevSibling          = NULL;
    pWindow->extraDataSize         = extraDataSize;

    if (pExtraData != NULL) {
        memcpy(pWindow->pExtraData, pExtraData, extraDataSize);
    }


    // All windows have a panel assigned to them which is what we use as the root GUI element. The panel is just
    // a drgui_element.
    pWindow->pPanel = ak_create_window_panel(pApplication, pWindow);
    if (pWindow->pPanel == NULL)
    {
        ak_errorf(pApplication, "Failed to create panel element for window.");

        free(pWindow);
        return NULL;
    }


    // Because we are drawing the GUI ourselves rather than through GTK, we want to disable GTK's default painting
    // procedure. To do this we use gtk_widget_set_app_paintable(pWindow, TRUE) which will disable GTK's default
    // painting on the main window. We then connect to the "draw" signal which is where we'll do all of our custom
    // painting.
    gtk_widget_set_app_paintable(pGTKWindow, TRUE);

    gtk_widget_add_events(pGTKWindow,
        GDK_ENTER_NOTIFY_MASK   |
        GDK_LEAVE_NOTIFY_MASK   |
        GDK_POINTER_MOTION_MASK |
        GDK_BUTTON_PRESS_MASK   |
        GDK_BUTTON_RELEASE_MASK |
        GDK_SCROLL_MASK         |
        GDK_KEY_PRESS_MASK      |
        GDK_KEY_RELEASE_MASK    |
        GDK_FOCUS_CHANGE_MASK);

    // Event handlers.
    g_signal_connect(pGTKWindow, "show",                 G_CALLBACK(ak_gtk_on_show),              pWindow);     // Show.
    g_signal_connect(pGTKWindow, "hide",                 G_CALLBACK(ak_gtk_on_hide),              pWindow);     // Hide.
    g_signal_connect(pGTKWindow, "draw",                 G_CALLBACK(ak_gtk_on_paint),             pWindow);     // Paint
    g_signal_connect(pGTKWindow, "configure-event",      G_CALLBACK(ak_gtk_on_onfigure),          pWindow);     // Reposition and resize.
    g_signal_connect(pGTKWindow, "enter-notify-event",   G_CALLBACK(ak_gtk_on_mouse_enter),       pWindow);     // Mouse enter.
    g_signal_connect(pGTKWindow, "leave-notify-event",   G_CALLBACK(ak_gtk_on_mouse_leave),       pWindow);     // Mouse leave.
    g_signal_connect(pGTKWindow, "motion-notify-event",  G_CALLBACK(ak_gtk_on_mouse_move),        pWindow);     // Mouse move.
    g_signal_connect(pGTKWindow, "button-press-event",   G_CALLBACK(ak_gtk_on_mouse_button_down), pWindow);     // Mouse button down.
    g_signal_connect(pGTKWindow, "button-release-event", G_CALLBACK(ak_gtk_on_mouse_button_up),   pWindow);     // Mouse button up.
    g_signal_connect(pGTKWindow, "scroll-event",         G_CALLBACK(ak_gtk_on_mouse_wheel),       pWindow);     // Mouse wheel.
    g_signal_connect(pGTKWindow, "key-press-event",      G_CALLBACK(ak_gtk_on_key_down),          pWindow);     // Key down.
    g_signal_connect(pGTKWindow, "key-release-event",    G_CALLBACK(ak_gtk_on_key_up),            pWindow);     // Key up.
    g_signal_connect(pGTKWindow, "delete-event",         G_CALLBACK(ak_gtk_on_close),             pWindow);     // Close.
    g_signal_connect(pGTKWindow, "destroy",              G_CALLBACK(ak_gtk_on_destroy),           pWindow);     // Fired when the widget is deleted for real.
    g_signal_connect(pGTKWindow, "focus-in-event",       G_CALLBACK(ak_gtk_on_receive_focus),     pWindow);     // Receive focus.
    g_signal_connect(pGTKWindow, "focus-out-event",      G_CALLBACK(ak_gtk_on_lose_focus),        pWindow);     // Lose focus.

    if (pParent != NULL) {
        gtk_window_set_transient_for(GTK_WINDOW(pGTKWindow), GTK_WINDOW(pParent->pGTKWindow));
        gtk_window_set_destroy_with_parent(GTK_WINDOW(pGTKWindow), true);
    }

    // The application needs to track this window.
    if (pParent == NULL) {
        ak_application_track_top_level_window(pWindow);
    } else {
        ak_append_window(pWindow, pParent);
    }


    return pWindow;
}

void ak_uninit_and_free_window_gtk(ak_window* pWindow)
{
    if (pWindow->pParent == NULL) {
        ak_application_untrack_top_level_window(pWindow);
    } else {
        ak_detach_window(pWindow);
    }


    ak_delete_window_panel(pWindow->pPanel);
    pWindow->pPanel = NULL;

    dr2d_delete_surface(pWindow->pSurface);
    pWindow->pSurface = NULL;

    free(pWindow);
}


PRIVATE ak_window* ak_create_application_window(ak_application* pApplication, ak_window* pParent, size_t extraDataSize, const void* pExtraData)
{
    assert(pApplication != NULL);

    GtkWidget* pGTKWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    if (pGTKWindow == NULL) {
        return NULL;
    }

    ak_window* pWindow = ak_alloc_and_init_window_gtk(pApplication, pParent, ak_window_type_application, pGTKWindow, extraDataSize, pExtraData);
    if (pWindow == NULL) {
        gtk_widget_destroy(pGTKWindow);
        return NULL;
    }

    return pWindow;
}

PRIVATE ak_window* ak_create_child_window(ak_application* pApplication, ak_window* pParent, size_t extraDataSize, const void* pExtraData)
{
    assert(pApplication  != NULL);

    (void)pApplication;
    (void)pParent;
    (void)extraDataSize;
    (void)pExtraData;

    // TODO: Implement Me.
    // gtk_window_set_attached_to()???
    return NULL;
}

PRIVATE ak_window* ak_create_dialog_window(ak_application* pApplication, ak_window* pParent, size_t extraDataSize, const void* pExtraData)
{
    assert(pApplication != NULL);

    // A dialog window must always have a parent.
    if (pParent == NULL) {
        ak_errorf(pApplication, "Attempting to create a dialog window without a parent.");
        return NULL;
    }

    GtkWidget* pGTKWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    if (pGTKWindow == NULL) {
        return NULL;
    }

    gtk_window_set_type_hint(GTK_WINDOW(pGTKWindow), GDK_WINDOW_TYPE_HINT_DIALOG);

    ak_window* pWindow = ak_alloc_and_init_window_gtk(pApplication, pParent, ak_window_type_dialog, pGTKWindow, extraDataSize, pExtraData);
    if (pWindow == NULL) {
        gtk_widget_destroy(pGTKWindow);
        return NULL;
    }

    return pWindow;
}

PRIVATE ak_window* ak_create_popup_window(ak_application* pApplication, ak_window* pParent, size_t extraDataSize, const void* pExtraData)
{
    assert(pApplication != NULL);

    GtkWidget* pGTKWindow = gtk_window_new(GTK_WINDOW_POPUP);
    if (pGTKWindow == NULL) {
        return NULL;
    }

    ak_window* pWindow = ak_alloc_and_init_window_gtk(pApplication, pParent, ak_window_type_popup, pGTKWindow, extraDataSize, pExtraData);
    if (pWindow == NULL) {
        gtk_widget_destroy(pGTKWindow);
        return NULL;
    }

    gtk_window_set_attached_to(GTK_WINDOW(pGTKWindow), pParent->pGTKWindow);

    return pWindow;
}


ak_window* ak_create_window(ak_application* pApplication, ak_window_type type, ak_window* pParent, size_t extraDataSize, const void* pExtraData)
{
    if (pApplication == NULL || type == ak_window_type_unknown) {
        return NULL;
    }

    switch (type)
    {
        case ak_window_type_application:
        {
            return ak_create_application_window(pApplication, pParent, extraDataSize, pExtraData);
        }

        case ak_window_type_child:
        {
            return ak_create_child_window(pApplication, pParent, extraDataSize, pExtraData);
        }

        case ak_window_type_dialog:
        {
            return ak_create_dialog_window(pApplication, pParent, extraDataSize, pExtraData);
        }

        case ak_window_type_popup:
        {
            return ak_create_popup_window(pApplication, pParent, extraDataSize, pExtraData);
        }

        case ak_window_type_unknown:
        default:
        {
            return NULL;
        }
    }
}

void ak_delete_window(ak_window* pWindow)
{
    if (pWindow == NULL/* || pWindow->isMarkedAsDeleted*/) {
        return;
    }

    assert(pWindow->isMarkedAsDeleted == false);        // <-- If you've hit this assert it means you're trying to delete a window multiple times.
    pWindow->isMarkedAsDeleted = true;

    // We just destroy the GtkWindow. This will post a destroy-event message which is where we delete our own window data structure.
    gtk_widget_destroy(pWindow->pGTKWindow);
}





void ak_set_window_title(ak_window* pWindow, const char* pTitle)
{
    if (pWindow == NULL) {
        return;
    }

    gtk_window_set_title(GTK_WINDOW(pWindow->pGTKWindow), pTitle);
}

void ak_get_window_title(ak_window* pWindow, char* pTitleOut, size_t titleOutSize)
{
    if (pTitleOut == NULL || titleOutSize == 0) {
        return;
    }

    if (pWindow == NULL) {
        pTitleOut[0] = '\0';
        return;
    }

    const char* pTitle = gtk_window_get_title(GTK_WINDOW(pWindow->pGTKWindow));
    dr_strncpy_s(pTitleOut, titleOutSize, pTitle, _TRUNCATE);
}


void ak_set_window_size(ak_window* pWindow, int width, int height)
{
    if (pWindow == NULL) {
        return;
    }

    gtk_window_resize(GTK_WINDOW(pWindow->pGTKWindow), width, height);
}

void ak_get_window_size(ak_window* pWindow, int* pWidthOut, int* pHeightOut)
{
    if (pWindow == NULL) {
        return;
    }

    gtk_window_get_size(GTK_WINDOW(pWindow->pGTKWindow), pWidthOut, pHeightOut);
}


void ak_set_window_position(ak_window* pWindow, int posX, int posY)
{
    if (pWindow == NULL) {
        return;
    }

    gtk_window_move(GTK_WINDOW(pWindow->pGTKWindow), posX, posY);
}

void ak_get_window_position(ak_window* pWindow, int* pPosXOut, int* pPosYOut)
{
    if (pWindow == NULL) {
        return;
    }

    gtk_window_get_position(GTK_WINDOW(pWindow->pGTKWindow), pPosXOut, pPosYOut);
}

void ak_center_window(ak_window* pWindow)
{
    if (pWindow == NULL) {
        return;
    }

    gtk_window_set_position(GTK_WINDOW(pWindow->pGTKWindow), GTK_WIN_POS_CENTER);
}


void ak_show_window(ak_window* pWindow)
{
    if (pWindow == NULL) {
        return;
    }

    gtk_window_present(GTK_WINDOW(pWindow->pGTKWindow));
}

void ak_show_window_maximized(ak_window* pWindow)
{
    if (pWindow == NULL) {
        return;
    }

    gtk_window_present(GTK_WINDOW(pWindow->pGTKWindow));    // <-- Is this needed?
    gtk_window_maximize(GTK_WINDOW(pWindow->pGTKWindow));
}

void show_window_sized(ak_window* pWindow, int width, int height)
{
    if (pWindow == NULL) {
        return;
    }

    // Set the size first.
    ak_set_window_size(pWindow, width, height);

    // Now show the window in it's default state.
    ak_show_window(pWindow);
}

void ak_hide_window(ak_window* pWindow, unsigned int flags)
{
    if (pWindow == NULL) {
        return;
    }

    pWindow->onHideFlags = flags;
    gtk_widget_hide(GTK_WIDGET(pWindow->pGTKWindow));
}


bool ak_is_window_descendant(ak_window* pDescendant, ak_window* pAncestor)
{
    if (pDescendant == NULL || pAncestor == NULL) {
        return false;
    }

    return ak_is_window_ancestor(pAncestor, pDescendant);
}

bool ak_is_window_ancestor(ak_window* pAncestor, ak_window* pDescendant)
{
    if (pAncestor == NULL || pDescendant == NULL) {
        return false;
    }

    ak_window* pParent = ak_get_parent_window(pDescendant);
    if (pParent != NULL)
    {
        if (pParent == pAncestor) {
            return true;
        } else {
            return ak_is_window_ancestor(pAncestor, pParent);
        }
    }

    return false;
}


ak_window* ak_get_panel_window(drgui_element* pPanel)
{
    drgui_element* pTopLevelPanel = drgui_find_top_level_element(pPanel);
    if (pTopLevelPanel == NULL) {
        return NULL;
    }

    if (!ak_panel_is_of_type(pTopLevelPanel, "AK.RootWindowPanel")) {
        return NULL;
    }

    ak_element_user_data_gtk* pWindowData = ak_panel_get_extra_data(pTopLevelPanel);
    assert(pWindowData != NULL);

    assert(ak_panel_get_extra_data_size(pTopLevelPanel) == sizeof(ak_element_user_data_gtk));      // A loose check to help ensure we're working with the right kind of panel.
    return pWindowData->pWindow;
}


void ak_set_window_cursor(ak_window* pWindow, ak_cursor_type cursor)
{
    assert(pWindow != NULL);

    switch (cursor)
    {
        case ak_cursor_type_ibeam:
        {
            pWindow->pGTKCursor = g_GTKCursor_IBeam;
        } break;


        case ak_cursor_type_none:
        {
            pWindow->pGTKCursor = NULL;
        } break;

        //case cursor_type_arrow:
        case ak_cursor_type_default:
        default:
        {
            pWindow->pGTKCursor = g_GTKCursor_Default;
        } break;
    }

    // If the cursor is currently inside the window it needs to be changed right now.
    if (ak_is_cursor_over_window(pWindow))
    {
        gdk_window_set_cursor(gtk_widget_get_window(GTK_WIDGET(pWindow->pGTKWindow)), pWindow->pGTKCursor);
    }
}

bool ak_is_cursor_over_window(ak_window* pWindow)
{
    assert(pWindow != NULL);
    return pWindow->isCursorOver;
}


void ak_get_window_dpi(ak_window* pWindow, int* pDPIXOut, int* pDPIYOut)
{
    (void)pWindow;

    if (pDPIXOut) {
        *pDPIXOut = 96;
    }
    if (pDPIYOut) {
        *pDPIYOut = 96;
    }
}

void ak_get_window_dpi_scale(ak_window* pWindow, float* pDPIScaleXOut, float* pDPIScaleYOut)
{
    float scaleX = 1;
    float scaleY = 1;

    if (pWindow != NULL)
    {
        // TODO: Add support for scaling to GTK.
        scaleX = 1;
        scaleY = 1;
    }

    if (pDPIScaleXOut) {
        *pDPIScaleXOut = scaleX;
    }
    if (pDPIScaleYOut) {
        *pDPIScaleYOut = scaleY;
    }
}


PRIVATE void ak_on_global_capture_mouse(drgui_element* pElement)
{
    drgui_element* pTopLevelElement = drgui_find_top_level_element(pElement);
    assert(pTopLevelElement != NULL);

    ak_element_user_data_gtk* pElementData = ak_panel_get_extra_data(pTopLevelElement);
    if (pElementData != NULL) {
        //gdk_device_grab(gdk_device_manager_get_client_pointer(gdk_display_get_device_manager(gdk_display_get_default())),
        //    GDK_WINDOW(pElementData->pWindow->pGTKWindow), GDK_OWNERSHIP_APPLICATION, false, GDK_POINTER_MOTION_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_SCROLL_MASK, NULL, GDK_CURRENT_TIME);
    }
}

PRIVATE void ak_on_global_release_mouse(drgui_element* pElement)
{
    drgui_element* pTopLevelElement = drgui_find_top_level_element(pElement);
    assert(pTopLevelElement != NULL);

    ak_element_user_data_gtk* pElementData = ak_panel_get_extra_data(pTopLevelElement);
    if (pElementData != NULL) {
        //gdk_device_ungrab(gdk_device_manager_get_client_pointer(gdk_display_get_device_manager(gdk_display_get_default())), GDK_CURRENT_TIME);
    }
}

PRIVATE void ak_on_global_capture_keyboard(drgui_element* pElement, drgui_element* pPrevCapturedElement)
{
    (void)pPrevCapturedElement;

    drgui_element* pTopLevelElement = drgui_find_top_level_element(pElement);
    assert(pTopLevelElement != NULL);

    ak_element_user_data_gtk* pElementData = ak_panel_get_extra_data(pTopLevelElement);
    if (pElementData != NULL) {
        gtk_widget_grab_focus(GTK_WIDGET(pElementData->pWindow->pGTKWindow));
    }
}

PRIVATE void ak_on_global_release_keyboard(drgui_element* pElement, drgui_element* pNewCapturedElement)
{
    (void)pNewCapturedElement;

    drgui_element* pTopLevelElement = drgui_find_top_level_element(pElement);
    assert(pTopLevelElement != NULL);

    ak_element_user_data_gtk* pElementData = ak_panel_get_extra_data(pTopLevelElement);
    if (pElementData != NULL) {
        gtk_widget_grab_focus(NULL);
    }
}

PRIVATE void ak_on_global_dirty(drgui_element* pElement, drgui_rect relativeRect)
{
    drgui_element* pTopLevelElement = drgui_find_top_level_element(pElement);
    assert(pTopLevelElement != NULL);

    if (!drgui_is_of_type(pTopLevelElement, "AK.RootWindowPanel")) {
        return;
    }

    ak_element_user_data_gtk* pElementData = ak_panel_get_extra_data(pTopLevelElement);
    if (pElementData != NULL && pElementData->pWindow != NULL && pElementData->pWindow->pGTKWindow != NULL)
    {
        drgui_rect absoluteRect = relativeRect;
        drgui_make_rect_absolute(pElement, &absoluteRect);

        gtk_widget_queue_draw_area(pElementData->pWindow->pGTKWindow,
            (gint)absoluteRect.left, (gint)absoluteRect.top, (gint)(absoluteRect.right - absoluteRect.left), (gint)(absoluteRect.bottom - absoluteRect.top));
    }
}

void ak_gtk_post_quit_message(int resultCode)
{
    g_GTKResultCode = resultCode;
    gtk_main_quit();
}
#endif



//// Functions below are cross-platform ////

ak_application* ak_get_window_application(ak_window* pWindow)
{
    if (pWindow == NULL) {
        return NULL;
    }

    return pWindow->pApplication;
}

ak_window_type ak_get_window_type(ak_window* pWindow)
{
    if (pWindow == NULL) {
        return ak_window_type_unknown;
    }

    return pWindow->type;
}

ak_window* ak_get_parent_window(ak_window* pWindow)
{
    if (pWindow == NULL) {
        return NULL;
    }

    return pWindow->pParent;
}

size_t ak_get_window_extra_data_size(ak_window* pWindow)
{
    if (pWindow == NULL) {
        return 0;
    }

    return pWindow->extraDataSize;
}

void* ak_get_window_extra_data(ak_window* pWindow)
{
    if (pWindow == NULL) {
        return NULL;
    }

    return pWindow->pExtraData;
}

drgui_element* ak_get_window_panel(ak_window* pWindow)
{
    if (pWindow == NULL) {
        return NULL;
    }

    return pWindow->pPanel;
}

dr2d_surface* ak_get_window_surface(ak_window* pWindow)
{
    if (pWindow == NULL) {
        return NULL;
    }

    return pWindow->pSurface;
}


bool ak_set_window_name(ak_window* pWindow, const char* pName)
{
    if (pWindow == NULL) {
        return false;
    }

    if (pName == NULL) {
        return strcpy_s(pWindow->name, sizeof(pWindow->name), "") == 0;
    } else {
        return strcpy_s(pWindow->name, sizeof(pWindow->name), pName) == 0;
    }
}

const char* ak_get_window_name(ak_window* pWindow)
{
    if (pWindow == NULL) {
        return NULL;
    }

    return pWindow->name;
}


void ak_window_set_on_close(ak_window* pWindow, ak_window_on_close_proc proc)
{
    if (pWindow == NULL) {
        return;
    }

    pWindow->onClose = proc;
}

void ak_window_set_on_hide(ak_window* pWindow, ak_window_on_hide_proc proc)
{
    if (pWindow == NULL) {
        return;
    }

    pWindow->onHide = proc;
}

void ak_window_set_on_show(ak_window* pWindow, ak_window_on_show_proc proc)
{
    if (pWindow == NULL) {
        return;
    }

    pWindow->onShow = proc;
}


void ak_window_on_close(ak_window* pWindow)
{
    if (pWindow == NULL) {
        return;
    }

    if (pWindow->onClose) {
        pWindow->onClose(pWindow);
    }
}

bool ak_window_on_hide(ak_window* pWindow, unsigned int flags)
{
    if (pWindow == NULL) {
        return false;
    }

    if (pWindow->onHide) {
        return pWindow->onHide(pWindow, flags);
    }

    return true;
}

bool ak_window_on_show(ak_window* pWindow)
{
    if (pWindow == NULL) {
        return false;
    }

    if (pWindow->onShow) {
        return pWindow->onShow(pWindow);
    }

    return true;
}

void ak_window_on_activate(ak_window* pWindow)
{
    if (pWindow == NULL) {
        return;
    }

    if (pWindow->onActivate) {
        pWindow->onActivate(pWindow);
    }
}

void ak_window_on_deactivate(ak_window* pWindow)
{
    if (pWindow == NULL) {
        return;
    }

    if (pWindow->onDeactivate) {
        pWindow->onDeactivate(pWindow);
    }
}

void ak_window_on_mouse_enter(ak_window* pWindow)
{
    if (pWindow == NULL) {
        return;
    }

    if (pWindow->onMouseEnter) {
        pWindow->onMouseEnter(pWindow);
    }
}

void ak_window_on_mouse_leave(ak_window* pWindow)
{
    if (pWindow == NULL) {
        return;
    }

    if (pWindow->onMouseLeave) {
        pWindow->onMouseLeave(pWindow);
    }
}

void ak_window_on_mouse_button_down(ak_window* pWindow, int mouseButton, int relativeMousePosX, int relativeMousePosY)
{
    if (pWindow == NULL) {
        return;
    }

    if (pWindow->onMouseButtonDown) {
        pWindow->onMouseButtonDown(pWindow, mouseButton, relativeMousePosX, relativeMousePosY);
    }
}

void ak_window_on_mouse_button_up(ak_window* pWindow, int mouseButton, int relativeMousePosX, int relativeMousePosY)
{
    if (pWindow == NULL) {
        return;
    }

    if (pWindow->onMouseButtonUp) {
        pWindow->onMouseButtonUp(pWindow, mouseButton, relativeMousePosX, relativeMousePosY);
    }
}

void ak_window_on_mouse_button_dblclick(ak_window* pWindow, int mouseButton, int relativeMousePosX, int relativeMousePosY)
{
    if (pWindow == NULL) {
        return;
    }

    if (pWindow->onMouseButtonDblClick) {
        pWindow->onMouseButtonDblClick(pWindow, mouseButton, relativeMousePosX, relativeMousePosY);
    }
}

void ak_window_on_mouse_wheel(ak_window* pWindow, int delta, int relativeMousePosX, int relativeMousePosY)
{
    if (pWindow == NULL) {
        return;
    }

    if (pWindow->onMouseWheel) {
        pWindow->onMouseWheel(pWindow, delta, relativeMousePosX, relativeMousePosY);
    }
}

void ak_window_on_key_down(ak_window* pWindow, drgui_key key, int stateFlags)
{
    if (pWindow == NULL) {
        return;
    }

    if (pWindow->onKeyDown) {
        pWindow->onKeyDown(pWindow, key, stateFlags);
    }
}

void ak_window_on_key_up(ak_window* pWindow, drgui_key key, int stateFlags)
{
    if (pWindow == NULL) {
        return;
    }

    if (pWindow->onKeyUp) {
        pWindow->onKeyUp(pWindow, key, stateFlags);
    }
}

void ak_window_on_printable_key_down(ak_window* pWindow, unsigned int character, int stateFlags)
{
    if (pWindow == NULL) {
        return;
    }

    if (pWindow->onPrintableKeyDown) {
        pWindow->onPrintableKeyDown(pWindow, character, stateFlags);
    }
}


void ak_connect_gui_to_window_system(drgui_context* pGUI)
{
    assert(pGUI != NULL);

    drgui_set_global_on_capture_mouse(pGUI, ak_on_global_capture_mouse);
    drgui_set_global_on_release_mouse(pGUI, ak_on_global_release_mouse);
    drgui_set_global_on_capture_keyboard(pGUI, ak_on_global_capture_keyboard);
    drgui_set_global_on_release_keyboard(pGUI, ak_on_global_release_keyboard);
    drgui_set_global_on_dirty(pGUI, ak_on_global_dirty);
}


ak_window* ak_get_first_child_window(ak_window* pWindow)
{
    assert(pWindow != NULL);
    return pWindow->pFirstChild;
}

ak_window* ak_get_last_child_window(ak_window* pWindow)
{
    assert(pWindow != NULL);
    return pWindow->pLastChild;
}


void ak_set_next_sibling_window(ak_window* pWindow, ak_window* pNextWindow)
{
    assert(pWindow != NULL);
    pWindow->pNextSibling = pNextWindow;
}

ak_window* ak_get_next_sibling_window(ak_window* pWindow)
{
    assert(pWindow != NULL);
    return pWindow->pNextSibling;
}

void ak_set_prev_sibling_window(ak_window* pWindow, ak_window* pPrevWindow)
{
    assert(pWindow != NULL);
    pWindow->pPrevSibling = pPrevWindow;
}

ak_window* ak_get_prev_sibling_window(ak_window* pWindow)
{
    assert(pWindow != NULL);
    return pWindow->pPrevSibling;
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
