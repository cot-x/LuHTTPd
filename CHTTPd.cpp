#include "CHTTPd.h"
#include "system.h"
#include "Resource.h"
#include <stdio.h>


// private section //

#define ONMETHOD(METHOD_NAME)   void CHTTPd::METHOD_NAME(SOCKET sock, WORD nErrorCode)
ONMETHOD(OnAccept)
{
    SOCKADDR_IN     addr;
    int             addrlen, nRet;
    SOCKET          ClientSocket;
    CHTTPdClient    *pClient;

    addrlen = sizeof(addr);
    ClientSocket = accept(ServerSocket, (LPSOCKADDR)&addr, &addrlen);
    if(ClientSocket == INVALID_SOCKET){
        nRet = WSAGetLastError();
        if(nRet != WSAEWOULDBLOCK)
            AppendLog("unknown", GetErrorString(nRet), LOG_SERIOUSERROR_HEAD);
        return;
    }

    // 通常、非同期に設定されるが、一部のWinSock1.1実装では確実に行われないことがあるために行う
    if(WSAAsyncSelect(ClientSocket, hWnd, MSGS_SERVER, FD_ACCEPT|FD_CLOSE|FD_READ|FD_WRITE) == SOCKET_ERROR){
        nRet = WSAGetLastError();
        closesocket(ClientSocket);
        AppendLog(inet_ntoa(addr.sin_addr), GetErrorString(nRet), LOG_SERIOUSERROR_HEAD);
        return;
    }

    pClient = clients.AddClient(ClientSocket, &addr);
    if(!pClient || !pClient->NewBuffer()){
        if(pClient)
            clients.DeleteClient(ClientSocket);
        else
            closesocket(ClientSocket);
        AppendLog(inet_ntoa(addr.sin_addr), "メモリ不足のため処理を続行できませんでした。(accept)", LOG_SERIOUSERROR_HEAD);
        return;
    }
}

ONMETHOD(OnClose)
{
    clients.DeleteClient(sock);
}

ONMETHOD(OnRead)
{
    CHTTPdClient    *pClient;
    char            *buf;
    int             nLen, nRet;

    pClient = clients.GetClient(sock);
    if(!pClient){
        closesocket(sock);
        return;
    }
    if(pClient->GetMethod() != CHTTPdClient::METHOD_NULL){
        DoMethod(pClient);
        return;
    }
    buf = pClient->GetBuffer();

    nLen = (int)strlen(buf);
    nRet = recv(sock, buf+nLen, CLIENTBUFSIZE-1-nLen, 0);
    if(nRet == SOCKET_ERROR){
        nRet = WSAGetLastError();
        if(nRet != WSAEWOULDBLOCK){
            AppendLog(pClient->GetAddress(), GetErrorString(nRet), LOG_SERIOUSERROR_HEAD);
            clients.DeleteClient(sock);
        }
        return;
    }
    if(!nRet){
        OnClose(sock, 0);
        return;
    }
    while(*(buf++)){
        if(*buf=='\n'){
            if(ParseRequest(pClient))
                DoMethod(pClient);
            return;
        }
    }
    if(strlen(buf)+1 == CLIENTBUFSIZE){
        AppendLog(pClient->GetAddress(), "要求メッセージを受信しきれませんでした。", LOG_SERIOUSERROR_HEAD);
        clients.DeleteClient(sock);
        return;
    }
}

ONMETHOD(OnWrite)
{
    CHTTPdClient    *pClient;

    pClient = clients.GetClient(sock);
    if(!pClient){
        closesocket(sock);
        return;
    }

    DoMethod(pClient);
}
#undef ONMETHOD

