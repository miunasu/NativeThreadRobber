#include <Windows.h>
#include <winnt.h>
#include <stdio.h>


#ifdef _WIN64
int Cx_off = 0x80;
int Ip_off = 0xf8;
#else
int Cx_off = 0xac;
int Ip_off = 0xb4;
#endif


void test(LPTHREAD_START_ROUTINE oep, LPVOID parameter);

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    {
        PVOID stack = 0;
        PVOID point = 0;
        HMODULE process = GetModuleHandleA(0);
        IMAGE_DOS_HEADER* pDosHeader = (IMAGE_DOS_HEADER*)process;
        IMAGE_NT_HEADERS* pNtHeaders = (IMAGE_NT_HEADERS*)((BYTE*)process + pDosHeader->e_lfanew);
        PVOID oep = (PVOID)((BYTE*)process + pNtHeaders->OptionalHeader.AddressOfEntryPoint);
        FARPROC ntdll_RtlUserThreadStart = GetProcAddress(GetModuleHandleA("ntdll.dll"), "RtlUserThreadStart");

        for (int i = 0; i < 0x1000; i++)
        {
            point = (PVOID)((BYTE*)&stack + i);
            if ((*(PVOID*)((BYTE*)point + Cx_off) == oep) && (*(PVOID*)((BYTE*)point + Ip_off) == ntdll_RtlUserThreadStart))
            {
                *(PVOID*)((BYTE*)point + Ip_off) = &test;
                return 1;
            }
        }
    }
    case DLL_PROCESS_DETACH:
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    }
    return TRUE;
}


void test(LPTHREAD_START_ROUTINE oep, LPVOID parameter)
{
    char message[100];
    DWORD threadId = GetCurrentThreadId();
    typedef void (WINAPI *RtlUserThreadStart)(LPTHREAD_START_ROUTINE, LPVOID);
    RtlUserThreadStart ntdll_RtlUserThreadStart = NULL;
    ntdll_RtlUserThreadStart = (RtlUserThreadStart)GetProcAddress(GetModuleHandleA("ntdll.dll"), "RtlUserThreadStart");
    sprintf_s(message, sizeof(message), "Main thread Has been robbed!\nCurrent TID: %d\n Of course, you can back to exe's oep in x64.\n", threadId);
    MessageBoxA(NULL, message, "Message", MB_OK);
#ifdef _WIN64
    ntdll_RtlUserThreadStart(oep, parameter);
#else
    exit(0);
#endif
}


__declspec(dllexport) void ExportedFunction()
{
    printf("This is dll export function.\nDefining fake function what you need export.\nEnd dll export function.\n");
}


