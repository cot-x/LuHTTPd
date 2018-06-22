#include "CMain.h"
#include "system.h"
#include "Resource.h"
#include <new>


// private section //

// static
bool CMain::s_bRegisterClass = false;

#define ONMETHOD(METHOD_NAME)   LRESULT CALLBACK CMain::METHOD_NAME(WPARAM wParam, LPARAM lParam)
// static
LRESULT CALLBACK CMain::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CMain *This = (CMain *)GetWindowLong(hWnd, GWL_USERDATA);
    if(!This)
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    switch(uMsg){
    case WM_COMMAND:
        switch(LOWORD(wParam)){
        case IDM_EXIT:
            return This->OnTrayMenuExit(wParam, lParam);
        case IDM_ABOUT:
            return This->OnTrayMenuAbout(wParam, lParam);
        case IDM_OPENWWWROOT:
            return This->OnTrayMenuOpenWWWRoot(wParam, lParam);
        case IDM_OPENLOG:
            return This->OnTrayMenuOpenLog(wParam, lParam);
        }
        break;
    case MSGT_MAIN:
        switch(lParam){
        case WM_RBUTTONUP:
            return This->OnTrayRButtonUp(wParam, lParam);
        case WM_LBUTTONDBLCLK:
            return This->OnTrayLButtonDBLCLK(wParam, lParam);
        }
        break;
    case WM_DESTROY:
        This->hWnd = NULL;
        PostQuitMessage(0);
        break;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

ONMETHOD(OnTrayMenuExit)
{
    DestroyWindow(hWnd);
    return 0;
}

ONMETHOD(OnTrayMenuAbout)
{
    DialogBox(hInstance, "IDD_ABOUT", hWnd, AboutDlgProc);
    return 0;
}

ONMETHOD(OnTrayMenuOpenWWWRoot)
{
    if(ShellExecute(hWnd, NULL, WWWROOTDIRECTORY, NULL, NULL, SW_SHOWNORMAL)==(HINSTANCE)SE_ERR_FNF){
        CreateDirectory(WWWROOTDIRECTORY, NULL);
        ShellExecute(hWnd, NULL, WWWROOTDIRECTORY, NULL, NULL, SW_SHOWNORMAL);
    }

    return 0;
}

ONMETHOD(OnTrayMenuOpenLog)
{
    ShellExecute(hWnd, NULL, LOGFILENAME, NULL, NULL, SW_SHOWNORMAL);
    return 0;
}

ONMETHOD(OnTrayRButtonUp)
{
    POINT   point;
    SIZE    ScreenSize;
    UINT    uFlags=0;

    GetCursorPos(&point);
    ScreenSize.cx = GetSystemMetrics(SM_CXSCREEN);
    ScreenSize.cy = GetSystemMetrics(SM_CYSCREEN);
    if(ScreenSize.cx/2 <= point.x)
        uFlags += TPM_RIGHTALIGN;
    else
        uFlags += TPM_LEFTALIGN;
    if(ScreenSize.cy/2 <= point.y)
        uFlags += TPM_BOTTOMALIGN;
    else
        uFlags += TPM_TOPALIGN;
    SetForegroundWindow(hWnd);
    TrackPopupMenu(hPopMenu, uFlags, point.x, point.y, 0, hWnd, NULL);

    return 0;
}

ONMETHOD(OnTrayLButtonDBLCLK)
{
    OnTrayMenuOpenWWWRoot(wParam, lParam);
    return 0;
}
#undef ONMETHOD

// static
BOOL CALLBACK CMain::AboutDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg){
    case WM_INITDIALOG:
        SetWindowLong(hDlg, GWL_EXSTYLE, GetWindowLong(hDlg, GWL_EXSTYLE)|WS_EX_TOOLWINDOW);
        SetWindowPos(hDlg, NULL, 0, 0, 0, 0, SWP_FRAMECHANGED|SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER);
        return TRUE;
    case WM_COMMAND:
        switch(LOWORD(wParam)){
        case IDOK:
        case IDCANCEL:
            EndDialog(hDlg, 0);
            return TRUE;
        case IDC_OPENREADME:
            if(ShellExecute(hDlg, NULL, FILE_README, NULL, NULL, SW_SHOWNORMAL)==(HINSTANCE)SE_ERR_FNF){
                ERRORBOX(FILE_README "が見つかりません");
            }
            return TRUE;
        }
        break;
    }

    return FALSE;
}


