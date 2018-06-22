/*
    CMain:
        メインクラス
        トレイアイコンの処理、CHTTPdクラスのスタート、メインメッセージループ処理を行う
*/

#ifndef _CMain_
#define _CMain_

#include "CHTTPd.h"
#include "CSplashWindow.h"
#include <windows.h>


class CMain
{
    // コピー及び代入の禁止
    CMain(const CMain &);
    CMain & operator =(const CMain &);

    // データメンバ
    static bool     s_bRegisterClass;
    HINSTANCE       hInstance;
    HWND            hWnd;
    HMENU           hMenu, hPopMenu;
    NOTIFYICONDATA  nid;
    CHTTPd          *phttpd;
    CSplashWindow   *psplash;

#define ONMETHOD(METHOD_NAME)   LRESULT CALLBACK METHOD_NAME(WPARAM wParam, LPARAM lParam)
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    virtual ONMETHOD(OnTrayMenuExit);
    virtual ONMETHOD(OnTrayMenuAbout);
    virtual ONMETHOD(OnTrayMenuOpenWWWRoot);
    virtual ONMETHOD(OnTrayMenuOpenLog);
    virtual ONMETHOD(OnTrayRButtonUp);
    virtual ONMETHOD(OnTrayLButtonDBLCLK);
#undef ONMETHOD

    static BOOL CALLBACK AboutDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

protected:
    // データメンバ
    bool    bInit;

public:
    CMain(HINSTANCE hInstance);
    virtual ~CMain();

    int Run();
};

#endif  // _CMain_
