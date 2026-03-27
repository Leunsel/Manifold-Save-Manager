#include "SaveManagerApp.Internal.hpp"

namespace manifold
{
    using detail::ThemeRuntime;

    void SaveManagerApp::Impl::DrawRootWindow()
    {
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        if (viewport == nullptr)
        {
            return;
        }

        const ImVec2 rootPos(viewport->Pos.x, viewport->Pos.y + HeaderBarHeight);
        const ImVec2 rootSize(viewport->Size.x, viewport->Size.y - HeaderBarHeight - FooterBarHeight);

        ImGui::SetNextWindowPos(rootPos);
        ImGui::SetNextWindowSize(rootSize);
        ImGui::SetNextWindowViewport(viewport->ID);

        ImGuiWindowFlags hostFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus |
            ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_MenuBar;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("ManifoldRootDockspace", nullptr, hostFlags);
        ImGui::PopStyleVar(3);

        ApplyThemeIfNeeded();
        DrawMainMenuBar();

        ImGuiID dockspaceId = ImGui::GetID("ManifoldDockspace");
        ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_NoCloseButton);
        InitializeDefaultDockLayout(dockspaceId);
        ImGui::End();

        DrawRetroScreenOverlay();

        DrawNavigatorWindow();
        DrawProfilesWindow();
        DrawGameConfigWindow();
        DrawScopeRulesWindow();
        DrawBackupsWindow();
        DrawBackupDetailsWindow();
        DrawActivityWindow();

