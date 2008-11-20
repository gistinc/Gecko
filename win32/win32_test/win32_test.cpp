// win32_test.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "win32_test.h"
#include "embed.h"

#include <iostream>
#include <set>
using namespace std;

#define MAX_LOADSTRING 100

class MyListener : public MozViewListener
{
public:
    void SetTitle(const char* newTitle);
    void StatusChanged(const char* newStatus, PRUint32 statusType);
    void LocationChanged(const char* newLocation);
    PRBool OpenURI(const char* newLocation);
    void DocumentLoaded();
    MozView* OpenWindow(PRUint32 flags);
    void SizeTo(PRUint32 width, PRUint32 height);
    void SetVisibility(PRBool visible);
    void StartModal();
    void ExitModal(nsresult result);
};

// Global Variables:
HINSTANCE hInst;                // current instance
TCHAR szTitle[MAX_LOADSTRING];          // The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];      // the main window class name
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

void MyListener::SetTitle(const char *newTitle)
{
    HWND hWnd = (HWND)pMozView->GetParentWindow();
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

    pNewView->SetParentView(pMozView);
    pNewView->SetListener(new MyListener());

    SetWindowLongPtr(hWnd, GWLP_USERDATA, (__int3264)(LONG_PTR)(pNewView));

    gViews.insert(pNewView);
    return pNewView;
}

void MyListener::SizeTo(PRUint32 width, PRUint32 height)
{
    HWND hParentWnd = (HWND)pMozView->GetParentWindow();
    HWND hWnd = (HWND)pMozView->GetNativeWindow();
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
    HWND hWnd = (HWND)pMozView->GetParentWindow();
    ::ShowWindow(hWnd, visible ? SW_SHOW : SW_HIDE);
}

void MyListener::StartModal()
{
    gDoModal = true;
    MSG msg;
    MozView* parentView = pMozView->GetParentView();
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
    MozView* parentView = pMozView->GetParentView();
    ::EnableWindow((HWND)parentView->GetParentWindow(), TRUE);
    HWND hWnd = (HWND)pMozView->GetParentWindow();
    DestroyWindow(hWnd);
}

// Forward declarations of functions included in this code module:
ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);

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
    //mozView->LoadURI("file:///C:/mozilla/test/test.html");
    //mozView->LoadURI("chrome://test/content");

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
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    int wmId, wmEvent;

    MozView* pMozView = (MozView*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

    switch (message) {
    case WM_COMMAND:
        wmId = LOWORD(wParam);
        wmEvent = HIWORD(wParam);
        // Parse the menu selections:
        switch (wmId) {
        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        case IDM_EXIT:
        {
            MozView* parentView = pMozView->GetParentView();
            if (parentView && gDoModal)
                ::EnableWindow((HWND)parentView->GetParentWindow(), TRUE);
            DestroyWindow(hWnd);
            break;
        }
        case IDM_VIEW_STOP:
            pMozView->Stop();
            break;
        case IDM_VIEW_RELOAD:
            pMozView->Reload();
            break;
        case IDM_HISTORY_BACK:
            pMozView->GoBack();
            break;
        case IDM_HISTORY_FORWARD:
            pMozView->GoForward();
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;
    case WM_CLOSE:
    {
        MozView* parentView = pMozView->GetParentView();
        if (parentView && gDoModal)
            ::EnableWindow((HWND)parentView->GetParentWindow(), TRUE);
        return DefWindowProc(hWnd, message, wParam, lParam);
        break;
    }
    case WM_DESTROY:
        if (gViews.erase(pMozView) > 0) {
            pMozView->Stop();
            delete pMozView->GetListener();
            delete pMozView;
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
        if (pMozView) {
            RECT rect;
            GetClientRect(hWnd, &rect);
            pMozView->SetPositionAndSize(rect.left, rect.top, 
                    rect.right - rect.left, rect.bottom - rect.top);
        }
        break;
    case WM_ACTIVATE:
        if (pMozView) {
            switch (wParam) {
            case WA_CLICKACTIVE:
            case WA_ACTIVE:
                pMozView->SetFocus(true);
                break;
            case WA_INACTIVE:
                pMozView->SetFocus(false);
                break;
            default:
                break;
            }
        }
        break;
    case WM_INITMENUPOPUP:
    {
        HMENU hMenu = (HMENU)wParam;
        ::EnableMenuItem(hMenu, IDM_HISTORY_BACK, (pMozView->CanGoBack() ? MF_ENABLED : (MF_DISABLED | MF_GRAYED)));
        ::EnableMenuItem(hMenu, IDM_HISTORY_FORWARD, (pMozView->CanGoForward() ? MF_ENABLED : (MF_DISABLED | MF_GRAYED)));
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
