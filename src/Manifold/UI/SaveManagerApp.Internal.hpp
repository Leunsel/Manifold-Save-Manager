#pragma once

#include <windows.h>
#include <shellapi.h>

#include <algorithm>
#include <array>
#include <ctime>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "../../ImGui/imgui_internal.h"
#include "../../Imgui/imgui.h"
#include "../Core/Math.hpp"
#include "../Core/SaveManager.hpp"
#include "../Core/Utils.hpp"
#include "../Misc/Version.hpp"
#include "../Font/Font.hpp"
#include "../Font/IconsFontAwesome5.hpp"
#include "SaveManagerApp.hpp"

namespace manifold
{
    namespace detail
    {
        enum class EditMode
        {
            Create,
            Edit
        };

        struct PanelVisibility
        {
            bool Navigator = true;
            bool Profiles = true;
            bool GameConfig = true;
            bool ScopeRules = true;
            bool Backups = true;
            bool BackupDetails = true;
            bool ActivityLog = true;
        };

        struct PopupState
        {
            bool About = false;
            bool RestoreBackup = false;
            bool DeleteGame = false;
            bool DeleteBackup = false;
        };

        struct BufferText
        {
            template <size_t N>
            static void Clear(std::array<char, N>& buffer)
            {
                buffer.fill('\0');
            }

            template <size_t N>
            static void Assign(std::array<char, N>& buffer, const std::string& value)
            {
                WriteToBuffer(buffer, value);
            }
        };

        struct GameEditorState
        {
            std::array<char, 128> Id{};
            std::array<char, 128> Name{};
            std::array<char, 512> SavePath{};
            std::array<char, 128> ProcessName{};
            std::array<char, 2048> Notes{};

            void Clear();
            void LoadFrom(const GameDefinition& game);
            void ApplyTo(GameDefinition& game) const;
        };

        struct ProfileEditorState
        {
            std::array<char, 128> Id{};
            std::array<char, 128> Name{};
            std::array<char, 256> Description{};
            std::array<char, 2048> Notes{};
            std::array<char, 260> ScopeRule{};

            void Clear();
            void LoadFrom(const GameProfile& profile);
            void ApplyTo(GameProfile& profile) const;
        };

        struct GameModalState
        {
            std::array<char, 128> Id{};
            std::array<char, 128> Name{};
            std::array<char, 512> SavePath{};
            std::array<char, 128> ProcessName{};
            std::array<char, 2048> Notes{};
            bool Enabled = true;
            int ScopeMode = 0;
            EditMode Mode = EditMode::Create;
            bool OpenRequested = false;

            void LoadCreateDefaults();
            void LoadFrom(const GameDefinition& game);
            void ApplyTo(GameDefinition& game) const;
            const char* PopupTitle() const;
        };

        struct ProfileModalState
        {
            std::array<char, 128> Id{};
            std::array<char, 128> Name{};
            std::array<char, 256> Description{};
            std::array<char, 2048> Notes{};
            bool Favorite = false;
            bool AutoBackup = true;
            int BackupLimit = 20;
            EditMode Mode = EditMode::Create;
            bool OpenRequested = false;

            void LoadCreateDefaults();
            void LoadFrom(const GameProfile& profile);
            void ApplyTo(GameProfile& profile) const;
            const char* PopupTitle() const;
        };

        struct ThemeRuntime
        {
            enum class AppTheme
            {
                RetroConsoleTeal = 0,
                RetroConsoleRed,
                RetroConsoleGreen
            };

            struct Palette
            {
                ImVec4 Accent{};
                ImVec4 AccentHover{};
                ImVec4 AccentSoft{};
                ImVec4 AccentStrong{};
                ImVec4 HeaderTop{};
                ImVec4 HeaderBottom{};
                ImVec4 FooterBg{};
                ImVec4 WindowBg{};
                ImVec4 Border{};
                ImVec4 Text{};
                ImVec4 TextDim{};
                ImVec4 Gold{};
                ImVec4 Button{};
                ImVec4 ButtonHovered{};
                ImVec4 ButtonActive{};
                ImVec4 TitleBg{};
                ImVec4 TitleBgActive{};
                ImVec4 OverlayA{};
                ImVec4 OverlayB{};
                ImVec4 OverlayC{};
                ImVec4 OverlayD{};
                ImVec4 OverlayBorder{};
                float WindowBorderSize = 1.0f;
                float ChildBorderSize = 1.0f;
                float FrameBorderSize = 1.0f;
                float PopupBorderSize = 1.0f;
                float TabBorderSize = 1.0f;
            };

