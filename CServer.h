/*
    CServer:
        サーバー用基底クラス
        サーバーの基本処理を行い、基本機能を提供する
*/

#ifndef _CServer_
#define _CServer_

#include <winsock2.h>


class CServer
{
    // コピー及び代入の禁止
    CServer(const CServer &);
    CServer & operator =(const CServer &);

    // データメンバ
    static bool s_bRegisterClass;
    bool        bWSAStartup;

#define ONMETHOD(METHOD_NAME)   void METHOD_NAME(SOCKET sock, WORD nErrorCode)
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    virtual ONMETHOD(OnAccept) = 0;
    virtual ONMETHOD(OnClose) = 0;
    virtual ONMETHOD(OnRead) = 0;
    virtual ONMETHOD(OnWrite) = 0;
#undef ONMETHOD

protected:
    // データメンバ
    bool        bInit;
    SOCKET      ServerSocket;
    HINSTANCE   hInstance;
    HWND        hWnd, hWndParent;
    u_short     port;

public:
    CServer(HINSTANCE hInstance, HWND hWndParent, u_short port);
    virtual ~CServer();

    static char * GetErrorString(int nErrorCode);

    bool Run();
};

#endif  // _CServer_
