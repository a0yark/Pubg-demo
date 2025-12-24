# Game Hack - Build Instructions

## Project Structure

```
Cheat/
├── dllmain.cpp      # DLL entry, D3D11 hook, ImGui integration
├── GameHack.h       # Core hack classes (memory, game wrappers, math)
├── Renderer.cpp     # ImGui rendering implementation
├── imgui/           # ImGui library (you need to add this)
│   ├── imgui.h
│   ├── imgui.cpp
│   ├── imgui_draw.cpp
│   ├── imgui_tables.cpp
│   ├── imgui_widgets.cpp
│   ├── imgui_impl_win32.h
│   ├── imgui_impl_win32.cpp
│   ├── imgui_impl_dx11.h
│   └── imgui_impl_dx11.cpp
└── README.md
```

## Setup Instructions

### 1. Download ImGui

Download ImGui from: https://github.com/ocornut/imgui

Copy these files to `Cheat/imgui/`:
- imgui.h, imgui.cpp
- imgui_draw.cpp
- imgui_tables.cpp
- imgui_widgets.cpp
- imgui_internal.h
- imstb_rectpack.h
- imstb_textedit.h
- imstb_truetype.h
- backends/imgui_impl_win32.h
- backends/imgui_impl_win32.cpp
- backends/imgui_impl_dx11.h
- backends/imgui_impl_dx11.cpp

### 2. Create Visual Studio Project

1. Open Visual Studio 2022
2. Create New Project > Dynamic-Link Library (DLL) > C++
3. Add all .cpp and .h files to the project
4. Configure project settings:

**Project Properties:**
- Configuration: Release
- Platform: x64
- C/C++ > General > Additional Include Directories: `$(ProjectDir)`
- C/C++ > Code Generation > Runtime Library: `/MT`
- Linker > Input > Additional Dependencies: `d3d11.lib;dxgi.lib`

### 3. Build

Build the project (Ctrl+Shift+B) to generate the DLL.

## Usage

### Inject the DLL

Use a DLL injector to load the compiled DLL into the game process.

Popular injectors:
- Process Hacker 2
- Xenos Injector
- Manual mapping tools

### Controls

| Key | Action |
|-----|--------|
| INSERT | Toggle menu |
| END | Unload hack |
| Right Mouse Button | Aimbot (when enabled) |

## Key Offsets

All offsets are based on SDK analysis:

```cpp
// Global
GWorld:        0x0A5140F0
GObjects:      0x0A5092B0

// Important class offsets
World_GameState:                0x0158
GameInstance_LocalPlayers:      0x0038
Player_PlayerController:        0x0030
GameState_PlayerArray:          0x02C8
Controller_ControlRotation:     0x0328
PlayerController_CameraManager: 0x0368
P2BaseCharacter_HealthSet:      0x07C8
PlayerState_TeamId:             0x04A4
```

## Troubleshooting

### Crash on inject
- Verify offsets match your game version
- Check if game uses anti-cheat that blocks injection
- Try different injection method (manual mapping)

### ESP not showing
- Check if WorldToScreen is working (verify camera offsets)
- Verify player array is being read correctly
- Add debug logging to track issues

### Aimbot not working
- Verify ControlRotation offset
- Check if game uses different rotation method
- Adjust smoothing value

## Customization

### Change bone target
Edit `BoneID` namespace in `GameHack.h`:
```cpp
namespace BoneID {
    constexpr int Head = 98;   // Adjust based on game skeleton
    constexpr int Neck = 97;
    constexpr int Chest = 66;
}
```

### Add new ESP features
Modify `Renderer::RenderESP()` in `Renderer.cpp`

### Change colors
Edit color definitions in `RenderESP()`:
```cpp
ImU32 colorEnemy = IM_COL32(255, 0, 0, 255);  // Red
ImU32 colorTeam = IM_COL32(0, 255, 0, 255);   // Green
```

## Disclaimer

This is for educational purposes only. Use only on games you own or have permission to modify.
