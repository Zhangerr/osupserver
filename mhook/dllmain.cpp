#undef UNICODE
#define WIN32_LEAN_AND_MEAN //so winsock.h doesnt mess with winsock2
#include <stdio.h>
#include <windows.h>
#include <Windows.h>
#include <cstdio>
#include <sstream>
#include <string>
#include <WinSock2.h>
#include <wchar.h>
#include <iostream>
#include <DbgHelp.h>
#include <fstream>
#include "mhook-lib\mhook.h"
#include <dos.h>
using namespace std;

//typedefs of original winapi functions
typedef int (WINAPI *pSend)(SOCKET, const char*, int, int);
typedef int (WINAPI *pWSASend)(SOCKET, LPWSABUF, DWORD, LPDWORD, DWORD, LPWSAOVERLAPPED, LPWSAOVERLAPPED_COMPLETION_ROUTINE);
typedef int (WINAPI *pWSAConnect)(SOCKET, const struct sockaddr *,int,LPWSABUF,LPWSABUF,LPQOS,LPQOS);
typedef BOOL (WINAPI *pSetWindowText)(HWND,LPCWSTR);
typedef BOOL (WINAPI *pCreateProcessW)(LPCTSTR, LPTSTR, LPSECURITY_ATTRIBUTES, LPSECURITY_ATTRIBUTES, BOOL, DWORD, LPVOID, LPCTSTR, LPSTARTUPINFO, LPPROCESS_INFORMATION);
typedef BOOL (WINAPI *pBASS_ChannelSetAttribute)(DWORD, DWORD, float);
typedef BOOL (WINAPI *pQueryPerformanceCounter)(LARGE_INTEGER*);
typedef DWORD (WINAPI *pTimeGetTime)(void);
typedef DWORD (WINAPI *pGetTickCount)(void);

//prototypes for our functions that are called instead
BOOL WINAPI MyCreateProcessW(LPCTSTR, LPTSTR, LPSECURITY_ATTRIBUTES, LPSECURITY_ATTRIBUTES, BOOL, DWORD, LPVOID, LPCTSTR, LPSTARTUPINFO, LPPROCESS_INFORMATION);
BOOL WINAPI MySetWindowText(HWND,LPCWSTR);
int WINAPI MyWSAConnect(SOCKET, const struct sockaddr *,int,LPWSABUF,LPWSABUF,LPQOS,LPQOS);
int WINAPI MyWSASend(SOCKET, LPWSABUF, DWORD, LPDWORD, DWORD, LPWSAOVERLAPPED, LPWSAOVERLAPPED_COMPLETION_ROUTINE);
int WINAPI MySend(SOCKET, const char*, int, int);
BOOL WINAPI MyBASS_ChannelSetAttribute(DWORD, DWORD, float);
BOOL WINAPI MyQueryPerformanceCounter(LARGE_INTEGER*);
DWORD WINAPI MyTimeGetTime(void);
DWORD WINAPI MyGetTickCount(void);

//variables to contain the original functions
pWSAConnect pOrigWSA = NULL;
pSetWindowText pOrigSetWindowText = NULL;
pCreateProcessW pOrigCreate = NULL;
pWSASend pOrigWSASend = NULL;
pSend pOrigSend = NULL;
pBASS_ChannelSetAttribute pOrigAttr = NULL;
pQueryPerformanceCounter pOrigQuery = NULL;
pTimeGetTime pOrigTime = NULL;
pGetTickCount pOrigTick = NULL;

//bools to indicate whether hook was successful or not
BOOL wsahook;
BOOL wthook;
BOOL cpwhook;
BOOL wsashook;
BOOL sendhook;
BOOL basshook;
BOOL queryhook;
BOOL timehook;
BOOL tickhook;

//stuff for hack
DWORD baseTime, baseTicks;
LARGE_INTEGER basePerf = LARGE_INTEGER();
std::stringstream ss;
float speed = 0.75;
float realSpeed;
float shSpeed;
BOOL worked = TRUE;
 //ip to redirect to
sockaddr_in * redirect;

BOOL CALLBACK EnumerateLoadedModulesProc( PSTR ModuleName, ULONG ModuleBase, ULONG ModuleSize, PVOID UserContext)
{
    cout << ModuleName << endl;
    return TRUE;
}

