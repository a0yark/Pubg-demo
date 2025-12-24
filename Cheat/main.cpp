/*
 * Game Hack DLL Entry Point
 * D3D11 Hook with ImGui Integration
 */
#define _CRT_SECURE_NO_WARNINGS
#pragma execution_character_set("utf-8")

#include <Windows.h>
#include <d3d11.h>
#include <dwmapi.h>
#include <string>
#include <tchar.h>
#include <vector>
#include <algorithm>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_impl_win32.h"

#include "GameHack.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dwmapi.lib")

// Data
static ID3D11Device*            g_pd3dDevice = NULL;
static ID3D11DeviceContext*     g_pd3dDeviceContext = NULL;
static IDXGISwapChain*          g_pSwapChain = NULL;
static ID3D11RenderTargetView*  g_mainRenderTargetView = NULL;

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Global State
bool g_Running = true;
bool g_Attached = false;
char g_TargetWindowName[128] = "UnrealWindow"; // Default UE4 window class/name

// Window Enumeration Helper
struct WindowInfo {
    std::string DisplayTitle;
    std::string RawTitle;
    HWND Hwnd;
    DWORD Pid;
};

std::vector<WindowInfo> g_WindowList;
int g_SelectedWindowIndex = -1;

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    if (IsWindowVisible(hwnd)) {
        char title[256];
        GetWindowTextA(hwnd, title, sizeof(title));
        if (strlen(title) > 0) {
            // Filter out our own window and Program Manager
            if (strcmp(title, "GameHack External") == 0 || strcmp(title, "Program Manager") == 0) return TRUE;

            DWORD pid;
            GetWindowThreadProcessId(hwnd, &pid);
            WindowInfo info;
            info.RawTitle = std::string(title);
            info.DisplayTitle = std::string(title) + " [PID:" + std::to_string(pid) + "]";
            info.Hwnd = hwnd;
            info.Pid = pid;
            g_WindowList.push_back(info);
        }
    }
    return TRUE;
}

void RefreshWindowList() {
    g_WindowList.clear();
    EnumWindows(EnumWindowsProc, 0);
    g_SelectedWindowIndex = -1;
}

// Game Manager
GameHack::GameManager g_GameManager;
GameHack::Renderer g_Renderer(g_GameManager);

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    // Create application window
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, _T("GameHack Overlay"), NULL };
    ::RegisterClassEx(&wc);
    
    // Create a transparent, topmost window
    HWND hwnd = ::CreateWindow(wc.lpszClassName, _T("GameHack External"), WS_POPUP, 0, 0, 1920, 1080, NULL, NULL, wc.hInstance, NULL);

    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClass(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    // Show the window
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Load Chinese Font (Microsoft YaHei)
    ImFontConfig font_config;
    font_config.MergeMode = false;
    io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\msyh.ttc", 18.0f, &font_config, io.Fonts->GetGlyphRangesChineseFull());

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    // Make window transparent
    MARGINS margins = { -1 };
    DwmExtendFrameIntoClientArea(hwnd, &margins);

    // Main loop
    while (g_Running)
    {
        MSG msg;
        while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                g_Running = false;
        }
        if (!g_Running) break;

        // Handle Window Position/Size (Follow Game Window if attached)
        if (g_Attached) {
            HWND hGame = FindWindowA(NULL, g_TargetWindowName);
            if (hGame) {
                RECT rect;
                GetWindowRect(hGame, &rect);
                SetWindowPos(hwnd, HWND_TOPMOST, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SWP_NOACTIVATE);
            }
        }

        // Start the Dear ImGui frame
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // Render Menu
        {
            ImGui::Begin("GameHack 外部控制台", &g_Running);
            
            // Window Selection Logic
            if (ImGui::Button("刷新窗口列表")) {
                RefreshWindowList();
            }
            ImGui::SameLine();
            ImGui::Text("找到 %d 个窗口", (int)g_WindowList.size());

            if (!g_WindowList.empty()) {
                std::string previewValue = (g_SelectedWindowIndex >= 0 && g_SelectedWindowIndex < g_WindowList.size()) 
                    ? g_WindowList[g_SelectedWindowIndex].DisplayTitle 
                    : "请选择目标窗口...";

                if (ImGui::BeginCombo("选择游戏窗口", previewValue.c_str())) {
                    for (int i = 0; i < g_WindowList.size(); i++) {
                        const bool is_selected = (g_SelectedWindowIndex == i);
                        if (ImGui::Selectable(g_WindowList[i].DisplayTitle.c_str(), is_selected)) {
                            g_SelectedWindowIndex = i;
                            strncpy(g_TargetWindowName, g_WindowList[i].RawTitle.c_str(), sizeof(g_TargetWindowName) - 1);
                        }

                        if (is_selected)
                            ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }
            }
            
            ImGui::InputText("手动输入窗口名", g_TargetWindowName, IM_ARRAYSIZE(g_TargetWindowName));
            
            if (!g_Attached) {
                if (ImGui::Button("注入 / 附加")) {
                    // Convert char* to wchar_t* for FindWindowW
                    wchar_t wWindowName[128];
                    MultiByteToWideChar(CP_ACP, 0, g_TargetWindowName, -1, wWindowName, 128);
                    
                    if (GameHack::HackMemory::Attach(wWindowName)) {
                        g_Attached = true;
                        GameHack::HackMemory::Initialize(); // Re-init if needed
                    } else {
                        ImGui::OpenPopup("附加失败");
                    }
                }
            } else {
                ImGui::TextColored(ImVec4(0, 1, 0, 1), "已附加到进程 ID: %d", GameHack::HackMemory::ProcessId);
                if (ImGui::Button("分离")) {
                    GameHack::HackMemory::Close();
                    g_Attached = false;
                }
            }

            if (ImGui::BeginPopupModal("附加失败", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::Text("无法找到窗口或打开进程。\n请检查窗口名称并以管理员身份运行。");
                if (ImGui::Button("确定", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
                ImGui::EndPopup();
            }

            // Render the actual Cheat Menu
            g_Renderer.RenderMenu();

            ImGui::End();
        }

        // Render ESP (Overlay)
        if (g_Attached) {
            // Update Game Data
            g_GameManager.Update();
            
            // Render ESP
            // We need a full-screen transparent window for ESP
            ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::SetNextWindowSize(io.DisplaySize);
            ImGui::SetNextWindowBgAlpha(0.0f);
            ImGui::Begin("Overlay", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoBackground);
            
            g_Renderer.RenderESP();
            
            ImGui::End();
        }

        // Rendering
        ImGui::Render();
        const float clear_color_with_alpha[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        g_pSwapChain->Present(1, 0); // Present with vsync
    }

    // Cleanup
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClass(wc.lpszClassName, wc.hInstance);

    return 0;
}

// Helper functions

bool CreateDeviceD3D(HWND hWnd)
{
    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    //createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    if (D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext) != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = NULL; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = NULL; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
}

void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = NULL; }
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
        {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
            CreateRenderTarget();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProc(hWnd, msg, wParam, lParam);
}
