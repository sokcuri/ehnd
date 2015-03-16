#include "stdafx.h"
#include "hook.h"
#include "ehnd.h"

LPBYTE lpfnRetn, lpfnfopen;
LPBYTE lpfnwc2mb, lpfnmb2wc;

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

	GetLoadPath(lpEztPath, MAX_PATH);

	wcscpy_s(lpDllPath, lpEztPath);
	wcscat_s(lpDllPath, L"\\j2kengine.dlx");
	hDll = LoadLibrary(lpDllPath);
	if (!hDll)
	{
		MessageBox(0, L"J2KEngine.dlx Load Failed", L"EzTransHook", MB_ICONERROR);
		return false;
	}

	for (i = 0; i < _countof(aEztFunction); i++)
	{
		apfnEzt[i] = GetProcAddress(hDll, aEztFunction[i]);
		if (!apfnEzt[i])
		{
			MessageBox(0, L"J2KEngine.dlx Function Load Failed", L"EzTransHook", MB_ICONERROR);
			return false;
		}
	}

	lpDllPath[0] = 0;
	GetSystemDirectory(lpDllPath, MAX_PATH);
	wcscat_s(lpDllPath, L"\\msvcrt.dll");
	hDll2 = LoadLibrary(lpDllPath);
	if (!hDll2)
	{
		MessageBox(0, L"MSVCRT.DLL Load Failed", L"EzTransHook", MB_ICONERROR);
		return false;
	}

	for (i = 0; i < _countof(aMsvFunction); i++)
	{
		apfnMsv[i] = GetProcAddress(hDll2, aMsvFunction[i]);
		if (!apfnMsv[i])
		{
			MessageBox(0, L"MSVCRT.DLL Function Load Failed", L"EzTransHook", MB_ICONERROR);
			return false;
		}
	}

	hEzt = hDll;
	hMsv = hDll2;

	return true;
}

int search_ptn(LPWORD ptn, size_t ptn_size, LPBYTE *addr)
{
	HMODULE hDll = GetModuleHandle(L"j2kengine.dlx");
	if (hDll == NULL) MessageBox(0, L"J2KEngine.dlx Load Failed", L"", 0);

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
		WriteLog(NORMAL_LOG, L"HookUserDict : J2KEngine Pattern Search Failed\n");
		return false;
	}
	else if (r > 1)
	{
		WriteLog(NORMAL_LOG, L"HookUserDict : J2KEngine Pattern Search Failed\n");
		return false;
	}
	else
	{
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
		VirtualProtectEx(hHandle, (void *)addr, PatchSize, PAGE_EXECUTE_READWRITE, &OldProtect);
		memcpy(addr, Patch, PatchSize);
		hHandle = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE, FALSE, GetCurrentProcessId());
		VirtualProtectEx(hHandle, (void *)addr, PatchSize, OldProtect, &OldProtect2);
		WriteLog(NORMAL_LOG, L"HookUserDict : Success.\n");
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
		WriteLog(NORMAL_LOG, L"HookUserDict2 : J2KEngine Pattern Search Failed\n");
		return false;
	}
	else if (r > 1)
	{
		WriteLog(NORMAL_LOG, L"HookUserDict2 : J2kEngine Pattern Search Failed\n");
		return false;
	}
	else
	{
		addr += 5;
		BYTE Patch[4];
		int PatchSize = _countof(Patch);
		lpfnfopen = (LPBYTE)(fopen_patch);
		Patch[0] = (WORD)LOBYTE(LOWORD(&lpfnfopen));
		Patch[1] = (WORD)HIBYTE(LOWORD(&lpfnfopen));
		Patch[2] = (WORD)LOBYTE(HIWORD(&lpfnfopen));
		Patch[3] = (WORD)HIBYTE(HIWORD(&lpfnfopen));

		DWORD OldProtect, OldProtect2;
		HANDLE hHandle;
		hHandle = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE, FALSE, GetCurrentProcessId());
		VirtualProtectEx(hHandle, (void *)addr, PatchSize, PAGE_EXECUTE_READWRITE, &OldProtect);
		memcpy(addr, Patch, PatchSize);
		hHandle = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE, FALSE, GetCurrentProcessId());
		VirtualProtectEx(hHandle, (void *)addr, PatchSize, OldProtect, &OldProtect2);

		WriteLog(NORMAL_LOG, L"HookUserDict2 : Success.\n");
	}

	return true;
}