INT APIENTRY DllMain(HMODULE hDLL, DWORD Reason, LPVOID Reserved)
{
	BOOL speedhack = TRUE;
	//EnumerateLoadedModules(GetCurrentProcess(), (PENUMLOADED_MODULES_CALLBACK) EnumerateLoadedModulesProc, 0);
	switch(Reason)
	{
	case DLL_PROCESS_ATTACH:
		//try to get function addresses + hook, exceptions here are uncatchable and cause the game to crash

		//pOrigWSASend = (pWSASend) GetProcAddress(GetModuleHandle("Ws2_32.dll"), "WSASend");
		//wsashook = Mhook_SetHook((PVOID*)&pOrigWSASend, MyWSASend);
		//pOrigSetWindowText = (pSetWindowText) GetProcAddress(GetModuleHandle("user32.dll"),"SetWindowTextW");
		//wthook = Mhook_SetHook((PVOID*)&pOrigSetWindowText,MySetWindowText);
		pOrigWSA =  (pWSAConnect) GetProcAddress(GetModuleHandle("Ws2_32.dll"), "WSAConnect");
		wsahook =	Mhook_SetHook((PVOID*)&pOrigWSA,MyWSAConnect);
		pOrigCreate = (pCreateProcessW) GetProcAddress(GetModuleHandle("kernel32.dll"), "CreateProcessW");	
		cpwhook = Mhook_SetHook((PVOID*)&pOrigCreate, MyCreateProcessW);
		pOrigSend = (pSend) GetProcAddress(GetModuleHandle("Ws2_32.dll"), "send");
		sendhook = Mhook_SetHook((PVOID*)&pOrigSend, MySend);
		pOrigAttr = (pBASS_ChannelSetAttribute) GetProcAddress(GetModuleHandle("bass.dll"), "BASS_ChannelSetAttribute");
		if (GetModuleHandle("bass_fx.dll") == NULL)
		{
			MessageBoxA(NULL, "bass not found (add '2' to cmdline and run while osu is open)", "Info", MB_ICONEXCLAMATION);
		}
		else
		{
			basshook = Mhook_SetHook((PVOID*)&pOrigAttr, MyBASS_ChannelSetAttribute);
		}		

		//make the ip redirect for wsaconnect, ip etc is set each connection
		redirect = new sockaddr_in;		
		redirect->sin_family = AF_INET;		
		if(basshook==FALSE) {
			MessageBoxA(NULL,"Injection failed: bass hook", "Info", MB_ICONEXCLAMATION);
		}
		else if (speedhack == TRUE) {
			pOrigQuery = (pQueryPerformanceCounter)GetProcAddress(GetModuleHandle("kernel32.dll"), "QueryPerformanceCounter");
			queryhook = Mhook_SetHook((PVOID*)&pOrigQuery, MyQueryPerformanceCounter);
			pOrigTime = (pTimeGetTime)GetProcAddress(GetModuleHandle("winmm.dll"), "timeGetTime");
			timehook = Mhook_SetHook((PVOID*)&pOrigTime, MyTimeGetTime);
			pOrigTick = (pGetTickCount)GetProcAddress(GetModuleHandle("kernel32.dll"), "GetTickCount");
			tickhook = Mhook_SetHook((PVOID*)&pOrigTick, MyGetTickCount);
			if (tickhook==FALSE) {
				MessageBoxA(NULL,"Injection failed: tick hook", "Info", MB_ICONEXCLAMATION);
			}
			else if (timehook==FALSE) {
				MessageBoxA(NULL,"Injection failed: time hook", "Info", MB_ICONEXCLAMATION);
			}
			else if (queryhook==FALSE) {
				MessageBoxA(NULL,"Injection failed: query hook", "Info", MB_ICONEXCLAMATION);
			}
			else
			{
				worked = TRUE;
			}
			if (worked == TRUE)
			{		
				pOrigQuery(&basePerf);
				realSpeed = (speed * 100) - 100;
				shSpeed = speed / 1.5;
				baseTime = pOrigTime();
				baseTicks = pOrigTick();
			}
		}
		if(wsahook== FALSE) {
			MessageBoxA(NULL,"Injection failed: wsa hook", "Info", MB_ICONEXCLAMATION);
		}
		if(cpwhook== FALSE) {
			MessageBoxA(NULL,"Injection failed: cpw hook", "Info", MB_ICONEXCLAMATION);
		}
		if(sendhook==FALSE) {
			MessageBoxA(NULL,"Injection failed: send hook", "Info", MB_ICONEXCLAMATION);
		}
		else 
		{
			MessageBoxA(NULL,"Injection success", "Info", MB_ICONEXCLAMATION);
		}		
		break;
	case DLL_PROCESS_DETACH:
		Mhook_Unhook((PVOID*)&pOrigWSA);
		Mhook_Unhook((PVOID*)&pOrigSetWindowText);
		Mhook_Unhook((PVOID*)&pOrigCreate);
		Mhook_Unhook((PVOID*)&pOrigSend);
		Mhook_Unhook((PVOID*)&pOrigAttr);
		Mhook_Unhook((PVOID*)&pOrigQuery);
		Mhook_Unhook((PVOID*)&pOrigTick);
		Mhook_Unhook((PVOID*)&pOrigTime);
		delete redirect;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	}
	return TRUE;
}

