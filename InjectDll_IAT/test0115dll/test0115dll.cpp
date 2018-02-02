// test0115dll.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include <iostream> 
using namespace std; 
#include "test0115dll.h" 
 
int HelloShine()
{
    OutputDebugStringW(L"********************Hell0 \n");
    HANDLE hFileWrite = 
        CreateFileW(L"test3.txt", 
        GENERIC_WRITE, 
        FILE_SHARE_READ|FILE_SHARE_WRITE,
        NULL, 
        CREATE_ALWAYS, 
        FILE_ATTRIBUTE_NORMAL, NULL);

    if (INVALID_HANDLE_VALUE == hFileWrite)
    {
        DWORD deerr = GetLastError();
        printf("Dll createfile invalid dwerr:%d\n",deerr);
    }

    printf("DLL createfile ok\n");

    return 10000;
}