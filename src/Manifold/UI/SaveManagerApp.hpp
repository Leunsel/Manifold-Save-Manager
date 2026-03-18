#pragma once

#include <array>
#include <windows.h>

#include "../Core/SaveManager.hpp"

namespace manifold
{
    class SaveManagerApp
    {
    public:
        explicit SaveManagerApp(HWND hwnd);
        void Render();

    private:
        void DrawRootWindow();
        void InitializeDefaultDockLayout(ImGuiID dockspaceId);
        void DrawMainMenuBar();

        void DrawNavigatorWindow();
        void DrawProfilesWindow();
        void DrawProfileListPane(GameDefinition& game);
        void DrawProfileEditorPane();
        void DrawGameConfigWindow();
        void DrawScopeRulesWindow();
        void DrawBackupsWindow();
        void DrawBackupDetailsWindow();
        void DrawActivityWindow();
        void DrawScopeRuleTable(std::vector<ScopeRule>& rules, const char* id, float height);
        void DrawBackupActionButtons(const BackupEntry& backup);

        void DrawGameModal();
        void DrawProfileModal();
        void DrawRestoreModal();
        void DrawDeleteBackupPopup();
        void DrawAboutPopup();
        void DrawAboutCubeAnimation(float size);
        void DrawAboutCubeAnimation2(float size);

        void OpenCreateGameModal();
        void OpenEditGameModal();
        void OpenCreateProfileModal();
        void OpenEditProfileModal();
        void ExecuteInstantProfileSwitch();

        void LoadSelectedIntoEditor();
        void SaveEditorIntoSelected();
        void ClearGameEditor();
        void ClearProfileEditor();
        void AddLog(std::string message, ImVec4 color);

        static ImVec4 Good();
        static ImVec4 Warn();
        static ImVec4 Bad();
        static ImVec4 Accent();

    private:
        HWND m_MainWindow = nullptr;
        SaveManager m_SaveManager;
        bool m_ClearBeforeRestore = true;
        bool m_BackupBeforeOverwrite = true;
        bool m_DockLayoutBuilt = false;

        std::array<char, 128>  m_EditGameId{};
        std::array<char, 128>  m_EditGameName{};
        std::array<char, 512>  m_EditSavePath{};
        std::array<char, 128>  m_EditProcessName{};
        std::array<char, 2048> m_EditGameNotes{};

        std::array<char, 128>  m_EditProfileId{};
        std::array<char, 128>  m_EditProfileName{};
        std::array<char, 256>  m_EditProfileDescription{};
        std::array<char, 2048> m_EditProfileNotes{};
        std::array<char, 260>  m_EditScopeRule{};

        std::vector<LogEntry> m_Log;

        bool m_ShowNavigator = true;
        bool m_ShowProfiles = true;
        bool m_ShowGameConfig = true;
        bool m_ShowScopeRules = true;
        bool m_ShowBackups = true;
        bool m_ShowBackupDetails = true;
        bool m_ShowActivityLog = true;

        bool m_OpenProfileModal = false;
        bool m_OpenGameModal = false;
        bool m_OpenRestoreModal = false;
        bool m_OpenAboutPopup = false;

        int m_ProfileModalMode = 0;
        int m_GameModalMode = 0;

        std::array<char, 128>  m_ModalGameId{};
        std::array<char, 128>  m_ModalGameName{};
        std::array<char, 512>  m_ModalGameSavePath{};
        std::array<char, 128>  m_ModalGameProcessName{};
        std::array<char, 2048> m_ModalGameNotes{};
        bool m_ModalGameEnabled = true;
        int  m_ModalGameScopeMode = static_cast<int>(SaveScopeMode::FolderMode);

        std::array<char, 128>  m_ModalProfileId{};
        std::array<char, 128>  m_ModalProfileName{};
        std::array<char, 256>  m_ModalProfileDescription{};
        std::array<char, 2048> m_ModalProfileNotes{};
        bool m_ModalProfileFavorite = false;
        bool m_ModalProfileAutoBackup = true;
        int  m_ModalProfileBackupLimit = 20;
    };
}
