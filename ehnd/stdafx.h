// stdafx.h : 자주 사용하지만 자주 변경되지는 않는
// 표준 시스템 포함 파일 및 프로젝트 관련 포함 파일이
// 들어 있는 포함 파일입니다.
//

#pragma once

#include "targetver.h"
#define EHND_VER "V3.00.150219"
#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용은 Windows 헤더에서 제외합니다.
// Windows 헤더 파일:
#include <windows.h>
#include <WinBase.h>
#include <windef.h>
#include <stdarg.h>
#include <stdlib.h>
#include <Psapi.h>

#include <iostream>
#include <vector>
#include <regex>

using namespace std;
#define PREFILTER 1
#define POSTFILTER 2

// TODO: 프로그램에 필요한 추가 헤더는 여기에서 참조합니다.
#include "ehnd.h"
#include "hook.h"
#include "log.h"
#include "filter.h"

extern HINSTANCE g_hInst;
extern filter *pFilter;
extern int g_initTick;
extern char g_DicPath[MAX_PATH];
extern bool g_PreUsable;
extern bool g_PostUsable;

extern LPBYTE lpfnRetn;
extern LPBYTE lpfnfopen;
extern HMODULE hEzt, hMsv;

#ifdef _UNICODE
#define WIDEN2(x) L ## x
#define WIDEN(x) WIDEN2(x)
#else
#define WIDEN(x)
#endif