            AppTheme Current = AppTheme::RetroConsoleTeal;
            AppTheme Applied = static_cast<AppTheme>(-1);
            Palette Active{};
        };

        static const char* ToString(ThemeRuntime::AppTheme theme)
        {
            switch (theme)
            {
            case ThemeRuntime::AppTheme::RetroConsoleTeal:   return "Teal";
            case ThemeRuntime::AppTheme::RetroConsoleRed:    return "Blue";
            case ThemeRuntime::AppTheme::RetroConsoleGreen:  return "Green";
            default:                             return "Unknown";
            }
        }

        struct UiFilters
        {
            char Game[128]{};
            char Backup[128]{};
        };
    }

    struct SaveManagerApp::Impl
    {
        explicit Impl(HWND hwnd);

        static constexpr float HeaderBarHeight = 40.0f;
        static constexpr float FooterBarHeight = 28.0f;

        HWND MainWindow = nullptr;
        SaveManager SaveManager;

        detail::ThemeRuntime Theme;
        detail::PanelVisibility Panels;
        detail::PopupState Popups;
        detail::UiFilters Filters;
        detail::GameEditorState GameEditor;
        detail::ProfileEditorState ProfileEditor;
        detail::GameModalState GameModal;
        detail::ProfileModalState ProfileModal;

        std::vector<LogEntry> Log;

        bool ClearBeforeRestore = true;
        bool BackupBeforeOverwrite = true;
        bool DockLayoutBuilt = false;

        void Render();

        void Initialize();
        void LoadFonts();
        void SyncEditorFromSelection();
        void SyncSelectionFromEditor();

        void DrawRootWindow();
        void DrawMainMenuBar();
        void DrawHeaderBar();
        void DrawRetroBottomBar();
        void DrawRetroScreenOverlay();
        void InitializeDefaultDockLayout(ImGuiID dockspaceId);

        void DrawNavigatorWindow();
        void DrawProfilesWindow();
        void DrawProfileListPane(GameDefinition& game);
        void DrawProfileEditorPane();
        void DrawGameConfigWindow();
        void DrawScopeRulesWindow();
        void DrawScopeRuleTable(std::vector<ScopeRule>& rules, const char* id, float height);
        void DrawBackupsWindow();
        void DrawBackupDetailsWindow();
        void DrawBackupActionButtons(const BackupEntry& backup);
        void DrawActivityWindow();

        void DrawGameModal();
        void DrawProfileModal();
        void DrawRestoreModal();
        void DrawDeleteGamePopup();
        void DrawDeleteBackupPopup();
        void DrawAboutPopup();
        void DrawAboutCubeAnimation(float cubeSize);
        void DrawAboutCubeAnimation2(float cubeSize);

        void OpenCreateGameModal();
        void OpenEditGameModal();
        void OpenCreateProfileModal();
        void OpenEditProfileModal();
        void ExecuteInstantProfileSwitch();

        void ApplyBaseStyle();
        void ApplyThemeIfNeeded();
        void ApplyPaletteToImGui(const detail::ThemeRuntime::Palette& palette);
        void SetTheme(detail::ThemeRuntime::AppTheme theme);
        detail::ThemeRuntime::Palette BuildTealPalette() const;
        detail::ThemeRuntime::Palette BuildRedPalette() const;
        detail::ThemeRuntime::Palette BuildGreenPalette() const;

        bool BeginRetroWindow(const char* name, bool* pOpen = nullptr, ImGuiWindowFlags flags = 0);
        void EndRetroWindow();
        void DrawRetroPanelLabel(const char* text);
        bool RetroButton(const char* label, const ImVec2& size = ImVec2(0.0f, 0.0f));

        void AddLog(std::string message, ImVec4 color);
        static ImVec4 Good();
        static ImVec4 Warn();
        static ImVec4 Bad();
        static ImVec4 Accent();
    };
}
