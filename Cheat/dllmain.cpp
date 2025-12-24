/*
 * Game Hack DLL Entry Point
 * D3D11 Hook with ImGui Integration
 */

#include <Windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <thread>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

// ImGui
#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"

// Our hack
#include "GameHack.h"

// Forward declarations
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

//=============================================================================
// Globals
//=============================================================================
GameHack::GameManager g_GameManager;
GameHack::Renderer* g_Renderer = nullptr;

// D3D11 Hook
typedef HRESULT(__stdcall* PresentFn)(IDXGISwapChain*, UINT, UINT);
typedef HRESULT(__stdcall* ResizeBuffersFn)(IDXGISwapChain*, UINT, UINT, UINT, DXGI_FORMAT, UINT);

PresentFn oPresent = nullptr;
ResizeBuffersFn oResizeBuffers = nullptr;

ID3D11Device* g_Device = nullptr;
ID3D11DeviceContext* g_Context = nullptr;
ID3D11RenderTargetView* g_RenderTargetView = nullptr;
HWND g_GameWindow = nullptr;
WNDPROC g_OriginalWndProc = nullptr;

bool g_Initialized = false;
bool g_ShowMenu = true;
bool g_Running = true;

//=============================================================================
// Window Procedure Hook
//=============================================================================
LRESULT CALLBACK HookedWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == WM_KEYDOWN && wParam == VK_INSERT) {
        g_ShowMenu = !g_ShowMenu;
        return 0;
    }

    if (g_ShowMenu) {
        ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);

        // Block game input when menu is open
        if (ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard) {
            return 0;
        }
    }

    return CallWindowProc(g_OriginalWndProc, hWnd, uMsg, wParam, lParam);
}

//=============================================================================
// Initialize ImGui and Render Target
//=============================================================================
bool InitializeImGui(IDXGISwapChain* pSwapChain) {
    if (FAILED(pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&g_Device))) {
        return false;
    }
    g_Device->GetImmediateContext(&g_Context);

    DXGI_SWAP_CHAIN_DESC sd;
    pSwapChain->GetDesc(&sd);
    g_GameWindow = sd.OutputWindow;

    // Create render target view
    ID3D11Texture2D* pBackBuffer = nullptr;
    if (FAILED(pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer))) {
        return false;
    }

    if (FAILED(g_Device->CreateRenderTargetView(pBackBuffer, nullptr, &g_RenderTargetView))) {
        pBackBuffer->Release();
        return false;
    }
    pBackBuffer->Release();

    // Hook WndProc
    g_OriginalWndProc = (WNDPROC)SetWindowLongPtr(g_GameWindow, GWLP_WNDPROC, (LONG_PTR)HookedWndProc);

    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Style
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 5.0f;
    style.FrameRounding = 3.0f;
    style.Colors[ImGuiCol_TitleBg] = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.3f, 0.1f, 0.1f, 1.0f);

    ImGui_ImplWin32_Init(g_GameWindow);
    ImGui_ImplDX11_Init(g_Device, g_Context);

    // Initialize our hack
    g_GameManager.Initialize();
    g_Renderer = new GameHack::Renderer(g_GameManager);

    return true;
}

//=============================================================================
// Present Hook
//=============================================================================
HRESULT __stdcall HookedPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags) {
    if (!g_Initialized) {
        if (InitializeImGui(pSwapChain)) {
            g_Initialized = true;
        }
    }

    if (g_Initialized) {
        // Start ImGui frame
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // Update screen size
        ImGuiIO& io = ImGui::GetIO();
        g_GameManager.SetScreenSize((int)io.DisplaySize.x, (int)io.DisplaySize.y);

        // Update game data
        g_GameManager.Update();

        // Run aimbot
        g_GameManager.RunAimbot();
        g_GameManager.RunMagicBullet();

        // Render ESP
        g_Renderer->RenderESP();

        // Render menu
        if (g_ShowMenu) {
            g_Renderer->RenderMenu();
        }

        // End ImGui frame
        ImGui::EndFrame();
        ImGui::Render();

        g_Context->OMSetRenderTargets(1, &g_RenderTargetView, nullptr);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    }

    return oPresent(pSwapChain, SyncInterval, Flags);
}

