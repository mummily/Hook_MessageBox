// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "stdafx.h"
#include <string>

#include <detours.h>
#pragma comment(lib,"detours.lib")

typedef int (WINAPI * PFN_MESSAGEBOXA)(HWND hWnd,
    LPCSTR lpText,
    LPCSTR lpCaption,
    UINT uType);

typedef int (WINAPI * PFN_MESSAGEBOXW)(HWND hWnd,
    LPCWSTR lpText,
    LPCWSTR lpCaption,
    UINT uType);

//目标函数指针
PFN_MESSAGEBOXA g_pMessageBoxA = MessageBoxA;
PFN_MESSAGEBOXW g_pMessageBoxW = MessageBoxW;

int WINAPI HookMessageBoxA(HWND hWnd,
    LPCSTR lpText,
    LPCSTR lpCaption,
    UINT uType)
{
    std::string sCaption = lpCaption;
    sCaption += " Attached by Detour";
    std::string sText = lpText;
    sText += " Modified in MessageBoxA";
    return g_pMessageBoxA(hWnd, sText.data(), sCaption.data(), uType);
}

int WINAPI HookMessageBoxW(HWND hWnd,
    LPCWSTR lpText,
    LPCWSTR lpCaption,
    UINT uType)
{
    std::wstring sCaption = lpCaption;
    sCaption += L" Attached by Detour";
    std::wstring sText = lpText;
    sText += L" Modified in MessageBoxW";
    return g_pMessageBoxW(hWnd, sText.data(), sCaption.data(), uType);
}

extern "C" _declspec(dllexport) BOOL APIENTRY StartHook()
{
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread()); //只有一个线程，所以用GetCurrentThread  

    //关联目标函数和我们自定义的截获函数
    if (DetourAttach(&(PVOID&)g_pMessageBoxA, HookMessageBoxA) != NO_ERROR)
    {
        OutputDebugStringA("DetourAttach MessageBoxA Fail!\n");
    }

    if (DetourAttach(&(PVOID&)g_pMessageBoxW, HookMessageBoxW) != NO_ERROR)
    {
        OutputDebugStringA("DetourAttach MessageBoxW Fail!\n");
    }

    //完成事务
    return DetourTransactionCommit() == NO_ERROR;
}

extern "C" _declspec(dllexport) BOOL APIENTRY StopHook()
{
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    //解除截获关系
    if (DetourDetach(&(PVOID&)g_pMessageBoxA, HookMessageBoxA) != NO_ERROR)
    {
        OutputDebugStringA("DetourDetach MessageBoxA Fail!\n");
    }

    if (DetourDetach(&(PVOID&)g_pMessageBoxW, HookMessageBoxW) != NO_ERROR)
    {
        OutputDebugStringA("DetourDetach MessageBoxW Fail!\n");
    }

    //完成事务
    return DetourTransactionCommit() == NO_ERROR;
}

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        OutputDebugStringA("start hook....................................\n");
        StartHook();
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        StopHook();
        OutputDebugStringA("end hook....................................\n");
        break;
    }
    return TRUE;
}