// public section //

CMain::CMain(HINSTANCE hInstance)
:   hInstance(hInstance), phttpd(NULL), psplash(NULL), hMenu(NULL), hWnd(NULL)
{
    bInit = true;
    nid.cbSize = 0;

    if(!s_bRegisterClass){
        WNDCLASSEX wc;
        ZeroMemory(&wc, sizeof(wc));
        wc.cbSize           = sizeof(wc);
        wc.lpfnWndProc      = WndProc;
        wc.hInstance        = hInstance;
        wc.lpszClassName    = CNAME_MAIN;
        if(!RegisterClassEx(&wc)){
            ERRORBOX("タスクトレイ用のウィンドウクラスの登録に失敗しました。");
            bInit = false;
            return;
        }else{
            s_bRegisterClass = true;
        }
    }

    hMenu = LoadMenu(hInstance, "IDM_MAIN");
    hPopMenu = GetSubMenu(hMenu, 0);
    if(hPopMenu){
        MENUITEMINFO mii;
        mii.cbSize  = sizeof(mii);
        mii.fMask   = MIIM_STATE;
        mii.fState  = MFS_DEFAULT;
        SetMenuItemInfo(hPopMenu, IDM_OPENWWWROOT, FALSE, &mii);
    }else{
        ERRORBOX("メニューリソースの読み込みに失敗しました。");
        bInit = false;
        return;
    }
}

CMain::~CMain()
{
    if(nid.cbSize)
        Shell_NotifyIcon(NIM_DELETE, &nid);
    if(phttpd)
        delete phttpd;
    if(hWnd)
        DestroyWindow(hWnd);
    if(hMenu)
        DestroyMenu(hMenu);
    if(psplash)
        delete psplash;
}

int CMain::Run()
{
    MSG     msg;
    BOOL    bMsg;

    if(!bInit)
        return 0;

    hWnd = CreateWindow(CNAME_MAIN, "", 0, GetSystemMetrics(SM_CXFULLSCREEN)/2, GetSystemMetrics(SM_CYFULLSCREEN)*3/4, 0, 0, NULL, NULL, hInstance, NULL);
    if(!hWnd){
        ERRORBOX("タスクトレイ用のウィンドウの作成に失敗しました。");
        return 0;
    }
    SetLastError(0);
    SetWindowLong(hWnd, GWL_USERDATA, (LONG)this);
    if(GetLastError()){
        ERRORBOX("タスクトレイ用のウィンドウの関連メモリの書き換えに失敗しました。");
        return 0;
    }

    nid.cbSize              = sizeof(nid);
    nid.hWnd                = hWnd;
    nid.uID                 = IDTRAY_MAIN;
    nid.uFlags              = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage    = MSGT_MAIN;
    nid.hIcon               = LoadIcon(hInstance, "IDI_TRAY");
    lstrcpy(nid.szTip, APPNAME);
    if(!Shell_NotifyIcon(NIM_ADD, &nid)){
        ERRORBOX("タスクトレイアイコンの作成に失敗しました。");
        nid.cbSize = 0;
        return 0;
    }

    phttpd = new(std::nothrow) CHTTPd(hInstance, hWnd);
    if(!phttpd){
        ERRORBOX("メモリが足りません。");
        return 0;
    }
    if(!phttpd->Run())
        return 0;

    psplash = new(std::nothrow) CSplashWindow(hInstance, hWnd);
    if(psplash)
        psplash->ShowSplash();

    while(bMsg = GetMessage(&msg, NULL, 0, 0), bMsg != 0){
        if(bMsg == -1){
            ERRORBOX("メッセージループ中にエラーが発生しました。");
            return -1;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}
