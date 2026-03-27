#include "SaveManagerApp.Internal.hpp"

namespace manifold
{
    using detail::ThemeRuntime;

    void SaveManagerApp::Impl::ApplyBaseStyle()
    {
        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowRounding = 0.0f;
        style.ChildRounding = 0.0f;
        style.FrameRounding = 0.0f;
        style.PopupRounding = 0.0f;
        style.TabRounding = 0.0f;
        style.GrabRounding = 0.0f;
        style.ScrollbarRounding = 0.0f;
        style.WindowPadding = ImVec2(10.0f, 10.0f);
        style.FramePadding = ImVec2(8.0f, 6.0f);
        style.ItemSpacing = ImVec2(8.0f, 6.0f);
        style.ItemInnerSpacing = ImVec2(6.0f, 4.0f);
        style.CellPadding = ImVec2(8.0f, 6.0f);
        style.IndentSpacing = 16.0f;
        style.ScrollbarSize = 14.0f;
        style.GrabMinSize = 12.0f;
    }

    void SaveManagerApp::Impl::ApplyPaletteToImGui(const ThemeRuntime::Palette& p)
    {
        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowBorderSize = p.WindowBorderSize;
        style.ChildBorderSize = p.ChildBorderSize;
        style.FrameBorderSize = p.FrameBorderSize;
        style.PopupBorderSize = p.PopupBorderSize;
        style.TabBorderSize = p.TabBorderSize;

        ImVec4* c = style.Colors;
        c[ImGuiCol_Text] = p.Text;
        c[ImGuiCol_TextDisabled] = p.TextDim;
        c[ImGuiCol_WindowBg] = p.WindowBg;
        c[ImGuiCol_ChildBg] = ImVec4(p.WindowBg.x + 0.02f, p.WindowBg.y + 0.01f, p.WindowBg.z + 0.02f, 1.0f);
        c[ImGuiCol_PopupBg] = ImVec4(p.WindowBg.x + 0.01f, p.WindowBg.y + 0.01f, p.WindowBg.z + 0.01f, 0.98f);
        c[ImGuiCol_Border] = p.Border;
        c[ImGuiCol_BorderShadow] = ImVec4(0, 0, 0, 0);
        c[ImGuiCol_FrameBg] = p.Button;
        c[ImGuiCol_FrameBgHovered] = p.ButtonHovered;
        c[ImGuiCol_FrameBgActive] = p.ButtonActive;
        c[ImGuiCol_TitleBg] = p.TitleBg;
        c[ImGuiCol_TitleBgActive] = p.TitleBgActive;
        c[ImGuiCol_TitleBgCollapsed] = p.TitleBg;
        c[ImGuiCol_MenuBarBg] = p.HeaderTop;
        c[ImGuiCol_ScrollbarBg] = ImVec4(p.WindowBg.x * 0.8f, p.WindowBg.y * 0.8f, p.WindowBg.z * 0.8f, 1.0f);
        c[ImGuiCol_ScrollbarGrab] = p.AccentSoft;
        c[ImGuiCol_ScrollbarGrabHovered] = p.AccentStrong;
        c[ImGuiCol_ScrollbarGrabActive] = p.Accent;
        c[ImGuiCol_CheckMark] = p.Gold;
        c[ImGuiCol_SliderGrab] = p.Gold;
        c[ImGuiCol_SliderGrabActive] = ImVec4(1.00f, 0.94f, 0.48f, 1.0f);
        c[ImGuiCol_Button] = p.Button;
        c[ImGuiCol_ButtonHovered] = p.ButtonHovered;
        c[ImGuiCol_ButtonActive] = p.ButtonActive;
        c[ImGuiCol_Header] = p.AccentSoft;
        c[ImGuiCol_HeaderHovered] = p.AccentStrong;
        c[ImGuiCol_HeaderActive] = p.Accent;
        c[ImGuiCol_Separator] = p.Border;
        c[ImGuiCol_SeparatorHovered] = p.AccentHover;
        c[ImGuiCol_SeparatorActive] = p.Accent;
        c[ImGuiCol_ResizeGrip] = ImVec4(p.Border.x, p.Border.y, p.Border.z, 0.35f);
        c[ImGuiCol_ResizeGripHovered] = ImVec4(p.Border.x, p.Border.y, p.Border.z, 0.70f);
        c[ImGuiCol_ResizeGripActive] = p.Border;
        c[ImGuiCol_Tab] = p.Button;
        c[ImGuiCol_TabHovered] = p.ButtonHovered;
        c[ImGuiCol_TabActive] = p.AccentSoft;
        c[ImGuiCol_TabUnfocused] = p.Button;
        c[ImGuiCol_TabUnfocusedActive] = p.ButtonActive;
        c[ImGuiCol_TableHeaderBg] = p.TitleBg;
        c[ImGuiCol_TableBorderStrong] = p.Border;
        c[ImGuiCol_TableBorderLight] = ImVec4(p.Border.x, p.Border.y, p.Border.z, 0.35f);
        c[ImGuiCol_TableRowBg] = ImVec4(0, 0, 0, 0);
        c[ImGuiCol_TableRowBgAlt] = ImVec4(1, 1, 1, 0.03f);
        c[ImGuiCol_TextSelectedBg] = ImVec4(p.Accent.x, p.Accent.y, p.Accent.z, 0.25f);
        c[ImGuiCol_TabSelectedOverline] = p.Accent;
        c[ImGuiCol_NavHighlight] = p.Accent;
    }

