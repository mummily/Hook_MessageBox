// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "stdafx.h"

#include <Dbghelp.h>
#pragma comment(lib, "dbghelp.lib") 

#include <detours.h>
#include <string>
#pragma comment(lib,"detours.lib")

HANDLE process;
static HMODULE s_hDll;

typedef int (WINAPI * PFN_MESSAGEBOXA)(HWND hWnd,
    LPCSTR lpText,
    LPCSTR lpCaption,
    UINT uType);

typedef int (WINAPI * PFN_MESSAGEBOXW)(HWND hWnd,
    LPCWSTR lpText,
    LPCWSTR lpCaption,
    UINT uType);

//目标函数指针
PFN_MESSAGEBOXA g_oldMessageBoxA = NULL;
PFN_MESSAGEBOXW g_oldMessageBoxW = NULL;

int WINAPI MyMessageBoxA(HWND hWnd,
    LPCSTR lpText,
    LPCSTR lpCaption,
    UINT uType)
{
    std::string sCaption = lpCaption;
    sCaption += " Attached by Detour";
    std::string sText = lpText;
    sText += " Modified in MessageBoxA";
    return g_oldMessageBoxA(hWnd, sText.data(), sCaption.data(), uType);
}

int WINAPI MyMessageBoxW(HWND hWnd,
    LPCWSTR lpText,
    LPCWSTR lpCaption,
    UINT uType)
{
    std::wstring sCaption= lpCaption;
    sCaption += L" Attached by Detour";
    std::wstring sText = lpText;
    sText += L" Modified in MessageBoxW";
    return g_oldMessageBoxW(hWnd, sText.data(), sCaption.data(), uType);
}

extern "C" _declspec(dllexport) BOOL APIENTRY SetHook()
{
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    g_oldMessageBoxA = (PFN_MESSAGEBOXA)DetourFindFunction("user32.dll", "MessageBoxA");
    g_oldMessageBoxW = (PFN_MESSAGEBOXW)DetourFindFunction("user32.dll", "MessageBoxW");
    DetourAttach(&(PVOID&)g_oldMessageBoxA, MyMessageBoxA);
    DetourAttach(&(PVOID&)g_oldMessageBoxW, MyMessageBoxW);

    LONG ret = DetourTransactionCommit();
    return ret == NO_ERROR;
}

extern "C" _declspec(dllexport) BOOL APIENTRY DropHook()
{
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    DetourDetach(&(PVOID&)g_oldMessageBoxA, MyMessageBoxA);
    DetourDetach(&(PVOID&)g_oldMessageBoxW, MyMessageBoxW);

    LONG ret = DetourTransactionCommit();
    return ret == NO_ERROR;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
        process = GetCurrentProcess();
        SymInitialize(process, NULL, TRUE);
        OutputDebugStringA("start hook....................................");
        s_hDll = hModule;
        DisableThreadLibraryCalls(hModule);
        SetHook();
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
	case DLL_PROCESS_DETACH:
        DropHook();
        OutputDebugStringA("end hook....................................");
		break;
	}
	return TRUE;
}

