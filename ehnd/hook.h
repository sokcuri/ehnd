#pragma once
bool hook();
bool hook_userdict();
bool hook_userdict2();
void *fopen_patch(char *path, char *mode);
void userdict_patch();
UINT calculate_hash(LPCSTR s, int n);
size_t strlen_inline(LPCSTR str);