bool hook_wc2mb(void)
{
	HMODULE hDll = GetModuleHandle(L"kernel32.dll");
	lpfnwc2mb = (LPBYTE)GetProcAddress(hDll, "WideCharToMultiByte");

	// 00h: JMP XXX = (target addr - next inst addr)
	// 005: ~~~~

	BYTE Patch[5];
	int PatchSize = _countof(Patch);
	Patch[0] = 0xE9;
	Patch[1] = LOBYTE(LOWORD(((LPBYTE)&_func_wc2mb - lpfnwc2mb - 5)));
	Patch[2] = HIBYTE(LOWORD(((LPBYTE)&_func_wc2mb - lpfnwc2mb - 5)));
	Patch[3] = LOBYTE(HIWORD(((LPBYTE)&_func_wc2mb - lpfnwc2mb - 5)));
	Patch[4] = HIBYTE(HIWORD(((LPBYTE)&_func_wc2mb - lpfnwc2mb - 5)));

	DWORD OldProtect, OldProtect2;
	HANDLE hHandle;
	hHandle = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE, FALSE, GetCurrentProcessId());
	VirtualProtectEx(hHandle, (void *)lpfnwc2mb, PatchSize, PAGE_EXECUTE_READWRITE, &OldProtect);
	memcpy(lpfnwc2mb, Patch, PatchSize);
	hHandle = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE, FALSE, GetCurrentProcessId());
	VirtualProtectEx(hHandle, (void *)lpfnwc2mb, PatchSize, OldProtect, &OldProtect2);

	lpfnwc2mb += 5;

	//lpfnwc2mb = (LPBYTE)*ref + 0x05;
	//LPBYTE lpfnwc2mb_main = (LPBYTE)*(lpfnwc2mb);

	//WriteLog(NORMAL_LOG, L"lpfnwc2mb: %8x\n", lpfnwc2mb);

	return true;
}

bool hook_mb2wc(void)
{
	HMODULE hDll = GetModuleHandle(L"kernel32.dll");
	lpfnmb2wc = (LPBYTE)GetProcAddress(hDll, "MultiByteToWideChar");

	// 00h: JMP XXX = (target addr - next inst addr)
	// 005: ~~~~
	
	// disable hook_mb2wc
	
	/*
	BYTE Patch[5];
	int PatchSize = _countof(Patch);
	Patch[0] = 0xE9;
	Patch[1] = LOBYTE(LOWORD(((LPBYTE)&_func_mb2wc - lpfnmb2wc - 5)));
	Patch[2] = HIBYTE(LOWORD(((LPBYTE)&_func_mb2wc - lpfnmb2wc - 5)));
	Patch[3] = LOBYTE(HIWORD(((LPBYTE)&_func_mb2wc - lpfnmb2wc - 5)));
	Patch[4] = HIBYTE(HIWORD(((LPBYTE)&_func_mb2wc - lpfnmb2wc - 5)));

	DWORD OldProtect, OldProtect2;
	HANDLE hHandle;
	hHandle = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE, FALSE, GetCurrentProcessId());
	VirtualProtectEx(hHandle, (void *)lpfnmb2wc, PatchSize, PAGE_EXECUTE_READWRITE, &OldProtect);
	memcpy(lpfnmb2wc, Patch, PatchSize);
	hHandle = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE, FALSE, GetCurrentProcessId());
	VirtualProtectEx(hHandle, (void *)lpfnmb2wc, PatchSize, OldProtect, &OldProtect2);
	*/
	lpfnmb2wc += 5;

	//lpfnwc2mb = (LPBYTE)*ref + 0x05;
	//LPBYTE lpfnwc2mb_main = (LPBYTE)*(lpfnwc2mb);

	//WriteLog(NORMAL_LOG, L"lpfnmb2wc: %8x\n", lpfnmb2wc);

	return true;
}

