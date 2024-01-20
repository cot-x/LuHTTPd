/*
    CSplashWindow:
        スプラッシュウィンドウ作成クラス
*/

#ifndef _CSplashWindow_
#define _CSplashWindow_

#include <windows.h>


class CSplashWindow
{
    // コピー及び代入の禁止
    CSplashWindow(const CSplashWindow &);
    CSplashWindow & operator =(const CSplashWindow &);

    // データメンバ
    static bool     s_bRegisterClass;
    HINSTANCE       hInstance;
    HWND            hWnd, hWndParent;
    HBITMAP         hBitmap;
    BITMAP          BitmapInfo;

#define ONMETHOD(METHOD_NAME)   LRESULT CALLBACK METHOD_NAME(WPARAM wParam, LPARAM lParam)
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    virtual ONMETHOD(OnPaint);
    virtual ONMETHOD(OnLButtonUp);
#undef ONMETHOD

    static DWORD WINAPI ThreadProc(LPVOID lpParameter);

protected:
    // データメンバ
    bool    bInit;

public:
    CSplashWindow(HINSTANCE hInstance, HWND hWndParent);
    virtual ~CSplashWindow();

    bool ShowSplash();
};

#endif  // _CSplashWindow_
