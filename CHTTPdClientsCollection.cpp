#include "CHTTPdClientsCollection.h"
#include "system.h"
#include <new>


// private section //


// public section //

CHTTPdClientsCollection::CHTTPdClientsCollection()
:   pClientFirst(NULL), pClientEnd(NULL)
{
}

CHTTPdClientsCollection::~CHTTPdClientsCollection()
{
    if(pClientFirst){
        CHTTPdClient *pDeleteClient, *pClient = pClientFirst;
        while(pClient){
            pDeleteClient = pClient;
            pClient = pClient->next;
            delete pDeleteClient;
        }
    }
}

CHTTPdClient * CHTTPdClientsCollection::AddClient(SOCKET sock, LPSOCKADDR_IN addr)
{
    CHTTPdClient    *pNewClient;

    pNewClient = new(std::nothrow) CHTTPdClient(sock, addr);
    if(!pNewClient)
        return NULL;
    if(pClientEnd){
        pClientEnd->next = pNewClient;
        pClientEnd = pNewClient;
    }else{
        pClientFirst = pClientEnd = pNewClient;
    }
    return pNewClient;
}

void CHTTPdClientsCollection::DeleteClient(SOCKET sock)
{
    CHTTPdClient    *pClient, *pClient_old=NULL;

    if(!pClientFirst)
        return;
    pClient = pClientFirst;
    while(pClient){
        if(pClient->GetSocket() == sock){
            if(pClient == pClientEnd)
                pClientEnd = (pClient_old)?pClient_old:NULL;
            if(pClient_old)
                pClient_old->next = pClient->next;
            else
                pClientFirst = pClient->next;
            delete pClient;
            break;
        }
        pClient_old = pClient;
        pClient = pClient->next;
    }
}

CHTTPdClient * CHTTPdClientsCollection::GetClient(SOCKET sock)
{
    CHTTPdClient    *pClient;

    if(!pClientFirst)
        return NULL;
    pClient = pClientFirst;
    while(pClient){
        if(pClient->GetSocket() == sock)
            return pClient;
        pClient = pClient->next;
    }

    return NULL;
}
