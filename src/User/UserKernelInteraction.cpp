// By jasonfish4
#include "stdafx.h"
#include <Windows.h>
#include <fstream>

int main(void)
{
	const char* Filename = "C:\\dlog.txt";
	PVOID read = calloc(1024, 1);

	DWORD64 pMemory = (DWORD64)read;
	DWORD32 ProcessId = GetCurrentProcessId();

	std::ofstream myfile;
	myfile.open(Filename, std::ios::binary);

	myfile.write((char*)&pMemory, sizeof(DWORD64));
	myfile.write((char*)&ProcessId, sizeof(DWORD32));
	
	myfile.close();

	printf("ptr1: %p \n", read);

	MSG msg = { 0 };

	RegisterHotKey(NULL, 0, NULL, VK_ADD);
	RegisterHotKey(NULL, 1, NULL, VK_INSERT); 
	RegisterHotKey(NULL, 2, NULL, VK_DELETE); 
	RegisterHotKey(NULL, 3, NULL, VK_SUBTRACT);

	while (*(PBYTE)read == 0) 
		Sleep(1);

	puts("Driver loaded.");

	while (GetMessage(&msg, NULL, 0, 0) != 0)
	{
		if (msg.message == WM_HOTKEY)
		{
			switch (msg.wParam)
			{
			case 0:
				memcpy(read, "\x00\x00\x00\x00", 4);
				break;
			case 1:
				puts("Enable hack");
				memcpy(read, "\x01\x00\x00\x00", 4);
				break;
			case 2:
				puts("Disable hack");
				memcpy(read, "\x02\x00\x00\x00", 4);
				break;
			case 3:
				puts("exit");
				memcpy(read, "\x03\x00\x00\x00", 4);
				puts("wait for proper cleanup");
				Sleep(3000);
				free(read);
				return 0;;
			}
		}
	}
	free(read);
	return 0;
}

