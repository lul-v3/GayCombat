#include "includes.h"
#include "mem.h"
#include "cheats.h"
#include "helper.h"

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

bool menuOpen = false;


// Function to handle cheatmenu logic
void CheatMenuLogic()
{
    DWORD processId = helper::FindProcessId("CombatMaster.exe");    // Find the PID of CombatMaster.exe
    if (processId == 0) {
        std::cout << "CombatMaster.exe not found.";                 // Log if the process is not found
        return;
    }

    // Open the target process with required permissions
    HANDLE hProcess = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE, FALSE, processId);
    if (hProcess == NULL) {
        std::cout << "Failed to open process: " + std::to_string(GetLastError()); // Log if opening fails
        return;
    }

    HandleCheats(hProcess);
}

// Function to initialize ImGui context and settings
void InitImGui() {
    ImGui::CreateContext();                                 // Create a new ImGui context
    ImGuiIO& io = ImGui::GetIO();                           // Get the IO structure for ImGui
    io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;  // Configure ImGui settings
    ImGui_ImplWin32_Init(window);                           // Initialize ImGui for Windows
    ImGui_ImplDX11_Init(pDevice, pContext);                 // Initialize ImGui for DirectX 11

	// Purple Comfy style by RegularLunar from ImThemes
	ImGuiStyle& style = ImGui::GetStyle();

	style.Alpha = 1.0f;
	style.WindowPadding = ImVec2(8.0f, 8.0f);
	style.WindowRounding = 10.0f;
	style.WindowBorderSize = 0.0f;
	style.WindowMinSize = ImVec2(30.0f, 30.0f);
	style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
	style.WindowMenuButtonPosition = ImGuiDir_Right;
	style.ChildRounding = 5.0f;
	style.ChildBorderSize = 1.0f;
	style.PopupRounding = 10.0f;
	style.PopupBorderSize = 0.0f;
	style.FramePadding = ImVec2(5.0f, 3.5f);
	style.FrameRounding = 5.0f;
	style.FrameBorderSize = 0.0f;
	style.ItemSpacing = ImVec2(5.0f, 4.0f);
	style.ItemInnerSpacing = ImVec2(5.0f, 5.0f);
	style.IndentSpacing = 5.0f;
	style.ColumnsMinSpacing = 5.0f;
	style.ScrollbarSize = 15.0f;
	style.ScrollbarRounding = 9.0f;
	style.GrabMinSize = 15.0f;
	style.GrabRounding = 5.0f;
	style.TabRounding = 5.0f;
	style.TabBorderSize = 0.0f;
	style.ColorButtonPosition = ImGuiDir_Right;
	style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
	style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

	style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	style.Colors[ImGuiCol_TextDisabled] = ImVec4(1.0f, 1.0f, 1.0f, 0.3605149984359741f);
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.09803921729326248f, 0.09803921729326248f, 0.09803921729326248f, 1.0f);
	style.Colors[ImGuiCol_ChildBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.0f);
	style.Colors[ImGuiCol_PopupBg] = ImVec4(0.09803921729326248f, 0.09803921729326248f, 0.09803921729326248f, 1.0f);
	style.Colors[ImGuiCol_Border] = ImVec4(0.501960813999176f, 0.3019607961177826f, 1.0f, 0.5490196347236633f);
	style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	style.Colors[ImGuiCol_FrameBg] = ImVec4(0.1568627506494522f, 0.1568627506494522f, 0.1568627506494522f, 1.0f);
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.3803921639919281f, 0.4235294163227081f, 0.572549045085907f, 0.5490196347236633f);
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.501960813999176f, 0.3019607961177826f, 1.0f, 0.5490196347236633f);
	style.Colors[ImGuiCol_TitleBg] = ImVec4(0.09803921729326248f, 0.09803921729326248f, 0.09803921729326248f, 1.0f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.09803921729326248f, 0.09803921729326248f, 0.09803921729326248f, 1.0f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.2588235437870026f, 0.2588235437870026f, 0.2588235437870026f, 0.0f);
	style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.1568627506494522f, 0.1568627506494522f, 0.1568627506494522f, 0.0f);
	style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.1568627506494522f, 0.1568627506494522f, 0.1568627506494522f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.2352941185235977f, 0.2352941185235977f, 0.2352941185235977f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.294117659330368f, 0.294117659330368f, 0.294117659330368f, 1.0f);
	style.Colors[ImGuiCol_CheckMark] = ImVec4(0.501960813999176f, 0.3019607961177826f, 1.0f, 0.5490196347236633f);
	style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.501960813999176f, 0.3019607961177826f, 1.0f, 0.5490196347236633f);
	style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.501960813999176f, 0.3019607961177826f, 1.0f, 0.5490196347236633f);
	style.Colors[ImGuiCol_Button] = ImVec4(0.501960813999176f, 0.3019607961177826f, 1.0f, 0.5490196347236633f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.501960813999176f, 0.3019607961177826f, 1.0f, 0.5490196347236633f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.501960813999176f, 0.3019607961177826f, 1.0f, 0.5490196347236633f);
	style.Colors[ImGuiCol_Header] = ImVec4(0.501960813999176f, 0.3019607961177826f, 1.0f, 0.5490196347236633f);
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.501960813999176f, 0.3019607961177826f, 1.0f, 0.5490196347236633f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.501960813999176f, 0.3019607961177826f, 1.0f, 0.5490196347236633f);
	style.Colors[ImGuiCol_Separator] = ImVec4(0.501960813999176f, 0.3019607961177826f, 1.0f, 0.5490196347236633f);
	style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.501960813999176f, 0.3019607961177826f, 1.0f, 0.5490196347236633f);
	style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.501960813999176f, 0.3019607961177826f, 1.0f, 0.5490196347236633f);
	style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.501960813999176f, 0.3019607961177826f, 1.0f, 0.5490196347236633f);
	style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.501960813999176f, 0.3019607961177826f, 1.0f, 0.5490196347236633f);
	style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.501960813999176f, 0.3019607961177826f, 1.0f, 0.5490196347236633f);
	style.Colors[ImGuiCol_Tab] = ImVec4(0.501960813999176f, 0.3019607961177826f, 1.0f, 0.5490196347236633f);
	style.Colors[ImGuiCol_TabHovered] = ImVec4(0.501960813999176f, 0.3019607961177826f, 1.0f, 0.5490196347236633f);
	style.Colors[ImGuiCol_TabActive] = ImVec4(0.501960813999176f, 0.3019607961177826f, 1.0f, 0.5490196347236633f);
	style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.0f, 0.4509803950786591f, 1.0f, 0.0f);
	style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.1333333402872086f, 0.2588235437870026f, 0.4235294163227081f, 0.0f);
	style.Colors[ImGuiCol_PlotLines] = ImVec4(0.294117659330368f, 0.294117659330368f, 0.294117659330368f, 1.0f);
	style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.501960813999176f, 0.3019607961177826f, 1.0f, 0.5490196347236633f);
	style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.501960813999176f, 0.3019607961177826f, 1.0f, 0.5490196347236633f);
	style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.7372549176216125f, 0.6941176652908325f, 0.886274516582489f, 0.5490196347236633f);
	style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.501960813999176f, 0.3019607961177826f, 1.0f, 0.5490196347236633f);
	style.Colors[ImGuiCol_DragDropTarget] = ImVec4(1.0f, 1.0f, 0.0f, 0.8999999761581421f);
	style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 1.0f, 1.0f, 0.699999988079071f);
	style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.800000011920929f, 0.800000011920929f, 0.800000011920929f, 0.2000000029802322f);
	style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.800000011920929f, 0.800000011920929f, 0.800000011920929f, 0.3499999940395355f);
}

