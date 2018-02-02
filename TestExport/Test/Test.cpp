// Test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <Windows.h>
#include <atlconv.h>
#include <Imagehlp.h>
#pragma comment(lib,"Imagehlp.lib")
#include <iostream>
using namespace  std;

extern "C" {
    __declspec(dllimport) char gddr[128];
}

void xLogA(const CHAR *szfmt, ...)
{
    char buff[512]     = {0};
    va_list	arglist;
    va_start( arglist, szfmt );
    vsprintf_s( (char *)buff, 512, szfmt, arglist );
    va_end( arglist );
    OutputDebugStringA(buff);
}

BOOL Set_gaddr()
{
    WCHAR szCmdline[MAX_PATH] =  {};
    GetModuleFileNameW(NULL, szCmdline, MAX_PATH);
    wchar_t * pc = ( wcsrchr ( szCmdline, _T ( '\\' ) ) ); 
    pc[1] = 0;
    wcscat(szCmdline,L"gaddr.exe");

    char *szCmdlineA = NULL;
    USES_CONVERSION;
    szCmdlineA = W2A(szCmdline);

    STARTUPINFO  si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si,sizeof(si));
    ZeroMemory(&si,sizeof(si));
    
    si.cb = sizeof(si);

    if( !CreateProcessW( NULL,   // No module name (use command line)
        szCmdline,              // Command line
        NULL,                   // Process handle not inheritable
        NULL,                   // Thread handle not inheritable
        FALSE,                  // Set handle inheritance to FALSE
        CREATE_SUSPENDED,       // No creation flags
        NULL,                   // Use parent's environment block
        NULL,                   // Use parent's starting directory 
        &si,                    // Pointer to STARTUPINFO structure
        &pi )                   // Pointer to PROCESS_INFORMATION structure
        ) 
    {
        return FALSE;
    }

    CONTEXT context;
    context.ContextFlags = CONTEXT_ALL;
    if (!GetThreadContext(pi.hThread, &context)) //获取挂起进程CONTEXT，其中，EAX为进程入口点地址，EBX指向进程PEB;
    {
        CloseHandle(pi.hThread);
        TerminateProcess(pi.hProcess, 0);
        CloseHandle(pi.hProcess);
        return FALSE;
    }
    
    /* //1.获取进程加载的基地址
    DWORD dwVictimBaseAddr = 0;
    //1. EBX points to PEB, offset 8 is the pointer to the base address  //进程加载的基地址
    if (ReadProcessMemory(pi.hProcess, (LPCVOID)(context.Ebx + 8), &dwVictimBaseAddr, sizeof(PVOID), NULL) == 0)
    {
        DWORD derr = GetLastError();
        return -1;
    }
    */

    //2.获取进程加载的基地址--------------------------------------------------------------------------------------------------------
    DWORD pEntryPoint = (DWORD)context.Eax;//入口代码地址

    DWORD pAddressOfEntryPoint = NULL;
    LOADED_IMAGE *pImage = ImageLoad(szCmdlineA,NULL);
    if (pImage)
    {
        pAddressOfEntryPoint = pImage->FileHeader->OptionalHeader.AddressOfEntryPoint;//代码入口的RVA地址。
        //也就是说，把一个文件加载到内存的时候，基地址加上AddressOfEntryPoint就是我们的入口代码地址。
        ImageUnload(pImage);
    }

    DWORD pBase = pEntryPoint - pAddressOfEntryPoint; //gaddr进程加载基地址.
    xLogA("********pBase:0x%x \n",pBase);

//2------------------------------------------------------------------------------------------------------
    HMODULE hLoadExe = LoadLibraryW(szCmdline); //将gaddr.exe加载到Test的地址空间中 （映射文件的起始地址被称为模块的句柄）
    xLogA("********hLoadExe:0x%x\n",hLoadExe);

    if (hLoadExe)
    {
        DWORD dw = 0;
        char my_get_addr[128] = { 0 };
        PCHAR get_addr = (PCHAR)GetProcAddress(hLoadExe, "gaddr");//gaddr变量的内存地址。
        if (get_addr)
        {
            DWORD dwOffset = (DWORD)get_addr - (DWORD)hLoadExe;   //gaddr变量在test进程中的偏移量。与在gaddr.exe中偏移量一样。（也可ida静态分析gaddr.exe）

//3------------------------------------------------------------------------------------------
            PCHAR pRealGetAddr = (PCHAR)(pBase + dwOffset); //gaddr在gaddr进程中的VA内存地址。

            ReadProcessMemory(pi.hProcess,pRealGetAddr,my_get_addr,128,&dw);
            cout<<"------------------gaddr : "<<my_get_addr<<" bef WriteProcessMemory" <<endl;

            // 在映射的内存地址上写入了一个值。
            char cmd[128];
            memset(cmd,128,'\0');
            strcpy(cmd,"0987654321");
            WriteProcessMemory(pi.hProcess,pRealGetAddr,cmd,128,&dw);

            ReadProcessMemory(pi.hProcess,pRealGetAddr,my_get_addr,128,&dw);

            cout<<"------------------gaddr : "<<my_get_addr<<" aft WriteProcessMemory" <<endl;

        }
        FreeLibrary(hLoadExe);
    }

    ResumeThread(pi.hThread);
    CloseHandle(pi.hThread);

    return TRUE;
}

int _tmain(int argc, _TCHAR* argv[])
{
    Set_gaddr();
    getchar();
	return 0;
}

