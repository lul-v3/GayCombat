#include "includes.h"

// External declaration for ImGui window procedure handler
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Function pointer for present function
Present oPresent;

// Window handle and procedure
HWND window = NULL;
WNDPROC oWndProc;

// Direct3D device and context
ID3D11Device* pDevice = NULL;
ID3D11DeviceContext* pContext = NULL;
ID3D11RenderTargetView* mainRenderTargetView;

// Output file stream for debug logging
std::ofstream debugLog("debug_log.txt");

// Memory addresses for game values
DWORD_PTR ammoAddress = 0x1D78C471230; // Address for ammo value
DWORD_PTR magAddress = 0x1D78C4712D8; // Address for magazine value
DWORD_PTR grenadeAddress = 0x1D78C471260; // Address for grenade value

// Cheat settings flags
bool unlimitedAmmo = false; // Flag for unlimited ammo
bool unlimitedMag = false; // Flag for unlimited magazine
bool unlimitedGrenade = false; // Flag for unlimited grenades

// Function to write messages to the debug log
void WriteToDebugLog(const std::string& message) {
    if (debugLog.is_open()) {
        debugLog << message << std::endl; // Write message to log file
    }
}

// Function to close the debug log file
void CloseDebugLog() {
    if (debugLog.is_open()) {
        debugLog.close(); // Close the log file if open
    }
}

// Function to read a 4-byte integer value from the memory of a process
int ReadInt(HANDLE hProcess, DWORD_PTR address) {
    int value = 0; // Initialize to avoid random values
    SIZE_T bytesRead = 0;
    // Attempt to read the memory at the specified address
    if (!ReadProcessMemory(hProcess, (LPCVOID)address, &value, sizeof(value), &bytesRead) || bytesRead != sizeof(value)) {
        // Log error if reading fails
        WriteToDebugLog("Failed to read memory at address: " + std::to_string(address) + " Error: " + std::to_string(GetLastError()));
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
        WriteToDebugLog("Failed to write memory at address: " + std::to_string(address) + " Error: " + std::to_string(GetLastError()));
    }
}

// Function to find the process ID of "CombatMaster.exe"
DWORD FindProcessId(const std::string& processName) {
    DWORD processIds[1024], processesCount;
    // Enumerate the processes currently running
    if (!EnumProcesses(processIds, sizeof(processIds), &processesCount)) {
        WriteToDebugLog("Failed to enumerate processes.");
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

// Function to handle cheat codes for the game
void HandleCheatCode() {
    DWORD processId = FindProcessId("CombatMaster.exe"); // Find the PID of CombatMaster.exe
    if (processId == 0) {
        WriteToDebugLog("CombatMaster.exe not found."); // Log if the process is not found
        return;
    }

    // Open the target process with required permissions
    HANDLE hProcess = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE, FALSE, processId);
    if (hProcess == NULL) {
        WriteToDebugLog("Failed to open process: " + std::to_string(GetLastError())); // Log if opening fails
        return;
    }

    // Handle unlimited ammo setting
    if (unlimitedAmmo) {
        int currentAmmo = ReadInt(hProcess, ammoAddress); // Read current ammo
        WriteToDebugLog("Current Ammo: " + std::to_string(currentAmmo)); // Log current ammo
        WriteToDebugLog("Setting Ammo to 150."); // Log the action
        WriteInt(hProcess, ammoAddress, 150); // Set ammo to 150
    }

    // Handle unlimited magazine setting
    if (unlimitedMag) {
        WriteToDebugLog("Setting Magazine to 150."); // Log the action
        WriteInt(hProcess, magAddress, 150); // Set magazine to 150
    }

    // Handle unlimited grenade setting
    if (unlimitedGrenade) {
        WriteToDebugLog("Setting Grenade to 2."); // Log the action
        WriteInt(hProcess, grenadeAddress, 2); // Set grenades to 2
    }

    CloseHandle(hProcess); // Close the handle to the process
}

// Function to initialize ImGui context and settings
void InitImGui() {
    ImGui::CreateContext(); // Create a new ImGui context
    ImGuiIO& io = ImGui::GetIO(); // Get the IO structure for ImGui
    io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange; // Configure ImGui settings
    ImGui_ImplWin32_Init(window); // Initialize ImGui for Windows
    ImGui_ImplDX11_Init(pDevice, pContext); // Initialize ImGui for DirectX 11
}

// Window procedure function to handle messages for the ImGui window
LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
        return true; // Return if ImGui handled the message

    return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam); // Call the original window procedure
}

