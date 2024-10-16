#pragma once
#include <Windows.h>
#include <string>
#include <iostream>
#include <Psapi.h>


namespace helper
{
    // Function to read a 4-byte integer value from the memory of a process
    int ReadInt(HANDLE hProcess, DWORD_PTR address) {
        int value = 0; // Initialize to avoid random values
        SIZE_T bytesRead = 0;
        // Attempt to read the memory at the specified address
        if (!ReadProcessMemory(hProcess, (LPCVOID)address, &value, sizeof(value), &bytesRead) || bytesRead != sizeof(value)) {
            // Log error if reading fails
            std::cout << "Failed to read memory at address: " + std::to_string(address) + " Error: " + std::to_string(GetLastError());
            return -1; // Return -1 in case of error
        }
        return value; // Return the read value
    }

    // Function to write a 4-byte integer value to the memory of a process
    void WriteInt(HANDLE hProcess, DWORD_PTR address, int value) {
        SIZE_T bytesWritten = 0;
        // Attempt to write the value to the specified address
        if (!WriteProcessMemory(hProcess, (LPVOID)address, &value, sizeof(value), &bytesWritten) || bytesWritten != sizeof(value)) {
            // Log error if writing fails
            std::cout << "Failed to write memory at address: " + std::to_string(address) + " Error: " + std::to_string(GetLastError());
        }
    }

    // Function to find the process ID of "CombatMaster.exe"
    DWORD FindProcessId(const std::string& processName) {
        DWORD processIds[1024], processesCount;
        // Enumerate the processes currently running
        if (!EnumProcesses(processIds, sizeof(processIds), &processesCount)) {
            std::cout << "Failed to enumerate processes.";
            return 0; // Return 0 if enumeration fails
        }

        processesCount /= sizeof(DWORD); // Calculate the number of processes

        // Loop through all process IDs
        for (unsigned int i = 0; i < processesCount; i++) {
            // Open the process to query information
            HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processIds[i]);
            if (hProcess) {
                char processNameBuffer[MAX_PATH];
                // Get the executable name of the process
                if (GetModuleFileNameEx(hProcess, NULL, processNameBuffer, sizeof(processNameBuffer) / sizeof(char))) {
                    // Compare the name with the target process name
                    if (strstr(processNameBuffer, processName.c_str()) != NULL) {
                        CloseHandle(hProcess); // Close the handle to the process
                        return processIds[i]; // Return the found process ID
                    }
                }
                CloseHandle(hProcess); // Close the handle if not matching
            }
        }
        return 0; // Return 0 if the PID is not found
    }
}