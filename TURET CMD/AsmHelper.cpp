#include "AsmHelper.h"

AsmHelper::AsmHelper(HANDLE hProcess)
{
	pHelper = unique_ptr<ProcessHelper>(new ProcessHelper(hProcess));
}

AsmHelper::~AsmHelper()
{
}

vector<DISASM> AsmHelper::Disassemble(DWORD dwAddr, int size)
{
	DISASM MyDisasm = { 0 };
	DWORD dwAddress = dwAddr;
	vector<DISASM> Result;
	int len, i = 0;
	int Error = 0;
	MyDisasm.EIP = (UIntPtr)pHelper->Read(dwAddr, 50);
	while ((!Error) && (i < size))
	{
		len = Disasm(&MyDisasm);
		VirtualFree((void*)MyDisasm.EIP, 50, MEM_DECOMMIT);
		if (len != UNKNOWN_OPCODE)
		{
			VirtualFree((void*)MyDisasm.EIP, 50, MEM_DECOMMIT);
			MyDisasm.EIP = dwAddress;
			Result.push_back(MyDisasm);
			i += len;
			dwAddress += len;
			MyDisasm.EIP = (UIntPtr)pHelper->Read(dwAddress, 50);
		}
		else
			Error = 1;
	}
	return Result;
}

vector<DISASM> AsmHelper::DisassembleCode()
{
	vector<DISASM> Result;
	PIMAGE_SECTION_HEADER CodeSection = pHelper->GetCodeSection();
	DWORD dwAddress = pHelper->GetImageBase() + CodeSection->VirtualAddress;
	void* CodeBytes = pHelper->Read(dwAddress, dwAddress + CodeSection->Misc.VirtualSize);
	DISASM MyDisasm = { 0 };
	DWORD len = 0;
	bool Error = false;
	MyDisasm.EIP = (UIntPtr)CodeBytes;
	while ((!Error) && (MyDisasm.EIP < (DWORD)CodeBytes + CodeSection->Misc.VirtualSize))
	{
		len = Disasm(&MyDisasm);
		if (len != UNKNOWN_OPCODE)
		{
			DWORD TempEIP = MyDisasm.EIP;
			MyDisasm.EIP = dwAddress;
			Result.push_back(MyDisasm);
			dwAddress += len;
			MyDisasm.EIP = TempEIP;
			MyDisasm.EIP += len;

		}
		else
		{
			cout << "ERROR PARSING FUNCTION @0x" << hex << dwAddress << endl;
			Error = 1;
		}
	}
	return Result;
}

vector<DISASM> AsmHelper::DisassembleLines(DWORD dwAddr, int lines)
{
	DISASM MyDisasm = { 0 };
	DWORD dwAddress = dwAddr;
	vector<DISASM> Result;
	int len, i = 0;
	int Error = 0;
	MyDisasm.EIP = (UIntPtr)pHelper->Read(dwAddr, 50);
	while ((!Error) && (i < lines))
	{
		len = Disasm(&MyDisasm);
		VirtualFree((void*)MyDisasm.EIP, 50, MEM_DECOMMIT);
		if (len != UNKNOWN_OPCODE)
		{
			VirtualFree((void*)MyDisasm.EIP, 50, MEM_DECOMMIT);
			MyDisasm.EIP = dwAddress;
			Result.push_back(MyDisasm);
			i++;
			dwAddress += len;
			MyDisasm.EIP = (UIntPtr)pHelper->Read(dwAddress, 50);
		}
		else
			Error = 1;
	}
	return Result;
}


int AsmHelper::Assemble(DWORD dwAddr, char * Inst, unsigned char * dwResult)
{
	int Assemble(DWORD dwAddr, char* Inst, unsigned char* dwResult);
	{
		if (strlen(Inst) > XEDPARSE_MAXBUFSIZE)
			return -1;
		XEDPARSE Parse = { 0 };
#ifdef _WIN64
		Parse.x64 = true;
#else
		Parse.x64 = false;
#endif
		Parse.cbUnknown = cbUnknown;
		Parse.cip = dwAddr;
		strcpy_s(Parse.instr, Inst);
		if (XEDParseAssemble(&Parse) == XEDPARSE_ERROR)
		{
			cout << "[PARSE ERROR] " << Parse.error << endl;
			return -1;
		}
		if (dwResult)
			memcpy(dwResult, Parse.dest, Parse.dest_size);
		return Parse.dest_size;
	}
	return 0;
}
