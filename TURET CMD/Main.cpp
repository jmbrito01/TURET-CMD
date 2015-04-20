#include "Includes.h"
#include "ProcessHelper.h"
#include "GUIIncludes.h"
#include "resource.h"
#include "PtrScan.h"
#include "StringHelper.h"
#include "AsmHelper.h"

int main(int nargs, char** args)
{
	SetConsoleTitle("TURET - The Ultimate Reverse Engeneering Tool");
	if (nargs <= 1)
	{
		cout << "Welcome to the TURET CMD!" << endl;
		cout << "type turet -h for help." << endl;
		exit(0);
	}
	if (CHECK_ARG(1, "-h"))
	{
		cout << "To dump a executable file type \"turet -dump ProcessName.exe C:\dump.exe\"" << endl;
		cout << "To pointer scan type \"turet -ptrscan ProcessName.exe" << endl;
		cout << "To read in process memory type \"turet -read ProcessName.exe 0x401000 [size=4]" << endl;
		cout << "To write in a process memory type \"turet -write ProcessName.exe 0x401000 [size=4] [value=90 90 90 90]" << endl;
		cout << "To fix the IAT use: \"turet -iatrebuild ProcessName.exe C:\dump.exe\"" << endl;
		cout << "To dissassemble the process memory use one of this codes:" << endl;
		cout << "---> Disassemble lines: turet -disasm -line ProcessName.exe 0x401000 [lines=20]" << endl;
		cout << "---> Disassemble code: turet -disasm -code ProcessName.exe C:\\result.txt" << endl;
		cout << "---> Disassemble memory: turet -disasm -memory ProcessName.exe 0x401000 [size=0x50]" << endl;
	}
	else if (CHECK_ARG(1, "-disasm"))
	{
		HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, ProcessHelper::GetProcessIDbyName(args[3]));
		if (hProcess == INVALID_HANDLE_VALUE)
		{
			cout << "ERROR OPENING PROCESS!" << endl;
			CloseHandle(hProcess);
			exit(0);
		}
		ProcessHelper* pHelper = new ProcessHelper(hProcess);

		
		if (CHECK_ARG(2, "-line"))
		{
			AsmHelper* pAsm = new AsmHelper(pHelper->GetHandle());
			DWORD dwAddress = StringHelper::StringToHex(string(args[4]));
			int lines = atoi(args[5]);
			vector<DISASM> Result = pAsm->DisassembleLines(dwAddress, lines);
			for (auto it = Result.begin(); it != Result.end(); ++it)
			{
				cout << "0x" << hex << it->EIP << " - " << it->CompleteInstr << endl;
			}
		}
		else if (CHECK_ARG(2, "-code"))
		{
			ofstream logg;
			logg.open(args[4], ios::out);
			
			PIMAGE_SECTION_HEADER CodeSection = pHelper->GetCodeSection();
			DWORD dwAddress = pHelper->GetImageBase() + CodeSection->VirtualAddress;
			void* CodeBytes = pHelper->Read(dwAddress, dwAddress + CodeSection->Misc.VirtualSize);
			DISASM MyDisasm = { 0 };
			DWORD len = 0;
			bool Error = false;
			MyDisasm.EIP = (UIntPtr)CodeBytes;
			while ((!Error) && (MyDisasm.EIP < (DWORD)CodeBytes+CodeSection->Misc.VirtualSize))
			{
				system("CLS");
				cout << "Parsing address: 0x" << hex << dwAddress << " of 0x" << hex << pHelper->GetImageBase() + CodeSection->VirtualAddress + CodeSection->Misc.VirtualSize << endl;
				len = Disasm(&MyDisasm);
				if (len != UNKNOWN_OPCODE)
				{
					logg << "0x" << hex << dwAddress << " - " << MyDisasm.CompleteInstr << endl;
					dwAddress += len;
					MyDisasm.EIP += len;

				}
				else
				{
					logg << "ERROR PARSING FUNCTION @0x" << hex << dwAddress << endl;
					cout << "ERROR PARSING FUNCTION @0x" << hex << dwAddress << endl;
					Error = 1;
				}
			}
			cout << "Parsing finished!" << endl;
		}
		else if (CHECK_ARG(2, "-memory"))
		{
			DWORD dwAddress = StringHelper::StringToHex(string(args[4]));
			DWORD lines = StringHelper::StringToHex(args[5]);
			DISASM MyDisasm = { 0 };
			int len, i = 0;
			int Error = 0;
			MyDisasm.EIP = (UIntPtr)pHelper->Read(dwAddress, 50);
			while ((!Error) && (i < lines))
			{
				len = Disasm(&MyDisasm);
				VirtualFree((void*)MyDisasm.EIP, 50, MEM_DECOMMIT);
				if (len != UNKNOWN_OPCODE)
				{
					cout << "0x" << hex << dwAddress << " - " << MyDisasm.CompleteInstr << endl;
					i += len;
					dwAddress += len;
					MyDisasm.EIP = (UIntPtr)pHelper->Read(dwAddress, 50);
				}
				else
					Error = 1;
			}
		}
		CloseHandle(hProcess);
		delete pHelper;
		exit(0);
	}
	//TODO: FINISH PTR SCAN
	else if (CHECK_ARG(1, "-ptrscan"))
	{
		GUIForm* pPtrFrm = new GUIForm(IDD_PTRSCAN, (DLGPROC)PtrScanProc, (int)args[2]);
		exit(0);
	}
	else if (CHECK_ARG(1, "-write"))
	{
		char* ProcessName = args[2];
		string AddressString = string(args[3]);
		int Size = atoi(args[4]);
		char** ValuesString = &args[5];
		DWORD dwAddress = StringHelper::StringToHex(AddressString);
		cout << "[1/2] Retrieving informations from the process..." << endl;
		HANDLE hProcess = OpenProcess(PROCESS_VM_READ, false, ProcessHelper::GetProcessIDbyName(ProcessName));
		if (hProcess == INVALID_HANDLE_VALUE)
		{
			CloseHandle(hProcess);
			cout << "ERROR TRYING TO OPEN THE PROCESS!" << endl;
			cout << "Error(4) code: " << hex << GetLastError() << "( " << GetLastError() << " )" << endl;
			exit(0);
		}
		ProcessHelper* pHelper = new ProcessHelper(hProcess);
		cout << "[2/2] Writting the memory..." << endl;
		for (int i = 0; i < Size; i++)
		{
			//CONVERT THE VALUE STRING
			BYTE myValue = StringHelper::StringToByte(string(ValuesString[i]));
			pHelper->Write<BYTE>(dwAddress, myValue);
			CloseHandle(hProcess);
		}



	}
	else if (CHECK_ARG(1, "-read"))
	{
		char* ProcessName = args[2];
		string AddressString = string(args[3]);
		int Size = atoi(args[4]);
		DWORD dwAddress = StringHelper::StringToHex(AddressString);
		cout << "[1/2] Retrieving informations from the process..." << endl;
		HANDLE hProcess = OpenProcess(PROCESS_VM_READ, false, ProcessHelper::GetProcessIDbyName(ProcessName));
		if (hProcess == INVALID_HANDLE_VALUE)
		{
			CloseHandle(hProcess);
			cout << "ERROR TRYING TO OPEN THE PROCESS!" << endl;
			cout << "Error(4) code: " << hex << GetLastError() << "( " << GetLastError() << " )" << endl;
			exit(0);
		}
		ProcessHelper* pHelper = new ProcessHelper(hProcess);
		cout << "[2/2] Reading file..." << endl;
		byte* pRead = (byte*)pHelper->Read(dwAddress, Size);
		for (int i = 0; i < Size; i++)
		{
			cout << hex << (DWORD)pRead[i] << " ";
		}
		cout << endl;
		CloseHandle(hProcess);
	}
	//TODO: FINISH LISTING THE IAT TO WRITE IT AFTER.
	else if (CHECK_ARG(1, "-iatrebuild"))
	{
		char* ProcessName = args[2];
		char* DumpFile = args[3];

		cout << "[1/4] Retrieving informations from the process..." << endl;
		HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, ProcessHelper::GetProcessIDbyName(ProcessName));
		if (hProcess == INVALID_HANDLE_VALUE)
		{
			CloseHandle(hProcess);
			cout << "ERROR TRYING TO OPEN THE PROCESS!" << endl;
			cout << "Error(3) code: " << hex << GetLastError() << "( " << GetLastError() << " )" << endl;
			exit(0);
		}
		ProcessHelper* pHelper = new ProcessHelper(hProcess);
		PIMAGE_DOS_HEADER DOSHeader = pHelper->GetDOSHeader();
		PIMAGE_NT_HEADERS NTHeaders = pHelper->GetNTHeaders();
		DWORD IATSize = NTHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size;
		PIMAGE_IMPORT_DESCRIPTOR Import = (PIMAGE_IMPORT_DESCRIPTOR)pHelper->Read(pHelper->GetImageBase() + NTHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress, IATSize);
		for (int i = 0; i < IATSize / sizeof(IMAGE_IMPORT_DESCRIPTOR); i++)
		{
			char* pDllName = (char*)pHelper->Read(pHelper->GetImageBase() + Import[i].Name, pHelper->GetExternalStrLen(pHelper->GetImageBase() + Import[i].Name));
			PIMAGE_THUNK_DATA pThunk = pHelper->Read<IMAGE_THUNK_DATA>(pHelper->GetImageBase() + Import[i].OriginalFirstThunk);
			while (pThunk->u1.Function)
			{
				int len = pHelper->GetExternalStrLen(pHelper->GetImageBase() + Import[i].OriginalFirstThunk + pThunk->u1.AddressOfData + 2);
				char* funcName = (char*)pHelper->Read(pHelper->GetImageBase() + Import[i].OriginalFirstThunk + pThunk->u1.AddressOfData + 2, len);
				pThunk++;
			}
			cout << pDllName << endl;
		}
	}
	else if (CHECK_ARG(1, "-dump"))
	{
		char* ProcessName = args[2];
		cout << "[1/5] Opening proces..." << endl;
		DWORD PID = ProcessHelper::GetProcessIDbyName(ProcessName);
		if (PID == -1)
		{
			cout << "AN ERROR OCURRED WHEN TRYING TO FIND THE PROCESS !" << endl;
			cout << "Error(1) Code: " << hex << GetLastError() << "(" << GetLastError() << ")" << endl;
			exit(0);
		}
		HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, PID);
		if (hProcess == INVALID_HANDLE_VALUE)
		{
			CloseHandle(hProcess);
			cout << "AN ERROR OCURRED WHEN TRYING TO OPEN THE PROCESS !" << endl;
			cout << "Error(2) Code: " << hex << GetLastError() << "(" << GetLastError() << ")" << endl;
			exit(0);
		}
		cout << "[2/5] Retrieving informations..." << endl;
		ProcessHelper* pHelper = new ProcessHelper(hProcess);
		PIMAGE_DOS_HEADER DOSHeader = pHelper->GetDOSHeader();
		if (DOSHeader->e_magic != IMAGE_DOS_SIGNATURE)
		{
			CloseHandle(hProcess);
			cout << "I COULD NOT FIND THE DOSHEADER!" << endl;
			exit(0);
		}
		PIMAGE_NT_HEADERS NTHeaders = pHelper->GetNTHeaders();
		if (NTHeaders->Signature != IMAGE_NT_SIGNATURE)
		{
			CloseHandle(hProcess);
			cout << "I COULD NOT FIND THE NTHEADERS!" << endl;
			exit(0);
		}
		cout << "[3/5] Reading process..." << endl;
		void* pDump = pHelper->Read(pHelper->GetImageBase(), NTHeaders->OptionalHeader.SizeOfImage);
		cout << "[4/5] Aligning file..." << endl;
		PIMAGE_DOS_HEADER newDOS = (PIMAGE_DOS_HEADER)pDump;
		PIMAGE_NT_HEADERS newNT = (IMAGE_NT_HEADERS*)((DWORD)pDump + newDOS->e_lfanew);
		newNT->OptionalHeader.FileAlignment = 0x1000;
		newNT->OptionalHeader.SizeOfHeaders = 0x1000;
		for (int i = 0; i < newNT->FileHeader.NumberOfSections; i++)
		{
			PIMAGE_SECTION_HEADER SecHeader = (IMAGE_SECTION_HEADER*)((DWORD)pDump + newDOS->e_lfanew + sizeof(IMAGE_NT_HEADERS) + i*sizeof(IMAGE_SECTION_HEADER));
			SecHeader->PointerToRawData = SecHeader->VirtualAddress;
			SecHeader->SizeOfRawData = SecHeader->Misc.PhysicalAddress;

		}
		cout << "[5/5] Writting file..." << endl;
		HANDLE hFile = CreateFile(args[3], GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			CloseHandle(hProcess);
			CloseHandle(hFile);
			cout << "AN ERROR OCURRED WHEN TRYING TO CREATE THE FILE" << endl;
			exit(0);
		}
		SetFilePointer(hFile, NULL, NULL, FILE_BEGIN);
		DWORD Written;
		WriteFile(hFile, pDump, NTHeaders->OptionalHeader.SizeOfImage, &Written, NULL);
		CloseHandle(hFile);
		CloseHandle(hProcess);
		cout << "Process dumped to " << args[3] << " successfully!" << endl;
		exit(0);
	}
	
	return 0;
}