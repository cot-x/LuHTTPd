/*
    CHTTPdClientsCollection:
        CHTTPdClientクラスの管理を行う
        追加・削除・検索機能を提供する
        クラス削除時には、自動的に管理しているすべてのCHTTPdClientクラスのオブジェクトを削除する
*/

#ifndef _CHTTPdClientsCollection_
#define _CHTTPdClientsCollection_

#include <winsock2.h>
#include "CHTTPdClient.h"


class CHTTPdClientsCollection
{
    // コピー及び代入の禁止
    CHTTPdClientsCollection(const CHTTPdClientsCollection &);
    CHTTPdClientsCollection & operator =(const CHTTPdClientsCollection &);

    // データメンバ
    CHTTPdClient    *pClientFirst, *pClientEnd;

public:
    CHTTPdClientsCollection();
    virtual ~CHTTPdClientsCollection();

    CHTTPdClient * AddClient(SOCKET sock, LPSOCKADDR_IN addr);
    void DeleteClient(SOCKET sock);
    CHTTPdClient * GetClient(SOCKET sock);
};

#endif  // _CHTTPdClientsCollection_
