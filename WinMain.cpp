/*
    WinMain:
        エントリーポイント
*/

#include "system.h"
#include "CMain.h"
#include <windows.h>


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
    CMain _main(hInstance);
    return _main.Run();
}
