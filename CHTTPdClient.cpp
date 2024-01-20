#include "CHTTPdClient.h"
#include "system.h"


// private section //


// public section //

CHTTPdClient::CHTTPdClient(SOCKET sock, LPSOCKADDR_IN addr)
:   sock(sock), buf(NULL), method(METHOD_NULL), hFile(INVALID_HANDLE_VALUE), next(NULL)
{
    strcpy(this->addr, inet_ntoa(addr->sin_addr));
}

CHTTPdClient::~CHTTPdClient()
{
    if(buf)
        delete [] buf;
    if(hFile != INVALID_HANDLE_VALUE)
        CloseFileHandle();
    closesocket(sock);
}

void CHTTPdClient::SetFileHandle(HANDLE hFile)
{
    this->hFile=hFile;
    ulFilePtr=0;
}

HANDLE CHTTPdClient::GetFileHandle()
{
    LONG lFilePtrHigh=0;
    if(SetFilePointer(hFile, (LONG)ulFilePtr, &lFilePtrHigh, FILE_BEGIN)==-1 && GetLastError()!=NO_ERROR)
        return INVALID_HANDLE_VALUE;

    return hFile;
}

void CHTTPdClient::CloseFileHandle()
{
    CloseHandle(hFile);
    hFile = INVALID_HANDLE_VALUE;
}

bool CHTTPdClient::SeekReadBytes(LONG lDistanceToMove)
{
    if(lDistanceToMove<0 && ulFilePtr<(ULONG)(-lDistanceToMove))
        return false;

    ulFilePtr += lDistanceToMove;

    return true;
}

bool CHTTPdClient::NewBuffer()
{
    if(buf)
        DeleteBuffer();
    buf = new(std::nothrow) char[CLIENTBUFSIZE];
    if(!buf)
        return false;
    ulFilePtr = 0;
    ZeroMemory(buf, CLIENTBUFSIZE);
    return true;
}

void CHTTPdClient::DeleteBuffer(){
    if(buf){
        delete [] buf;
        buf=NULL;
    }
}

char * CHTTPdClient::GetBuffer()
{
    if(method==METHOD_ERROR)
        return buf+ulFilePtr;
    else
        return buf;
}
