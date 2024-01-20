#include "CServer.h"
#include "system.h"

#define REQWSAVERSION   MAKEWORD(1, 1)


// private section //

// static
bool CServer::s_bRegisterClass = false;

// static
LRESULT CALLBACK CServer::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CServer *This = (CServer *)GetWindowLong(hWnd, GWL_USERDATA);
    if(!This)
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    switch(uMsg){
    case MSGS_SERVER:
        switch(WSAGETSELECTEVENT(lParam)){
        case FD_ACCEPT:
            This->OnAccept((SOCKET)wParam, WSAGETSELECTERROR(lParam));
            return 0;
        case FD_CLOSE:
            This->OnClose((SOCKET)wParam, WSAGETSELECTERROR(lParam));
            return 0;
        case FD_READ:
            This->OnRead((SOCKET)wParam, WSAGETSELECTERROR(lParam));
            return 0;
        case FD_WRITE:
            This->OnWrite((SOCKET)wParam, WSAGETSELECTERROR(lParam));
            return 0;
        }
        break;
    case WM_DESTROY:
        This->hWnd = NULL;
        break;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


// public section //

CServer::CServer(HINSTANCE hInstance, HWND hWndParent, u_short port)
:   bWSAStartup(false), hInstance(hInstance), hWndParent(hWndParent), hWnd(NULL), port(port), ServerSocket(INVALID_SOCKET)
{
    bInit = true;

    int     nRet;
    WSADATA wsaData;

    nRet = WSAStartup(REQWSAVERSION, &wsaData);
    if(nRet!=0){
        ERRORBOX("WinSockの初期化に失敗しました。");
        bInit = false;
        return;
    }else if(wsaData.wVersion != REQWSAVERSION){
        WSACleanup();
        ERRORBOX("要求したWinSockのバージョンを利用できませんでした。");
        bInit = false;
        return;
    }else{
        bWSAStartup = true;
    }

    if(!s_bRegisterClass){
        WNDCLASSEX wc;
        ZeroMemory(&wc, sizeof(wc));
        wc.cbSize           = sizeof(wc);
        wc.lpfnWndProc      = WndProc;
        wc.hInstance        = hInstance;
        wc.lpszClassName    = CNAME_SERVER;
        if(!RegisterClassEx(&wc)){
            ERRORBOX("サーバー用のウィンドウクラスの登録に失敗しました。");
            bInit = false;
            return;
        }else{
            s_bRegisterClass = true;
        }
    }
}

CServer::~CServer()
{
    if(ServerSocket != INVALID_SOCKET)
        closesocket(ServerSocket);
    if(hWnd)
        DestroyWindow(hWnd);
    if(bWSAStartup)
        WSACleanup();
}

// static
char * CServer::GetErrorString(int nErrorCode)
{
	switch(nErrorCode){
	case WSAEACCES:
		return "アクセスが拒否されました。";
	case WSAEADDRINUSE:
		return "アドレスはすでに使用されています。";
	case WSAEADDRNOTAVAIL:
		return "無効なアドレス。";
	case WSAEAFNOSUPPORT:
		return "アドレスファミリはプロトコルファミリでサポートされません。";
	case WSAEALREADY:
		return "操作はすでに実行中です。";
	case WSAEBADF:
		return "不良ファイル記述子。";
	case WSAECANCELLED:
		return "取り消し。";
	case WSAECONNABORTED:
		return "ソフトウェアによって接続が拒否されました。";
	case WSAECONNREFUSED:
		return "接続が拒否されました。";
	case WSAECONNRESET:
		return "接続は相手によってリセットされました。";
	case WSAEDESTADDRREQ:
		return "受け側アドレスが必要です。";
	case WSAEDISCON:
		return "シャットダウン処理中。";
	case WSAEDQUOT:
		return "ディスククォータ。";
	case WSAEFAULT:
		return "不良なアドレスです。";
	case WSAEHOSTDOWN:
		return "ホストがダウンしています。";
	case WSAEHOSTUNREACH:
		return "ホストへのルートがありません。";
	case WSAEINPROGRESS:
		return "ブロッキング操作はすでに実行中です。";
	case WSAEINTR:
		return "関数呼び出しが中断されました。";
	case WSAEINVAL:
		return "無効な引数です。";
	case WSAEINVALIDPROCTABLE:
		return "サービスプロバイダからのプロシージャテーブルが無効です。";
	case WSAEINVALIDPROVIDER:
		return "無効なサービスプロバイダバージョン番号。";
	case WSAEISCONN:
		return "ソケットはすでに接続されています。";
	case WSAELOOP:
		return "ループ。";
	case WSAEMFILE:
		return "開いているソケットの数が多すぎます。";
	case WSAEMSGSIZE:
		return "メッセージが長すぎます。";
	case WSAENAMETOOLONG:
		return "名前が長すぎます。";
	case WSAENETDOWN:
		return "ネットワークがダウンしています。";
	case WSAENETRESET:
		return "ネットワーク接続が破棄されました。";
	case WSAENETUNREACH:
		return "ネットワークに到達できません。";
	case WSAENOBUFS:
		return "バッファに開き領域がありません。";
	case WSAENOMORE:
		return "データはこれ以上ありません。";
	case WSAENOPROTOOPT:
		return "不良なプロトコルオプションです。";
	case WSAENOTCONN:
		return "ソケットは接続されていません。";
	case WSAENOTEMPTY:
		return "ディレクトリが空ではありません。";
	case WSAENOTSOCK:
		return "指定されたソケットが無効です。";
	case WSAEOPNOTSUPP:
		return "操作がサポートされていません。";
	case WSAEPFNOSUPPORT:
		return "プロトコルファミリがサポートされていません。";
	case WSAEPROCLIM:
		return "プロセスの数が多すぎます。";
	case WSAEPROTONOSUPPORT:
		return "プロトコルがサポートされていません。";
	case WSAEPROTOTYPE:
		return "ソケットに対するプロトコルタイプが間違っています。";
	case WSAEPROVIDERFAILEDINIT:
		return "サービスプロバイダを初期化できません。";
	case WSAEREFUSED:
		return "拒否。";
	case WSAEREMOTE:
		return "リモート。";
	case WSAESHUTDOWN:
		return "ソケットのシャットダウンの後には通信できません。";
	case WSAESOCKTNOSUPPORT:
		return "ソケットタイプがサポートされていません。";
	case WSAESTALE:
		return "廃止。";
	case WSAETIMEDOUT:
		return "接続がタイムアウトしました。";
	case WSAETOOMANYREFS:
		return "参照の数が多すぎます。";
	case WSAEUSERS:
		return "ユーザーの数が多すぎます。";
	case WSAEWOULDBLOCK:
		return "操作はブロッキングされます。";
	case WSAHOST_NOT_FOUND:
		return "ホストが見つかりません。";
	case WSANOTINITIALISED:
		return "WSAStartup()がまだ正常に実行されていません。";
	case WSANO_DATA:
		return "名前は有効ですが、要求したタイプのデータレコードはありません。";
	case WSANO_RECOVERY:
		return "回復不可能なエラー。";
	case WSASERVICE_NOT_FOUND:
		return "サービスが見つかりません。";
	case WSASYSCALLFAILURE:
		return "システムコールに失敗しました。";
	case WSASYSNOTREADY:
		return "ネットワークサブシステムを利用できません。";
	case WSATRY_AGAIN:
		return "ホストが見つからないかサーバーの異常です。";
	case WSATYPE_NOT_FOUND:
		return "タイプが見つかりません。";
	case WSAVERNOTSUPPORTED:
		return "WINSOCK.DLLのバージョンが範囲外です。";
	case WSA_E_CANCELLED:
		return "検索が取り消されました。";
	case WSA_E_NO_MORE:
		return "データはこれ以上ありません。";
	default:
		return "不明なエラーコードです。";
	}

	return "";
}

bool CServer::Run()
{
    if(!bInit)
        return false;

    hWnd = CreateWindow(CNAME_SERVER, "", 0, 0, 0, 0, 0, hWndParent, NULL, hInstance, NULL);
    if(!hWnd){
        ERRORBOX("サーバー用のウィンドウの作成に失敗しました。");
        return false;
    }
    SetLastError(0);
    SetWindowLong(hWnd, GWL_USERDATA, (LONG)this);
    if(GetLastError()){
        ERRORBOX("サーバー用のウィンドウの関連メモリの書き換えに失敗しました。");
        return false;
    }

    ServerSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(ServerSocket == INVALID_SOCKET){
        ERRORBOX("サーバーソケットの作成に失敗しました。");
        return false;
    }
    if(WSAAsyncSelect(ServerSocket, hWnd, MSGS_SERVER, FD_ACCEPT|FD_CLOSE|FD_READ|FD_WRITE)==SOCKET_ERROR){
        ERRORBOX("サーバーソケットの非同期設定に失敗しました。");
        return false;
    }
    sockaddr_in sa;
    sa.sin_family       = AF_INET;
    sa.sin_port         = htons(port);
    sa.sin_addr.s_addr  = INADDR_ANY;
    if(bind(ServerSocket, (LPSOCKADDR)&sa, sizeof(sa))==SOCKET_ERROR){
        ERRORBOX("サーバーアドレス及びポートとソケットとの関連付けに失敗しました。");
        return false;
    }
    if(listen(ServerSocket, SOMAXCONN)==SOCKET_ERROR){
        ERRORBOX("クライアントからの接続要求待機に失敗しました。");
        return false;
    }

    return true;
}
