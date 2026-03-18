#include "Theme.hpp"

#include "../../ImGui/imgui.h"

namespace manifold
{
    void ApplyManifoldTheme()
    {
        ImGuiStyle* style = &ImGui::GetStyle();
        ImVec4* colors = style->Colors;

        style->Alpha = 1.0f;
        style->DisabledAlpha = 0.58f;

        style->WindowPadding = ImVec2(12.0f, 12.0f);
        style->FramePadding = ImVec2(10.0f, 7.0f);
        style->CellPadding = ImVec2(8.0f, 6.0f);
        style->ItemSpacing = ImVec2(10.0f, 8.0f);
        style->ItemInnerSpacing = ImVec2(6.0f, 6.0f);
        style->IndentSpacing = 20.0f;
        style->ScrollbarSize = 14.0f;
        style->GrabMinSize = 10.0f;

        style->WindowBorderSize = 1.0f;
        style->ChildBorderSize = 1.0f;
        style->PopupBorderSize = 1.0f;
        style->FrameBorderSize = 1.0f;
        style->TabBorderSize = 0.0f;

        style->WindowRounding = 12.0f;
        style->ChildRounding = 10.0f;
        style->FrameRounding = 8.0f;
        style->PopupRounding = 10.0f;
        style->ScrollbarRounding = 12.0f;
        style->GrabRounding = 8.0f;
        style->TabRounding = 8.0f;

        const ImVec4 text(0.90f, 0.94f, 0.97f, 1.00f);
        const ImVec4 textDim(0.56f, 0.63f, 0.69f, 1.00f);

        const ImVec4 bgWindow(0.020f, 0.024f, 0.031f, 1.00f);
        const ImVec4 bgPanel(0.030f, 0.037f, 0.047f, 1.00f);
        const ImVec4 bgFrame(0.043f, 0.052f, 0.066f, 1.00f);
        const ImVec4 bgElevated(0.055f, 0.067f, 0.084f, 1.00f);

        const ImVec4 border(0.110f, 0.145f, 0.180f, 1.00f);
        const ImVec4 borderSoft(0.080f, 0.110f, 0.140f, 1.00f);

        const ImVec4 accent(0.18f, 0.74f, 0.82f, 1.00f);
        const ImVec4 accentHover(0.28f, 0.82f, 0.90f, 1.00f);
        const ImVec4 accentActive(0.14f, 0.66f, 0.74f, 1.00f);

        colors[ImGuiCol_Text] = text;
        colors[ImGuiCol_TextDisabled] = textDim;

        colors[ImGuiCol_WindowBg] = bgWindow;
        colors[ImGuiCol_ChildBg] = bgPanel;
        colors[ImGuiCol_PopupBg] = bgPanel;

        colors[ImGuiCol_Border] = border;
        colors[ImGuiCol_BorderShadow] = ImVec4(0, 0, 0, 0);

        colors[ImGuiCol_FrameBg] = bgFrame;
        colors[ImGuiCol_FrameBgHovered] = bgElevated;
        colors[ImGuiCol_FrameBgActive] = ImVec4(0.070f, 0.084f, 0.102f, 1.00f);

        colors[ImGuiCol_TitleBg] = bgWindow;
        colors[ImGuiCol_TitleBgActive] = ImVec4(0.028f, 0.034f, 0.043f, 1.00f);
        colors[ImGuiCol_TitleBgCollapsed] = bgWindow;

        colors[ImGuiCol_MenuBarBg] = bgPanel;

        colors[ImGuiCol_ScrollbarBg] = bgWindow;
        colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.115f, 0.150f, 0.185f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.145f, 0.190f, 0.235f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.175f, 0.230f, 0.280f, 1.00f);

        colors[ImGuiCol_CheckMark] = accent;
        colors[ImGuiCol_SliderGrab] = accent;
        colors[ImGuiCol_SliderGrabActive] = accentHover;

        colors[ImGuiCol_Button] = bgFrame;
        colors[ImGuiCol_ButtonHovered] = bgElevated;
        colors[ImGuiCol_ButtonActive] = ImVec4(0.070f, 0.084f, 0.102f, 1.00f);

        colors[ImGuiCol_Header] = ImVec4(accent.x, accent.y, accent.z, 0.16f);
        colors[ImGuiCol_HeaderHovered] = ImVec4(accentHover.x, accentHover.y, accentHover.z, 0.26f);
        colors[ImGuiCol_HeaderActive] = ImVec4(accentActive.x, accentActive.y, accentActive.z, 0.40f);

        colors[ImGuiCol_Separator] = borderSoft;
        colors[ImGuiCol_SeparatorHovered] = ImVec4(accent.x, accent.y, accent.z, 0.65f);
        colors[ImGuiCol_SeparatorActive] = accentHover;

        colors[ImGuiCol_ResizeGrip] = ImVec4(accent.x, accent.y, accent.z, 0.16f);
        colors[ImGuiCol_ResizeGripHovered] = ImVec4(accentHover.x, accentHover.y, accentHover.z, 0.42f);
        colors[ImGuiCol_ResizeGripActive] = ImVec4(accentHover.x, accentHover.y, accentHover.z, 0.68f);

        colors[ImGuiCol_Tab] = bgPanel;
        colors[ImGuiCol_TabHovered] = ImVec4(0.060f, 0.076f, 0.094f, 1.00f);
        colors[ImGuiCol_TabSelected] = ImVec4(0.072f, 0.090f, 0.110f, 1.00f);
        colors[ImGuiCol_TabSelectedOverline] = accent;
        colors[ImGuiCol_TabDimmed] = bgWindow;
        colors[ImGuiCol_TabDimmedSelected] = bgPanel;

        colors[ImGuiCol_TableHeaderBg] = ImVec4(0.037f, 0.046f, 0.059f, 1.00f);
        colors[ImGuiCol_TableBorderStrong] = border;
        colors[ImGuiCol_TableBorderLight] = borderSoft;
        colors[ImGuiCol_TableRowBg] = ImVec4(0, 0, 0, 0);
        colors[ImGuiCol_TableRowBgAlt] = ImVec4(1, 1, 1, 0.024f);

        colors[ImGuiCol_TextSelectedBg] = ImVec4(accent.x, accent.y, accent.z, 0.22f);
        colors[ImGuiCol_DragDropTarget] = accentHover;
        colors[ImGuiCol_NavCursor] = accent;
        colors[ImGuiCol_NavWindowingHighlight] = ImVec4(accent.x, accent.y, accent.z, 0.70f);
        colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.62f);
    }
}