        DrawGameModal();
        DrawProfileModal();
        DrawRestoreModal();
        DrawDeleteGamePopup();
        DrawDeleteBackupPopup();
        DrawAboutPopup();
    }

    void SaveManagerApp::Impl::InitializeDefaultDockLayout(ImGuiID dockspaceId)
    {
        if (DockLayoutBuilt)
        {
            return;
        }

        DockLayoutBuilt = true;
        ImGui::DockBuilderRemoveNode(dockspaceId);
        ImGui::DockBuilderAddNode(dockspaceId, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(dockspaceId, ImGui::GetMainViewport()->WorkSize);

        ImGuiID dockMain = dockspaceId;
        ImGuiID dockLeft = 0;
        ImGuiID dockRight = 0;
        ImGuiID dockBottom = 0;
        ImGui::DockBuilderSplitNode(dockMain, ImGuiDir_Left, 0.23f, &dockLeft, &dockMain);
        ImGui::DockBuilderSplitNode(dockMain, ImGuiDir_Right, 0.33f, &dockRight, &dockMain);
        ImGui::DockBuilderSplitNode(dockMain, ImGuiDir_Down, 0.24f, &dockBottom, &dockMain);

        ImGui::DockBuilderDockWindow("Navigator", dockLeft);
        ImGui::DockBuilderDockWindow("Profiles", dockMain);
        ImGui::DockBuilderDockWindow("Game Config", dockMain);
        ImGui::DockBuilderDockWindow("Scope Rules", dockMain);
        ImGui::DockBuilderDockWindow("Backups", dockRight);
        ImGui::DockBuilderDockWindow("Backup Details", dockRight);
        ImGui::DockBuilderDockWindow("Activity Log", dockBottom);
        ImGui::DockBuilderFinish(dockspaceId);
    }

    void SaveManagerApp::Impl::DrawMainMenuBar()
    {
        if (!ImGui::BeginMenuBar())
        {
            return;
        }

        if (ImGui::BeginMenu(ICON_FA_FILE_ALT " File"))
        {
            if (ImGui::MenuItem(ICON_FA_SAVE " Save Config"))
            {
                SyncSelectionFromEditor();
                const bool ok = SaveManager.SaveConfig();
                AddLog(ok ? "Configuration saved." : "Configuration could not be saved.", ok ? Good() : Bad());
            }

            if (ImGui::MenuItem(ICON_FA_SYNC " Refresh"))
            {
                SyncSelectionFromEditor();
                SaveManager.Load();
                SyncEditorFromSelection();
                AddLog("Configuration and backups refreshed.", Accent());
            }

            ImGui::Separator();
            if (ImGui::MenuItem(ICON_FA_SIGN_OUT_ALT " Exit"))
            {
                PostMessage(MainWindow, WM_CLOSE, 0, 0);
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu(ICON_FA_GAMEPAD " Game"))
        {
            if (ImGui::MenuItem(ICON_FA_PLUS " Add Game")) OpenCreateGameModal();
            if (ImGui::MenuItem(ICON_FA_EDIT " Edit Game", nullptr, false, SaveManager.CurrentGame() != nullptr)) OpenEditGameModal();
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu(ICON_FA_USERS " Profile"))
        {
            if (ImGui::MenuItem(ICON_FA_PLUS " Add Profile", nullptr, false, SaveManager.CurrentGame() != nullptr)) OpenCreateProfileModal();
            if (ImGui::MenuItem(ICON_FA_EDIT " Edit Profile", nullptr, false, SaveManager.CurrentProfile() != nullptr)) OpenEditProfileModal();
            if (ImGui::MenuItem(ICON_FA_EXCHANGE_ALT " Instant Switch", nullptr, false, SaveManager.CurrentGame() != nullptr)) ExecuteInstantProfileSwitch();
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu(ICON_FA_ARCHIVE " Backup"))
        {
            if (ImGui::MenuItem(ICON_FA_SAVE " Create Backup", nullptr, false, SaveManager.CurrentProfile() != nullptr))
            {
                SyncSelectionFromEditor();
                const auto result = SaveManager.CreateBackupForCurrentProfile("Manual backup");
                AddLog(result.Message, result.Success ? Good() : Bad());
            }

            if (ImGui::MenuItem(ICON_FA_UNDO " Restore Selected", nullptr, false, SaveManager.CurrentBackup() != nullptr))
            {
                Popups.RestoreBackup = true;
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu(ICON_FA_EYE " View"))
        {
            ImGui::MenuItem(ICON_FA_GAMEPAD " Navigator", nullptr, &Panels.Navigator);
            ImGui::MenuItem(ICON_FA_USERS " Profiles", nullptr, &Panels.Profiles);
            ImGui::MenuItem(ICON_FA_COG " Game Config", nullptr, &Panels.GameConfig);
            ImGui::MenuItem(ICON_FA_FILTER " Scope Rules", nullptr, &Panels.ScopeRules);
            ImGui::MenuItem(ICON_FA_ARCHIVE " Backups", nullptr, &Panels.Backups);
            ImGui::MenuItem(ICON_FA_INFO_CIRCLE " Backup Details", nullptr, &Panels.BackupDetails);
            ImGui::MenuItem(ICON_FA_STREAM " Activity Log", nullptr, &Panels.ActivityLog);
            ImGui::Separator();

            if (ImGui::BeginMenu(ICON_FA_PALETTE " Theme"))
            {
                if (ImGui::MenuItem("Teal", nullptr, Theme.Current == ThemeRuntime::AppTheme::RetroConsoleTeal)) SetTheme(ThemeRuntime::AppTheme::RetroConsoleTeal);
                if (ImGui::MenuItem("Red", nullptr, Theme.Current == ThemeRuntime::AppTheme::RetroConsoleRed)) SetTheme(ThemeRuntime::AppTheme::RetroConsoleRed);
                if (ImGui::MenuItem("Green", nullptr, Theme.Current == ThemeRuntime::AppTheme::RetroConsoleGreen)) SetTheme(ThemeRuntime::AppTheme::RetroConsoleGreen);
                ImGui::EndMenu();
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu(ICON_FA_QUESTION_CIRCLE " Help"))
        {
            if (ImGui::MenuItem(ICON_FA_INFO_CIRCLE " About"))
            {
                Popups.About = true;
            }
            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }
}
