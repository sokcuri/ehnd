#include "stdafx.h"
#include "hook.h"
#include "ehnd.h"

LPBYTE lpfnRetn, lpfnfopen;
LPBYTE lpfnwc2mb, lpfnmb2wc;
int wc2mb_type = 0, mb2wc_type = 0;

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

bool GetRealWC2MB(void)
{
	HMODULE hDll = GetModuleHandle(L"kernel32.dll");
	lpfnwc2mb = (LPBYTE)GetProcAddress(hDll, "WideCharToMultiByte");

	// KERNEL32 HOOK
	// 756770D0 >  8BFF            MOV EDI, EDI
	// 756770D2    55              PUSH EBP
	// 756770D3    8BEC            MOV EBP, ESP

	BOOL bMatch = true;
	WORD ptn_kernel32[] = { 0x8B, 0xFF, 0x55, 0x8B, 0xEC, 0x5D };

	for (int i = 0; i < _countof(ptn_kernel32); i++)
		if (*(lpfnwc2mb + i) != ptn_kernel32[i])
		{
			bMatch = false;
			break;
		}

	if (bMatch)
	{
		lpfnwc2mb += 6;
		wc2mb_type = 1;
		return true;
	}

	// AILAYER HOOK
	// 230083D2    55              PUSH EBP
	// 230083D3    8BEC            MOV EBP, ESP
	// 230083D5    833D A8360123 0>CMP DWORD PTR DS : [230136A8], 0
	// 230083DC    74 11           JE SHORT AlLayer.230083EF
	// 230083DE    817D 08 E9FD000>CMP DWORD PTR SS : [EBP + 8], 0FDE9
	// 230083E5    74 08           JE SHORT AlLayer.230083EF


	bMatch = true;
	WORD ptn_ailayer[] = { 0x55, 0x8B, 0xEC, 0x83, 0x3D };

	for (int i = 0; i < _countof(ptn_ailayer); i++)
		if (*(lpfnwc2mb + i) != ptn_ailayer[i])
		{
			bMatch = false;
			break;
		}

	if (bMatch)
	{
		if (*(lpfnwc2mb + 0x1E) == 0xA1)
		{
			LPBYTE l1, l2, l3;
			char *p = (char *)&l1;
			for (int i = 0; i < 4; i++)
				p[i] = *(lpfnwc2mb + 0x1F + i);
			p = (char *)&l2;
			for (int i = 0; i < 4; i++)
				p[i] = *(l1 + i);
			l2 += 0x15C;
			p = (char *)&l3;
			for (int i = 0; i < 4; i++)
				p[i] = *(l2 + i);

			bMatch = true;
			for (int i = 0; i < _countof(ptn_kernel32); i++)
				if (*(l3 + i) != ptn_kernel32[i])
				{
					bMatch = false;
					break;
				}

			if (bMatch)
			{
				lpfnwc2mb = l3 + 6;
				wc2mb_type = 1;
				WriteLog(NORMAL_LOG, L"lpfnwc2mb: %x\n", lpfnwc2mb);
				return true;
			}
		}
	}
	return true;
}

