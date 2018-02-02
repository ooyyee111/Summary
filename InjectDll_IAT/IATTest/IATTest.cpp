//修改导入表的exe  
#include "stdafx.h"
#include <Windows.h>  

DWORD PEAlign(DWORD dwTarNum,DWORD dwAlignTo)  
{     
    return(((dwTarNum+dwAlignTo-1)/dwAlignTo)*dwAlignTo);  
}  

//  
//增加导入表项  
//  
BOOL AddNewSection(const char *  lpStrModulePath, DWORD dwNewSectionSize)  
{  
    bool   bSuccess = false;  
    LPVOID lpMemModule = NULL;  
    LPBYTE lpData = NULL;  
    HANDLE hFile = INVALID_HANDLE_VALUE, hFileMapping = INVALID_HANDLE_VALUE;  
    PIMAGE_NT_HEADERS pNtHeader = NULL;  
    PIMAGE_SECTION_HEADER pNewSection = NULL, pLastSection = NULL;  

    OutputDebugStringA("[!] AddNewSection Enter!\n");

    //TODO:可能还涉及关闭windows文件保护
    __try
    {
        //pe文件映射到内存
        hFile = CreateFileA(
            lpStrModulePath,
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL
            );

        if ( INVALID_HANDLE_VALUE == hFile )  
        {  
            OutputDebugStringA("[-] AddNewSection CreateFile fail!\n");  
            goto _EXIT_;  
        }  

        DWORD dwFileSize = GetFileSize(hFile, NULL);  
        hFileMapping = CreateFileMapping(hFile, NULL, PAGE_READWRITE/* | SEC_IMAGE*/, 0, dwFileSize, L"WINSUN_MAPPING_FILE");  
        if ( NULL == hFileMapping )  
        {  

            OutputDebugStringA("[-] AddNewSection CreateFileMapping fail!\n");  
            goto _EXIT_;  

        }  

        lpMemModule = MapViewOfFile(hFileMapping, FILE_MAP_ALL_ACCESS, 0, 0, dwFileSize);  
        if ( NULL == lpMemModule )  
        {  
            OutputDebugStringA("[-] AddNewSection MapViewOfFile fail!\n");  
            goto _EXIT_;  
        }  

        lpData = (LPBYTE)lpMemModule;  
        //判断是否是PE文件  
        if (((PIMAGE_DOS_HEADER)lpData)->e_magic != IMAGE_DOS_SIGNATURE )  
        {  
            OutputDebugStringA("[-] AddNewSection PE Header MZ error!\n");  
            goto _EXIT_;  
        }  

        pNtHeader = (PIMAGE_NT_HEADERS)(lpData + ((PIMAGE_DOS_HEADER)(lpData))->e_lfanew);  
        if ( pNtHeader->Signature != IMAGE_NT_SIGNATURE )  
        {  
            OutputDebugStringA("[-] AddNewSection PE Header PE error!\n");  
            goto _EXIT_;  
        }

        //判断是否可以增加一个新节  
        if ( ((pNtHeader->FileHeader.NumberOfSections + 1) * sizeof(IMAGE_SECTION_HEADER)) 
                > (pNtHeader->OptionalHeader.SizeOfHeaders) )   //SizeOfHeaders所有的头大小，
                                                                //这个可以通过IMAGE_BASE+SIZEOF_HEADERS来定位IMAGE_SECTION_HEADER的位置
        {  
            OutputDebugStringA("[-] AddNewSection cannot add a new section!\n");
            goto _EXIT_;
        }

        pNewSection  = (PIMAGE_SECTION_HEADER)(pNtHeader+1) + pNtHeader->FileHeader.NumberOfSections;

        pLastSection = pNewSection - 1;

        DWORD rsize,vsize,roffset,voffset;

        //对齐偏移和RVA
        rsize=PEAlign(dwNewSectionSize,  
            pNtHeader->OptionalHeader.FileAlignment);  

        roffset=PEAlign(pLastSection->PointerToRawData+pLastSection->SizeOfRawData,  
            pNtHeader->OptionalHeader.FileAlignment);  

        vsize=PEAlign(dwNewSectionSize,  
            pNtHeader->OptionalHeader.SectionAlignment);  

        voffset=PEAlign(pLastSection->VirtualAddress+pLastSection->Misc.VirtualSize,  
            pNtHeader->OptionalHeader.SectionAlignment);  

        //填充新节表  
        memcpy(pNewSection->Name, "WINSUN", strlen("WINSUN"));  
        pNewSection->VirtualAddress = voffset;  
        pNewSection->PointerToRawData = roffset;  
        pNewSection->Misc.VirtualSize = vsize;  
        pNewSection->SizeOfRawData = rsize;  
        pNewSection->Characteristics = IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_WRITE;  

        //修改IMAGE_NT_HEADERS，增加新节表  
        pNtHeader->FileHeader.NumberOfSections++;  
        pNtHeader->OptionalHeader.SizeOfImage += vsize;  
        pNtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT].Size = 0;  
        pNtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT].VirtualAddress = 0;  

        //增加新节到文件尾部  
        DWORD dwWriteBytes;  
        SetFilePointer(hFile,0,0,FILE_END);  
        PBYTE pbNewSectionContent = new BYTE[rsize];  
        ZeroMemory(pbNewSectionContent, rsize);  
        bSuccess = WriteFile(hFile, pbNewSectionContent, rsize, &dwWriteBytes, NULL);  
        if (!bSuccess)  
        {  
            MessageBox(NULL,L"新增节失败",L"error",MB_OK);  
            goto _EXIT_;  
        }  

    }  
    __except(EXCEPTION_EXECUTE_HANDLER)  
    {  
        OutputDebugStringA("[-] AddImportTableItem  Exception!\n");  
        return false;  
    }  
    OutputDebugStringA("[!] AddNewSection Exit!\n");  
    bSuccess = true;  