bool CHTTPd::ParseRequest(CHTTPdClient *pClient)
{
    char    buf[CLIENTBUFSIZE], delimiter[] = " \r\n", *szToken, szFilePath[_MAX_PATH];
    int     nLen;

    CopyMemory(buf, pClient->GetBuffer(), sizeof(buf));

    if(strstr(buf, ".."))
        return SetErrorBadRequest(pClient);

    szToken = strtok(buf, delimiter);
    if(_stricmp(szToken, "GET") == 0){
        pClient->SetMethod(CHTTPdClient::METHOD_GET);
        // ファイルハンドル取得
    }else{
        return SetErrorNotImplemented(pClient);
    }

    // ファイルハンドルを取得するメソッドは、この下へ

    szToken = strtok(NULL, delimiter);
    if(!szToken)
        return SetErrorBadRequest(pClient);
    strcpy(szFilePath, WWWROOTDIRECTORY);
    strncat(szFilePath, szToken, (sizeof(szFilePath)-1) - (sizeof(WWWROOTDIRECTORY)-1));
    nLen = (int)strlen(szFilePath);
    if(szFilePath[nLen-1] == '/')
        strncat(szFilePath, HTTP_INDEXFILE, (sizeof(szFilePath)-1) - nLen);

    pClient->SetFileHandle(CreateFile(szFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL|FILE_FLAG_RANDOM_ACCESS, NULL));
    if(pClient->GetFileHandle() == INVALID_HANDLE_VALUE)
        return SetErrorNotFound(pClient);

    AppendLog(pClient->GetAddress(), strtok(pClient->GetBuffer(), "\r\n"));
    pClient->DeleteBuffer();

    return true;
}

void CHTTPd::DoMethod(CHTTPdClient *pClient)
{
    switch(pClient->GetMethod()){
    case CHTTPdClient::METHOD_NULL:
        return;
    case CHTTPdClient::METHOD_ERROR:
    case CHTTPdClient::METHOD_GET:
        DoMethod_SendBody(pClient);
        return;
    }
}

void CHTTPd::DoMethod_SendBody(CHTTPdClient *pClient)
{
    char    *sendbuf, buf[SENDBUFSIZE];
    bool    bEOF = false;
    int     nBytesOfSend, nBytesSent;

    if(pClient->GetMethod() == CHTTPdClient::METHOD_ERROR){
        sendbuf = pClient->GetBuffer();
        nBytesOfSend = (int)strlen(sendbuf);
        bEOF = true;
    }else{
        DWORD dwBytesRead;
        if(ReadFile(pClient->GetFileHandle(), buf, sizeof(buf), &dwBytesRead, NULL)){
            if(dwBytesRead<sizeof(buf))
                bEOF = true;
        }else{
            pClient->CloseFileHandle();
            if(!pClient->NewBuffer()){
                SetErrorInternalServerError(pClient, "ファイルの読み取りに失敗しました。");
                DoMethod(pClient);
            }else{
                SetErrorInternalServerError(pClient, "メモリ不足のため処理を続行できませんでした。接続を切断します。");
                clients.DeleteClient(pClient->GetSocket());
            }
            return;
        }
        sendbuf = buf;
        nBytesOfSend = dwBytesRead;
    }

    if(nBytesOfSend){
        nBytesSent = send(pClient->GetSocket(), sendbuf, nBytesOfSend, 0);
        if(nBytesSent == SOCKET_ERROR){
            int nRet = WSAGetLastError();
            if(nRet != WSAEWOULDBLOCK){
                // AppendLog(pClient->GetAddress(), GetErrorString(nRet), LOG_SERIOUSERROR_HEAD);
                clients.DeleteClient(pClient->GetSocket());
            }
            return;
        }
        pClient->SeekReadBytes(nBytesSent);
        if(nBytesSent<nBytesOfSend){
            bEOF = false;
        }else if(!bEOF){
            DoMethod_SendBody(pClient);
        }
    }

    if(bEOF){
        shutdown(pClient->GetSocket(), SD_BOTH);
        clients.DeleteClient(pClient->GetSocket());
        return;
    }
}

bool CHTTPd::SetErrorBadRequest(CHTTPdClient *pClient, char *reason)   // default: reason=NULL
{
    char    *buf = pClient->GetBuffer(), *pText;

    AppendLog(pClient->GetAddress(), (reason?reason:(buf?strtok(buf, "\r\n"):"")), "400 Bad Request");
    if(buf)
        pText = (char *)LockResource(hResBadRequest);
    if(!buf || !pText){
        clients.DeleteClient(pClient->GetSocket());
        return false;
    }
    strcpy(buf, pText);

    pClient->SetMethod(CHTTPdClient::METHOD_ERROR);

    return true;
}