bool GetRealMB2WC(void)
{
	HMODULE hDll = GetModuleHandle(L"kernel32.dll");
	lpfnmb2wc = (LPBYTE)GetProcAddress(hDll, "MultiByteToWideChar");

	// KERNEL32 HOOK
	// 756770D0 >  8BFF            MOV EDI, EDI
	// 756770D2    55              PUSH EBP
	// 756770D3    8BEC            MOV EBP, ESP

	BOOL bMatch = true;
	WORD ptn_kernel32[] = { 0x8B, 0xFF, 0x55, 0x8B, 0xEC, 0x5D };

	for (int i = 0; i < _countof(ptn_kernel32); i++)
		if (*(lpfnmb2wc + i) != ptn_kernel32[i])
		{
			bMatch = false;
			break;
		}

	if (bMatch)
	{
		lpfnmb2wc += 6;
		mb2wc_type = 1;
		return true;
	}

	// AILAYER HOOK
	// 230083D2    55              PUSH EBP
	// 230083D3    8BEC            MOV EBP, ESP
	// 230083D5    833D A8360123 0>CMP DWORD PTR DS : [230136A8], 0
	// 230083DC    74 11           JE SHORT AlLayer.230083EF
	// 230083DE    817D 08 E9FD000>CMP DWORD PTR SS : [EBP + 8], 0FDE9
	// 230083E5    74 08           JE SHORT AlLayer.230083EF

	bMatch = true;
	WORD ptn_ailayer[] = { 0x55, 0x8B, 0xEC, 0x83, 0x3D };

	for (int i = 0; i < _countof(ptn_ailayer); i++)
		if (*(lpfnmb2wc + i) != ptn_ailayer[i])
		{
			bMatch = false;
			break;
		}

	if (bMatch)
	{
		if (*(lpfnmb2wc + 0x40) == 0xA1)
		{
			LPBYTE l1, l2, l3;
			char *p = (char*)&l1;
			for (int i = 0; i < 4; i++)
				p[i] = *(lpfnmb2wc + 0x41 + i);
			p = (char*)&l2;
			for (int i = 0; i < 4; i++)
				p[i] = *(l1 + i);
			l2 += 0x144;
			p = (char*)&l3;
			for (int i = 0; i < 4; i++)
				p[i] = *(l2 + i);

			bMatch = true;
			for (int i = 0; i < _countof(ptn_kernel32); i++)
				if (*(l3 + i) != ptn_kernel32[i])
				{
					bMatch = false;
					break;
				}

			if (bMatch)
			{
				lpfnmb2wc = l3 + 6;
				mb2wc_type = 1;
				WriteLog(NORMAL_LOG, L"lpfnlpfnmb2wc: %x\n", lpfnmb2wc);
				return true;
			}
		}
	}
	return true;
}

__declspec(naked) int __stdcall _WideCharToMultiByte(UINT a, DWORD b, LPCWSTR c, int d, LPSTR e, int f, LPCSTR g, LPBOOL h)
{
	if (wc2mb_type == 1)
	{
		__asm
		{
			MOV EDI, EDI // kernelbase
			PUSH EBP
			MOV EBP, ESP
			POP EBP
		}
	}
	__asm JMP lpfnwc2mb
}