_EXIT_:  

    if ( hFile )  
    {  
        CloseHandle(hFile);  
    }  

    if ( lpMemModule)  
    {  
        UnmapViewOfFile(lpMemModule);  
    }  

    if ( hFileMapping )  
    {  
        CloseHandle(hFileMapping);  
    }  
    return true;  
}  

//  
PIMAGE_SECTION_HEADER ImageRVA2Section(PIMAGE_NT_HEADERS pImgNTHeader, DWORD dwRVA)  
{  
    int i;  
    PIMAGE_SECTION_HEADER pSectionHeader  = (PIMAGE_SECTION_HEADER)(pImgNTHeader+1);  
    for(i=0;i<pImgNTHeader->FileHeader.NumberOfSections;i++)  
    {  
        if((dwRVA>=(pSectionHeader+i)->VirtualAddress) && (dwRVA<=((pSectionHeader+i)->VirtualAddress+(pSectionHeader+i)->SizeOfRawData)))  
        {  
            return ((PIMAGE_SECTION_HEADER)(pSectionHeader+i));  
        }  
    }  
    return(NULL);  
}  


//  
// calulates the Offset from a RVA  
// Base    - base of the MMF  
// dwRVA - the RVA to calculate  
// returns 0 if an error occurred else the calculated Offset will be returned  
DWORD RVA2Offset(PIMAGE_NT_HEADERS pImgNTHeader, DWORD dwRVA)  
{  
    DWORD _offset;  
    PIMAGE_SECTION_HEADER section;  
    section=ImageRVA2Section(pImgNTHeader,dwRVA);//ImageRvaToSection(pimage_nt_headers,Base,dwRVA);  
    if(section==NULL)  
    {  
        return(0);  
    }  
    _offset=dwRVA+section->PointerToRawData-section->VirtualAddress;  
    return(_offset);  
}  

