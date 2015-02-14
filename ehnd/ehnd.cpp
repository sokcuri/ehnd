// ehnd.cpp : DLL 응용 프로그램을 위해 내보낸 함수를 정의합니다.
//

#include "stdafx.h"
#include "ehnd.h"

FARPROC apfnEzt[100];
FARPROC apfnMsv[100];

// 이지트랜스 API
__declspec(naked) void J2K_Initialize(void)
{
	__asm JMP apfnEzt[4 * 0];
}
void __stdcall J2K_InitializeEx(int data0, LPSTR key)
{
	SetLogText(L"J2K_InitializeEx.\n");
	__asm
	{
		PUSH DWORD PTR DS : [key]
		PUSH data0
		CALL apfnEzt[4 * 1]
	}
}
__declspec(naked) void J2K_FreeMem(void)
{
	__asm JMP apfnEzt[4 * 2];
}
__declspec(naked) void J2K_GetPriorDict(void)
{
	__asm JMP apfnEzt[4 * 3];
}
__declspec(naked) void J2K_GetProperty(void)
{
	__asm JMP apfnEzt[4 * 4];
}
__declspec(naked) void J2K_ReloadUserDict(void)
{
	__asm JMP apfnEzt[4 * 5];
}
__declspec(naked) void J2K_SetDelJPN(void)
{
	__asm JMP apfnEzt[4 * 6];
}
__declspec(naked) void J2K_SetField(void)
{
	__asm JMP apfnEzt[4 * 7];
}
__declspec(naked) void J2K_SetHnj2han(void)
{
	__asm JMP apfnEzt[4 * 8];
}
__declspec(naked) void J2K_SetJWin(void)
{
	__asm JMP apfnEzt[4 * 9];
}
__declspec(naked) void J2K_SetPriorDict(void)
{
	__asm JMP apfnEzt[4 * 10];
}
__declspec(naked) void J2K_SetProperty(void)
{
	__asm JMP apfnEzt[4 * 11];
}
__declspec(naked) void J2K_StopTranslation(void)
{
	__asm JMP apfnEzt[4 * 12];
}
__declspec(naked) void J2K_Terminate(void)
{
	__asm JMP apfnEzt[4 * 13];
}
__declspec(naked) void J2K_TranslateChat(void)
{
	__asm JMP apfnEzt[4 * 14];
}
__declspec(naked) void J2K_TranslateFM(void)
{
	__asm JMP apfnEzt[4 * 15];
}
__declspec(naked) void J2K_TranslateMM(void)
{
	__asm JMP apfnEzt[4 * 16];
}
__declspec(naked) void J2K_TranslateMMEx(void)
{
	__asm JMP apfnEzt[4 * 17];
}
__declspec(naked) void *msvcrt_free(void *_Memory)
{
	__asm JMP apfnMsv[4 * 0];
}
__declspec(naked) void *msvcrt_malloc(size_t _Size)
{
	__asm JMP apfnMsv[4 * 1];
}
__declspec(naked) void *msvcrt_fopen(char *path, char *mode)
{
	__asm JMP apfnMsv[4 * 2];
}
void *__stdcall J2K_TranslateMMNT(int data0, LPSTR szIn)
{
	SetLogText(L"J2K_TranslateMMNT.\n");
	LPSTR szOut;
	wstring wsText, wsOriginal;
	int i_len;
	LPWSTR lpJPN, lpKOR;
	LPSTR szJPN, szKOR;
	i_len = MultiByteToWideChar(932, MB_PRECOMPOSED, szIn, -1, NULL, NULL);
	lpJPN = (LPWSTR)msvcrt_malloc((i_len + 1) * 3);
	if (lpJPN == NULL)
	{
		SetLogText(L"memory allocation error.\n");
		return 0;
	}
	MultiByteToWideChar(932, 0, szIn, -1, lpJPN, i_len);

	wsOriginal = lpJPN;
	wsText = lpJPN;
	msvcrt_free(lpJPN);


	if (!pEhnd->cmd(wsText))
	{
		pEhnd->pre(wsText);

		i_len = WideCharToMultiByte(932, 0, wsText.c_str(), -1, NULL, NULL, NULL, NULL);
		szJPN = (LPSTR)msvcrt_malloc((i_len + 1) * 3);
		if (szJPN == NULL)
		{
			SetLogText(L"memory allocation error.\n");
			return 0;
		}
		WideCharToMultiByte(932, 0, wsText.c_str(), -1, szJPN, i_len, NULL, NULL);

		__asm
		{
			PUSH DWORD PTR DS : [szJPN]
			PUSH data0
			CALL apfnEzt[4 * 18]
			MOV DWORD PTR DS : [szKOR], EAX
		}

		msvcrt_free(szJPN);

		i_len = MultiByteToWideChar(949, MB_PRECOMPOSED, szKOR, -1, NULL, NULL);
		lpKOR = (LPWSTR)msvcrt_malloc((i_len + 1) * 3);
		if (lpKOR == NULL)
		{
			SetLogText(L"memory allocation error.\n");
			return 0;
		}
		MultiByteToWideChar(949, 0, szKOR, -1, lpKOR, i_len);

		wsText = lpKOR;
		msvcrt_free(szKOR);
		msvcrt_free(lpKOR);

		pEhnd->post(wsText);
	}

	i_len = WideCharToMultiByte(949, 0, wsText.c_str(), -1, NULL, NULL, NULL, NULL);
	szOut = (LPSTR)msvcrt_malloc((i_len + 1) * 3);
	if (szOut == NULL)
	{
		SetLogText(L"memory allocation error.\n");
		return 0;
	}
	WideCharToMultiByte(949, 0, wsText.c_str(), -1, szOut, i_len, NULL, NULL);
	return (void *)szOut;
}
__declspec(naked) void J2K_GetJ2KMainDir(void)
{
	__asm JMP apfnEzt[4 * 19];
}

// 내보낸 클래스의 생성자입니다.
// 클래스 정의를 보려면 ehnd.h를 참조하십시오.
Cehnd::Cehnd()
{
	return;
}
Cehnd::~Cehnd()
{
	return;
}
bool Cehnd::pre(wstring &wsText)
{
	return true;
}
bool Cehnd::post(wstring &wsText)
{
	return true;
}
bool Cehnd::cmd(wstring &wsText)
{
	if (wsText[0] != L'/') return false;
	if (!wsText.compare(L"/ver") || !wsText.compare(L"/version"))
	{
		wsText = L"Ehnd ";
		wsText += WIDEN(MARI_VERSION);
		wsText += L", ";
		wsText += WIDEN(__DATE__);
		wsText += L", ";
		wsText += WIDEN(__TIME__);
		wsText += L"\r\n";

		SetLogText(wsText.c_str(), RGB(168, 25, 25), RGB(255, 255, 255));
		return true;
	}
	return false;
}