// DLL injection with LoadLibrary | by Arthur Von Groll

#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <string>
#include <Windows.h>
#include <TlHelp32.h>

// Change this value to 1 if you want to get the process name from input.txt file
#define RAW_INPUT 1

int getStrPos(char* buffer, const char* targetStr, int numberOfOccurence)
{
    int bkpPos = -1;

    char* filter = strstr(buffer, targetStr);
    bkpPos = filter ? filter - buffer : -1;

    if (numberOfOccurence == -1)
    {
        while (filter)
        {
            bkpPos = filter - buffer;
            filter = strstr(filter + 1, targetStr);
        }

        return bkpPos;
    }

    for (int i = 1; i < numberOfOccurence; i++)
    {
        filter = strstr(filter + 1, targetStr);
        if (!filter) return bkpPos;
        bkpPos = filter - buffer;
    }

    return bkpPos;
}

DWORD GetProcessID(std::wstring processName)
{
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32W pEntry{};
    pEntry.dwSize = sizeof(PROCESSENTRY32W);

    if (Process32FirstW(hSnap, &pEntry))
    {
        if (wcscmp(pEntry.szExeFile, processName.c_str()) == 0)
        {
            CloseHandle(hSnap);
            return pEntry.th32ProcessID;
        }

        while (Process32NextW(hSnap, &pEntry))
        {
            if (wcscmp(pEntry.szExeFile, processName.c_str()) == 0)
            {
                CloseHandle(hSnap);
                return pEntry.th32ProcessID;
            }
        }
    }

    CloseHandle(hSnap);

    return 0;
}

int main(int argc, char** argv)
{
    std::wstring processName;
#if RAW_INPUT

    // Change the target process's name here (if using raw input)
    processName = L"notepad.exe";
#else

    char* curDirectory = argv[0];

    curDirectory[getStrPos(curDirectory, "\\", -1)] = '\0'; // Cutting .exe file

    std::string targetFilePath = std::string(curDirectory) + std::string("\\input.txt");

    FILE* pFile = fopen(targetFilePath.c_str(), "r");

    MessageBoxA(NULL, targetFilePath.c_str(), "TEST", MB_OK | MB_ICONINFORMATION);

    if (!pFile)
    {
        std::cout << "[ERRO] Não foi possível abrir o arquivo de entrada que contém o processo alvo. Verifique se o arquivo está com o nome digitado corretamente e se o mesmo se encontra no mesmo diretório que o programa." << std::endl;
        system("pause");
    }

    fseek(pFile, 0, SEEK_END);
    int fileSize = ftell(pFile);
    fseek(pFile, 0, SEEK_SET);

    char* buffer = (char*)calloc(fileSize, sizeof(char));

    if (!buffer)
    {
        fclose(pFile);
        std::cout << "[ERRO] Não foi possível alocar memória para ler o arquivo de entrada." << std::endl;
        system("pause");
    }

    if (!fread(buffer, sizeof(char), fileSize, pFile))
    {
        fclose(pFile);
        std::cout << "[ERRO] Não foi possível ler o arquivo de entrada que contém o processo alvo." << std::endl;
        system("pause");
    }

    fclose(pFile);

    std::string bufferStrCpp = buffer;

    processName = std::wstring(bufferStrCpp.begin(), bufferStrCpp.end());
#endif

    if (argc < 2)
    {
        std::cout << "[ERRO] Não foi possível encontrar a DLL alvo para injeção. Tente novamente arrastando-a para o injetor.\n\n";
        system("pause");
        return 0;
    }

    std::string dllToInject = std::string(argv[1]);

    std::cout << "[+] Abrindo handle para o processo alvo..." << std::endl;

    HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, NULL, GetProcessID(processName));

    if (!hProc || hProc == INVALID_HANDLE_VALUE)
    {
        std::cout << "[ERRO] Não foi possível abrir uma handle para o processo alvo.\n\n";
        system("pause");
        return 0;
    }

    std::cout << "[+] Alocando memória para DLL no processo alvo..." << std::endl;

    DWORD bufferAddr = reinterpret_cast<DWORD>(VirtualAllocEx(hProc, NULL, dllToInject.size() + 1, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE));

    if (!bufferAddr)
    {
        CloseHandle(hProc);
        std::cout << "[ERRO] Não foi possível alocar memória no processo alvo.\n\n";
        system("pause");
        return 0;
    }

    std::cout << "[+] Escrevendo a DLL no processo alvo..." << std::endl;

    if (!WriteProcessMemory(hProc, reinterpret_cast<LPVOID>(bufferAddr), dllToInject.c_str(), dllToInject.size(), NULL))
    {
        VirtualFreeEx(hProc, reinterpret_cast<LPVOID>(bufferAddr), dllToInject.size(), MEM_FREE | MEM_RELEASE);
        CloseHandle(hProc);
        std::cout << "[ERRO] Não foi possível escrever na memória no processo alvo.\n\n";
        system("pause");
        return 0;
    }

    std::cout << "[+] Criando thread no processo alvo..." << std::endl;

    if (!CreateRemoteThread(hProc, NULL, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(GetProcAddress(GetModuleHandleW(L"KERNEL32.dll"), "LoadLibraryA")), reinterpret_cast<LPVOID>(bufferAddr), NULL, NULL))
    {
        VirtualFreeEx(hProc, reinterpret_cast<LPVOID>(bufferAddr), dllToInject.size(), MEM_FREE | MEM_RELEASE);
        CloseHandle(hProc);
        std::cout << "[ERRO] Não foi criar uma thread no processo alvo.\n\n";
        system("pause");
        return 0;
    }

    std::cout << "[+] DLL injetada com sucesso!" << std::endl << std::endl;

    system("pause");

    CloseHandle(hProc);
    return 0;
}