BOOL AddNewImportDescriptor(const char * szPEFilePath,char * szInjectDllName, char *szImportFuncName)  
{  
    BOOL bSuccess = FALSE;  
    LPVOID lpMemModule = NULL;  
    LPBYTE lpData = NULL;  
    HANDLE hFile = INVALID_HANDLE_VALUE, hFileMapping = INVALID_HANDLE_VALUE;  
    PIMAGE_NT_HEADERS pNtHeader = NULL;  
    PIMAGE_IMPORT_DESCRIPTOR pstImportTable = NULL;  
    PIMAGE_SECTION_HEADER    pstSectionHeader = NULL;  
    __try  
    {  
        //pe文件映射到内存  
        hFile = CreateFileA(  
            szPEFilePath,  
            GENERIC_READ | GENERIC_WRITE,  
            FILE_SHARE_READ | FILE_SHARE_WRITE,  
            NULL,  
            OPEN_EXISTING,  
            FILE_ATTRIBUTE_NORMAL,  
            NULL  
            );  
        if ( INVALID_HANDLE_VALUE == hFile )  
        {  
            OutputDebugStringA("[-] AddNewImportDescriptor CreateFile fail!\n");  
            goto _EXIT_;  
        }  

        DWORD dwFileSize = GetFileSize(hFile, NULL);  
        hFileMapping = CreateFileMappingW(hFile, NULL, PAGE_READWRITE/* | SEC_IMAGE*/, 0, dwFileSize, L"WINSUN_MAPPING_FILE");  
        if ( NULL == hFileMapping )  
        {  

            OutputDebugStringA("[-] AddNewImportDescriptor CreateFileMapping fail!\n");  
            goto _EXIT_;  

        }  

        lpMemModule = MapViewOfFile(hFileMapping, FILE_MAP_ALL_ACCESS, 0, 0, dwFileSize);  
        if ( NULL == lpMemModule )  
        {  
            OutputDebugStringA("[-] AddNewImportDescriptor MapViewOfFile fail!\n");  
            goto _EXIT_;  
        }  

        lpData = (LPBYTE)lpMemModule;  
        //判断是否是PE  
        if (((PIMAGE_DOS_HEADER)lpData)->e_magic != IMAGE_DOS_SIGNATURE )  
        {  
            OutputDebugStringA("[-] AddNewImportDescriptor PE Header MZ error!\n");  
            goto _EXIT_;  
        }  

        pNtHeader = (PIMAGE_NT_HEADERS)(lpData + ((PIMAGE_DOS_HEADER)(lpData))->e_lfanew);  
        if ( pNtHeader->Signature != IMAGE_NT_SIGNATURE )  
        {  
            OutputDebugStringA("[-] AddNewImportDescriptor PE Header PE error!\n");  
            goto _EXIT_;  
        }  
        pstImportTable = (PIMAGE_IMPORT_DESCRIPTOR)(lpData + RVA2Offset(pNtHeader,pNtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress));  
        BOOL bBoundImport = FALSE;  
        if (pstImportTable->Characteristics == 0 && pstImportTable->FirstThunk != 0)  
        {  
            bBoundImport = TRUE;  
            pNtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT].Size = 0;  
            pNtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT].VirtualAddress = 0;  
        }  

        pstSectionHeader = (PIMAGE_SECTION_HEADER)(pNtHeader+1)+pNtHeader->FileHeader.NumberOfSections-1;  
        PBYTE pbNewSection = pstSectionHeader->PointerToRawData + lpData;  
        int i = 0;  
        while(pstImportTable->FirstThunk != 0)  
        {  
            memcpy(pbNewSection, pstImportTable, sizeof(IMAGE_IMPORT_DESCRIPTOR));  
            pstImportTable++;  
            pbNewSection += sizeof(IMAGE_IMPORT_DESCRIPTOR);  
            i++;  
        }  
        memcpy(pbNewSection, (pbNewSection-sizeof(IMAGE_IMPORT_DESCRIPTOR)), sizeof(IMAGE_IMPORT_DESCRIPTOR));  



        DWORD dwDelt = pstSectionHeader->VirtualAddress - pstSectionHeader->PointerToRawData;  

        //avoid import not need table  
        PIMAGE_THUNK_DATA pImgThunkData = (PIMAGE_THUNK_DATA)(pbNewSection + sizeof(IMAGE_IMPORT_DESCRIPTOR)*2);  



        //import dll name  
        PBYTE pszDllNamePosition = (PBYTE)(pImgThunkData + 2);  
        memcpy(pszDllNamePosition, szInjectDllName, strlen(szInjectDllName));  
        pszDllNamePosition[strlen(szInjectDllName)] = 0;  

        //确定IMAGE_IMPORT_BY_NAM的位置  
        PIMAGE_IMPORT_BY_NAME pImgImportByName = (PIMAGE_IMPORT_BY_NAME)(pszDllNamePosition + strlen(szInjectDllName) + 1);  


        //init IMAGE_THUNK_DATA  
        pImgThunkData->u1.Ordinal = dwDelt + (DWORD)pImgImportByName - (DWORD)lpData ;  


        //init IMAGE_IMPORT_BY_NAME  
        pImgImportByName->Hint = 1;  
        memcpy(pImgImportByName->Name, szImportFuncName, strlen(szImportFuncName)); //== dwDelt + (DWORD)pszFuncNamePosition - (DWORD)lpData ;  
        pImgImportByName->Name[strlen(szImportFuncName)] = 0;  
        //init OriginalFirstThunk  
        if (bBoundImport)  
        {  
            ((PIMAGE_IMPORT_DESCRIPTOR)pbNewSection)->OriginalFirstThunk = 0;  
        }  
        else  
            ((PIMAGE_IMPORT_DESCRIPTOR)pbNewSection)->OriginalFirstThunk = dwDelt + (DWORD)pImgThunkData - (DWORD)lpData;  
        //init FirstThunk  
        ((PIMAGE_IMPORT_DESCRIPTOR)pbNewSection)->FirstThunk = dwDelt + (DWORD)pImgThunkData - (DWORD)lpData;  
        //init Name  
        ((PIMAGE_IMPORT_DESCRIPTOR)pbNewSection)->Name = dwDelt + (DWORD)pszDllNamePosition-(DWORD)lpData;  

        //改变导入表  
        pNtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress =  pstSectionHeader->VirtualAddress;   
        pNtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size            =  (i+1)*sizeof(IMAGE_IMPORT_DESCRIPTOR);  


    }  
    __except(EXCEPTION_EXECUTE_HANDLER)  
    {  
        OutputDebugStringA("[-] AddNewImportDescriptor  Exception!\n");  
        return false;  
    }  

