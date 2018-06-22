/*
    LuHTTPd.h:
        アプリケーションCONFIG用ヘッダ
*/

#ifndef _LuHTTPd_h_
#define _LuHTTPd_h_

// アプリケーション名
#define APPNAME             "LuHTTPd"
// バージョン
#define APPVER              "Version.1.0.1"
// ABOUTダイアログボックス - CAPTION
#define ABOUTDLG_CAPTION    "LuHTTPd - About"

// SplashWindow - 存続時間(ms)
#define SPLASHWND_LIVETIME  3000
// SplashWidnow - Width
#define SPLASHWND_WIDTH     320
// SplashWindow - Height
#define SPLASHWND_HEIGHT    240

// readme.txt
#define FILE_README         "readme.txt"

// ログファイル名
#define LOGFILENAME "httpd_log.txt"
// ログの最大行数
#define LOGMAXLINE  1000

// httpルートディレクトリ
#define WWWROOTDIRECTORY    "httproot"
// http - デフォルトインデックスファイル
#define HTTP_INDEXFILE      "index.html"
// HTTPdポート番号
#define HTTPDPORT           80

// 重大なエラーメッセージのヘッダにつけるテキスト
#define LOG_SERIOUSERROR_HEAD   "Disconnected for Error"

// CHTTPdClientの保持するバッファのサイズ（受信バッファにもなる）
#define CLIENTBUFSIZE   2048
// 送信バッファサイズ
#define SENDBUFSIZE     4096

#endif  // _LuHTTPd_h_