__declspec(naked) int _func_wc2mb(void)
{
	// stdcall head
	// 767BF830 >  8BFF            MOV EDI,EDI
	// 767BF832    55              PUSH EBP
	// 767BF833    8BEC            MOV EBP, ESP

	__asm
	{
		PUSH EBP
		MOV EBP, ESP
		//POP EBP
	
		PUSH DWORD PTR SS : [EBP + 0x24]
		PUSH DWORD PTR SS : [EBP + 0x20]
		PUSH DWORD PTR SS : [EBP + 0x1C]
		PUSH DWORD PTR SS : [EBP + 0x18]
		PUSH DWORD PTR SS : [EBP + 0x14]
		PUSH DWORD PTR SS : [EBP + 0x10]
		PUSH DWORD PTR SS : [EBP + 0x0C]
		PUSH DWORD PTR SS : [EBP + 0x08]
		CALL func_wc2mb
		ADD ESP, 0x20
		POP EBP

		MOV EDI, EDI
		PUSH EBP
		MOV EBP, ESP
		JMP [lpfnwc2mb]
	}
}

__declspec(naked) int __stdcall _func_mb2wc(UINT a, DWORD b, LPCSTR c, int d, LPWSTR e, int f)
{
	// stdcall head
	// 767BF830 >  8BFF            MOV EDI,EDI
	// 767BF832    55              PUSH EBP
	// 767BF833    8BEC            MOV EBP, ESP

	__asm
	{
		PUSH EBP
		MOV EBP, ESP
		POP EBP
		/*
		PUSH DWORD PTR SS : [EBP + 0x1C]
		PUSH DWORD PTR SS : [EBP + 0x18]
		PUSH DWORD PTR SS : [EBP + 0x14]
		PUSH DWORD PTR SS : [EBP + 0x10]
		PUSH DWORD PTR SS : [EBP + 0x0C]
		PUSH DWORD PTR SS : [EBP + 0x08]
		CALL func_mb2wc
		ADD ESP, 0x18
		POP EBP

		//CMP EAX, -1
		//JE lNext

		//RETN 0x18

		lNext:*/
		MOV EDI, EDI
		PUSH EBP
		MOV EBP, ESP
		JMP [lpfnmb2wc]
	}
}

int func_wc2mb(UINT a, DWORD b, LPCWSTR c, int d, LPSTR e, int f, LPCSTR g, LPBOOL h)
{
	if (a == 932)
		watchStr.assign(c);
	return true;
	//WriteLog(NORMAL_LOG, L"func_wc2mb: %d %d %s %d\n", a, b, c, d);
	// Unicode -> 932
	/*if (a == 932)
	{
		watchStr = c;
		WriteLog(NORMAL_LOG, L"watchStr: %s\n", c);
	}
	return true;*/
}

int func_mb2wc(UINT a, DWORD b, LPCSTR c, int d, LPWSTR e, int f)
{
	/*
	// EHND PADDING 문자열이 발견되면 뒤쪽에 있는 유니코드 문자열을 냅다 꺼낸다
	if (c[strlen(c) + 1] == '#')
	{
		if (!strcmp(c + strlen(c) + 1, "##EHND##"))
		{
			char *p = (char *)c;
			p += strlen(c) + 10;
			int len = wcslen((wchar_t *)p);

			if (f == 0)
				return len;

			if (len < f) f = len;
			//wcscpy_s(e, f, (wchar_t *)p);
			memcpy(e, p, (f + 1) * 2);

			WriteLog(NORMAL_LOG, L"EHND_PADDING : %s\n", e);
			return (int)e;
		}
	}*/
	return -1;
}

__declspec(naked) int __stdcall _WideCharToMultiByte(UINT a, DWORD b, LPCWSTR c, int d, LPSTR e, int f, LPCSTR g, LPBOOL h)
{
	__asm
	{
		MOV EDI, EDI // kernelbase
		PUSH EBP
		MOV EBP, ESP

		JMP lpfnwc2mb
	}
}

__declspec(naked) int __stdcall _MultiByteToWideChar(UINT a, DWORD b, LPCSTR c, int d, LPWSTR e, int f)
{
	__asm
	{
		MOV EDI, EDI // kernelbase
		PUSH EBP
		MOV EBP, ESP

		JMP lpfnmb2wc
	}
}

void *fopen_patch(char *path, char *mode)
{
	if (strstr(path, "UserDict.jk"))
	{
		path = g_DicPath;
		//WriteLog(NORMAL_LOG, L"fopen_path\n");
	}
	return msvcrt_fopen(path, mode);
}

