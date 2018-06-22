#include "CSplashWindow.h"
#include "system.h"


// private section //

// static
bool CSplashWindow::s_bRegisterClass = false;

#define ONMETHOD(METHOD_NAME)   LRESULT CALLBACK CSplashWindow::METHOD_NAME(WPARAM wParam, LPARAM lParam)
// static
LRESULT CALLBACK CSplashWindow::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg){
    case WM_CREATE:
        if(!SetTimer(hWnd, IDTIMER_SPLASH, SPLASHWND_LIVETIME, NULL))
            return -1;
        return 0;
    case WM_TIMER:
        KillTimer(hWnd, IDTIMER_SPLASH);
        DestroyWindow(hWnd);
        return 0;
    }

    CSplashWindow *This = (CSplashWindow *)GetWindowLong(hWnd, GWL_USERDATA);
    if(!This)
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    switch(uMsg){
    case WM_PAINT:
        return This->OnPaint(wParam, lParam);
    case WM_LBUTTONUP:
        return This->OnLButtonUp(wParam, lParam);
    case WM_DESTROY:
        This->hWnd = NULL;
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

ONMETHOD(OnPaint)
{
    PAINTSTRUCT ps;
    HDC         hdcMem;

    if(BeginPaint(hWnd, &ps)){
        hdcMem = CreateCompatibleDC(ps.hdc);
        if(hdcMem){
            SelectObject(hdcMem, hBitmap);
            StretchBlt(ps.hdc, 0, 0, SPLASHWND_WIDTH, SPLASHWND_HEIGHT, hdcMem, 0, 0, BitmapInfo.bmWidth, BitmapInfo.bmHeight, SRCCOPY);
            DeleteDC(hdcMem);
        }
        EndPaint(hWnd, &ps);
    }

    return 0;
}

ONMETHOD(OnLButtonUp)
{
    DestroyWindow(hWnd);
    return 0;
}
#undef ONMETHOD

// static
DWORD WINAPI CSplashWindow::ThreadProc(LPVOID lpParameter)
{
    CSplashWindow   *This = (CSplashWindow *)lpParameter;
    RECT    rect;
    int     x, y;
    MSG     msg;
    BOOL    bMsg;

    if(SystemParametersInfo(SPI_GETWORKAREA, 0, &rect, 0)){
        x = (rect.right - rect.left - SPLASHWND_WIDTH)/2;
        y = (rect.bottom - rect.top - SPLASHWND_HEIGHT)/2;
    }else{
        if(x = GetSystemMetrics(SM_CXFULLSCREEN))
            x = (x - SPLASHWND_WIDTH)/2;
        if(y = GetSystemMetrics(SM_CYFULLSCREEN))
            y = (y - SPLASHWND_HEIGHT)/2;
    }

    This->hWnd = CreateWindowEx(WS_EX_TOPMOST | WS_EX_TOOLWINDOW, CNAME_SPLASH, "", WS_POPUP | WS_VISIBLE, x, y, SPLASHWND_WIDTH, SPLASHWND_HEIGHT, This->hWndParent, NULL, This->hInstance, NULL);
    if(!This->hWnd)
        return -1;
    SetLastError(0);
    SetWindowLong(This->hWnd, GWL_USERDATA, (LONG)This);
    if(GetLastError())
        return -1;

    while(bMsg = GetMessage(&msg, NULL, 0, 0), bMsg != 0){
        if(bMsg == -1)
            return -1;
        DispatchMessage(&msg);
    }

    return (DWORD)msg.wParam;
}


// public section //

CSplashWindow::CSplashWindow(HINSTANCE hInstance, HWND hWndParent)
:   hInstance(hInstance), hWndParent(hWndParent), hBitmap(NULL), hWnd(NULL)
{
    bInit = true;
    
    if(!s_bRegisterClass){
        WNDCLASSEX wc;
        ZeroMemory(&wc, sizeof(wc));
        wc.cbSize           = sizeof(wc);
        wc.lpfnWndProc      = WndProc;
        wc.hInstance        = hInstance;
        wc.hCursor          = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground    = (HBRUSH)(COLOR_WINDOW+1);
        wc.lpszClassName    = CNAME_SPLASH;
        if(!RegisterClassEx(&wc)){
            bInit = false;
            return;
        }else{
            s_bRegisterClass = true;
        }
    }

    hBitmap = LoadBitmap(hInstance, "IDB_SPLASH");
    if(!hBitmap){
        bInit = false;
        return;
    }
    if(!GetObject(hBitmap, sizeof(BitmapInfo), &BitmapInfo)){
        bInit = false;
        return;
    }
}

CSplashWindow::~CSplashWindow()
{
    if(hBitmap)
        DeleteObject(hBitmap);
    if(hWnd)
        DestroyWindow(hWnd);
}

bool CSplashWindow::ShowSplash()
{
    DWORD   dwThreadID;
    HANDLE  hThread;

    if(!bInit)
        return false;

    hThread = CreateThread(NULL, 0, ThreadProc, this, 0, &dwThreadID);
    if(!hThread)
        return false;
    CloseHandle(hThread);
    return true;
}
