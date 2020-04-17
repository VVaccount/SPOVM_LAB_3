#include "pch.h"
#include <iostream>
#include <windows.h> 
#include <string>

using namespace std;

int main(int argc, char* argv[])
{
	HANDLE hMyPipe;
	HANDLE Semaphores[3];
	char buffer[20];				 // Буфер для передачи
	int bufferSize = sizeof(buffer); // Размер буфера
	string message;
	bool successFlag;
	Semaphores[0] = OpenSemaphore(SEMAPHORE_ALL_ACCESS, TRUE, (LPCWSTR)L"SEMAPHORE_lab3");
	Semaphores[1] = OpenSemaphore(SEMAPHORE_ALL_ACCESS, TRUE, (LPCWSTR)L"SEMAPHORE_end_lab3");
	Semaphores[2] = OpenSemaphore(SEMAPHORE_ALL_ACCESS, TRUE, (LPCWSTR)L"SEMAPHORE_EXIT_lab3");
	cout << "Child process\n\n";
	hMyPipe = CreateFile((LPCWSTR)L"\\\\.\\pipe\\MyPipe", GENERIC_READ, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	while (1)
	{
		successFlag = TRUE;
		DWORD NumberOfBytesRead;
		message.clear();

		int index = WaitForMultipleObjects(3, Semaphores, FALSE, INFINITE) - WAIT_OBJECT_0; // Получил уведомление о возможности чтения, семафор занят
		if (index == 2) // Если сигнальный семафор выхода
			break;

		//				cout<<"\n\t\t\tServer is ready to send message\n";

		int NumberOfBlocks;
		if (!ReadFile(hMyPipe, &NumberOfBlocks, sizeof(NumberOfBlocks), &NumberOfBytesRead, NULL)) break;

		int size;
		if (!ReadFile(hMyPipe, &size, sizeof(size), &NumberOfBytesRead, NULL)) break;

		for (int i = 0; i < NumberOfBlocks; i++)
		{
			successFlag = ReadFile(hMyPipe, buffer, bufferSize, &NumberOfBytesRead, NULL);
			if (!successFlag) break;

			message.append(buffer, bufferSize); // Добавляем полученный буфер в строку
		}
		if (!successFlag) break;
		message.resize(size);
		//	cout << message << "\n\n";
		for (int i = 0; i < size; i++)
		{
			cout << message[i];
			Sleep(100);
		}
		cout << endl;
		//				cout<<"\n\t\t\tMessage was printed successfully\n";
		ReleaseSemaphore(Semaphores[1], 1, NULL);		// Уведомление родительского процесса об успешном получении строки
	}
	CloseHandle(hMyPipe);
	CloseHandle(Semaphores[0]);
	CloseHandle(Semaphores[1]);
	return 0;
}