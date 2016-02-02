// Public domain. See "unlicense" statement at the end of this file.

#include "../include/easy_appkit/ak_window.h"
#include "../include/easy_appkit/ak_application.h"
#include "../include/easy_appkit/ak_build_config.h"
#include "../include/easy_appkit/ak_panel.h"
#include "ak_window_private.h"
#include "ak_application_private.h"
#include <easy_gui/easy_gui.h>
#include <dr_libs/dr_util.h>
#include <assert.h>

#ifdef AK_USE_WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

struct ak_window
{
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



    /// A pointer to the application that owns this window.
    ak_application* pApplication;

    /// The window type.
    ak_window_type type;

    /// The top-level GUI element to draw to this window.
    easygui_element* pPanel;

    /// The easy_draw surface we'll be drawing to.
    easy2d_surface* pSurface;

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



PRIVATE static void ak_on_global_capture_mouse(easygui_element* pElement)
{
    easygui_element* pTopLevelElement = easygui_find_top_level_element(pElement);
    assert(pTopLevelElement != NULL);

    element_user_data* pElementData = ak_panel_get_extra_data(pTopLevelElement);
    if (pElementData != NULL) {
        SetCapture(pElementData->hWnd);
    }
}

PRIVATE static void ak_on_global_release_mouse(easygui_element* pElement)
{
    easygui_element* pTopLevelElement = easygui_find_top_level_element(pElement);
    assert(pTopLevelElement != NULL);

    element_user_data* pElementData = ak_panel_get_extra_data(pTopLevelElement);
    if (pElementData != NULL) {
        ReleaseCapture();
    }
}

PRIVATE static void ak_on_global_capture_keyboard(easygui_element* pElement, easygui_element* pPrevCapturedElement)
{
    (void)pPrevCapturedElement;

    easygui_element* pTopLevelElement = easygui_find_top_level_element(pElement);
    assert(pTopLevelElement != NULL);

    element_user_data* pElementData = ak_panel_get_extra_data(pTopLevelElement);
    if (pElementData != NULL) {
        SetFocus(pElementData->hWnd);
    }
}

PRIVATE static void ak_on_global_release_keyboard(easygui_element* pElement, easygui_element* pNewCapturedElement)
{
    (void)pNewCapturedElement;

    easygui_element* pTopLevelElement = easygui_find_top_level_element(pElement);
    assert(pTopLevelElement != NULL);

    element_user_data* pElementData = ak_panel_get_extra_data(pTopLevelElement);
    if (pElementData != NULL) {
        SetFocus(NULL);
    }
}

PRIVATE static void ak_on_global_dirty(easygui_element* pElement, easygui_rect relativeRect)
{
    easygui_element* pTopLevelElement = easygui_find_top_level_element(pElement);
    assert(pTopLevelElement != NULL);

    element_user_data* pElementData = ak_panel_get_extra_data(pTopLevelElement);
    if (pElementData != NULL)
    {
        easygui_rect absoluteRect = relativeRect;
        easygui_make_rect_absolute(pElement, &absoluteRect);


        RECT rect;
        rect.left   = (LONG)absoluteRect.left;
        rect.top    = (LONG)absoluteRect.top;
        rect.right  = (LONG)absoluteRect.right;
        rect.bottom = (LONG)absoluteRect.bottom;
        InvalidateRect(pElementData->hWnd, &rect, FALSE);
    }
}


PRIVATE easygui_element* ak_create_window_panel(ak_application* pApplication, ak_window* pWindow, HWND hWnd)
{
    easygui_element* pElement = ak_create_panel(pApplication, NULL, sizeof(element_user_data), NULL);
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
    
    easygui_set_inner_scale(pElement, dpiScaleX, dpiScaleY);

    element_user_data* pUserData = ak_panel_get_extra_data(pElement);
    assert(pUserData != NULL);

    pUserData->hWnd    = hWnd;
    pUserData->pWindow = pWindow;

    return pElement;
}

PRIVATE void ak_delete_window_panel(easygui_element* pTopLevelElement)
{
    easygui_delete_element(pTopLevelElement);
}


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




ak_window* ak_alloc_and_init_window_win32(ak_application* pApplication, ak_window* pParent, ak_window_type type, HWND hWnd, size_t extraDataSize, const void* pExtraData)
{
    ak_window* pWindow = malloc(sizeof(*pWindow) + extraDataSize - sizeof(pWindow->pExtraData));
    if (pWindow == NULL)
    {
        ak_errorf(pApplication, "Failed to allocate memory for window.");

        DestroyWindow(hWnd);
        return NULL;
    }

    pWindow->pSurface = easy2d_create_surface_gdi_HWND(ak_get_application_drawing_context(pApplication), hWnd);
    if (pWindow->pSurface == NULL)
    {
        ak_errorf(pApplication, "Failed to create drawing surface for window.");

        free(pWindow);
        DestroyWindow(hWnd);
        return NULL;
    }

    pWindow->pPanel = ak_create_window_panel(pApplication, pWindow, hWnd);
    if (pWindow->pPanel == NULL)
    {
        ak_errorf(pApplication, "Failed to create panel element for window.");

        free(pWindow);
        DestroyWindow(hWnd);
        return NULL;
    }

    // The GUI panel needs to have it's initial size set.
    int windowWidth;
    int windowHeight;
    ak_get_window_size(pWindow, &windowWidth, &windowHeight);
    easygui_set_size(pWindow->pPanel, (float)windowWidth, (float)windowHeight);

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

    easy2d_delete_surface(pWindow->pSurface);
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

easygui_key ak_win32_to_easygui_key(WPARAM wParam)
{
    switch (wParam)
    {
    case VK_BACK:   return EASYGUI_BACKSPACE;
    case VK_SHIFT:  return EASYGUI_SHIFT;
    case VK_ESCAPE: return EASYGUI_ESCAPE;
    case VK_PRIOR:  return EASYGUI_PAGE_UP;
    case VK_NEXT:   return EASYGUI_PAGE_DOWN;
    case VK_END:    return EASYGUI_END;
    case VK_HOME:   return EASYGUI_HOME;
    case VK_LEFT:   return EASYGUI_ARROW_LEFT;
    case VK_UP:     return EASYGUI_ARROW_UP;
    case VK_RIGHT:  return EASYGUI_ARROW_RIGHT;
    case VK_DOWN:   return EASYGUI_ARROW_DOWN;
    case VK_DELETE: return EASYGUI_DELETE;

    default: break;
    }

    return (easygui_key)wParam;
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
                return 1;       // Never draw the background of the window - always leave that to easy_gui.
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

                easygui_post_inbound_event_mouse_move(pWindow->pPanel, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), ak_win32_get_mouse_event_state_flags(wParam));
                break;
            }


