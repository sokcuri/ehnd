// dllmain.cpp : DLL 응용 프로그램의 진입점을 정의합니다.
#include "stdafx.h"
HINSTANCE g_hInst;
filter *pFilter;
HMODULE hEzt, hMsv;
int g_initTick;
char g_DicPath[MAX_PATH];
bool g_PreUsable = true;
bool g_PostUsable = true;

BOOL APIENTRY DllMain(HINSTANCE hInstance,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
	)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
		// init ehnd dic path
		char szInitTick[12];
		g_initTick = GetTickCount() + rand();
		_itoa_s(g_initTick, szInitTick, 10);

		GetTempPathA(MAX_PATH, g_DicPath);
		strcat_s(g_DicPath, "UserDict_");
		strcat_s(g_DicPath, szInitTick);
		strcat_s(g_DicPath, ".ehnd");

		// init
		pFilter = new filter();
		CreateLogWin(g_hInst);
		ShowLogWin(true);
		WriteLog(L"Log Start.\n", RGB(0, 0, 0), RGB(255, 255, 255));
		g_hInst = hInstance;
		hook_wc2mb();
		hook_mb2wc();

		hook();
		hook_userdict();
		hook_userdict2();

		WriteLog(L"Hook Success.\n", RGB(0, 0, 0), RGB(255, 255, 255));

		pFilter->pre_load();
		pFilter->post_load();
		pFilter->userdic_load();
		pFilter->skiplayer_load();
	}
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		FreeLibrary(hEzt);
		FreeLibrary(hMsv);
		break;
	}
	return TRUE;
}

