#include "Theme.h"

#include "../../ImGui/imgui.h"

namespace manifold
{
    void ApplyManifoldTheme()
    {
        ImGuiStyle* style = &ImGui::GetStyle();
        ImVec4* colors = style->Colors;

        style->Alpha = 1.0f;
        style->DisabledAlpha = 0.60f;
        style->WindowPadding = ImVec2(12.0f, 12.0f);
        style->FramePadding = ImVec2(10.0f, 6.0f);
        style->CellPadding = ImVec2(8.0f, 6.0f);
        style->ItemSpacing = ImVec2(10.0f, 8.0f);
        style->ItemInnerSpacing = ImVec2(6.0f, 6.0f);
        style->IndentSpacing = 20.0f;
        style->ScrollbarSize = 14.0f;
        style->GrabMinSize = 10.0f;
        style->WindowBorderSize = 1.0f;
        style->ChildBorderSize = 1.0f;
        style->PopupBorderSize = 1.0f;
        style->FrameBorderSize = 0.0f;
        style->TabBorderSize = 0.0f;
        style->WindowRounding = 12.0f;
        style->ChildRounding = 10.0f;
        style->FrameRounding = 8.0f;
        style->PopupRounding = 10.0f;
        style->ScrollbarRounding = 12.0f;
        style->GrabRounding = 8.0f;
        style->TabRounding = 8.0f;

        const ImVec4 text(0.90f, 0.95f, 0.98f, 1.00f);
        const ImVec4 textDim(0.50f, 0.60f, 0.66f, 1.00f);

        // Deutlich dunklere Hintergründe
        const ImVec4 bg0(0.015f, 0.020f, 0.028f, 1.00f); // WindowBg
        const ImVec4 bg1(0.028f, 0.036f, 0.048f, 1.00f); // Child/Popup
        const ImVec4 bg2(0.045f, 0.055f, 0.072f, 1.00f); // Frames/Buttons

        const ImVec4 border(0.12f, 0.18f, 0.22f, 1.00f);
        const ImVec4 borderSoft(0.09f, 0.14f, 0.18f, 1.00f);

        const ImVec4 accent(0.15f, 0.82f, 0.88f, 1.00f);
        const ImVec4 accentHover(0.26f, 0.90f, 0.96f, 1.00f);
        const ImVec4 accentActive(0.10f, 0.72f, 0.80f, 1.00f);

        colors[ImGuiCol_Text] = text;
        colors[ImGuiCol_TextDisabled] = textDim;

        colors[ImGuiCol_WindowBg] = bg0;
        colors[ImGuiCol_ChildBg] = bg1;
        colors[ImGuiCol_PopupBg] = bg1;

        colors[ImGuiCol_Border] = border;
        colors[ImGuiCol_BorderShadow] = ImVec4(0, 0, 0, 0);

        colors[ImGuiCol_FrameBg] = bg2;
        colors[ImGuiCol_FrameBgHovered] = ImVec4(0.060f, 0.075f, 0.095f, 1.00f);
        colors[ImGuiCol_FrameBgActive] = ImVec4(0.075f, 0.092f, 0.115f, 1.00f);

        colors[ImGuiCol_TitleBg] = bg0;
        colors[ImGuiCol_TitleBgActive] = bg1;
        colors[ImGuiCol_TitleBgCollapsed] = bg0;

        colors[ImGuiCol_MenuBarBg] = bg1;

        colors[ImGuiCol_ScrollbarBg] = bg0;
        colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.10f, 0.15f, 0.19f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.14f, 0.21f, 0.26f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.18f, 0.26f, 0.32f, 1.00f);

        colors[ImGuiCol_CheckMark] = accent;
        colors[ImGuiCol_SliderGrab] = accent;
        colors[ImGuiCol_SliderGrabActive] = accentHover;

        colors[ImGuiCol_Button] = bg2;
        colors[ImGuiCol_ButtonHovered] = ImVec4(0.060f, 0.075f, 0.095f, 1.00f);
        colors[ImGuiCol_ButtonActive] = ImVec4(0.075f, 0.092f, 0.115f, 1.00f);

        colors[ImGuiCol_Header] = ImVec4(accent.x, accent.y, accent.z, 0.18f);
        colors[ImGuiCol_HeaderHovered] = ImVec4(accentHover.x, accentHover.y, accentHover.z, 0.30f);
        colors[ImGuiCol_HeaderActive] = ImVec4(accentActive.x, accentActive.y, accentActive.z, 0.45f);

        colors[ImGuiCol_Separator] = borderSoft;
        colors[ImGuiCol_SeparatorHovered] = accent;
        colors[ImGuiCol_SeparatorActive] = accentHover;

        colors[ImGuiCol_ResizeGrip] = ImVec4(accent.x, accent.y, accent.z, 0.20f);
        colors[ImGuiCol_ResizeGripHovered] = ImVec4(accentHover.x, accentHover.y, accentHover.z, 0.50f);
        colors[ImGuiCol_ResizeGripActive] = ImVec4(accentHover.x, accentHover.y, accentHover.z, 0.75f);

        colors[ImGuiCol_Tab] = bg1;
        colors[ImGuiCol_TabHovered] = ImVec4(0.055f, 0.072f, 0.090f, 1.00f);
        colors[ImGuiCol_TabSelected] = ImVec4(0.070f, 0.088f, 0.108f, 1.00f);
        colors[ImGuiCol_TabSelectedOverline] = accent;
        colors[ImGuiCol_TabDimmed] = bg0;
        colors[ImGuiCol_TabDimmedSelected] = bg1;

        colors[ImGuiCol_TableHeaderBg] = ImVec4(0.032f, 0.042f, 0.055f, 1.00f);
        colors[ImGuiCol_TableBorderStrong] = border;
        colors[ImGuiCol_TableBorderLight] = borderSoft;
        colors[ImGuiCol_TableRowBg] = ImVec4(0, 0, 0, 0);
        colors[ImGuiCol_TableRowBgAlt] = ImVec4(1, 1, 1, 0.018f);

        colors[ImGuiCol_TextSelectedBg] = ImVec4(accent.x, accent.y, accent.z, 0.28f);
        colors[ImGuiCol_DragDropTarget] = accentHover;
        colors[ImGuiCol_NavCursor] = accent;
        colors[ImGuiCol_NavWindowingHighlight] = ImVec4(accent.x, accent.y, accent.z, 0.70f);
        colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.55f);
    }
}
