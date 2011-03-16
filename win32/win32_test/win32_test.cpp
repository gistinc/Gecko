/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is
 * Mozilla Corporation.
 * Portions created by the Initial Developer are Copyright (C) 2009
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *    Anton Rogaynis <wildriding@gmail.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#include "stdafx.h"
#include "win32_test.h"
#include "embed.h"

#include <iostream>
#include <set>
using namespace std;

#define MAX_LOADSTRING 100
#define MAX_LOCATION 1024

class MyListener : public MozViewListener
{
public:
    void SetTitle(const char* newTitle);
    void StatusChanged(const char* newStatus, PRUint32 statusType);
    void LocationChanged(const char* newLocation);
    PRBool OpenURI(const char* newLocation);
    void DocumentLoaded();
    void OnConsoleMessage(const char * aMessage);
    void OnFocusChanged(PRBool aForward);

    MozView* OpenWindow(PRUint32 flags);
    void SizeTo(PRUint32 width, PRUint32 height);
    void SetVisibility(PRBool visible);
    void StartModal();
    void ExitModal(nsresult result);
};

// Global variables:
HINSTANCE hInst;                      // Current instance
TCHAR szTitle[MAX_LOADSTRING];        // The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];  // The main window class name
HWND hMainWnd;
set<MozView*> gViews;
HACCEL hAccelTable;
bool gDoModal = false;
bool gQuit = false;

WCHAR* Utf8ToWchar(const char* str)
{
    int len = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
    WCHAR* result = new WCHAR[len];
    MultiByteToWideChar(CP_UTF8, 0, str, -1, result, len);
    return result;
}

char* WcharToUtf8(WCHAR* str)
{
    int len = WideCharToMultiByte(CP_UTF8, 0, str, -1, NULL, 0, NULL, NULL);
    char *result = new char[len];
    WideCharToMultiByte(CP_UTF8, 0, str, -1, result, len, NULL, NULL);
    return result;
}

void MyListener::SetTitle(const char *newTitle)
{
    HWND hWnd = (HWND)mMozView->GetParentWindow();
    WCHAR* newTitleW = Utf8ToWchar(newTitle);
    ::SetWindowTextW(hWnd, newTitleW);
    delete[] newTitleW;
}

void MyListener::StatusChanged(const char *newStatus, PRUint32 statusType)
{
    WCHAR* newStatusW = Utf8ToWchar(newStatus);
    wcout << "STATUS:" << newStatusW << endl;
    delete[] newStatusW;
}

void MyListener::LocationChanged(const char *newLocation)
{
    WCHAR* newLocationW = Utf8ToWchar(newLocation);
    wcout << "LOCATION:" << newLocationW << endl;
    delete[] newLocationW;
}

PRBool MyListener::OpenURI(const char* newLocation)
{
    WCHAR* newLocationW = Utf8ToWchar(newLocation);
    wcout << "OPEN URI:" << newLocationW << endl;
    delete[] newLocationW;
    return false;
}

void MyListener::DocumentLoaded()
{
    cout << "FINISHED" << endl;
}

void MyListener::OnConsoleMessage(const char * aMessage)
{
    cout << "CONSOLE: " << aMessage << endl;
}

void MyListener::OnFocusChanged(PRBool aForward)
{
}

MozView* MyListener::OpenWindow(PRUint32 flags)
{
    HWND hWnd;

    hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInst, NULL);

    if (!hWnd) {
        return 0;
    }

    ShowWindow(hWnd, SW_SHOW);
    UpdateWindow(hWnd);

    RECT rect;
    GetClientRect(hWnd, &rect);

    MozView* pNewView = new MozView();
    int res = pNewView->CreateBrowser(hWnd, rect.left, rect.top,
            rect.right - rect.left, rect.bottom - rect.top, flags);

    if (res) {
        return 0;
    }

    pNewView->SetParentView(mMozView);
    pNewView->SetListener(new MyListener());

    SetWindowLongPtr(hWnd, GWLP_USERDATA, (__int3264)(LONG_PTR)(pNewView));

    gViews.insert(pNewView);
    return pNewView;
}

