// TestExport.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <Windows.h>

extern "C"
{ 
    __declspec(dllexport) char gaddr[128] =  { 'a','b',0 };/*{};*/
}

//char gaddr[128] =  { 'a','b',0 };
int _tmain(int argc, _TCHAR* argv[])
{

   // while (1)
    while(1)
    {
        printf("gaddr :%s\n",gaddr);
        Sleep(1000);
    }

    system("pause");
	return 0;
}