/*BOOL WINAPI MySetWindowText(HWND hwnd, LPCWSTR str) {
	std::wstringstream ss;	
	ss << "hwnd is ";
	ss << &hwnd;
	ss << " title is " << std::wstring(str);
	OutputDebugStringW(ss.str().c_str());
	if(std::wstring(str) != std::wstring(L"type to begin!")) { //nfi but apparently setwindowtext is used to set the text of the search bar, and if that's hooked it starts spamming (probably trying to search? or reset)
	//found to be because windows uses setwindowtext to set the text on child controls of a dialogbox/classwindow, so need to compare hwnd to see if it's osu window
		return pOrigSetWindowText(hwnd, L"hacked by zhanger~");
	}
	else {
		return pOrigSetWindowText(hwnd,str);
	}
}*/

int WINAPI  MyWSAConnect(SOCKET s , const struct sockaddr * sa,int namelen ,LPWSABUF lpCallerData,LPWSABUF lpCalleeDatas,LPQOS lpSQOSss,LPQOS lpSQOSas) {	
	
		return pOrigWSA(s,sa,namelen,lpCallerData,lpCalleeDatas,lpSQOSss,lpSQOSas);
	sockaddr_in * originalAddress = (sockaddr_in *)sa;
	if (originalAddress->sin_addr.S_un.S_addr == inet_addr("50.23.74.90")) //this is the ip of cho.ppy.sh, the game/irc server. osu.ppy.sh ends with 91
	{		
		return pOrigWSA(s,sa,namelen,lpCallerData,lpCalleeDatas,lpSQOSss,lpSQOSas);
		//alternatively set the port here and use the redirect return
		//redirect->sin_port = htons(3333);
		//redirect->sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	}
	else 
	{
		redirect->sin_port = htons(80);
		redirect->sin_addr.S_un.S_addr = inet_addr("91.121.201.125");
	}
	return pOrigWSA(s,(sockaddr*)redirect,namelen,lpCallerData,lpCalleeDatas,lpSQOSss,lpSQOSas);
}

BOOL WINAPI MyCreateProcessW(LPCTSTR appName, LPTSTR cmdLine, LPSECURITY_ATTRIBUTES processAttrib, LPSECURITY_ATTRIBUTES threadAttrib, BOOL inheritHandles, DWORD creationFlags, LPVOID environment, LPCTSTR currentDir, LPSTARTUPINFO startupInfo, LPPROCESS_INFORMATION processInfo)
{		
	wchar_t* cmd = (wchar_t*)cmdLine;
	wstring launch;
	launch = wcstok(cmd, L":");
	launch += wcstok(NULL, L":");
	cmd = wcstok(NULL, L":");
	//default cmd given by osu
	wstring newCmd = launch + wstring(L":") + cmd;
	if (wcscmp(L"//osu.ppy.sh/web/osu-title-image.php?l=1\"", cmd) == 0)
	{		
		newCmd = launch + wstring(L"://www.google.com/search?q=aevv\"");
	}
	else //add any number of comparisons here for website, and where to redirect to
	{		
	}	
	const wchar_t* final = newCmd.c_str();
	cmdLine = (LPTSTR)final;
	return pOrigCreate(appName, cmdLine, processAttrib, threadAttrib, inheritHandles, creationFlags, environment, NULL, startupInfo, processInfo);
}
//osu doesnt appear to use wsasend, perhaps uses regular send
int WINAPI MyWSASend(SOCKET s, LPWSABUF buff, DWORD buffCount, LPDWORD bytesSent, DWORD flags, LPWSAOVERLAPPED overlap, LPWSAOVERLAPPED_COMPLETION_ROUTINE compRoutine)
{
	MessageBoxA(NULL,"wsasend called", "Info", MB_ICONEXCLAMATION);
	return pOrigWSASend(s, 0, buffCount, bytesSent, flags, overlap, compRoutine);
}
int WINAPI MySend(SOCKET socket, const char* buffer, int len,int flags)
{
	ofstream mainlog;
	mainlog.open("ch.log", ios::out | ios::app);
	mainlog << buffer << endl;
	mainlog.close();
	return pOrigSend(socket, buffer, len , flags);
	ofstream file;
	file.open("fst.txt", ios::out | ios::app);
	//string of buffer
	string s = string(buffer);
	//check if GET
	string fst = s.substr(0, 3);
	//eventual GET line
	string line = string("");
	//used for tokens
	char *next, *it;
	//rest of the http req
	string rest = string("");
	//check if first 3 bytes are GET
	if (strcmp(fst.c_str(), string("GET").c_str()) == 0)
	{
		//check if first 24 bytes show it's a score submit request
		string snd = s.substr(0, 24);
		if (strcmp(snd.c_str(), string("GET /rating/ingame-rate2").c_str()) == 0)
		{
			//dont know why i did this, prob related to trying to copy memory/editing it and cant use const
			char* originalBuffer = (char*)s.c_str();
			//char* o = "";
			//token the buffer with newlines as delim
			s = strtok(originalBuffer, "\n");
			//first line, so the GET line
			char* n = (char*)s.c_str();
			//token the GET line with ? as delim
			string i = strtok(n, "?");
			//everything up to the first ? and then append a ?
			line = i + "?";
			//need to initialise before use
			it = (char*)i.c_str();
			//was using while (it != NULL) but gave assert failures
			int count = 0;
			//3x, u, p, c params
			while (count < 3)
			{
				count++;
				//token the iterator by & to check param vs arg
				it = strtok(NULL, "&");
				string par = string(it);
				par = par.substr(0, 2);
				//if param is p, add custom password
				if (strcmp(par.c_str(), string("p=").c_str()) == 0)
				{
					line += "p=newpassword";
				}
				//else add what was already there
				else
				{
					line += string(it);
				}
				//make sure request is formed
				if (count < 3)
				{
					line += "&";
				}
			}
			//rest of the http req
			/*s = strtok(o, "\n");
			next = (char*)s.c_str();
			while (next != NULL)
			{
				next = strtok(NULL, "\n");
				file << next << endl;
				rest+=string(next);
			}*/
			//rest of request, not sure how much this matters, host may need changing - done manually instead of looping tokens cause cba
			rest = "\r\nAccept: */*\r\nAccept-Encoding: gzip, deflate\r\nHost: osu.ppy.sh\r\nUser-Agent: Mozilla/4.0 (compatible; Clever Internet Suite 6.1)\r\nConnection: Keep-Alive\r\n";			
			//add the GET and the rest of req
			string total = line + rest;
			//convert back to buffer
			const char* newBuffer = total.c_str();
			file << newBuffer << endl;
			file << buffer << endl;
			//send
			file.close();
			return pOrigSend(socket, newBuffer, len , flags);
		}
	}
	file.close();
	return pOrigSend(socket, buffer, len , flags);
}
BOOL WINAPI MyBASS_ChannelSetAttribute(DWORD handle, DWORD attrib, float value)
{
	if (attrib == 3 || attrib == 2)
	{		
		return pOrigAttr(handle, attrib, value);
	}
	return pOrigAttr(handle, attrib, realSpeed);
}
BOOL WINAPI MyQueryPerformanceCounter(LARGE_INTEGER *count)
{
	ofstream o;
	o.open("e", 0);
	o.close();
	pOrigQuery(count);
	if (worked == TRUE)
	{
		LARGE_INTEGER current;
		pOrigQuery(&current);
		LARGE_INTEGER change;
		change.QuadPart = current.QuadPart - basePerf.QuadPart;
		change.QuadPart = change.QuadPart * shSpeed;
		count->QuadPart = basePerf.QuadPart + change.QuadPart;
	}	
	return TRUE;
}
DWORD WINAPI MyTimeGetTime(void)
{
	if (worked == TRUE)
	{
		DWORD current = pOrigTime();
		DWORD change = current - baseTime;
		change = change * shSpeed;
		DWORD newTime = baseTime + change;
		if (newTime == 0)
		{
			return pOrigTime();
		}
		return newTime;
	}
	return pOrigTime();
}
DWORD WINAPI MyGetTickCount(void)
{
	if (worked == TRUE)
	{
		DWORD current = pOrigTick();
		DWORD change = current - baseTicks;
		change = change * shSpeed;
		DWORD newTicks = baseTicks + change;
		if (newTicks == 0)
		{
			return pOrigTick();
		}
		return newTicks;
	}
	return pOrigTick();
}