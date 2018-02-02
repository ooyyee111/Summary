#include <Windows.h>
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")
#include <atlconv.h >
extern "C"  __declspec (dllexport) void  _stdcall  InjectFunc(void)  
{  
    MessageBoxA(NULL, "Dll export Inject Success", "Dll Inject", MB_OKCANCEL);  
}
HINSTANCE   g_hModule                           = NULL;

DWORD WINAPI MyThread( LPVOID lpParam ) 
{
     //创建一个标志文件，表面这个dll已经注入成功了
    WCHAR szTmp1Path[MAX_PATH]       = { 0 };
    WCHAR szTmp2Path[MAX_PATH]       = { 0 }; 
    WCHAR szFlagFileName[MAX_PATH]  = { L"{E997A5D9-35D1-46b0-ADED-A456A140DC90}" };
    GetTempPath(MAX_PATH, szTmp1Path);
    PathAppend(szTmp1Path, szFlagFileName);
    HANDLE hFile = CreateFileW(szTmp1Path,GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);  
    if (INVALID_HANDLE_VALUE != hFile &&
        NULL != hFile)  
    {
        CloseHandle(hFile); 
    }

    //创建一个标志文件，表面这个dll已经完成下载.
    WCHAR szFlagFileName2[MAX_PATH]  = { L"{C0363902-25D5-4e11-8389-8675890DB74E}" };
    GetTempPath(MAX_PATH, szTmp2Path);
    PathAppend(szTmp2Path, szFlagFileName2);
    HANDLE hFile2 = CreateFileW(szTmp2Path,GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);  
    if (INVALID_HANDLE_VALUE != hFile2 &&
        NULL != hFile2)  
    {
        CloseHandle(hFile2);
    }

    FreeLibraryAndExitThread(g_hModule,0);
    return 0;
}

BOOL StartUnloadThisDllThread()
{
    DWORD dwTid = 0; 
    HANDLE hThread = CreateThread( 
        NULL,              // default security attributes
        0,                 // use default stack size  
        MyThread,          // thread function 
        NULL,             // argument to thread function 
        0,                 // use default creation flags 
        &dwTid);   // returns the thread identifier
    if (hThread)
    {
        if(WaitForSingleObject(hThread,1000*5) != WAIT_OBJECT_0)
        {
            //5min后terminate thread
            TerminateThread(hThread,0);
        }
        CloseHandle(hThread);
        hThread = NULL;
    }
    return FALSE;
}

BOOL APIENTRY DllMain( HMODULE hModule,  
                      DWORD  ul_reason_for_call,  
                      LPVOID lpReserved  
                      )  
{  
    switch (ul_reason_for_call)  
    {  
    case DLL_PROCESS_ATTACH:  
        MessageBoxA(NULL, "the simple inject success", "Dll Inject", MB_OKCANCEL);
        g_hModule = hModule;

        StartUnloadThisDllThread();

        break;  
    case DLL_THREAD_ATTACH:  
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        {
            MessageBoxA(NULL, "DLL_PROCESS_DETACH", "Dll Inject", MB_OKCANCEL);
        }
        break;  
    }  
    return TRUE;  
}  