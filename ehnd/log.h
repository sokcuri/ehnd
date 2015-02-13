#pragma once
#include <Richedit.h>

//bool write_log(const wchar_t* format);

bool CreateLogWin(HINSTANCE);
void SetLogText(LPCWSTR);
void SetLogText(LPCWSTR, COLORREF, COLORREF);
void ClearLog(void);
void ShowLogWin(bool bShow);
LRESULT CALLBACK LogProc(HWND, UINT, WPARAM, LPARAM);
extern HWND hLogRes, hLogWin;