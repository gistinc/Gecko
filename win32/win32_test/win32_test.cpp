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
};

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
HWND hWnd;
set<MozView*> gViews;

void MyListener::SetTitle(const char *newTitle)
{
  HWND hWnd = (HWND)pMozView->GetNativeWindow();
  ::SetWindowTextA(hWnd, newTitle);
}

void MyListener::StatusChanged(const char *newStatus, PRUint32 statusType)
{
  cout << "STATUS:" << newStatus << endl;
}

void MyListener::LocationChanged(const char *newLocation)
{
  cout << "LOCATION:" << newLocation << endl;
}

PRBool MyListener::OpenURI(const char* newLocation)
{
  cout << "OPEN URI:" << newLocation << endl;
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

  if (!hWnd)
  {
    return 0;
  }

  ShowWindow(hWnd, SW_SHOW);
  UpdateWindow(hWnd);

  RECT rect;
  GetClientRect(hWnd, &rect);

  MozView* pNewView = new MozView();
  int res = pNewView->CreateBrowser(hWnd, rect.left, rect.top,
      rect.right - rect.left, rect.bottom - rect.top, flags);

  if(res)
      return 0;

  pNewView->SetListener(this);

  SetWindowLongPtr(hWnd, GWLP_USERDATA, (__int3264)(LONG_PTR)(pNewView));

  gViews.insert(pNewView);
  return pNewView;
}

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_WIN32_TEST, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WIN32_TEST));

    RECT rect;
    GetClientRect(hWnd, &rect);

    MozView mozView;
    int res = mozView.CreateBrowser(hWnd, rect.left, rect.top,
        rect.right - rect.left, rect.bottom - rect.top);

    if(res)
        return res;

    MyListener myListener;
    mozView.SetListener(&myListener);

    SetWindowLongPtr(hWnd, GWLP_USERDATA, (__int3264)(LONG_PTR)(&mozView));

    mozView.LoadURI("http://google.com");
    //mozView.LoadURI("file:///C:/mozilla/test/test.html");
    //mozView.LoadURI("chrome://test/content");

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

  // delete extra views
  set<MozView*>::const_iterator itr;
  for(itr = gViews.begin(); itr != gViews.end(); ++itr) {
    delete *itr;
  }

    /*
    HANDLE hFakeEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
    PRBool aRunCondition = PR_TRUE;

    while (aRunCondition ) {
        // Process pending messages
        while (::PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) {
            if (!::GetMessage(&msg, NULL, 0, 0)) {
            // WM_QUIT
                aRunCondition = PR_FALSE;
                break;
            }
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
        }

        // Do idle stuff
        ::MsgWaitForMultipleObjects(1, &hFakeEvent, FALSE, 100, QS_ALLEVENTS);
    }
    ::CloseHandle(hFakeEvent);
    */

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

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WIN32_TEST));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_WIN32_TEST);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

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

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;

  MozView* pMozView = (MozView*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
      if(gViews.erase(pMozView) > 0) {
        delete pMozView;
      }
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
  case WM_SYSCOMMAND:
    if (wParam == SC_CLOSE)
    {
      if(gViews.erase(pMozView) > 0) {
        LRESULT res = DefWindowProc(hWnd, message, wParam, lParam);
        delete pMozView;
        return res;
      }
      return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
    case WM_SIZE:
        {
            
            if(pMozView)
            {
                RECT rect;
                GetClientRect(hWnd, &rect);
                pMozView->SetPositionAndSize(rect.left, rect.top,
                    rect.right - rect.left, rect.bottom - rect.top);
            }
        }
        break;
    case WM_ACTIVATE:
        {
            if(pMozView)
            {
                switch (wParam)
                {
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
        }
        break;
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
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
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