void MyListener::SizeTo(PRUint32 width, PRUint32 height)
{
    HWND hParentWnd = (HWND)mMozView->GetParentWindow();
    HWND hWnd = (HWND)mMozView->GetNativeWindow();
    RECT parentRect;
    RECT rect;
    ::GetWindowRect(hParentWnd, &parentRect);
    ::GetWindowRect(hWnd, &rect);
    ::SetWindowPos(hParentWnd, 0, 0, 0,
        width + (parentRect.right - parentRect.left) - (rect.right - rect.left),
        height + (parentRect.bottom - parentRect.top) - (rect.bottom - rect.top),
        SWP_NOMOVE | SWP_NOZORDER);
}

void MyListener::SetVisibility(PRBool visible)
{
    HWND hWnd = (HWND)mMozView->GetParentWindow();
    ::ShowWindow(hWnd, visible ? SW_SHOW : SW_HIDE);
}

void MyListener::StartModal()
{
    gDoModal = true;
    MSG msg;
    MozView* parentView = mMozView->GetParentView();
    ::EnableWindow((HWND)parentView->GetParentWindow(), FALSE);
    int res;
    while (gDoModal && (res = GetMessage(&msg, NULL, 0, 0))) {
        if (res == -1) {
            printf("ERROR: GetMessage == -1\n");
            break;
        }
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
}

void MyListener::ExitModal(nsresult result)
{
    MozView* parentView = mMozView->GetParentView();
    ::EnableWindow((HWND)parentView->GetParentWindow(), TRUE);
    HWND hWnd = (HWND)mMozView->GetParentWindow();
    DestroyWindow(hWnd);
}

// Forward declarations of functions included in this code module:
ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK OpenLocation(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(HINSTANCE hInstance,
                       HINSTANCE hPrevInstance,
                       LPTSTR    lpCmdLine,
                       int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.
    MSG msg;

    // Initialize global strings
    LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadString(hInstance, IDC_WIN32_TEST, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow)) {
        return FALSE;
    }

    hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WIN32_TEST));

    RECT rect;
    GetClientRect(hMainWnd, &rect);

    MozApp mozApp;

    MozView* mozView = new MozView();
    gViews.insert(mozView);
    int res = mozView->CreateBrowser(hMainWnd, rect.left, rect.top,
            rect.right - rect.left, rect.bottom - rect.top);

    if (res) {
        return res;
    }

    mozView->SetListener(new MyListener);

    SetWindowLongPtr(hMainWnd, GWLP_USERDATA, (__int3264)(LONG_PTR)(mozView));

    mozView->LoadURI("http://google.com");

    // Main message loop:
    while (!gQuit) {
        WaitMessage();
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                gQuit = true;
                break;
            }
            if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
    }

    // delete extra views
    set<MozView*>::const_iterator itr;
    for (itr = gViews.begin(); itr != gViews.end(); ++itr) {
        delete *itr;
    }

    return (int) msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style      = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc  = WndProc;
    wcex.cbClsExtra    = 0;
    wcex.cbWndExtra    = 0;
    wcex.hInstance    = hInstance;
    wcex.hIcon      = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WIN32_TEST));
    wcex.hCursor    = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName  = MAKEINTRESOURCE(IDC_WIN32_TEST);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm    = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // Store instance handle in our global variable

    hMainWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
                            CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL,
                            hInstance, NULL);

    if (!hMainWnd) {
        return FALSE;
    }

    ShowWindow(hMainWnd, nCmdShow);
    UpdateWindow(hMainWnd);

    return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT  - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    int wmId, wmEvent;

    MozView* mMozView = (MozView*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

    switch (message) {
    case WM_COMMAND:
        wmId = LOWORD(wParam);
        wmEvent = HIWORD(wParam);
        // Parse the menu selections:
        switch (wmId) {
        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        case IDM_OPENLOCATION:
            DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_OPENLOCATION), hWnd, OpenLocation, (LONG_PTR)mMozView);
            break;
        case IDM_EXIT:
        {
            MozView* parentView = mMozView->GetParentView();
            if (parentView && gDoModal)
                ::EnableWindow((HWND)parentView->GetParentWindow(), TRUE);
            DestroyWindow(hWnd);
            break;
        }
        case IDM_VIEW_STOP:
            mMozView->Stop();
            break;
        case IDM_VIEW_RELOAD:
            mMozView->Reload();
            break;
        case IDM_HISTORY_BACK:
            mMozView->GoBack();
            break;
        case IDM_HISTORY_FORWARD:
            mMozView->GoForward();
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;
    case WM_CLOSE:
    {
        MozView* parentView = mMozView->GetParentView();
        if (parentView && gDoModal)
            ::EnableWindow((HWND)parentView->GetParentWindow(), TRUE);
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    case WM_DESTROY:
        if (gViews.erase(mMozView) > 0) {
            mMozView->Stop();
            delete mMozView->GetListener();
            delete mMozView;
        }

        if (gDoModal) {
            gDoModal = false;
        }

        if (gViews.size() == 0) {
            PostQuitMessage(0);
            gQuit = true;
        }
        break;
    case WM_SIZE:
        if (mMozView) {
            RECT rect;
            GetClientRect(hWnd, &rect);
            mMozView->SetPositionAndSize(rect.left, rect.top,
                    rect.right - rect.left, rect.bottom - rect.top);
        }
        break;
    case WM_ACTIVATE:
        if (mMozView) {
            switch (wParam) {
            case WA_CLICKACTIVE:
            case WA_ACTIVE:
                mMozView->SetFocus(true);
                break;
            case WA_INACTIVE:
                mMozView->SetFocus(false);
                break;
            default:
                break;
            }
        }
        break;
    case WM_INITMENUPOPUP:
    {
        HMENU hMenu = (HMENU)wParam;
        ::EnableMenuItem(hMenu, IDM_HISTORY_BACK, (mMozView->CanGoBack() ? MF_ENABLED : (MF_DISABLED | MF_GRAYED)));
        ::EnableMenuItem(hMenu, IDM_HISTORY_FORWARD, (mMozView->CanGoForward() ? MF_ENABLED : (MF_DISABLED | MF_GRAYED)));
        break;
    }
    case WM_ERASEBKGND:
        // Reduce flicker by not painting the non-visible background
        return 1;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Handler for Open Location dialog.
INT_PTR CALLBACK OpenLocation(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    MozView *mozView = (MozView*)GetWindowLongPtr(hDlg, GWLP_USERDATA);
    switch (message) {
    case WM_INITDIALOG:
        mozView = (MozView*)lParam;
        if(mozView == NULL)
            EndDialog(hDlg, LOWORD(wParam));

        SetWindowLongPtr( hDlg, GWLP_USERDATA, (__int3264)(LONG_PTR)(mozView));
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDGO) {
            WCHAR urlForm[MAX_LOCATION];
            char *url;

            GetDlgItemText(hDlg, IDC_EDIT1, urlForm, MAX_LOCATION);
            url = WcharToUtf8(urlForm);
            MozView* mozView = (MozView*)GetWindowLongPtr(hDlg, GWLP_USERDATA);
            if(mozView) {
                mozView->LoadURI(url);
            }
            delete[] url;
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }

        if (LOWORD(wParam) == IDCANCEL) {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message) {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}


// for running as console app
int main(int argc, char *argv[])
{
    HINSTANCE hInstanceApp = GetModuleHandle(NULL);
    _tWinMain(hInstanceApp, NULL, NULL, SW_SHOW);
}