//=============================================================================
// ResizeBuffers Hook
//=============================================================================
HRESULT __stdcall HookedResizeBuffers(IDXGISwapChain* pSwapChain, UINT BufferCount,
    UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags) {

    if (g_RenderTargetView) {
        g_RenderTargetView->Release();
        g_RenderTargetView = nullptr;
    }

    HRESULT hr = oResizeBuffers(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);

    // Recreate render target view
    ID3D11Texture2D* pBackBuffer = nullptr;
    if (SUCCEEDED(pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer))) {
        g_Device->CreateRenderTargetView(pBackBuffer, nullptr, &g_RenderTargetView);
        pBackBuffer->Release();
    }

    return hr;
}

//=============================================================================
// Get SwapChain VTable
//=============================================================================
bool GetD3D11SwapChainVTable(void** pVTable) {
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, DefWindowProc, 0L, 0L,
                      GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr,
                      L"DummyClass", nullptr };
    RegisterClassEx(&wc);

    HWND hWnd = CreateWindow(wc.lpszClassName, L"Dummy", WS_OVERLAPPEDWINDOW,
                             0, 0, 100, 100, nullptr, nullptr, wc.hInstance, nullptr);
    if (!hWnd) {
        UnregisterClass(wc.lpszClassName, wc.hInstance);
        return false;
    }

    DXGI_SWAP_CHAIN_DESC sd = {};
    sd.BufferCount = 1;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.Width = 100;
    sd.BufferDesc.Height = 100;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    IDXGISwapChain* pSwapChain = nullptr;
    ID3D11Device* pDevice = nullptr;
    ID3D11DeviceContext* pContext = nullptr;
    D3D_FEATURE_LEVEL featureLevel;

    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0,
        D3D11_SDK_VERSION, &sd, &pSwapChain, &pDevice, &featureLevel, &pContext
    );

    if (FAILED(hr)) {
        DestroyWindow(hWnd);
        UnregisterClass(wc.lpszClassName, wc.hInstance);
        return false;
    }

    memcpy(pVTable, *reinterpret_cast<void***>(pSwapChain), sizeof(void*) * 18);

    pSwapChain->Release();
    pDevice->Release();
    pContext->Release();
    DestroyWindow(hWnd);
    UnregisterClass(wc.lpszClassName, wc.hInstance);

    return true;
}

//=============================================================================
// Hook Functions using VTable
//=============================================================================
void HookVTable(void** pVTable, int index, void* pHook, void** pOriginal) {
    DWORD oldProtect;
    VirtualProtect(&pVTable[index], sizeof(void*), PAGE_EXECUTE_READWRITE, &oldProtect);
    *pOriginal = pVTable[index];
    pVTable[index] = pHook;
    VirtualProtect(&pVTable[index], sizeof(void*), oldProtect, &oldProtect);
}

//=============================================================================
// Main Hook Thread
//=============================================================================
void MainThread(HMODULE hModule) {
    // Wait for game to load
    Sleep(5000);

    void* vTable[18];
    if (!GetD3D11SwapChainVTable(vTable)) {
        return;
    }

    // Hook Present (index 8) and ResizeBuffers (index 13)
    // Note: For production use, consider MinHook or similar library
    oPresent = (PresentFn)vTable[8];
    oResizeBuffers = (ResizeBuffersFn)vTable[13];

    // Simple IAT/VTable hook - for real use, consider MinHook
    // This is a simplified example

    // Create console for debug (optional)
    #ifdef _DEBUG
    AllocConsole();
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);
    printf("[+] Game Hack Loaded\n");
    printf("[+] Press INSERT to toggle menu\n");
    #endif

    // Main loop
    while (g_Running) {
        if (GetAsyncKeyState(VK_END) & 1) {
            g_Running = false;
        }
        Sleep(100);
    }

    // Cleanup
    if (g_Initialized) {
        SetWindowLongPtr(g_GameWindow, GWLP_WNDPROC, (LONG_PTR)g_OriginalWndProc);
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();

        if (g_RenderTargetView) g_RenderTargetView->Release();
        if (g_Context) g_Context->Release();
        if (g_Device) g_Device->Release();
    }

    delete g_Renderer;

    #ifdef _DEBUG
    FreeConsole();
    #endif

    FreeLibraryAndExitThread(hModule, 0);
}

//=============================================================================
// DLL Entry Point
//=============================================================================
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
            DisableThreadLibraryCalls(hModule);
            CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)MainThread, hModule, 0, nullptr);
            break;
        case DLL_PROCESS_DETACH:
            g_Running = false;
            break;
    }
    return TRUE;
}