            case WM_NCLBUTTONDOWN:
            {
                POINT p;
                p.x = GET_X_LPARAM(lParam);
                p.y = GET_Y_LPARAM(lParam);
                ScreenToClient(hWnd, &p);

                ak_application_on_mouse_button_down(pWindow, EASYGUI_MOUSE_BUTTON_LEFT, p.x, p.y, ak_win32_get_mouse_event_state_flags(wParam) | AK_MOUSE_BUTTON_LEFT_DOWN);
                break;
            }
            case WM_NCLBUTTONUP:
            {
                POINT p;
                p.x = GET_X_LPARAM(lParam);
                p.y = GET_Y_LPARAM(lParam);
                ScreenToClient(hWnd, &p);

                ak_application_on_mouse_button_up(pWindow, EASYGUI_MOUSE_BUTTON_LEFT, p.x, p.y, ak_win32_get_mouse_event_state_flags(wParam));
                break;
            }
            case WM_NCLBUTTONDBLCLK:
            {
                POINT p;
                p.x = GET_X_LPARAM(lParam);
                p.y = GET_Y_LPARAM(lParam);
                ScreenToClient(hWnd, &p);

                ak_application_on_mouse_button_down(pWindow, EASYGUI_MOUSE_BUTTON_LEFT, p.x, p.y, ak_win32_get_mouse_event_state_flags(wParam) | AK_MOUSE_BUTTON_LEFT_DOWN);
                ak_application_on_mouse_button_dblclick(pWindow, EASYGUI_MOUSE_BUTTON_LEFT, p.x, p.y, ak_win32_get_mouse_event_state_flags(wParam) | AK_MOUSE_BUTTON_LEFT_DOWN);
                break;
            }

