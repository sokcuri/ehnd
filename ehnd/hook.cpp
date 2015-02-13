#include "stdafx.h"
#include "hook.h"
#include "ehnd.h"


bool hook()
{
	HMODULE hDll, hDll2;
	TCHAR lpEztPath[MAX_PATH], lpDllPath[MAX_PATH];
	LPCSTR aEztFunction[] = {
		"J2K_Initialize",
		"J2K_InitializeEx",
		"J2K_FreeMem",
		"J2K_GetPriorDict",
		"J2K_GetProperty",
		"J2K_ReloadUserDict",
		"J2K_SetDelJPN",
		"J2K_SetField",
		"J2K_SetHnj2han",
		"J2K_SetJWin",
		"J2K_SetPriorDict",
		"J2K_SetProperty",
		"J2K_StopTranslation",
		"J2K_Terminate",
		"J2K_TranslateChat",
		"J2K_TranslateFM",
		"J2K_TranslateMM",
		"J2K_TranslateMMEx",
		"J2K_TranslateMMNT",
		"?GetJ2KMainDir@@YA?AVCString@@XZ" };
	LPCSTR aMsvFunction[] = {
		"free",
		"malloc",
		"fopen" };
	int i;

	GetModuleFileName(g_hInst, lpEztPath, MAX_PATH);
	i = wcslen(lpEztPath);
	while (i--)
	{
		if (lpEztPath[i] == L'\\')
		{
			lpEztPath[i] = 0;
			break;
		}
	}

	wcscpy_s(lpDllPath, lpEztPath);
	wcscat_s(lpDllPath, L"\\j2kengine.dlx");
	hDll = LoadLibrary(lpDllPath);
	if (!hDll)
	{
		MessageBox(0, L"eztrans load failed", 0, 0);
		return false;
	}

	for (i = 0; i < _countof(aEztFunction); i++)
	{
		apfnEzt[i] = GetProcAddress(hDll, aEztFunction[i]);
		if (!apfnEzt[i])
		{
			MessageBox(0, L"eztrans function load error", 0, 0);
			return false;
		}
	}

	lpDllPath[0] = 0;
	GetSystemDirectory(lpDllPath, MAX_PATH);
	wcscat_s(lpDllPath, L"\\msvcrt.dll");
	hDll2 = LoadLibrary(lpDllPath);
	if (!hDll2)
	{
		MessageBox(0, L"msvcrt load failed", 0, 0);
		return false;
	}

	for (i = 0; i < _countof(aMsvFunction); i++)
	{
		apfnMsv[i] = GetProcAddress(hDll2, aMsvFunction[i]);
		if (!apfnMsv[i])
		{
			MessageBox(0, L"msvcrt function load error", 0, 0);
			return false;
		}
	}

	return true;
}