_EXIT_:  

    if ( hFile )  
    {  
        CloseHandle(hFile);  
    }  

    if ( lpMemModule)  
    {  
        UnmapViewOfFile(lpMemModule);  
    }  

    if ( hFileMapping )  
    {  
        CloseHandle(hFileMapping);  
    }  
    return true;  
}  

BOOL AddImportTable(const char * szPEFilePath, char * szInjectDllName,char *szFuncName)  
{  
    BOOL bSuccess = FALSE;  
    try  
    {  
        //增加一个叫"WINSUN"的节  
        bSuccess = AddNewSection(szPEFilePath, 256);  
        if (!bSuccess)  
        {  
            MessageBoxW(NULL,L"add new section fail", L"error", MB_OK);  
            return bSuccess;  
        }  
        //增加一个导入表  
        AddNewImportDescriptor(szPEFilePath, szInjectDllName,szFuncName);  
    }  
    catch ( ... )//CException* e)  
    {  
        return bSuccess;  
    }  
    return bSuccess;  
}  

void BackupPE(char * pszPeFilePath)  
{
    CHAR szPath[MAX_PATH] = {0};  
    PCHAR pszPath = pszPeFilePath;

    strcpy(szPath,pszPath);
    strcat(szPath,".tmp");
    CopyFileA(pszPeFilePath,szPath,TRUE);
    return;  
}  


#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")
#include <atlconv.h >
void _tmain(int argc, char **argv)  
{
    CHAR szExePath[MAX_PATH] = {};
    GetModuleFileNameA( NULL, szExePath, MAX_PATH);
    PathAppendA( szExePath, "..\\EXE.exe");
    
    BackupPE(szExePath);

    // Modify IAT table
    //ModifyIATTableToLoadDll(pi.dwProcessId);
    AddImportTable(szExePath,"Dll.dll","HelloShine");

    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );

    USES_CONVERSION;

    wchar_t* wc = A2W(szExePath);

    if( !CreateProcess( NULL,   // No module name (use command line)
        wc,        // Command line
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        FALSE,          // Set handle inheritance to FALSE
        0,              // No creation flags
        NULL,           // Use parent's environment block
        NULL,           // Use parent's starting directory 
        &si,            // Pointer to STARTUPINFO structure
        &pi)           // Pointer to PROCESS_INFORMATION structure
        ) 
    {
        printf( "CreateProcess failed (%d)\n", GetLastError() );
        return;
    }

    getchar();
}  