#include "Includes.h"
#include "ProcessHelper.h"

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
		cout << "To read a process memory type \"turet -read ProcessName.exe 0x401000 [size]" << endl;
		cout << "To fix the IAT use: \"turet -iatrebuild ProcessName.exe C:\dump.exe\"" << endl;
	}
	else if (CHECK_ARG(1, "-read"))
	{
		char* ProcessName = args[2];
		string AddressString = string(args[3]);
		int Size = atoi(args[4]);
		DWORD dwAddress = 0;
		//CONVERT ADDRESS STRING IN DWORD
		AddressString.erase(0, 2);
		for (int i = 0; i < AddressString.size(); i++)
		{
			if (AddressString[i] >= '0' && AddressString[i] <= '9')
			{
				dwAddress += AddressString[i] - 48;
				dwAddress *= 0x10;
			}
			else if (AddressString[i] >= 'A' && AddressString[i] <= 'F')
			{
				dwAddress += AddressString[i] - 55;
				dwAddress *= 0x10;
			}
			else if (AddressString[i] >= 'a' && AddressString[i] <= 'f') 
			{
				dwAddress += AddressString[i] - 87;
				dwAddress *= 0x10;
			}
		}
		dwAddress /= 0x10;
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
	}
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
		PIMAGE_DOS_HEADER DOSHeader = pHelper->Read<IMAGE_DOS_HEADER>(pHelper->GetImageBase());
		PIMAGE_NT_HEADERS NTHeaders = pHelper->Read<IMAGE_NT_HEADERS>(pHelper->GetImageBase() + DOSHeader->e_lfanew);
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
		PIMAGE_DOS_HEADER DOSHeader = pHelper->Read<IMAGE_DOS_HEADER>(pHelper->GetImageBase());
		if (DOSHeader->e_magic != IMAGE_DOS_SIGNATURE)
		{
			CloseHandle(hProcess);
			cout << "I COULD NOT FIND THE DOSHEADER!" << endl;
			exit(0);
		}
		PIMAGE_NT_HEADERS NTHeaders = pHelper->Read<IMAGE_NT_HEADERS>(pHelper->GetImageBase() + DOSHeader->e_lfanew);
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