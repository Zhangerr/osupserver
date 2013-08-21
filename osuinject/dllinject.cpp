// injectdll.cpp : Defines the entry point for the console application.
//


#include "stdafx.h"
#include <stdio.h>
#include <TlHelp32.h>
#include <iostream>
#include <Shlwapi.h>
using namespace std;
DWORD realGetTickCountAddr;
DWORD realQueryPerformanceCounterAddr;
DWORD getPayloadExportAddr( LPCWSTR path, HMODULE base, LPCSTR function)
{
	HMODULE loaded = LoadLibrary(path);

	if (loaded == NULL) 
	{
		cout << "failed load of addr" << endl;
		return NULL;
	}
	else
	{
		void *func = GetProcAddress(loaded, function);
		DWORD offset = (char*)func - (char*)loaded;
		FreeLibrary(loaded);
		return (DWORD)base + offset;
	}
}
BOOL initPayload(HANDLE proc, LPCWSTR path, HMODULE base, PVOID hwnd)
{
	DWORD init = getPayloadExportAddr(path, base, "InitializeSpeedhack");
	realGetTickCountAddr = getPayloadExportAddr(path, base, "realGetTickCount");
	realQueryPerformanceCounterAddr = getPayloadExportAddr(path, base, "realQueryPerformanceCounterAddr");
	cout << "init addr: " << init << endl;
	cout << "tick addr: " << realGetTickCountAddr << endl;
	cout << "qp addr: " << realQueryPerformanceCounterAddr << endl;
	if (init == NULL)
	{
		return FALSE;
	}
	else
	{
		HANDLE thread = CreateRemoteThread(proc, NULL, 0, (LPTHREAD_START_ROUTINE)init, (LPVOID)2, 0, NULL);		
		if (thread == NULL)
		{
			cout << "error" << endl;
			return FALSE;
		}
		else
		{
			CloseHandle(thread);
		}
	}

	return TRUE;
}
int getProc() {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
    if(hSnapshot) {
        PROCESSENTRY32 pe32;
        pe32.dwSize = sizeof(PROCESSENTRY32);
        if(Process32First(hSnapshot,&pe32)) {
            do {
			   if (wcscmp(pe32.szExeFile, L"osu!.exe")==0)
				   return pe32.th32ProcessID;
            } while(Process32Next(hSnapshot,&pe32));
         }
         CloseHandle(hSnapshot);
    }
	return -1;
}
//todo: stop using system("pause") lol
int main(int argc, char* argv[])
{
	STARTUPINFOA lpStartupInfo = {sizeof(STARTUPINFOA)};
	PROCESS_INFORMATION lpProcessInfo={0};
	memset(&lpProcessInfo, 0, sizeof(lpProcessInfo));
	memset(&lpProcessInfo, 0, sizeof(lpProcessInfo));
	HANDLE hProcess;
	int id;
	char appname[] = "D:\\Program Files (x86)\\osu!\\osu!.exe";
	if (argc == 1)
	{
		cout << CreateProcessA(appname,0 ,NULL,NULL,FALSE,NORMAL_PRIORITY_CLASS,NULL,NULL, &lpStartupInfo,&lpProcessInfo) << endl;
		cout << GetLastError() << endl;
		// PROCESS_CREATE_THREAD|PROCESS_VM_WRITE|PROCESS_VM_READ|PROCESS_QUERY_INFORMATION|PROCESS_VM_OPERATION|THREAD_QUERY_INFORMATION	
		hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE,lpProcessInfo.dwProcessId);
	}
	else
	{
		id = getProc();
		cout << "proc id: " << id << endl;
		hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE,id);
	}
	if (hProcess == INVALID_HANDLE_VALUE)
	{
		fprintf(stderr, "cannot open that pid\n");
		system("pause");
		return 1;
	}
	const char * path = "C:\\Users\\matt\\Desktop\\osuserver\\osu-shgui\\osu-shgui\\bin\\Debug\\dll.dll";	
	PVOID mem = VirtualAllocEx(hProcess, NULL, strlen(path) + 1, MEM_COMMIT, PAGE_READWRITE);
	HMODULE sh;
	if (mem == NULL)
	{
		fprintf(stderr, "can't allocate memory in that pid\n");
		CloseHandle(hProcess);
		system("pause");
		return 1;
	}
	
	if (WriteProcessMemory(hProcess, mem, (void*)path, strlen(path) + 1, NULL) == 0)
	{
		fprintf(stderr, "can't write to memory in that pid\n");
		VirtualFreeEx(hProcess, mem, strlen(path) + 1, MEM_RELEASE);
		CloseHandle(hProcess);
		system("pause");
		return 1;
	}
	cout << "load addr" << (DWORD)GetProcAddress(GetModuleHandleA("KERNEL32.DLL"),"LoadLibraryA") << endl;
	HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE) GetProcAddress(GetModuleHandleA("KERNEL32.DLL"),"LoadLibraryA"), mem, 0, NULL);
	if (hThread == INVALID_HANDLE_VALUE)
	{
		fprintf(stderr, "can't create a thread in that pid\n");
		VirtualFreeEx(hProcess, mem, strlen(path) + 1, MEM_RELEASE);
		CloseHandle(hProcess);
		system("pause");
		return 1;
	}

	WaitForSingleObject(hThread, INFINITE);


	HMODULE hLibrary = NULL;
	if (!GetExitCodeThread(hThread, (LPDWORD)&hLibrary))
	{
		printf("can't get exit code for thread GetLastError() = %i.\n", GetLastError());
		CloseHandle(hThread);
		VirtualFreeEx(hProcess, mem, strlen(path) + 1, MEM_RELEASE);
		CloseHandle(hProcess);
		system("pause");
		return 1;
	}

	CloseHandle(hThread);
	//hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE) GetProcAddress(GetModuleHandleA("KERNEL32.DLL"),"GetModuleHandleA"),mem, 0, NULL);
	VirtualFreeEx(hProcess, mem, strlen(path) + 1, MEM_RELEASE);
	
	/*printf("injected");
	system("pause");
	hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE) GetProcAddress(GetModuleHandleA("KERNEL32.DLL"),"FreeLibraryA"),hLibrary, 0, NULL);
	if(!GetExitCodeThread(hThread,(LPDWORD)&hLibrary)) {
			printf("can't get exit code for thread GetLastError() = %i.\n", GetLastError());
		CloseHandle(hThread);
		VirtualFreeEx(hProcess, mem, strlen(path) + 1, MEM_RELEASE);
		CloseHandle(hProcess);
		system("pause");
		return 1;
	}
	printf("ejected");
	system("pause");*/
	if (hLibrary == NULL)
	{
		hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE) GetProcAddress(GetModuleHandleA("KERNEL32.DLL"),"GetLastError"), 0, 0, NULL);
		if (hThread == INVALID_HANDLE_VALUE)
		{
			fprintf(stderr, "LoadLibraryA returned NULL and can't get last error.\n");
			CloseHandle(hProcess);
			system("pause");
			return 1;
		}

		WaitForSingleObject(hThread, INFINITE);
		DWORD error;
		GetExitCodeThread(hThread, &error);

		CloseHandle(hThread);

		printf("LoadLibrary return NULL, GetLastError() is %i\n", error);
		CloseHandle(hProcess);
		system("pause");
		return 1;
	}
	
	CloseHandle(hProcess);

	//printf("injected %08x\n", (DWORD)hLibrary);
	//system("pause");
	//note to self -- add unloadlib freelibrary
	return 0;
}
