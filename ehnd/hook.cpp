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

int search_ptn(LPWORD ptn, size_t ptn_size, LPBYTE *addr)
{
	HMODULE hDll = GetModuleHandle(L"j2kengine.dlx");
	if (hDll == NULL) MessageBox(0, L"j2kengine can't get handle", 0, 0);

	MODULEINFO dllInfo;
	GetModuleInformation(GetCurrentProcess(), hDll, &dllInfo, sizeof(dllInfo));

	UINT i;
	int scan;
	LPBYTE p;

	UINT defSkipLen;
	UINT skipLen[UCHAR_MAX + 1];
	UINT searchSuccessCount;

	UINT ptnEnd = ptn_size - 1;
	while ((HIBYTE(ptn[ptnEnd]) != 0x00) && (ptnEnd > 0)) {
		ptnEnd--;
	}
	defSkipLen = ptnEnd;
	for (i = 0; i < ptnEnd; i++)
	{
		if (HIBYTE(ptn[i]) != 0x00) {
			defSkipLen = ptnEnd - i;
		}
	}

	for (i = 0; i < UCHAR_MAX + 1; i++)
	{
		skipLen[i] = defSkipLen;
	}

	for (i = 0; i < ptnEnd; i++)
	{
		if (HIBYTE(ptn[i]) == 0x00)
		{
			skipLen[LOBYTE(ptn[i])] = ptnEnd - i;
		}
	}

	searchSuccessCount = 0;
	p = (LPBYTE)dllInfo.lpBaseOfDll;
	LPBYTE searchEnd = (LPBYTE)dllInfo.lpBaseOfDll + dllInfo.SizeOfImage;

	while (p + ptn_size < searchEnd)
	{
		scan = ptnEnd;
		while (scan >= 0)
		{
			WCHAR asdfd[100];
			wsprintf(asdfd, L"ptn try search at 0x%08X\n", p);
			//SetLogText(asdfd);
			if ((HIBYTE(ptn[scan]) == 0x00) && (LOBYTE(ptn[scan]) != p[scan]))
				break;
			if (scan == 0)
			{
				*addr = p;
				searchSuccessCount++;
			}
			scan--;
		}
		p += skipLen[p[ptnEnd]];
	}
	if (searchSuccessCount != 1) addr = 0;
	return searchSuccessCount;
}

bool hook_userdict(void)
{
	WORD ptn[] = { 0x8B, 0x43, 0x14, 0x8B, 0x3D, -1, -1, -1, -1, 0x68, -1, -1, -1, -1, 0x50, 0xFF, 0xD7, 0x8B, 0xF0 };

	LPBYTE addr = 0;
	int r = search_ptn(ptn, _countof(ptn), &addr);

	if (r == 0)
	{
		SetLogText(L"j2kengine ptn search failed\n");
	}
	else if (r > 1)
	{
		SetLogText(L"j2kengine ptn multi found\n");
		return false;
	}
	else
	{
		WCHAR asdf[100];
		wsprintf(asdf, L"ptn found at address 0x%08X\n", addr);
		SetLogText(asdf);
	}

	return true;
}

__declspec(naked) void fopen_patch(void)
{
	LPSTR _path;
	__asm
	{
		MOV EAX, DWORD PTR SS : [ESP+0x04]
		MOV DWORD PTR DS : [_path], EAX
	}
	if (strstr(_path, "UserDict.jk") > 0)
	{
		__asm
		{
			LEA EAX, _path; // temp
			MOV DWORD PTR SS : [ESP+0x14+0x04], EAX
		}
	}

	__asm JMP msvcrt_fopen;
}