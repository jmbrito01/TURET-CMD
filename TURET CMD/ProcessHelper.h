#pragma once

#include "Includes.h"

class ProcessHelper
{
public:
	ProcessHelper(HANDLE hProc);

	DWORD GetImageBase();
	char* GetProcessName();
	void* Read(DWORD Address, DWORD size);
	template<typename t> t* Read(DWORD Address);
	int GetExternalStrLen(DWORD Address);
	~ProcessHelper();
public:
	static DWORD GetProcessIDbyName(char* lpName);

private:
	static string toLower(string s);
	HANDLE hProcess;
};

template <typename t>
inline t * ProcessHelper::Read(DWORD Address)
{
	t* result = (t*)VirtualAlloc(NULL, sizeof(t) + 1, MEM_COMMIT, PAGE_READWRITE);
	ZeroMemory(result, sizeof(result));
	ReadProcessMemory(hProcess, (void*)Address, (void*)result, sizeof(t), NULL);
	return result;
}
