#include "stdafx.h"
#include "hook.h"
#include "ehnd.h"

LPBYTE lpfnRetn;

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

	// Modified BMH - http://en.wikipedia.org/wiki/Boyer-Moore-Horspool_algorithm

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
	WORD ptn[] = { 0x8B, 0x4D, 0x04, 0x03, 0xC1, 0x80, 0x38 };

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
		wsprintf(asdf, L"userdict_patch 0x%08X\n", &userdict_patch);
		SetLogText(asdf);
		BYTE Patch[] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0xE9, -1, -1, -1, -1, 0x90, 0x90, 0x74, 0x08 };

		int PatchSize = _countof(Patch);
		LPBYTE Offset = (LPBYTE)((LPBYTE)&userdict_patch - (addr + 10));
		lpfnRetn = addr + 10;
		Patch[6] = (WORD)LOBYTE(LOWORD(Offset));
		Patch[7] = (WORD)HIBYTE(LOWORD(Offset));
		Patch[8] = (WORD)LOBYTE(HIWORD(Offset));
		Patch[9] = (WORD)HIBYTE(HIWORD(Offset));

		DWORD OldProtect, OldProtect2;
		HANDLE hHandle;
		hHandle = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE, FALSE, GetCurrentProcessId());
		VirtualProtectEx(hHandle, (void *)addr, 5, PAGE_EXECUTE_READWRITE, &OldProtect);
		memcpy(addr, Patch, PatchSize);
		hHandle = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE, FALSE, GetCurrentProcessId());
		VirtualProtectEx(hHandle, (void *)addr, PatchSize, OldProtect, &OldProtect2);
		SetLogText(L"success userdict hook.\n");
	}

	return true;
}

bool hook_userdict2(void)
{
	// 101C4B00 . 8B43 14        MOV EAX,DWORD PTR DS:[EBX+14]
	// 101C4B03 . 8B3D A4192A10  MOV EDI,DWORD PTR DS:[<&MSVCRT.fopen>]   ;  msvcrt.fopen << intercept here
	// 101C4B09 . 68 60982110    PUSH J2KEngin.10219860                   ; /mode = "rb"
	// 101C4B0E . 50             PUSH EAX                                 ; |path
	// 101C4B0F . FFD7           CALL EDI                                 ; \fopen
	// 101C4B11 . 8BF0           MOV ESI,EAX

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
		
		addr += 5;
		BYTE Patch[4];
		int PatchSize = _countof(Patch);
		LPBYTE Offset = (LPBYTE)(fopen_patch);
		Patch[0] = (WORD)LOBYTE(LOWORD(&Offset));
		Patch[1] = (WORD)HIBYTE(LOWORD(&Offset));
		Patch[2] = (WORD)LOBYTE(HIWORD(&Offset));
		Patch[3] = (WORD)HIBYTE(HIWORD(&Offset));

		DWORD OldProtect, OldProtect2;
		HANDLE hHandle;
		hHandle = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE, FALSE, GetCurrentProcessId());
		VirtualProtectEx(hHandle, (void *)addr, 5, PAGE_EXECUTE_READWRITE, &OldProtect);
		memcpy(addr, Patch, PatchSize);
		hHandle = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE, FALSE, GetCurrentProcessId());
		VirtualProtectEx(hHandle, (void *)addr, PatchSize, OldProtect, &OldProtect2);

		SetLogText(L"success userdict2 hook.\n");
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
	if (strstr(_path, "userdict.jk"))
	{
		SetLogText(L"fopen: userdict.jk\n");

		__asm
		{
			LEA EAX, _path; // temp
			MOV DWORD PTR SS : [ESP+0x14+0x04], EAX
		}
	}

	__asm JMP msvcrt_fopen;
}

__declspec(naked) void userdict_patch(void)
{
	SetLogText(L"userdict_patch call\n");

	__asm
	{
		MOV EDI, DWORD PTR SS : [EBP+0x04]
		MOV EBX, DWORD PTR SS : [ESP+0x38]
		MOV EAX, DWORD PTR SS : [ESP+0x10]

	CntLoop:
		ADD EAX, EDI
		LEA ESI, DWORD PTR DS : [EAX+1]

		MOV AX, WORD PTR DS : [ESI] // userdict
		MOV CX, WORD PTR DS : [EBX] // word

		XCHG AL, AH
		XCHG CL, CH

		TEST AL, AL
		JE Mismatch
		CMP AX, CX
		JA Mismatch
		JB Finish

		PUSH -1
		PUSH ESI
		CALL calculate_hash
		ADD ESP, 0x04
		PUSH EAX

		PUSH ESI
		CALL strlen_inline
		ADD ESP, 0x04

		PUSH EAX
		PUSH EBX
		CALL calculate_hash
		ADD ESP, 0x08

		POP ECX
		CMP EAX, ECX
		JE Match

	Mismatch:
		INC DWORD PTR SS : [ESP+0x18]		// cur
		MOV EAX, DWORD PTR SS : [ESP+0x10]	// addr
		MOV EDX, DWORD PTR SS : [EBP+0x08]	// total
		ADD EAX, 0x6E
		CMP DWORD PTR DS : [ESP+0x18], EDX
		MOV DWORD PTR SS : [ESP+0x10], EAX

		JL CntLoop
		XOR AL, AL
		TEST AL, AL
		JMP lpfnRetn
	Match:
		AND AL, AL
		TEST AL, AL
		MOV EAX, DWORD PTR SS : [ESP+0x10]	// addr
		ADD EAX, DWORD PTR SS : [EBP+0x04]
		JMP lpfnRetn

	Finish:
		MOV EDX, DWORD PTR SS : [EBP+0x08]
		MOV DWORD PTR SS : [ESP+0x18], EDX
		XOR AL, AL
		TEST AL, AL
		JMP lpfnRetn
	}
}

UINT calculate_hash(LPCSTR s, int n)
{
	UINT hash = 5381;
	int c = 0;
	for (int i = 0; c = *s++ || i != n; i++)
		hash = ((hash << 5) + hash) + c;
	return (hash & 0x7FFFFFFF);
}

size_t strlen_inline(LPCSTR str)
{
	register const char* i;
	for (i = str; *i; ++i);
	return (i - str);
}