    void SaveManagerApp::Impl::SetTheme(ThemeRuntime::AppTheme theme)
    {
        if (Theme.Current == theme) return;
        AddLog(std::string("Theme changed from ") + ToString(Theme.Current) + " to " + ToString(theme), Good());
        Theme.Current = theme;
        Theme.Applied = static_cast<ThemeRuntime::AppTheme>(-1);
    }

    ThemeRuntime::Palette SaveManagerApp::Impl::BuildTealPalette() const
    {
        ThemeRuntime::Palette palette{};
        palette.Accent = ImVec4(0.35f, 0.95f, 0.78f, 1.0f);
        palette.AccentHover = ImVec4(0.30f, 0.86f, 0.73f, 1.0f);
        palette.AccentSoft = ImVec4(0.10f, 0.45f, 0.53f, 1.0f);
        palette.AccentStrong = ImVec4(0.18f, 0.59f, 0.66f, 1.0f);
        palette.HeaderTop = ImVec4(0.04f, 0.16f, 0.19f, 0.98f);
        palette.HeaderBottom = ImVec4(0.05f, 0.10f, 0.18f, 0.98f);
        palette.FooterBg = ImVec4(0.03f, 0.18f, 0.21f, 0.95f);
        palette.WindowBg = ImVec4(0.08f, 0.09f, 0.16f, 0.98f);
        palette.Border = ImVec4(0.35f, 0.95f, 0.78f, 1.0f);
        palette.Text = ImVec4(0.91f, 0.95f, 0.78f, 1.0f);
        palette.TextDim = ImVec4(0.58f, 0.66f, 0.54f, 1.0f);
        palette.Gold = ImVec4(0.97f, 0.87f, 0.35f, 1.0f);
        palette.Button = ImVec4(0.14f, 0.17f, 0.28f, 1.0f);
        palette.ButtonHovered = ImVec4(0.21f, 0.24f, 0.42f, 1.0f);
        palette.ButtonActive = ImVec4(0.10f, 0.45f, 0.53f, 1.0f);
        palette.TitleBg = ImVec4(0.06f, 0.30f, 0.35f, 1.0f);
        palette.TitleBgActive = ImVec4(0.10f, 0.54f, 0.58f, 1.0f);
        palette.OverlayA = ImVec4(0.02f, 0.03f, 0.08f, 0.16f);
        palette.OverlayB = ImVec4(0.02f, 0.05f, 0.10f, 0.06f);
        palette.OverlayC = ImVec4(0.00f, 0.00f, 0.00f, 0.12f);
        palette.OverlayD = ImVec4(0.00f, 0.04f, 0.08f, 0.10f);
        palette.OverlayBorder = ImVec4(0.35f, 0.95f, 0.78f, 0.35f);
        return palette;
    }

