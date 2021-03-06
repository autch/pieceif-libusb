// dllmain.cpp : DLL アプリケーションのエントリ ポイントを定義します。
#include "stdafx.h"

extern CRITICAL_SECTION csLibUSBInit; // pieceif.c

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
	// never call libusb_*() here, it'll hang
	switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
		InitializeCriticalSection(&csLibUSBInit);
		break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
		break;
    case DLL_PROCESS_DETACH:
		DeleteCriticalSection(&csLibUSBInit);
        break;
    }
    return TRUE;
}

