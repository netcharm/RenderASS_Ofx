// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "stdafx.h"
#include <shlobj.h>
#include <tchar.h>
#include <time.h>
#include <string>

TCHAR current_dll_path[MAX_PATH]; // hack

// the point of this win32 api exercise is to get the path of the dll,
// so we can try to load a fontconfig configuration file from a path
// relative to the dll's path.
DWORD GetModulePath(HMODULE hModule, LPTSTR pszBuffer, DWORD dwSize) 
{
	DWORD dwLength = GetModuleFileName(hModule, pszBuffer, dwSize);
	if (dwLength) {
		while (dwLength && pszBuffer[dwLength] != _T('\\')) {
			dwLength--;
		}
		if (dwLength)
			pszBuffer[dwLength + 1] = _T('\000');
	}
	return dwLength;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	GetModulePath(hModule, current_dll_path, MAX_PATH);
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
	}
	return TRUE;
}