__declspec(naked) int __stdcall _MultiByteToWideChar(UINT a, DWORD b, LPCSTR c, int d, LPWSTR e, int f)
{
	if (mb2wc_type == 1)
	{
		__asm
		{
			MOV EDI, EDI // kernelbase
			PUSH EBP
			MOV EBP, ESP
			POP EBP
		}
	}
	__asm JMP lpfnmb2wc
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

unsigned int CalHash(const char *str)
{
	unsigned int hash = 5381;

	unsigned int test = 0;
	int c = 0;

	while (c = *str++)
	{
		hash = ((hash << 5) + hash) + c;
	}

	return (hash & 0x7FFFFFFF);
}
unsigned int CalHashC(const char *str, int count)
{
	unsigned int hash = 5381;

	unsigned int test = 0;
	int c = 0;
	int i = 0;

	while (c = *str++)
	{
		hash = ((hash << 5) + hash) + c;
		i++;
		if (i == count) break;
	}

	return (hash & 0x7FFFFFFF);
}
size_t strlen_inline(const char *str)
{
	register const char* i;
	for (i = str; *i; ++i);
	return (i - str);
}

__declspec(naked) void userdict_patch(void)
{

	// binary search (lower bound)
	// [ESP + 0x10]	// current addr (start)
	// [ESP + 0x18]	// current count (end)
	// [EBP + 0x08]	// total count
	// [ESP + 0x38] // word_string
	//
	// [EBP + 0x04] + 0x6E * cnt + 0x01 = USERDICT_JPN
	//

	// 일치하는 단어가 나오면 단어 등록을 하고 다시 돌아옴

	__asm
	{
		// word_str
		MOV ESI, DWORD PTR SS : [ESP+0x38]

		CMP DWORD PTR SS : [EBP+0x08], 0
		JE lFinish

		MOV EAX, DWORD PTR SS : [ESP+0x18]
		MOV DWORD PTR SS : [ESP+0x10], EAX // start
		MOV EAX, DWORD PTR SS : [EBP+0x08]
		DEC EAX
		MOV DWORD PTR SS : [ESP+0x18], EAX // end
		//MOV DWORD PTR SS : [ESP+0x08], 0 // index

	lLoop:
		// start < end, loop
		MOV EAX, DWORD PTR SS : [ESP+0x10]
		MOV ECX, DWORD PTR SS : [ESP+0x18]
		CMP EAX, ECX
		JE lMatch
		JA lFinish

		// check = (start+end)/2
		ADD EAX, ECX
		XOR EDX, EDX
		MOV ECX, 2
		DIV ECX

		// EDX = check
		MOV EBX, EAX

		// dic_str = base + 0x6E * check + 0x01
		MOV ECX, 0x6E
		MUL ECX
		ADD EAX, DWORD PTR SS : [EBP+0x04]
		ADD EAX, 1

		// EDI = dic_str
		MOV EDI, EAX

		XOR ECX, ECX
	lCompare:
		CMP CL, 31
		JAE lHigh
		MOV AL, BYTE PTR DS : [ESI+ECX]
		MOV DL, BYTE PTR DS : [EDI+ECX]
		INC CL
		CMP AL, 0x7E
		JBE lC1
		JMP lC2

		// 0x00~0x7E
	lC1:
		CMP AL, 0
		JE lLow
		CMP AL, DL
		JE lCompare
		JA lLow
		JB lHigh

		// 0x7F~0xFF
	lC2:
		CMP AL, DL
		JA lLow
		JB lHigh
		
		MOV AL, BYTE PTR DS : [ESI+ECX]
		MOV DL, BYTE PTR DS : [EDI+ECX]
		INC CL
		
		JMP lC1

	lHigh:
		INC EBX
		MOV DWORD PTR SS : [ESP+0x10], EBX
		JMP lLoop	
	lLow:
		MOV DWORD PTR SS : [ESP+0x18], EBX
		JMP lLoop
	lMatch:
		// start = key
		MOV EAX, DWORD PTR SS : [ESP+0x10]

		// dic_str = base+key*0x6E+0x01
		MOV ECX, 0x6E
		MUL ECX
		ADD EAX, DWORD PTR SS : [EBP+0x04]
		ADD EAX, 1

		// ESI=word_str
		// EDI=dic_str
		MOV EDI, EAX

		XOR ECX, ECX
	lMCompare:
		CMP CL, 31
		JAE lMatchEnd
		MOV AL, BYTE PTR DS : [ESI+ECX]
		MOV DL, BYTE PTR DS : [EDI+ECX]
		INC CL
		CMP AL, 0x7E
		JBE lMC1
		JMP lMC2

		// 0x00~0x7E
	lMC1:
		CMP AL, DL
		JNE lFinish
		CMP AL, 0
		JE lMatchEnd
		JMP lMCompare

		// 0x7F~0xFF
	lMC2:
		CMP AL, DL
		JNE lFinish
		
		MOV AL, BYTE PTR DS : [ESI+ECX]
		MOV DL, BYTE PTR DS : [EDI+ECX]
		INC CL
		
		JMP lMC1
	lMatchEnd:
		// addr=base+point*0x6E
		MOV EAX, DWORD PTR SS : [ESP+0x10]
		MOV ECX, 0x6E
		MUL ECX
		MOV DWORD PTR SS : [ESP+0x10], EAX
		ADD EAX, DWORD PTR SS : [EBP + 0x04]
		MOV EBX, DWORD PTR SS : [ESP + 0x38]
		MOV CL, 1
		TEST CL, CL
		JMP lpfnRetn
	lFinish:
		MOV EDX, DWORD PTR SS : [EBP+0x08]
		MOV DWORD PTR SS : [ESP+0x18], EDX
		MOV EBX, DWORD PTR SS:[ESP+0x38]
		MOV CL, 0
		TEST CL, CL
		JMP lpfnRetn
	}

}