bool CHTTPd::SetErrorNotFound(CHTTPdClient *pClient, char *reason)   // default: reason=NULL
{
    char    *buf = pClient->GetBuffer(), *pText;

    AppendLog(pClient->GetAddress(), (reason?reason:(buf?strtok(buf, "\r\n"):"")), "404 Not Found");
    if(buf)
        pText = (char *)LockResource(hResNotFound);
    if(!buf || !pText){
        clients.DeleteClient(pClient->GetSocket());
        return false;
    }
    strcpy(buf, pText);

    pClient->SetMethod(CHTTPdClient::METHOD_ERROR);

    return true;
}

bool CHTTPd::SetErrorInternalServerError(CHTTPdClient *pClient, char *reason)   // default: reason=NULL
{
    char    *buf = pClient->GetBuffer(), *pText;

    AppendLog(pClient->GetAddress(), (reason?reason:(buf?strtok(buf, "\r\n"):"")), "500 Internal Server Error");
    if(buf)
        pText = (char *)LockResource(hResInternalServerError);
    if(!buf || !pText){
        clients.DeleteClient(pClient->GetSocket());
        return false;
    }
    strcpy(buf, pText);

    pClient->SetMethod(CHTTPdClient::METHOD_ERROR);

    return true;
}

bool CHTTPd::SetErrorNotImplemented(CHTTPdClient *pClient, char *reason)   // default: reason=NULL
{
    char    *buf = pClient->GetBuffer(), *pText;

    AppendLog(pClient->GetAddress(), (reason?reason:(buf?strtok(buf, "\r\n"):"")), "501 Not Implemented");
    if(buf)
        pText = (char *)LockResource(hResNotImplemented);
    if(!buf || !pText){
        clients.DeleteClient(pClient->GetSocket());
        return false;
    }
    strcpy(buf, pText);

    pClient->SetMethod(CHTTPdClient::METHOD_ERROR);

    return true;
}

bool CHTTPd::SlimLog()
{
    char    buf[1024];
    DWORD   dwBytesRead=sizeof(buf), dwLineNum=0,
            dwSlimLineNum, dwSlimBytes=0, dwBytesWritten, dwSeekBytes=0;

    // 現在の行数を調べる
    if(SetFilePointer(hLogFile, 0, NULL, FILE_BEGIN)==-1)
        return false;
    while(dwBytesRead==sizeof(buf)){
        if(!ReadFile(hLogFile, buf, sizeof(buf), &dwBytesRead, NULL))
            return false;
        for(UINT i=0; i<dwBytesRead; i++){
            if(buf[i]=='\n')
                dwLineNum++;
        }
    }

    if(dwLineNum<LOGMAXLINE)
        return true;

    // 先頭から削除するバイト数を調べる
    dwSlimLineNum = dwLineNum - LOGMAXLINE + 1;
    if(SetFilePointer(hLogFile, 0, NULL, FILE_BEGIN)==-1)
        return false;
    dwLineNum=0;
    dwBytesRead=sizeof(buf);
    while(dwBytesRead==sizeof(buf)){
        if(!ReadFile(hLogFile, buf, sizeof(buf), &dwBytesRead, NULL))
            return false;
        for(UINT i=0; i<dwBytesRead; i++){
            if(buf[i]=='\n'){
                dwLineNum++;
                if(dwLineNum==dwSlimLineNum){
                    dwSlimBytes += i+1;
                    break;
                }
            }
        }
        if(dwLineNum==dwSlimLineNum)
            break;
        else
            dwSlimBytes += dwBytesRead;
    }

    // 先頭行から削除する
    while(true){
        if(SetFilePointer(hLogFile, dwSlimBytes+dwSeekBytes, NULL, FILE_BEGIN)==-1)
            return false;
        if(!ReadFile(hLogFile, buf, sizeof(buf), &dwBytesRead, NULL))
            return false;
        if(dwBytesRead){
            if(SetFilePointer(hLogFile, dwSeekBytes, NULL, FILE_BEGIN)==-1)
                return false;
            if(!WriteFile(hLogFile, buf, dwBytesRead, &dwBytesWritten, NULL))
                return false;
        }
        if(dwBytesRead<sizeof(buf)){
            SetEndOfFile(hLogFile);
            break;
        }
        dwSeekBytes+=dwBytesRead;
    }

    return true;
}

