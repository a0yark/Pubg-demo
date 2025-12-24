/*
 * ImGui Renderer Implementation - Enhanced UI
 */

#pragma execution_character_set("utf-8")

#include "GameHack.h"
#include "imgui/imgui.h"
#include <string>
#include <cstdio>

namespace GameHack
{
    // Helper to setup style
    void SetupStyle() {
        ImGuiStyle& style = ImGui::GetStyle();
        
        style.WindowPadding = ImVec2(15, 15);
        style.WindowRounding = 8.0f;
        style.FramePadding = ImVec2(6, 6);
        style.FrameRounding = 4.0f;
        style.ItemSpacing = ImVec2(12, 8);
        style.ItemInnerSpacing = ImVec2(8, 6);
        style.IndentSpacing = 25.0f;
        style.ScrollbarSize = 15.0f;
        style.ScrollbarRounding = 9.0f;
        style.GrabMinSize = 5.0f;
        style.GrabRounding = 3.0f;
        style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
        style.TabRounding = 4.0f;

        // Colors - Modern Dark Blue Theme
        ImVec4* colors = style.Colors;
        colors[ImGuiCol_Text]                   = ImVec4(0.95f, 0.96f, 0.98f, 1.00f);
        colors[ImGuiCol_TextDisabled]           = ImVec4(0.36f, 0.42f, 0.47f, 1.00f);
        colors[ImGuiCol_WindowBg]               = ImVec4(0.10f, 0.10f, 0.13f, 0.96f);
        colors[ImGuiCol_ChildBg]                = ImVec4(0.12f, 0.12f, 0.15f, 1.00f);
        colors[ImGuiCol_PopupBg]                = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
        colors[ImGuiCol_Border]                 = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
        colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_FrameBg]                = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
        colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.22f, 0.28f, 0.33f, 1.00f);
        colors[ImGuiCol_FrameBgActive]          = ImVec4(0.09f, 0.12f, 0.14f, 1.00f);
        colors[ImGuiCol_TitleBg]                = ImVec4(0.09f, 0.09f, 0.12f, 1.00f);
        colors[ImGuiCol_TitleBgActive]          = ImVec4(0.09f, 0.09f, 0.12f, 1.00f);
        colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
        colors[ImGuiCol_MenuBarBg]              = ImVec4(0.15f, 0.18f, 0.22f, 1.00f);
        colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.02f, 0.02f, 0.02f, 0.39f);
        colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.18f, 0.22f, 0.25f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.09f, 0.21f, 0.31f, 1.00f);
        colors[ImGuiCol_CheckMark]              = ImVec4(0.28f, 0.56f, 1.00f, 1.00f); 
        colors[ImGuiCol_SliderGrab]             = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
        colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.37f, 0.61f, 1.00f, 1.00f);
        colors[ImGuiCol_Button]                 = ImVec4(0.18f, 0.22f, 0.27f, 1.00f);
        colors[ImGuiCol_ButtonHovered]          = ImVec4(0.26f, 0.32f, 0.38f, 1.00f);
        colors[ImGuiCol_ButtonActive]           = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
        colors[ImGuiCol_Header]                 = ImVec4(0.20f, 0.25f, 0.29f, 0.55f);
        colors[ImGuiCol_HeaderHovered]          = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
        colors[ImGuiCol_HeaderActive]           = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        colors[ImGuiCol_Separator]              = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
        colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
        colors[ImGuiCol_SeparatorActive]        = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
        colors[ImGuiCol_ResizeGrip]             = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
        colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
        colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
        colors[ImGuiCol_PlotLines]              = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
        colors[ImGuiCol_PlotLinesHovered]       = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
        colors[ImGuiCol_PlotHistogram]          = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
        colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
        colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
        colors[ImGuiCol_DragDropTarget]         = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
        colors[ImGuiCol_NavHighlight]           = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
        colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
        colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
    }

    void Renderer::RenderESP() {
        Settings& s = mgr.GetSettings();
        if (!s.espEnabled) return;

        ImDrawList* draw = ImGui::GetBackgroundDrawList();
        const auto& players = mgr.GetPlayers();

        // Colors
        ImU32 colEnemy = IM_COL32(235, 87, 87, 255);   // Soft Red
        ImU32 colTeam = IM_COL32(87, 235, 140, 255);   // Soft Green
        ImU32 colText = IM_COL32(255, 255, 255, 255);
        ImU32 colTextShadow = IM_COL32(0, 0, 0, 180);

        for (const auto& p : players) {
            // Apply VisOnly for ESP as well if desired, usually ESP shows all
            // But we respect Settings if there was one, but default ESP shows all.
            // AimbotVisOnly is for aimbot. ESP usually has its own IsVisible check if you want VisCheck ESP.
            // Current code just draws all.
            if (!p.visible && s.aimbotVisOnly && false) continue; // Optional: Only show visible in ESP too

            ImU32 color = p.enemy ? colEnemy : colTeam;
            if (p.visible && p.enemy) color = IM_COL32(255, 215, 0, 255); // Gold for visible enemy

            float boxHeight = fabsf(p.screenPos.Y - p.headScreen.Y);
            if (boxHeight < 1.0f) boxHeight = 1.0f;
            float boxWidth = boxHeight * 0.5f;

            float boxTop = p.headScreen.Y - (boxHeight * 0.1f);
            float boxBottom = p.screenPos.Y + (boxHeight * 0.05f);
            float boxLeft = p.screenPos.X - boxWidth / 2;
            float boxRight = p.screenPos.X + boxWidth / 2;

            // ESP Box
            if (s.espBox) {
                // Outer outline
                draw->AddRect(ImVec2(boxLeft-1, boxTop-1), ImVec2(boxRight+1, boxBottom+1), IM_COL32(0,0,0,200), 0, 0, 3.0f);
                // Inner outline
                draw->AddRect(ImVec2(boxLeft+1, boxTop+1), ImVec2(boxRight-1, boxBottom-1), IM_COL32(0,0,0,200), 0, 0, 3.0f);
                // Main box
                draw->AddRect(ImVec2(boxLeft, boxTop), ImVec2(boxRight, boxBottom), color, 0, 0, 1.5f);
            }

            // Health bar
            if (s.espHealth && p.maxHealth > 0) {
                float hp = p.health / p.maxHealth;
                float hpHeight = (boxBottom - boxTop) * hp;
                
                float barLeft = boxLeft - 6;
                float barRight = boxLeft - 2;
                
                // Background
                draw->AddRectFilled(ImVec2(barLeft, boxTop), ImVec2(barRight, boxBottom), IM_COL32(20, 20, 20, 180));
                
                // Color gradient based on HP
                ImU32 hpColor = IM_COL32((int)(255 * (1.1f - hp)), (int)(255 * hp), 0, 255);
                
                draw->AddRectFilled(ImVec2(barLeft + 1, boxBottom - hpHeight), ImVec2(barRight - 1, boxBottom), hpColor);
            }

            // Distance & Name
            if (s.espDistance || s.espName) {
                char txt[64];
                if (s.espDistance)
                    sprintf_s(txt, "dist: %.0fm", p.distance);
                else
                    txt[0] = 0; // Just visual separator if needed in future

                ImVec2 textSize = ImGui::CalcTextSize(txt);
                ImVec2 textPos = ImVec2(p.screenPos.X - textSize.x / 2, boxBottom + 2);

                draw->AddText(ImVec2(textPos.x + 1, textPos.y + 1), colTextShadow, txt);
                draw->AddText(textPos, colText, txt);
            }

            // Head Circle
            if (s.espHeadCircle) {
                draw->AddCircle(ImVec2(p.headScreen.X, p.headScreen.Y), 4.0f, IM_COL32(0,0,0,200), 12, 3.0f);
                draw->AddCircle(ImVec2(p.headScreen.X, p.headScreen.Y), 3.0f, color, 12, 1.0f);
            }

            // Snapline
            if (s.espSnapline) {
                ImGuiIO& io = ImGui::GetIO();
                draw->AddLine(ImVec2(io.DisplaySize.x / 2, io.DisplaySize.y), ImVec2(p.screenPos.X, boxBottom), color, 1.0f);
            }
        }

        // Aimbot FOV Circle
        if (s.aimbotEnabled) {
            ImGuiIO& io = ImGui::GetIO();
            float radius = tanf(s.aimbotFOV * 3.14159f / 180.0f / 2.0f) * (io.DisplaySize.x / 2);
            draw->AddCircle(ImVec2(io.DisplaySize.x / 2, io.DisplaySize.y / 2), radius, IM_COL32(255, 255, 255, 30), 64, 1.0f);
            draw->AddCircle(ImVec2(io.DisplaySize.x / 2, io.DisplaySize.y / 2), radius + 1.0f, IM_COL32(0, 0, 0, 30), 64, 1.0f);
        }
    }

    void Renderer::RenderMenu() {
        static bool styleInit = false;
        if (!styleInit) {
            SetupStyle();
            styleInit = true;
        }

        Settings& s = mgr.GetSettings();

        ImGui::SetNextWindowSize(ImVec2(650, 400), ImGuiCond_FirstUseEver);

        if (ImGui::Begin("GameHack Internal", nullptr, ImGuiWindowFlags_NoCollapse)) {
            
            // Layout: Sidebar (Tabs) + Content
            static int activeTab = 0;
            
            ImGui::Columns(2, "MainColumns", false);
            ImGui::SetColumnWidth(0, 150); // Width of sidebar
            
            // Sidebar
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 8));
            
            if (ImGui::Button("ESP 视觉", ImVec2(130, 35))) activeTab = 0;
            if (ImGui::Button("自瞄辅助", ImVec2(130, 35))) activeTab = 1;
            if (ImGui::Button("玩家列表", ImVec2(130, 35))) activeTab = 2;
            if (ImGui::Button("设置", ImVec2(130, 35))) activeTab = 3;
            
            ImGui::PopStyleVar();
            
            ImGui::Spacing();
            ImGui::TextDisabled("版本: Shipping");
            ImGui::TextDisabled("Ver: 1.0.2");
            
            ImGui::NextColumn();
            
            // Content Area
            ImGui::BeginChild("ContentArea", ImVec2(0, 0), true);
            
            if (activeTab == 0) // ESP
            {
                ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "[ 视觉功能 ]");
                ImGui::Separator();
                ImGui::Spacing();

                ImGui::Checkbox("启用 ESP", &s.espEnabled);
                if (s.espEnabled) {
                    ImGui::Indent();
                    ImGui::Checkbox("2D 方框", &s.espBox);
                    ImGui::Checkbox("血量条", &s.espHealth);
                    ImGui::Checkbox("距离显示", &s.espDistance);
                    ImGui::Checkbox("头部圆圈", &s.espHeadCircle);
                    ImGui::Checkbox("射线", &s.espSnapline);
                    
                    ImGui::Spacing();
                    ImGui::Text("最大距离");
                    ImGui::SliderFloat("##MaxDist", &s.espMaxDist, 50.0f, 2000.0f, "%.0fm");
                    ImGui::Unindent();
                }
            }
            else if (activeTab == 1) // Aimbot
            {
                ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "[ 瞄准辅助 ]");
                ImGui::Separator();
                ImGui::Spacing();

                ImGui::Checkbox("启用自瞄", &s.aimbotEnabled);
                ImGui::Checkbox("静默自瞄", &s.silentAimEnabled);
                ImGui::Checkbox("魔法子弹", &s.magicBulletEnabled);
                
                ImGui::Spacing();
                ImGui::Separator();
                
                ImGui::Text("配置");
                ImGui::Checkbox("可见性检查", &s.aimbotVisOnly);
                ImGui::Checkbox("子弹预判", &s.bulletPrediction);
                
                ImGui::Spacing();
                ImGui::Text("FOV 范围");
                ImGui::SliderFloat("##FOV", &s.aimbotFOV, 1.0f, 180.0f, "%.1f 度");
                
                ImGui::Text("平滑度");
                ImGui::SliderFloat("##Smooth", &s.aimbotSmooth, 1.0f, 50.0f, "%.1f");
                
                if (ImGui::Button("弹道追踪开关")) {
                    s.bulletTracerEnabled = !s.bulletTracerEnabled;
                }
                ImGui::SameLine();
                ImGui::Text("追踪: %s", s.bulletTracerEnabled ? "开启" : "关闭");
            }
            else if (activeTab == 2) // Players
            {
                const auto& players = mgr.GetPlayers();
                ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.6f, 1.0f), "[ 玩家列表 (%d) ]", (int)players.size());
                ImGui::Separator();
                
                ImGui::BeginChild("PlayerListChild");
                for (int i = 0; i < (int)players.size(); i++) {
                    const auto& p = players[i];
                    std::string nameStr(p.name.begin(), p.name.end());
                    ImGui::Text("%s", nameStr.c_str());
                    ImGui::SameLine(200);
                    ImGui::TextColored(p.enemy ? ImVec4(1,0.4f,0.4f,1) : ImVec4(0.4f,1,0.6f,1), 
                        "%.0fm", p.distance);
                    
                    ImGui::SameLine(300);
                    std::string btn = "踢出##" + std::to_string(i);
                    if (ImGui::Button(btn.c_str(), ImVec2(50, 20))) {
                        mgr.KickPlayer(i);
                    }
                }
                ImGui::EndChild();
            }
            else if (activeTab == 3) // Settings
            {
                ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.9f, 1.0f), "[ 设置 ]");
                ImGui::Separator();
                ImGui::Spacing();
                
                ImGui::Text("菜单键: INSERT");
                ImGui::Text("紧急键: END");
                
                ImGui::Spacing();
                if (ImGui::Button("保存配置")) {
                    // Save config logic
                }
                ImGui::SameLine();
                if (ImGui::Button("加载配置")) {
                    // Load config logic
                }
            }
            
            ImGui::EndChild();
        }
        ImGui::End();
    }

} // namespace GameHack
