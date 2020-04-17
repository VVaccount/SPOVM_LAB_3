#include "pch.h"
#include <iostream>
#include <windows.h>
#include <string>

using namespace std;

int main()
{
	STARTUPINFO si;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);

	PROCESS_INFORMATION childProcessInfo;
	ZeroMemory(&childProcessInfo, sizeof(childProcessInfo));

	HANDLE hMyPipe;
	HANDLE Semaphores[3];

	char buffer[20];				 // Буфер для передачи
	int bufferSize = sizeof(buffer); // Размер буфера

	string message;


	Semaphores[0] = CreateSemaphore(NULL, 0, 1, (LPCWSTR)L"SEMAPHORE_lab3");      // Семафор, уведомляющий о разрешении печати
	Semaphores[1] = CreateSemaphore(NULL, 0, 1, (LPCWSTR)L"SEMAPHORE_end_lab3");      // Семафор, уведомляющий о разрешении печати
	Semaphores[2] = CreateSemaphore(NULL, 0, 1, (LPCWSTR)L"SEMAPHORE_EXIT_lab3");	// Семафор, уведомляющий о завершении работы			

	cout << "Server process\n\n";

	hMyPipe = CreateNamedPipe((LPCWSTR)L"\\\\.\\pipe\\MyPipe", PIPE_ACCESS_OUTBOUND, PIPE_TYPE_MESSAGE | PIPE_WAIT, PIPE_UNLIMITED_INSTANCES, 0, 0, INFINITE, (LPSECURITY_ATTRIBUTES)NULL);
	CreateProcess((LPCWSTR)L"C:\\Users\\Вадим\\source\\repos\\client\\Debug\\client.exe", (LPWSTR)L"C:\\Users\\Вадим\\source\\repos\\client\\Debug\\client.exe 2", NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &childProcessInfo);

	if (!ConnectNamedPipe(hMyPipe, (LPOVERLAPPED)NULL))
		cout << "Connection failure\n";

	while (1)
	{
		DWORD NumberOfBytesWritten;

		cout << "\nEnter message:\n";
		cin.clear();
		getline(cin, message);


		if (message == "quit")
		{
			ReleaseSemaphore(Semaphores[2], 1, NULL);  // Сообщаем дочернему процессу о завершении работы
			WaitForSingleObject(childProcessInfo.hProcess, INFINITE);
			break;
		}

		ReleaseSemaphore(Semaphores[0], 1, NULL);   // Сообщаем дочернему процессу о готовности начала передачи данных

		int NumberOfBlocks = message.size() / bufferSize + 1;	// Количество блоков размером с буфер			
		WriteFile(hMyPipe, &NumberOfBlocks, sizeof(NumberOfBlocks), &NumberOfBytesWritten, (LPOVERLAPPED)NULL);

		int size = message.size();
		WriteFile(hMyPipe, &size, sizeof(size), &NumberOfBytesWritten, (LPOVERLAPPED)NULL);

		for (int i = 0; i < NumberOfBlocks; i++)
		{
			message.copy(buffer, bufferSize, i*bufferSize);		// Заполняем буфер
			if (!WriteFile(hMyPipe, buffer, bufferSize, &NumberOfBytesWritten, (LPOVERLAPPED)NULL)) 
				cout << "Write Error\n";
		}

		WaitForSingleObject(Semaphores[1], INFINITE); // Ожидание, пока клиентский процесс не напечатал строку
	}

	CloseHandle(hMyPipe);
	CloseHandle(Semaphores[0]);
	CloseHandle(Semaphores[1]);
	cout << "\n\n";
	system("pause");
	return 0;
}