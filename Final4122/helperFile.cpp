/*
Author: Kenny Dang
Class: ECE4122
Last Date Modified: 12/3/2024
Description:

*/
#include <iostream>
#include <string>
#include <windows.h>

HANDLE handleInWrite, handleInRead;
HANDLE handleOutWrite, handleOutRead;

void StartEngine() {
    // Create pipes for input and output
    SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
    CreatePipe(&handleOutRead, &handleOutWrite, &sa, 0);
    CreatePipe(&handleInRead, &handleInWrite, &sa, 0);

    // Start the Komodo engine
    STARTUPINFOW si = { sizeof(STARTUPINFOW) };
    PROCESS_INFORMATION pi;
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = handleInRead;
    si.hStdOutput = handleOutWrite;
    si.hStdError = handleOutWrite;

    // Path to Komodo executable
    std::wstring enginePath = L"komodo.exe"; // Use wide string (L"")
    if (!CreateProcessW(NULL, const_cast<LPWSTR>(enginePath.c_str()), NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
        std::cerr << "Failed to start engine" << std::endl;
        return;
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}


void SendToEngine(const std::string& command) {
    DWORD written;
    WriteFile(handleInWrite, command.c_str(), command.length(), &written, NULL);
    WriteFile(handleInWrite, "\n", 1, &written, NULL);
}

std::string ReadFromEngine() {
    char buffer[4096];
    DWORD read;
    std::string output;
    if (ReadFile(handleOutRead, buffer, sizeof(buffer) - 1, &read, NULL) && read > 0) {
        buffer[read] = '\0';
        output = buffer;
    }
    return output;
}