    ThemeRuntime::Palette SaveManagerApp::Impl::BuildRedPalette() const
    {
        ThemeRuntime::Palette palette{};
        palette.Accent = ImVec4(0.90f, 0.22f, 0.28f, 1.0f);
        palette.AccentHover = ImVec4(0.96f, 0.34f, 0.38f, 1.0f);
        palette.AccentSoft = ImVec4(0.50f, 0.12f, 0.16f, 1.0f);
        palette.AccentStrong = ImVec4(0.66f, 0.17f, 0.21f, 1.0f);
        palette.HeaderTop = ImVec4(0.20f, 0.07f, 0.09f, 0.98f);
        palette.HeaderBottom = ImVec4(0.12f, 0.06f, 0.10f, 0.98f);
        palette.FooterBg = ImVec4(0.16f, 0.06f, 0.09f, 0.95f);
        palette.WindowBg = ImVec4(0.08f, 0.07f, 0.10f, 0.98f);
        palette.Border = ImVec4(0.90f, 0.22f, 0.28f, 1.0f);
        palette.Text = ImVec4(0.94f, 0.92f, 0.90f, 1.0f);
        palette.TextDim = ImVec4(0.63f, 0.56f, 0.56f, 1.0f);
        palette.Gold = ImVec4(0.97f, 0.87f, 0.35f, 1.0f);
        palette.Button = ImVec4(0.24f, 0.11f, 0.16f, 1.0f);
        palette.ButtonHovered = ImVec4(0.33f, 0.14f, 0.20f, 1.0f);
        palette.ButtonActive = ImVec4(0.50f, 0.12f, 0.16f, 1.0f);
        palette.TitleBg = ImVec4(0.30f, 0.07f, 0.10f, 1.0f);
        palette.TitleBgActive = ImVec4(0.56f, 0.11f, 0.16f, 1.0f);
        palette.OverlayA = ImVec4(0.08f, 0.02f, 0.03f, 0.16f);
        palette.OverlayB = ImVec4(0.10f, 0.02f, 0.03f, 0.06f);
        palette.OverlayC = ImVec4(0.00f, 0.00f, 0.00f, 0.12f);
        palette.OverlayD = ImVec4(0.08f, 0.01f, 0.02f, 0.10f);
        palette.OverlayBorder = ImVec4(0.90f, 0.22f, 0.28f, 0.35f);
        return palette;
    }

    ThemeRuntime::Palette SaveManagerApp::Impl::BuildGreenPalette() const
    {
        ThemeRuntime::Palette palette{};
        palette.Accent = ImVec4(0.38f, 0.96f, 0.52f, 1.0f);
        palette.AccentHover = ImVec4(0.50f, 1.00f, 0.62f, 1.0f);
        palette.AccentSoft = ImVec4(0.12f, 0.42f, 0.18f, 1.0f);
        palette.AccentStrong = ImVec4(0.18f, 0.62f, 0.24f, 1.0f);
        palette.HeaderTop = ImVec4(0.05f, 0.16f, 0.07f, 0.98f);
        palette.HeaderBottom = ImVec4(0.04f, 0.10f, 0.06f, 0.98f);
        palette.FooterBg = ImVec4(0.04f, 0.14f, 0.06f, 0.95f);
        palette.WindowBg = ImVec4(0.06f, 0.09f, 0.07f, 0.98f);
        palette.Border = ImVec4(0.38f, 0.96f, 0.52f, 1.0f);
        palette.Text = ImVec4(0.90f, 0.98f, 0.90f, 1.0f);
        palette.TextDim = ImVec4(0.56f, 0.68f, 0.56f, 1.0f);
        palette.Gold = ImVec4(0.97f, 0.87f, 0.35f, 1.0f);
        palette.Button = ImVec4(0.11f, 0.17f, 0.12f, 1.0f);
        palette.ButtonHovered = ImVec4(0.15f, 0.24f, 0.16f, 1.0f);
        palette.ButtonActive = ImVec4(0.12f, 0.42f, 0.18f, 1.0f);
        palette.TitleBg = ImVec4(0.08f, 0.28f, 0.11f, 1.0f);
        palette.TitleBgActive = ImVec4(0.14f, 0.48f, 0.18f, 1.0f);
        palette.OverlayA = ImVec4(0.02f, 0.05f, 0.02f, 0.16f);
        palette.OverlayB = ImVec4(0.02f, 0.07f, 0.03f, 0.06f);
        palette.OverlayC = ImVec4(0.00f, 0.00f, 0.00f, 0.12f);
        palette.OverlayD = ImVec4(0.01f, 0.06f, 0.02f, 0.10f);
        palette.OverlayBorder = ImVec4(0.38f, 0.96f, 0.52f, 0.35f);
        return palette;
    }