            case WM_LBUTTONDOWN:
            {
                ak_application_on_mouse_button_down(pWindow, EASYGUI_MOUSE_BUTTON_LEFT, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), ak_win32_get_mouse_event_state_flags(wParam) | AK_MOUSE_BUTTON_LEFT_DOWN);
                break;
            }
            case WM_LBUTTONUP:
            {
                ak_application_on_mouse_button_up(pWindow, EASYGUI_MOUSE_BUTTON_LEFT, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), ak_win32_get_mouse_event_state_flags(wParam));
                break;
            }
            case WM_LBUTTONDBLCLK:
            {
                ak_application_on_mouse_button_down(pWindow, EASYGUI_MOUSE_BUTTON_LEFT, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), ak_win32_get_mouse_event_state_flags(wParam) | AK_MOUSE_BUTTON_LEFT_DOWN);
                ak_application_on_mouse_button_dblclick(pWindow, EASYGUI_MOUSE_BUTTON_LEFT, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), ak_win32_get_mouse_event_state_flags(wParam) | AK_MOUSE_BUTTON_LEFT_DOWN);
                break;
            }


            case WM_NCRBUTTONDOWN:
            {
                POINT p;
                p.x = GET_X_LPARAM(lParam);
                p.y = GET_Y_LPARAM(lParam);
                ScreenToClient(hWnd, &p);

                ak_application_on_mouse_button_down(pWindow, EASYGUI_MOUSE_BUTTON_RIGHT, p.x, p.y, ak_win32_get_mouse_event_state_flags(wParam) | AK_MOUSE_BUTTON_RIGHT_DOWN);
                break;
            }
            case WM_NCRBUTTONUP:
            {
                POINT p;
                p.x = GET_X_LPARAM(lParam);
                p.y = GET_Y_LPARAM(lParam);
                ScreenToClient(hWnd, &p);

                ak_application_on_mouse_button_up(pWindow, EASYGUI_MOUSE_BUTTON_RIGHT, p.x, p.y, ak_win32_get_mouse_event_state_flags(wParam));
                break;
            }
            case WM_NCRBUTTONDBLCLK:
            {
                POINT p;
                p.x = GET_X_LPARAM(lParam);
                p.y = GET_Y_LPARAM(lParam);
                ScreenToClient(hWnd, &p);

                ak_application_on_mouse_button_down(pWindow, EASYGUI_MOUSE_BUTTON_RIGHT, p.x, p.y, ak_win32_get_mouse_event_state_flags(wParam) | AK_MOUSE_BUTTON_RIGHT_DOWN);
                ak_application_on_mouse_button_dblclick(pWindow, EASYGUI_MOUSE_BUTTON_RIGHT, p.x, p.y, ak_win32_get_mouse_event_state_flags(wParam) | AK_MOUSE_BUTTON_RIGHT_DOWN);
                break;
            }

            case WM_RBUTTONDOWN:
            {
                ak_application_on_mouse_button_down(pWindow, EASYGUI_MOUSE_BUTTON_RIGHT, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), ak_win32_get_mouse_event_state_flags(wParam) | AK_MOUSE_BUTTON_RIGHT_DOWN);
                break;
            }
            case WM_RBUTTONUP:
            {
                ak_application_on_mouse_button_up(pWindow, EASYGUI_MOUSE_BUTTON_RIGHT, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), ak_win32_get_mouse_event_state_flags(wParam));
                break;
            }
            case WM_RBUTTONDBLCLK:
            {
                ak_application_on_mouse_button_down(pWindow, EASYGUI_MOUSE_BUTTON_RIGHT, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), ak_win32_get_mouse_event_state_flags(wParam) | AK_MOUSE_BUTTON_RIGHT_DOWN);
                ak_application_on_mouse_button_dblclick(pWindow, EASYGUI_MOUSE_BUTTON_RIGHT, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), ak_win32_get_mouse_event_state_flags(wParam) | AK_MOUSE_BUTTON_RIGHT_DOWN);
                break;
            }


            case WM_NCMBUTTONDOWN:
            {
                POINT p;
                p.x = GET_X_LPARAM(lParam);
                p.y = GET_Y_LPARAM(lParam);
                ScreenToClient(hWnd, &p);

                ak_application_on_mouse_button_down(pWindow, EASYGUI_MOUSE_BUTTON_MIDDLE, p.x, p.y, ak_win32_get_mouse_event_state_flags(wParam) | AK_MOUSE_BUTTON_MIDDLE_DOWN);
                break;
            }
            case WM_NCMBUTTONUP:
            {
                POINT p;
                p.x = GET_X_LPARAM(lParam);
                p.y = GET_Y_LPARAM(lParam);
                ScreenToClient(hWnd, &p);

                ak_application_on_mouse_button_up(pWindow, EASYGUI_MOUSE_BUTTON_MIDDLE, p.x, p.y, ak_win32_get_mouse_event_state_flags(wParam));
                break;
            }
            case WM_NCMBUTTONDBLCLK:
            {
                POINT p;
                p.x = GET_X_LPARAM(lParam);
                p.y = GET_Y_LPARAM(lParam);
                ScreenToClient(hWnd, &p);

                ak_application_on_mouse_button_down(pWindow, EASYGUI_MOUSE_BUTTON_MIDDLE, p.x, p.y, ak_win32_get_mouse_event_state_flags(wParam) | AK_MOUSE_BUTTON_MIDDLE_DOWN);
                ak_application_on_mouse_button_dblclick(pWindow, EASYGUI_MOUSE_BUTTON_MIDDLE, p.x, p.y, ak_win32_get_mouse_event_state_flags(wParam) | AK_MOUSE_BUTTON_MIDDLE_DOWN);
                break;
            }

            case WM_MBUTTONDOWN:
            {
                ak_application_on_mouse_button_down(pWindow, EASYGUI_MOUSE_BUTTON_MIDDLE, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), ak_win32_get_mouse_event_state_flags(wParam) | AK_MOUSE_BUTTON_MIDDLE_DOWN);
                break;
            }
            case WM_MBUTTONUP:
            {
                ak_application_on_mouse_button_up(pWindow, EASYGUI_MOUSE_BUTTON_MIDDLE, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), ak_win32_get_mouse_event_state_flags(wParam));
                break;
            }
            case WM_MBUTTONDBLCLK:
            {
                ak_application_on_mouse_button_down(pWindow, EASYGUI_MOUSE_BUTTON_MIDDLE, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), ak_win32_get_mouse_event_state_flags(wParam) | AK_MOUSE_BUTTON_MIDDLE_DOWN);
                ak_application_on_mouse_button_dblclick(pWindow, EASYGUI_MOUSE_BUTTON_MIDDLE, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), ak_win32_get_mouse_event_state_flags(wParam) | AK_MOUSE_BUTTON_MIDDLE_DOWN);
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

                        ak_application_on_key_down(pWindow, ak_win32_to_easygui_key(wParam), stateFlags);
                    }

                    break;
                }

            case WM_KEYUP:
                {
                    if (!ak_is_win32_mouse_button_key_code(wParam))
                    {
                        int stateFlags = ak_win32_get_modifier_key_state_flags();
                        ak_application_on_key_up(pWindow, ak_win32_to_easygui_key(wParam), stateFlags);
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
                                character = utf16pair_to_utf32(pWindow->utf16HighSurrogate, (unsigned short)wParam);
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
                easygui_set_size(pWindow->pPanel, LOWORD(lParam), HIWORD(lParam));
                break;
            }

            case WM_PAINT:
            {
                RECT rect;
                if (GetUpdateRect(hWnd, &rect, FALSE)) {
                    easygui_draw(pWindow->pPanel, easygui_make_rect((float)rect.left, (float)rect.top, (float)rect.right, (float)rect.bottom), pWindow->pSurface);
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

easygui_element* ak_get_window_panel(ak_window* pWindow)
{
    if (pWindow == NULL) {
        return NULL;
    }

    return pWindow->pPanel;
}

easy2d_surface* ak_get_window_surface(ak_window* pWindow)
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


ak_window* ak_get_panel_window(easygui_element* pPanel)
{
    easygui_element* pTopLevelPanel = easygui_find_top_level_element(pPanel);
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
        win32_get_system_dpi(pDPIXOut, pDPIYOut);
        return;
    }

    PFN_GetDpiForMonitor _GetDpiForMonitor = (PFN_GetDpiForMonitor)GetProcAddress(hSHCoreDLL, "GetDpiForMonitor");
    if (_GetDpiForMonitor == NULL)
    {
        win32_get_system_dpi(pDPIXOut, pDPIYOut);
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
        win32_get_system_dpi(pDPIXOut, pDPIYOut);
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
        win32_get_base_dpi(&baseDPIX, &baseDPIY);

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

void ak_window_on_key_down(ak_window* pWindow, easygui_key key, int stateFlags)
{
    if (pWindow == NULL) {
        return;
    }

    if (pWindow->onKeyDown) {
        pWindow->onKeyDown(pWindow, key, stateFlags);
    }
}

void ak_window_on_key_up(ak_window* pWindow, easygui_key key, int stateFlags)
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







///////////////////////////////////////////////////////////////////////////////
//
// Private APIs
//
///////////////////////////////////////////////////////////////////////////////

unsigned int g_Win32ClassRegCounter = 0;

bool ak_win32_register_window_classes()
{
    if (g_Win32ClassRegCounter > 0)
    {
        g_Win32ClassRegCounter += 1;
        return true;
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
    if (!RegisterClassExA(&wc))
    {
        return false;
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
    if (!RegisterClassExA(&wcDialog))
    {
        UnregisterClassA(g_WindowClass, NULL);
        return false;
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
    if (!RegisterClassExA(&wcPopup))
    {
        UnregisterClassA(g_WindowClass, NULL);
        UnregisterClassA(g_WindowClass_Popup, NULL);
        return false;
    }


    g_Win32ClassRegCounter += 1;
    return true;
}

void ak_win32_unregister_window_classes()
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
#endif



//// Functions below are cross-platform ////

void ak_connect_gui_to_window_system(easygui_context* pGUI)
{
    assert(pGUI != NULL);

    easygui_set_global_on_capture_mouse(pGUI, ak_on_global_capture_mouse);
    easygui_set_global_on_release_mouse(pGUI, ak_on_global_release_mouse);
    easygui_set_global_on_capture_keyboard(pGUI, ak_on_global_capture_keyboard);
    easygui_set_global_on_release_keyboard(pGUI, ak_on_global_release_keyboard);
    easygui_set_global_on_dirty(pGUI, ak_on_global_dirty);
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