__declspec(naked) void userdict_patch(void)
{
	//WriteLog(NORMAL_LOG, L"userdict_patch call\n");

	//
	// [ESP + 0x18]	// current count - start
	// [ESP + 0x10]	// current addr - end
	// [EBP + 0x08]	// total count
	// [ESP + 0x38] // word_string
	//
	// [EBP + 0x04] + 0x6E * cnt + 0x01 = USERDICT_JPN
	//

	__asm
	{
		// word_string
		MOV ESI, DWORD PTR SS : [ESP + 0x38]

		// end=total_cnt
		MOV EAX, DWORD PTR SS : [EBP + 0x08]
		
		// if total_cnt = 0 goto zFinish
		CMP EAX, 0
		JE zFinish

		// store end (total_cnt-1)
		SUB EAX, 1
		MOV DWORD PTR SS : [ESP + 0x10], EAX

	zLoop:
		// if start = end goto zMatch
		// if start > end goto zFinish
		MOV ECX, DWORD PTR SS : [ESP + 0x18]
		MOV EAX, DWORD PTR SS : [ESP + 0x10]
		MOV EDI, DWORD PTR SS : [ESP + 0x18]
		CMP ECX, EAX
		JE zMatch
		JA zFinish

		// check=celi(end-start/2)+start
		SUB EAX, ECX
		XOR EDX, EDX
		MOV ECX, 2
		DIV ECX
		ADD EAX, EDX
		ADD EAX, DWORD PTR SS : [ESP + 0x18]

		// store check
		MOV EDI, EAX

		// check_addr=check*0x6e
		MOV ECX, 0x6E
		MUL ECX

		// dic_string=base+check_addr+1
		ADD EAX, DWORD PTR SS : [EBP + 0x04]
		ADD EAX, 1
		MOV EDX, EAX

		// compare
		XOR ECX, ECX
	
	sCompare:
		MOV AL, BYTE PTR DS : [ESI+ECX]
		MOV BL, BYTE PTR DS : [EDX+ECX]
		INC ECX

		CMP AL, 0x7E
		JA sCompare2
		CMP AL, 0
		JE zUpper
		CMP AL, BL
		JA zLower
		JB zUpper
		JMP sCompare
	sCompare2:
		CMP AL, BL
		JA zLower
		JB zUpper

		MOV AL, BYTE PTR DS : [ESI + ECX]
		MOV BL, BYTE PTR DS : [EDX + ECX]
		INC ECX
		CMP AL, BL
		JA zLower
		JB zUpper
		JMP sCompare

	zLower:
		DEC EDI
		MOV DWORD PTR SS : [ESP + 0x10], EDI
		JMP zLoop

	zUpper:
		MOV DWORD PTR SS : [ESP + 0x18], EDI
		JMP zLoop

	zMatch:
		XOR ECX, ECX

		MOV EAX, EDI
		MOV ECX, 0x6E
		MUL ECX
		
		ADD EAX, DWORD PTR SS : [EBP + 0x04]
		ADD EAX, 1
		MOV EDX, EAX

	zCompare :
		CMP ECX, 31
		JAE zMatchEnd
		MOV AL, BYTE PTR DS : [ESI+ECX]
		MOV BL, BYTE PTR DS : [EDX + ECX]
		INC ECX
		CMP AL, 0x7E
		JA zCompare2
		CMP AL, 0
		JE zMatchEnd
		CMP AL, BL
		JNE zFinish
		JMP zCompare
	zCompare2:
		CMP AL, BL
		JNE zFinish
		MOV AL, BYTE PTR DS : [ESI+ECX]
		MOV BL, BYTE PTR DS : [EDX+ECX]
		INC ECX
		CMP AL, BL
		JNE zFinish
		JMP zCompare
	zMatchEnd:
		// addr=base+point*0x6E
		MOV DWORD PTR SS : [ESP + 0x18], EDI
		MOV EAX, DWORD PTR SS : [ESP + 0x18]
		MOV ECX, 0x6E
		MUL ECX
		MOV DWORD PTR SS : [ESP + 0x10], EAX
		ADD EAX, DWORD PTR SS : [EBP + 0x04]
		MOV ECX, 1
		TEST CL, CL
		JMP lpfnRetn
	zFinish:
		MOV EDX, DWORD PTR SS : [EBP + 0x08]
		MOV DWORD PTR SS : [ESP + 0x18], EDX
		XOR ECX, ECX
		TEST CL, CL
		JMP lpfnRetn
	}

}