// Window procedure function to handle messages for the ImGui window
LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

    if (uMsg == WM_KEYDOWN && wParam == VK_INSERT)
        menuOpen = !menuOpen;

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
            pDevice->GetImmediateContext(&pContext);                                        // Get the immediate device context
            DXGI_SWAP_CHAIN_DESC sd;
            pSwapChain->GetDesc(&sd);                                                       // Get swap chain description
            window = sd.OutputWindow;                                                       // Store the output window handle
            ID3D11Texture2D* pBackBuffer;
            pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);     // Get the back buffer
            pDevice->CreateRenderTargetView(pBackBuffer, NULL, &mainRenderTargetView);      // Create a render target view
            pBackBuffer->Release();                                                         // Release the back buffer
            oWndProc = (WNDPROC)SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)WndProc);  // Set the new window procedure
            InitImGui();                                                                    // Initialize ImGui
            init = true;                                                                    // Set the initialized flag
        }
        else {
            return oPresent(pSwapChain, SyncInterval, Flags);                               // Call the original present function if initialization fails
        }
    }

    // Start a new ImGui frame
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();                          // Begin a new ImGui frame
    ImGui::SetNextWindowSize(ImVec2(700, 450)); // Set the size of the ImGui window
	if (menuOpen)
	{

        // Create ImGui window for cheat settings
        ImGui::Begin("GoofyCombat Menu", &menuOpen, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);

        if (ImGui::BeginTabBar("Main"))
        {
            if (ImGui::BeginTabItem("Home"))
            {
                ImGui::Text("Please read!");

                // Menu Infos Section
                if (ImGui::CollapsingHeader("Menu Infos")) {
                    ImGui::TextWrapped(
                        "Welcome to lulv3's GoofyCombat menu.\n"
                        "This menu is very basic and is far from finished.\n"
                        "If you come across errors or have other concerns about this mod menu, "
                        "you can create an 'issue' on Github.\n\n"
                        "Menu version: v0.2.0\n"
                        "Developer: lulv3\n"
                        "Github: https://github.com/lulv3/GoofyCombat\n"
                        "For more information and updates, visit the repository."
                    );

                    // Füge mehr Informationen oder Optionen hinzu
                    ImGui::Separator();
                    ImGui::Text("Additional Info:");
                    ImGui::BulletText("Missing features:");
                    ImGui::BulletText("- Aimbot and ESP enhancements");
                    ImGui::BulletText("- Customizable key bindings");
                    ImGui::BulletText("- Performance optimizations");
                    ImGui::BulletText("- Bug fixes and stability improvements");
                }

                // Disclaimer Section
                if (ImGui::CollapsingHeader("Disclaimer")) {
                    ImGui::TextWrapped(
                        "Disclaimer: By using this mod menu, you acknowledge the following:\n\n"
                        "1. This software is provided 'as is', without warranty of any kind.\n"
                        "2. You are solely responsible for any actions taken with this software.\n"
                        "3. The developer is not liable for any damages, including bans or legal issues.\n\n"
                        "Use this mod at your own risk. Cheating in online games may result in a ban or other consequences."
                    );

                    // Hinzufügung einer Checkbox für die Zustimmung zum Disclaimer
                    ImGui::Text("By using the menu you agree to the terms of use!");
                }
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Equip"))
            {
                if (ImGui::CollapsingHeader("Primary")) {
                    ImGui::Checkbox("Unlimited Ammo", &unlimitedPrimaryAmmo);   // Checkbox for unlimited ammo
                }
                ImGui::Separator();
                if (ImGui::CollapsingHeader("Secondary")) {
                    ImGui::Checkbox("Unlimited Ammo", &unlimitedSecondaryAmmo); // Checkbox for unlimited ammo
                }
                ImGui::Separator();
                ImGui::Checkbox("Unlimited Grenade", &unlimitedGrenade);        // Checkbox for unlimited grenades

                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }

        ImGui::End(); // End ImGui window
	}
    ImGui::Render(); // Render the ImGui frame

	// Handle cheat menu logic
	CheatMenuLogic(); 

    pContext->OMSetRenderTargets(1, &mainRenderTargetView, NULL);   // Set the render target
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());            // Render ImGui draw data
    return oPresent(pSwapChain, SyncInterval, Flags);               // Call the original present function
}

// Main thread function for initializing the hook
DWORD WINAPI MainThread(LPVOID lpReserved) {
    bool init_hook = false; // Flag for hook initialization
    do {
        // Attempt to initialize the kiero library for Direct3D
        if (kiero::init(kiero::RenderType::D3D11) == kiero::Status::Success) {
            kiero::bind(8, (void**)&oPresent, hkPresent);                       // Bind the present function to the hook
            init_hook = true;                                                   // Set hook initialized flag
        }
    } while (!init_hook);                                                       // Loop until initialization is successful
    return TRUE;                                                                // Return TRUE upon completion
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
        DisableThreadLibraryCalls(hMod);                        // Disable thread notifications for the DLL
        // AttachConsole();                                     // Attach console for debugging
        CreateThread(nullptr, 0, MainThread, hMod, 0, nullptr); // Create a new thread for the main hook
        break;
    case DLL_PROCESS_DETACH:
        kiero::shutdown();                                      // Shut down the kiero library
        break;
    }
    return TRUE;                                                // Return TRUE for successful DLL entry
}