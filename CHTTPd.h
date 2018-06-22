/*
    CHTTPd:
        HTTPd処理用クラス
        サーバー機能、WWWROOTDIRECTORYの作成、ログファイルの作成・管理を行う
*/

#ifndef _CHTTPd_
#define _CHTTPd_

#include "CServer.h"
#include "CHTTPdClientsCollection.h"


class CHTTPd : public CServer
{
    // コピー及び代入の禁止
    CHTTPd(const CHTTPd &);
    CHTTPd & operator =(const CHTTPd &);

    // データメンバ
    CHTTPdClientsCollection clients;
    HANDLE  hLogFile;
    HGLOBAL hResBadRequest, hResNotFound, hResInternalServerError, hResNotImplemented;

#define ONMETHOD(METHOD_NAME)   void METHOD_NAME(SOCKET sock, WORD nErrorCode)
    virtual ONMETHOD(OnAccept);
    virtual ONMETHOD(OnClose);
    virtual ONMETHOD(OnRead);
    virtual ONMETHOD(OnWrite);
#undef ONMETHOD

    bool ParseRequest(CHTTPdClient *pClient);
    void DoMethod(CHTTPdClient *pClient);
    void DoMethod_SendBody(CHTTPdClient *pClient);

    bool SetErrorBadRequest(CHTTPdClient *pClient, char *reason=NULL);
    bool SetErrorNotFound(CHTTPdClient *pClient, char *reason=NULL);
    bool SetErrorInternalServerError(CHTTPdClient *pClient, char *reason=NULL);
    bool SetErrorNotImplemented(CHTTPdClient *pClient, char *reason=NULL);

    bool SlimLog();
    bool AppendLog(const char *name, const char *body, const char *error=NULL);

public:
    CHTTPd(HINSTANCE hInstance, HWND hWndParent);
    virtual ~CHTTPd();
};

#endif  // _CHTTPd_
