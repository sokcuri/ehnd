// dllmain.cpp : DLL 응용 프로그램의 진입점을 정의합니다.
#include "stdafx.h"
HINSTANCE g_hInst;
Cehnd *pEhnd;
filter *pFilter;
HMODULE hEzt, hMsv;

BOOL APIENTRY DllMain(HINSTANCE hInstance,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
	)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
		pEhnd = new Cehnd();
		pFilter = new filter();
		CreateLogWin(g_hInst);
		ShowLogWin(true);
		WriteLog(L"Log Start.\n", RGB(0, 0, 0), RGB(255, 255, 255));
		g_hInst = hInstance;
		hook();

		WriteLog(L"Hook Success.\n", RGB(0, 0, 0), RGB(255, 255, 255));
		hook_userdict();
		hook_userdict2();

		pFilter->pre_load();
		pFilter->post_load();
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