// Flag for initialization
bool init = false;

// Hook function for rendering
HRESULT __stdcall hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags) {
    if (!init) {
        // If not initialized, attempt to initialize the Direct3D device
        if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&pDevice))) {
            pDevice->GetImmediateContext(&pContext); // Get the immediate device context
            DXGI_SWAP_CHAIN_DESC sd;
            pSwapChain->GetDesc(&sd); // Get swap chain description
            window = sd.OutputWindow; // Store the output window handle
            ID3D11Texture2D* pBackBuffer;
            pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer); // Get the back buffer
            pDevice->CreateRenderTargetView(pBackBuffer, NULL, &mainRenderTargetView); // Create a render target view
            pBackBuffer->Release(); // Release the back buffer
            oWndProc = (WNDPROC)SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)WndProc); // Set the new window procedure
            InitImGui(); // Initialize ImGui
            init = true; // Set the initialized flag
        }
        else {
            return oPresent(pSwapChain, SyncInterval, Flags); // Call the original present function if initialization fails
        }
    }

    // Start a new ImGui frame
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame(); // Begin a new ImGui frame
    ImGui::SetNextWindowSize(ImVec2(400, 500)); // Set the size of the ImGui window

    // Create ImGui window for cheat settings
    ImGui::Begin("GayCombat V.0.1");
    ImGui::Checkbox("Unlimited Ammo", &unlimitedAmmo); // Checkbox for unlimited ammo
    ImGui::NewLine(); // New line
    ImGui::Checkbox("Unlimited Mag", &unlimitedMag); // Checkbox for unlimited magazine
    ImGui::NewLine(); // New line
    ImGui::Checkbox("Unlimited Grenade", &unlimitedGrenade); // Checkbox for unlimited grenades
    ImGui::End(); // End ImGui window
    ImGui::Render(); // Render the ImGui frame

    // Handle cheat codes such as unlimited ammo, magazines, and grenades
    HandleCheatCode();

    pContext->OMSetRenderTargets(1, &mainRenderTargetView, NULL); // Set the render target
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData()); // Render ImGui draw data
    return oPresent(pSwapChain, SyncInterval, Flags); // Call the original present function
}

// Main thread function for initializing the hook
DWORD WINAPI MainThread(LPVOID lpReserved) {
    bool init_hook = false; // Flag for hook initialization
    do {
        // Attempt to initialize the kiero library for Direct3D
        if (kiero::init(kiero::RenderType::D3D11) == kiero::Status::Success) {
            kiero::bind(8, (void**)&oPresent, hkPresent); // Bind the present function to the hook
            init_hook = true; // Set hook initialized flag
        }
    } while (!init_hook); // Loop until initialization is successful
    return TRUE; // Return TRUE upon completion
}

// Function to attach a console for debugging
void AttachConsole() {
    AllocConsole(); // Allocate a new console
    FILE* f;
    // Redirect standard output and error streams to the console
    freopen_s(&f, "CONOUT$", "w", stdout);
    freopen_s(&f, "CONOUT$", "w", stderr);
}

// Entry point for the DLL
BOOL WINAPI DllMain(HMODULE hMod, DWORD dwReason, LPVOID lpReserved) {
    switch (dwReason) {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hMod); // Disable thread notifications for the DLL
        AttachConsole(); // Attach console for debugging
        CreateThread(nullptr, 0, MainThread, hMod, 0, nullptr); // Create a new thread for the main hook
        break;
    case DLL_PROCESS_DETACH:
        CloseDebugLog(); // Close the debug log file
        kiero::shutdown(); // Shut down the kiero library
        break;
    }
    return TRUE; // Return TRUE for successful DLL entry
}
