#include "ProcessHelper.h"


ProcessHelper::ProcessHelper(HANDLE hProc)
{
	if (hProc != INVALID_HANDLE_VALUE)
		hProcess = hProc;
}

DWORD ProcessHelper::GetImageBase()
{
	char lpProc[MAX_PATH] = { 0 };
	GetModuleBaseName(hProcess, NULL, lpProc, sizeof(lpProc));
	int cb = 100;
	DWORD Needed;
MoreSpace:
	HMODULE* Modules = (HMODULE*)calloc(cb, sizeof(HMODULE));
	EnumProcessModules(hProcess, Modules, cb*sizeof(Modules), &Needed);
	if (Needed > cb*sizeof(Modules))
	{
		cb = Needed / sizeof(Modules);
		goto MoreSpace;
	}
	for (int i = 0; i < Needed / sizeof(Modules); i++)
	{
		char lpMod[MAX_PATH] = { 0 };
		GetModuleBaseName(hProcess, Modules[i], lpMod, sizeof(lpMod));
		if (strcmp(lpMod, lpProc) == 0)
		{
			MODULEINFO ModInfo = { 0 };
			GetModuleInformation(hProcess, Modules[i], &ModInfo, sizeof(ModInfo));
			return (DWORD)ModInfo.lpBaseOfDll;
		}
	}
}

char* ProcessHelper::GetProcessName()
{
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnap == INVALID_HANDLE_VALUE)
	{
		CloseHandle(hSnap);
		return NULL;
	}
	PROCESSENTRY32 pe = { 0 };
	pe.dwSize = sizeof(pe);
	Process32First(hSnap, &pe);
	do
	{
		if (pe.th32ProcessID == GetProcessId(hProcess))
		{
			CloseHandle(hSnap);
			char* lpTmp = new char[strlen(pe.szExeFile) + 1];
			ZeroMemory(lpTmp, strlen(pe.szExeFile) + 1);
			return lpTmp;
			break;
		}
	} while (Process32Next(hSnap, &pe));
	CloseHandle(hSnap);
	return NULL;
}

void * ProcessHelper::Read(DWORD Address, DWORD size)
{
	void* result = VirtualAlloc(NULL, size + 1, MEM_COMMIT, PAGE_READWRITE);
	ZeroMemory(result, size + 1);
	ReadProcessMemory(hProcess, (void*)Address, (void*)result, size, NULL);
	return result;
}

ProcessHelper::~ProcessHelper()
{
}


DWORD ProcessHelper::GetProcessIDbyName(char* ProcessName)
{
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnap == INVALID_HANDLE_VALUE)
	{
		CloseHandle(hSnap);
		return -1;
	}
	PROCESSENTRY32 pe = { 0 };
	pe.dwSize = sizeof(pe);
	Process32First(hSnap, &pe);
	do
	{
		string pName = string(ProcessName);
		string szExeFile = string(pe.szExeFile);
		if (pName.compare(toLower(szExeFile)) == 0)
		{
			CloseHandle(hSnap);
			return pe.th32ProcessID;
		}
	} while (Process32Next(hSnap, &pe));
	CloseHandle(hSnap);
	return -1;
}

string ProcessHelper::toLower(string s)
{
	for (auto it = s.begin(); it != s.end(); ++it)
		*it = tolower(*it);
	return s;
}
