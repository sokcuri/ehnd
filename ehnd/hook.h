#pragma once
bool hook();
bool hook_userdict();
bool hook_userdict2();

bool hook_wc2mb();
bool hook_mb2wc();
int _func_wc2mb(UINT a, DWORD b, LPCWSTR c, int d, LPSTR e, int f, LPCSTR g, LPBOOL h);
int __stdcall _func_mb2wc(UINT a, DWORD b, LPCSTR c, int d, LPWSTR e, int f);
int func_wc2mb(UINT a, DWORD b, LPCWSTR c, int d, LPSTR e, int f, LPCSTR g, LPBOOL h);
int func_mb2wc(UINT a, DWORD b, LPCSTR c, int d, LPWSTR e, int f);
void *fopen_patch(char *path, char *mode);
void userdict_patch();
UINT calculate_hash(LPCSTR s, int n);
size_t strlen_inline(LPCSTR str);
int __stdcall _WideCharToMultiByte(UINT a, DWORD b, LPCWSTR c, int d, LPSTR e, int f, LPCSTR g, LPBOOL h);
int __stdcall _MultiByteToWideChar(UINT a, DWORD b, LPCSTR c, int d, LPWSTR e, int f);