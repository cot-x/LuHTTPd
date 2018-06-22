/*
    CHTTPdClient:
        HTTPdを利用するクライアントの情報を管理する
        オブジェクトが削除される時にソケットのクローズも行う
*/

#ifndef _CHTTPdClient_
#define _CHTTPdClient_

#include <winsock2.h>
#include <new>


class CHTTPdClient
{
    // コピー及び代入の禁止
    CHTTPdClient(const CHTTPdClient &);
    CHTTPdClient & operator =(const CHTTPdClient &);

public:
    enum HTTPMethod{METHOD_NULL, METHOD_ERROR, METHOD_GET};

private:
    // データメンバ
    char        *buf;
    SOCKET      sock;
    char        addr[16];
    HTTPMethod  method;
    HANDLE      hFile;
    ULONG       ulFilePtr;

public:
    CHTTPdClient(SOCKET sock, LPSOCKADDR_IN addr);
    virtual ~CHTTPdClient();

    // データメンバ
    CHTTPdClient    *next;

    SOCKET GetSocket(){ return sock; }
    const char * GetAddress(){ return addr; }
    void SetMethod(HTTPMethod method){ this->method=method; }
    HTTPMethod GetMethod(){ return method; }
    void SetFileHandle(HANDLE hFile);
    HANDLE GetFileHandle();
    void CloseFileHandle();
    bool SeekReadBytes(LONG lDistanceToMove);
    bool NewBuffer();
    void DeleteBuffer();
    char * GetBuffer();
};

#endif  // _CHTTPdClient_
