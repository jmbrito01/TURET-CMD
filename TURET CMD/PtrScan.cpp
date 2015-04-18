#include "PtrScan.h"


char* ProcessName = 0;

LRESULT WINAPI PtrScanProc(HWND hWin, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		ProcessName = (char*)lParam;
		HANDLE hProcess;
		hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, ProcessHelper::GetProcessIDbyName(ProcessName));
		if (hProcess == INVALID_HANDLE_VALUE)
		{
			cout << "COULD NOT OPEN PROCESS!" << endl;
			EndDialog(hWin, 0);
		}
		ProcessHelper* pHelper;
		pHelper = new ProcessHelper(hProcess);
		GUITextBox* txtStart;
		txtStart = new GUITextBox(hWin, IDC_STARTADDR);
		GUITextBox* txtEnd;
		txtEnd = new GUITextBox(hWin, IDC_ENDADDR);
		txtStart->SetText(StringHelper::HexToString(pHelper->GetImageBase()));
		txtEnd->SetText(StringHelper::HexToString(pHelper->GetNTHeaders()->OptionalHeader.SizeOfImage+pHelper->GetImageBase()));
		break;
	case WM_COMMAND:
		switch (wParam)
		{
		case IDSEARCH:
			GUIButton* btnScan;
			btnScan = new GUIButton(hWin, IDSEARCH);
			btnScan->Disable();
			GUITextBox* txtStart;
			txtStart = new GUITextBox(hWin, IDC_STARTADDR);
			GUITextBox* txtEnd;
			txtEnd = new GUITextBox(hWin, IDC_ENDADDR);
			GUITextBox* txtAddress;
			txtAddress = new GUITextBox(hWin, IDC_ADDR);
			GUITextBox* txtMaxOffset;
			txtMaxOffset = new GUITextBox(hWin, IDC_MAXOFFSET);
			GUITextBox* txtNumOffset;
			txtNumOffset = new GUITextBox(hWin, IDC_NUMOFFSET);
			DWORD StartPtr = StringHelper::StringToHex(txtStart->GetText());
			DWORD EndPtr = StringHelper::StringToHex(txtEnd->GetText());
			DWORD Address = StringHelper::StringToHex(txtAddress->GetText());
			DWORD MaxOffset = StringHelper::StringToHex(txtMaxOffset->GetText());
			DWORD NumOffset = txtNumOffset->GetIntValue();
			for (DWORD i = StartPtr; i < EndPtr; i++)
			{
				//TO FINISH
			}
			break;
		}
		break;
	}
	return DefWindowProc(hWin, uMsg, wParam, lParam);
}