// Test0118.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "Test0118.h"
#include <windows.h>

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

#include <Psapi.h>
#pragma comment(lib,"Psapi.lib")


typedef DWORD (WINAPI *QUERY_FULL_PROCESS_IMAGE_NAME_FUNC)(HANDLE, DWORD, LPTSTR, DWORD*);

BOOL QueryProcessPath( HANDLE hProcess, LPTSTR psPath, DWORD dwLen )
{
    if (!hProcess || !psPath || !dwLen)
        return FALSE;

    static QUERY_FULL_PROCESS_IMAGE_NAME_FUNC pQryFunc = NULL;
    if (! pQryFunc)
    {
        HMODULE hMdl = GetModuleHandle(_T("Kernel32.dll"));
        if (! hMdl)
            return FALSE;

        pQryFunc = (QUERY_FULL_PROCESS_IMAGE_NAME_FUNC)::GetProcAddress(hMdl, "QueryFullProcessImageNameW");
        if (! pQryFunc)
            return FALSE;
    }

    return pQryFunc(hProcess, 0, psPath, &dwLen);
}

BOOL _GetProcessFileNameFromPID( DWORD dwPID,  DWORD cchSize )
{
    BOOL bRet = FALSE;

    if (dwPID > 4)
    {
        HANDLE hproc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwPID);
        if (hproc )
        {
            TCHAR szModName[MAX_PATH];
            if (GetModuleFileNameEx(hproc, 0, szModName, cchSize))
                bRet = TRUE;
            else
                bRet = QueryProcessPath(hproc, szModName, cchSize);

            CloseHandle(hproc);
        }
    }

    return bRet;
}
#include "stdio.h"
#include "Shlwapi.h"
#pragma  comment (lib,"shlwapi.lib")
#include "strsafe.h"
#include <string>
#include <windows.h>
using namespace std;


BOOL IsRegPathExists(wstring *pstrRegPath);

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{

   // wstring wstr = L"HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\WeGame.exe\\";
    wstring wstr = L"HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Applets\\Paint\\Text\\PointSize";
    IsRegPathExists(&wstr);

    getchar();

    char* m_char = "Hello";

    int size1 = sizeof(char);

    wchar_t* m_wchar = L"Hello";
    int size2 = sizeof(wchar_t);
    
    WCHAR szKeyBuf[MAX_PATH] = {};
    StringCchPrintfW(szKeyBuf, MAX_PATH, L"%s_%d", L"winmain", 100011);
   
    StringCchPrintfW(szKeyBuf, MAX_PATH, L"%s_%d", L"1", 10002);


    PWCHAR pszBuf = (PWCHAR)malloc(1024 * 1024);
    if (!pszBuf)
    {
        return FALSE;
    }
    memset(pszBuf, 0, 1024 * 1024);
    WCHAR szConfPath[MAX_PATH] = {0};
    wcscpy(szConfPath,L"D:\\vctest\\PeSign\\Debug\\KitTipConf.ini");

    DWORD dwNum = GetPrivateProfileStringW(L"cid_conf", L"acccid", L"", pszBuf, 1024 * 1024 - 1, szConfPath);
    if (dwNum > 0)
    {
        printf("GetPrivateProfileString1------dwnum:%d,pszbuf:%s\n",dwNum,pszBuf);
    }

    dwNum = GetPrivateProfileStringW(L"param_conf", L"cid_9510015_homepage", L"", pszBuf, 1024 * 1024 - 1, szConfPath);
    if (dwNum > 0)
    {
         printf("GetPrivateProfileString2------dwnum:%d,pszbuf:%s\n",dwNum,pszBuf);
    }


    WCHAR pOrgExe[MAX_PATH] = {0};
    wcscpy(pOrgExe,L"hfs.exe");

    WCHAR *pProcName = PathFindFileNameW(pOrgExe);

    if(pProcName == pOrgExe)
    {
        printf ("path find failed !");
    }

    printf(" path find filename %s \n",pProcName);

	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

    DWORD dwPID  = /*7144*/ 14740;
    CHAR *pFileName = NULL;
    DWORD dwchsize = MAX_PATH;

    _GetProcessFileNameFromPID(dwPID,dwchsize);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_TEST0118, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_TEST0118));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}

BOOL IsRegPathExists(wstring *pstrRegPath)
{
    SIZE_T reglen = pstrRegPath->size();

    if ( reglen <= 0)
    {
        return FALSE;
    }

    wstring str;
    HKEY hMainKey = NULL;
    HKEY hKey = NULL;

    int nPos = pstrRegPath->find(L"\\");
    if (nPos == wstring::npos)
    {
        return FALSE;
    }

    str = pstrRegPath->substr(0, nPos);
    if (str == L"HKEY_CLASSES_ROOT")
    {
        hMainKey = HKEY_CLASSES_ROOT;
    }
    else if (str == L"HKEY_CURRENT_CONFIG")
    {
        hMainKey = HKEY_CURRENT_CONFIG;
    }
    else if (str == L"HKEY_CURRENT_USER")
    {
        hMainKey = HKEY_CURRENT_USER;
    }
    else if (str == L"HKEY_LOCAL_MACHINE")
    {
        hMainKey = HKEY_LOCAL_MACHINE;
    }
    else if (str == L"HKEY_USERS")
    {
        hMainKey = HKEY_USERS;
    }
    if (!hMainKey)
    {
        return FALSE;
    }

    BOOL bQuerySubKey = FALSE;
    wstring strSubKey;
    wstring strValue;

    //最后一个字符是斜杠 则认为检测注册表项
    str = pstrRegPath->substr(reglen - 1, 1);
    if (str == L"\\")
    {
        bQuerySubKey = TRUE;
        strSubKey = pstrRegPath->substr(nPos + 1, reglen - nPos - 2);
    }
    else
    {
        int nPos2 = pstrRegPath->rfind(L"\\");
        if (nPos >= nPos2)
        {
            return FALSE;
        }
        strSubKey = pstrRegPath->substr(nPos + 1, nPos2 - nPos - 1);
        strValue = pstrRegPath->substr(nPos2 + 1, reglen - nPos2 - 1);
    }

    if (strSubKey == L"")
    {
        return FALSE;
    }

    DWORD dwAccess = KEY_READ | KEY_WOW64_64KEY;
    OutputDebugStringW(L"dwaccess......");

    LSTATUS nRet = RegOpenKeyExW(hMainKey, strSubKey.c_str(), 0, dwAccess, &hKey);  
    if (nRet != ERROR_SUCCESS)
    {
        OutputDebugStringW(L"RegOpenKeyExW nRet......");

        return FALSE;
    }
    else if (bQuerySubKey)
    {
        OutputDebugStringW(L"RegOpenKeyExW bQuerySubKey nRet......");
        return TRUE;
    }


    OutputDebugStringW(L"RegOpenKeyExW aft......");

    DWORD dwType = REG_SZ;
    nRet = RegQueryValueEx(hKey, strValue.c_str(), NULL, &dwType, NULL, NULL);
    RegCloseKey(hKey);

     OutputDebugStringW(L"RegQueryValueEx aft......");

    if (nRet == ERROR_SUCCESS || nRet == ERROR_MORE_DATA) 
    {   
        OutputDebugStringW(L"RegQueryValueEx ERROR_SUCCESS......");
        return TRUE;
    }  
    else
    {
        OutputDebugStringW(L"RegQueryValueEx not ERROR_SUCCESS......");
        return FALSE;
    }
}


//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TEST0118));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_TEST0118);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