bool CHTTPd::AppendLog(const char *name, const char *body, const char *error)   // default: error=NULL
{
    DWORD       dwBytesWritten;
    SYSTEMTIME  st;
    char        time[20];

    GetLocalTime(&st);
    sprintf(time, "%d/%.2d/%.2d %.2d:%.2d:%.2d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
    SlimLog();
    if(SetFilePointer(hLogFile, 0, NULL, FILE_END)==-1)
        return false;
    if(!WriteFile(hLogFile, time, (DWORD)strlen(time), &dwBytesWritten, NULL))
        return false;
    if(!WriteFile(hLogFile, "\t", 1, &dwBytesWritten, NULL))
        return false;
    if(!WriteFile(hLogFile, name, (DWORD)strlen(name), &dwBytesWritten, NULL))
        return false;
    if(!WriteFile(hLogFile, "\t", 1, &dwBytesWritten, NULL))
        return false;
    if(error){
        if(!WriteFile(hLogFile, "[", 1, &dwBytesWritten, NULL))
            return false;
        if(!WriteFile(hLogFile, error, (DWORD)strlen(error), &dwBytesWritten, NULL))
            return false;
        if(!WriteFile(hLogFile, "]", 1, &dwBytesWritten, NULL))
            return false;
    }
    if(!WriteFile(hLogFile, body, (DWORD)strlen(body), &dwBytesWritten, NULL))
        return false;
    if(!WriteFile(hLogFile, "\r\n", 2, &dwBytesWritten, NULL))
        return false;

    return true;
}


// public section //

CHTTPd::CHTTPd(HINSTANCE hInstance, HWND hWndParent)
: CServer(hInstance, hWndParent, HTTPDPORT)
{
    if(!CreateDirectory(WWWROOTDIRECTORY, NULL) && GetLastError()!=ERROR_ALREADY_EXISTS){
        ERRORBOX(WWWROOTDIRECTORY "の作成に失敗しました。");
        bInit = false;
        return;
    }
    hLogFile = CreateFile(LOGFILENAME, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL|FILE_FLAG_RANDOM_ACCESS, NULL);
    if(hLogFile==INVALID_HANDLE_VALUE){
        ERRORBOX(LOGFILENAME "の作成に失敗しました。");
        bInit = false;
        return;
    }

    hResBadRequest = LoadResource(hInstance, FindResource(hInstance, MAKEINTRESOURCE(IDTEXT_HTTP_BADREQUEST), TEXT("TEXT")));
    hResNotFound = LoadResource(hInstance, FindResource(hInstance, MAKEINTRESOURCE(IDTEXT_HTTP_NOTFOUND), TEXT("TEXT")));
    hResInternalServerError = LoadResource(hInstance, FindResource(hInstance, MAKEINTRESOURCE(IDTEXT_HTTP_INTERNALSERVERERROR), TEXT("TEXT")));
    hResNotImplemented = LoadResource(hInstance, FindResource(hInstance, MAKEINTRESOURCE(IDTEXT_HTTP_NOTIMPLEMENTED), TEXT("TEXT")));
    if(!hResBadRequest || !hResNotFound || !hResInternalServerError || !hResNotImplemented){
        ERRORBOX("エラーテキストリソースの読み込みに失敗しました。");
        bInit = false;
        return;
    }
}

CHTTPd::~CHTTPd()
{
    if(hLogFile!=INVALID_HANDLE_VALUE)
        CloseHandle(hLogFile);
}
