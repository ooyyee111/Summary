// Test0115.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <Windows.h>

int _tmain(int argc, _TCHAR* argv[])
{
    printf("tmain\n");
    int i = 0;  
    while(true)  
    {  
        __asm{  
            mov eax,i  
            inc eax  
        }

      printf("tmain createfile i= %d\n",i);

    }

    printf("tmain createfile ok\n");

	return 0;
}