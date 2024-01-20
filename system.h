/*
    system.h:
        各種定義ファイル
*/

#ifndef _system_h_
#define _system_h_

#include "LuHTTPd.h"


#define ERRORBOX(msg)   MessageBox(NULL, msg, APPNAME " - エラー", MB_ICONERROR)

#define MSGT_MAIN   (WM_USER+1)
#define MSGS_SERVER (WM_USER+2)

#define IDTRAY_MAIN    1

#define IDTIMER_SPLASH  1

#define CNAME_MAIN      APPNAME "_MAIN"
#define CNAME_SERVER    APPNAME "_SERVER"
#define CNAME_SPLASH    APPNAME "_SPLASH"

#endif  // _system_h_