    void SaveManagerApp::Impl::ApplyThemeIfNeeded()
    {
        if (Theme.Applied == Theme.Current)
        {
            return;
        }

        ApplyBaseStyle();
        switch (Theme.Current)
        {
        case ThemeRuntime::AppTheme::RetroConsoleTeal:
            Theme.Active = BuildTealPalette();
            break;
        case ThemeRuntime::AppTheme::RetroConsoleRed:
            Theme.Active = BuildRedPalette();
            break;
        case ThemeRuntime::AppTheme::RetroConsoleGreen:
            Theme.Active = BuildGreenPalette();
            break;
        default:
            Theme.Active = BuildTealPalette();
            break;
        }

        ApplyPaletteToImGui(Theme.Active);
        Theme.Applied = Theme.Current;
    }

    bool SaveManagerApp::Impl::BeginRetroWindow(const char* name, bool* pOpen, ImGuiWindowFlags flags)
    {
        ImGui::PushStyleColor(ImGuiCol_TitleBg, Theme.Active.TitleBg);
        ImGui::PushStyleColor(ImGuiCol_TitleBgActive, Theme.Active.TitleBgActive);
        ImGui::PushStyleColor(ImGuiCol_WindowBg, Theme.Active.WindowBg);
        ImGui::PushStyleColor(ImGuiCol_Border, Theme.Active.Border);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 10.0f));
        return ImGui::Begin(name, pOpen, flags);
    }

    void SaveManagerApp::Impl::EndRetroWindow()
    {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        const ImVec2 p0 = ImGui::GetWindowPos();
        const ImVec2 p1 = ImVec2(p0.x + ImGui::GetWindowWidth(), p0.y + ImGui::GetWindowHeight());
        dl->AddRect(p0, p1, ImGui::GetColorU32(Theme.Active.Gold), 0.0f, 0, 1.0f);
        dl->AddRect(ImVec2(p0.x + 3.0f, p0.y + 3.0f), ImVec2(p1.x - 3.0f, p1.y - 3.0f), ImGui::GetColorU32(Theme.Active.AccentSoft), 0.0f, 0, 1.0f);
        ImGui::End();
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(4);
    }

    void SaveManagerApp::Impl::DrawRetroPanelLabel(const char*)
    {
    }

    bool SaveManagerApp::Impl::RetroButton(const char* label, const ImVec2& size)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, Theme.Active.Button);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Theme.Active.ButtonHovered);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, Theme.Active.ButtonActive);
        const bool pressed = ImGui::Button(label, size);
        ImGui::PopStyleColor(3);
        return pressed;
    }

    void SaveManagerApp::Impl::DrawRetroBottomBar()
    {
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        if (viewport == nullptr)
        {
            return;
        }

        ImDrawList* dl = ImGui::GetForegroundDrawList();
        if (dl == nullptr)
        {
            return;
        }

        const ImVec2 p0(viewport->WorkPos.x, viewport->WorkPos.y + viewport->WorkSize.y - FooterBarHeight);
        const ImVec2 p1(viewport->WorkPos.x + viewport->WorkSize.x, viewport->WorkPos.y + viewport->WorkSize.y);
        dl->AddRectFilled(p0, p1, ImGui::GetColorU32(Theme.Active.FooterBg));
        dl->AddLine(p0, ImVec2(p1.x, p0.y), ImGui::GetColorU32(Theme.Active.Accent), 2.0f);

        const GameDefinition* game = SaveManager.CurrentGame();
        const GameProfile* profile = SaveManager.CurrentProfile();

        std::string text = "READY | GAME: ";
        text += game ? (game->DisplayName.empty() ? game->Id : game->DisplayName) : "NONE";
        text += " | PROFILE: ";
        text += profile ? profile->Name : "NONE";
        text += " | BACKUPS: ";
        text += std::to_string(static_cast<int>(SaveManager.Backups().size()));

        dl->AddText(ImVec2(p0.x + 12.0f, p0.y + 7.0f), ImGui::GetColorU32(Theme.Active.Text), text.c_str());
    }

    void SaveManagerApp::Impl::DrawHeaderBar()
    {
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        if (viewport == nullptr)
        {
            return;
        }

        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, HeaderBarHeight));
        ImGui::SetNextWindowViewport(viewport->ID);

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoNavFocus;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));

        if (!ImGui::Begin("ManifoldHeaderBar", nullptr, flags))
        {
            ImGui::End();
            ImGui::PopStyleColor();
            ImGui::PopStyleVar(3);
            return;
        }

        ImDrawList* dl = ImGui::GetWindowDrawList();
        const ImVec2 winPos = ImGui::GetWindowPos();
        const ImVec2 winSize = ImGui::GetWindowSize();
        const ImVec2 winEnd(winPos.x + winSize.x, winPos.y + winSize.y);
        dl->AddRectFilledMultiColor(winPos, winEnd, ImGui::GetColorU32(Theme.Active.HeaderTop), ImGui::GetColorU32(Theme.Active.HeaderTop), ImGui::GetColorU32(Theme.Active.HeaderBottom), ImGui::GetColorU32(Theme.Active.HeaderBottom));
        dl->AddLine(ImVec2(winPos.x, winEnd.y - 1.0f), ImVec2(winEnd.x, winEnd.y - 1.0f), ImGui::GetColorU32(Theme.Active.Accent), 1.0f);

        const float padX = 12.0f;
        const float padY = 6.0f;
        const float buttonSize = HeaderBarHeight - padY * 2.0f;
        const float buttonSpacing = 6.0f;
        const bool isZoomed = MainWindow && IsZoomed(MainWindow) != FALSE;
        const char* toggleGlyph = isZoomed ? "o" : "[]";

        const float closeX = winSize.x - padX - buttonSize;
        const float maxX = closeX - buttonSpacing - buttonSize;
        const float minX = maxX - buttonSpacing - buttonSize;

        auto drawWindowButton = [&](const char* id, const char* glyph, ImVec4 base, ImVec4 hover, float x, auto&& onClick)
        {
            ImGui::SetCursorPos(ImVec2(x, padY));
            ImGui::PushStyleColor(ImGuiCol_Button, base);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hover);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, hover);
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
            if (ImGui::Button(id, ImVec2(buttonSize, buttonSize)))
            {
                onClick();
            }
            const ImVec2 b0 = ImGui::GetItemRectMin();
            const ImVec2 b1 = ImGui::GetItemRectMax();
            const ImVec2 ts = ImGui::CalcTextSize(glyph);
            dl->AddText(ImVec2(b0.x + ((b1.x - b0.x) - ts.x) * 0.5f, b0.y + ((b1.y - b0.y) - ts.y) * 0.5f - 1.0f), ImGui::GetColorU32(Theme.Active.Text), glyph);
            ImGui::PopStyleVar(2);
            ImGui::PopStyleColor(3);
        };

        drawWindowButton("##MinimizeWindow", "-", Theme.Active.Button, Theme.Active.ButtonHovered, minX, [&]() { if (MainWindow) ShowWindow(MainWindow, SW_MINIMIZE); });
        drawWindowButton("##ToggleMaximizeWindow", toggleGlyph, Theme.Active.Button, Theme.Active.ButtonHovered, maxX, [&]() { if (MainWindow) ShowWindow(MainWindow, isZoomed ? SW_RESTORE : SW_MAXIMIZE); });
        drawWindowButton("##CloseWindow", "x", ImVec4(0.27f, 0.10f, 0.12f, 1.0f), ImVec4(0.50f, 0.16f, 0.19f, 1.0f), closeX, [&]() { if (MainWindow) PostMessage(MainWindow, WM_CLOSE, 0, 0); });

        const char* title = ICON_FA_SAVE " MANIFOLD SAVE MANAGER";
        dl->AddText(ImVec2(winPos.x + padX, winPos.y + (HeaderBarHeight - ImGui::CalcTextSize(title).y) * 0.5f - 1.0f), ImGui::GetColorU32(Theme.Active.Gold), title);

        const char* statusText = "READY";
        const ImVec2 statusSize = ImGui::CalcTextSize(statusText);
        const float statusRight = minX - 16.0f;
        const ImVec2 statusPos(statusRight - statusSize.x, winPos.y + (HeaderBarHeight - statusSize.y) * 0.5f);
        dl->AddCircleFilled(ImVec2(statusPos.x - 10.0f, statusPos.y + statusSize.y * 0.5f), 3.5f, ImGui::GetColorU32(Theme.Active.Accent));
        dl->AddText(statusPos, ImGui::GetColorU32(Theme.Active.TextDim), statusText);

        const float dragStartX = padX;
        const float dragEndX = statusPos.x - 24.0f;
        if (dragEndX - dragStartX > 60.0f)
        {
            ImGui::SetCursorPos(ImVec2(dragStartX, 0.0f));
            ImGui::InvisibleButton("##HeaderDragZone", ImVec2(dragEndX - dragStartX, HeaderBarHeight));
            if (ImGui::IsItemHovered()) ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && MainWindow)
            {
                ShowWindow(MainWindow, isZoomed ? SW_RESTORE : SW_MAXIMIZE);
            }
            else if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && MainWindow)
            {
                ReleaseCapture();
                SendMessage(MainWindow, WM_NCLBUTTONDOWN, HTCAPTION, 0);
            }
        }

        ImGui::End();
        ImGui::PopStyleColor();
        ImGui::PopStyleVar(3);
    }

    void SaveManagerApp::Impl::DrawRetroScreenOverlay()
    {
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        if (viewport == nullptr)
        {
            return;
        }

        ImDrawList* dl = ImGui::GetForegroundDrawList();
        if (dl == nullptr)
        {
            return;
        }

        const ImVec2 p0 = viewport->Pos;
        const ImVec2 p1(viewport->Pos.x + viewport->Size.x, viewport->Pos.y + viewport->Size.y);
        const float t = static_cast<float>(ImGui::GetTime());
        const float borderPulse = 0.22f + 0.08f * sinf(t * 1.35f);
        const float scanOffset = fmodf(t * 18.0f, 4.0f);

        dl->AddRectFilledMultiColor(p0, p1,
            ImGui::GetColorU32(Theme.Active.OverlayA),
            ImGui::GetColorU32(Theme.Active.OverlayB),
            ImGui::GetColorU32(Theme.Active.OverlayC),
            ImGui::GetColorU32(Theme.Active.OverlayD));

        for (float y = p0.y - 4.0f + scanOffset; y < p1.y; y += 4.0f)
        {
            dl->AddLine(ImVec2(p0.x, y), ImVec2(p1.x, y), ImGui::GetColorU32(ImVec4(0.0f, 0.0f, 0.0f, 0.05f)), 1.0f);
        }

        ImVec4 border = Theme.Active.OverlayBorder;
        border.w = borderPulse;
        dl->AddRect(p0, p1, ImGui::GetColorU32(border), 0.0f, 0, 2.0f);
        DrawHeaderBar();
        DrawRetroBottomBar();
    }
}
