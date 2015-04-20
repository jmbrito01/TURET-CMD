#pragma once

#include "Includes.h"
#include "ProcessHelper.h"

static bool cbUnknown(const char* text, ULONGLONG* value)
{
	if (!text || !value)
		return false;
	return true;
}
class AsmHelper
{
public:
	AsmHelper(HANDLE hProcess);
	~AsmHelper();
	vector<DISASM> Disassemble(DWORD dwAddr, int size);
	vector<DISASM> DisassembleCode();
	vector<DISASM> DisassembleLines(DWORD dwAddr, int lines);
	int Assemble(DWORD dwAddr, char* Inst, unsigned char* dwResult);
private:
	unique_ptr<ProcessHelper> pHelper;
};

