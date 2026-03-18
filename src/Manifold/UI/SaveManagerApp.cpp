#include "SaveManagerApp.h"

#include "../Core/Math.h"
#include "../Core/SaveManager.h"

#include <algorithm>

#include "Theme.h"
#include "../Core/Utils.h"
#include "../../ImGui/imgui.h"
#include "../../ImGui/imgui_internal.h"

namespace manifold
{
    SaveManagerApp::SaveManagerApp(HWND hwnd) : m_MainWindow(hwnd)
    {
        m_SaveManager.Load();
        LoadSelectedIntoEditor();
        AddLog("Manifold Save Manager initialized.", Accent());
    }

    void SaveManagerApp::Render()
    {
        ApplyManifoldTheme();
        DrawRootWindow();
    }

    void SaveManagerApp::DrawRootWindow()
    {
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);

        ImGuiWindowFlags hostFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground |
            ImGuiWindowFlags_MenuBar;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("ManifoldRootDockspace", nullptr, hostFlags);
        ImGui::PopStyleVar(3);

        DrawMainMenuBar();
        ImGuiID dockspaceId = ImGui::GetID("ManifoldDockspace");
        ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_NoCloseButton);
        InitializeDefaultDockLayout(dockspaceId);
        ImGui::End();

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
        DrawAboutPopup();
    }

    void SaveManagerApp::InitializeDefaultDockLayout(ImGuiID dockspaceId)
    {
        if (m_DockLayoutBuilt) return;
        m_DockLayoutBuilt = true;

        ImGui::DockBuilderRemoveNode(dockspaceId);
        ImGui::DockBuilderAddNode(dockspaceId, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(dockspaceId, ImGui::GetMainViewport()->WorkSize);

        ImGuiID dockMain = dockspaceId;
        ImGuiID dockLeft = 0, dockRight = 0, dockBottom = 0, dockCenterRight = 0;
        ImGui::DockBuilderSplitNode(dockMain, ImGuiDir_Left, 0.23f, &dockLeft, &dockMain);
        ImGui::DockBuilderSplitNode(dockMain, ImGuiDir_Right, 0.33f, &dockRight, &dockMain);
        ImGui::DockBuilderSplitNode(dockMain, ImGuiDir_Down, 0.24f, &dockBottom, &dockMain);
        ImGui::DockBuilderSplitNode(dockRight, ImGuiDir_Down, 0.48f, &dockCenterRight, &dockRight);

        ImGui::DockBuilderDockWindow("Navigator", dockLeft);
        ImGui::DockBuilderDockWindow("Profiles", dockMain);
        ImGui::DockBuilderDockWindow("Game Config", dockMain);
        ImGui::DockBuilderDockWindow("Scope Rules", dockMain);
        ImGui::DockBuilderDockWindow("Backups", dockRight);
        // ImGui::DockBuilderDockWindow("Backup Details", dockCenterRight);
        ImGui::DockBuilderDockWindow("Backup Details", dockRight);
        ImGui::DockBuilderDockWindow("Activity Log", dockBottom);
        ImGui::DockBuilderFinish(dockspaceId);
    }

    void SaveManagerApp::DrawMainMenuBar()
    {
        if (!ImGui::BeginMenuBar()) return;

        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Save Config"))
            {
                SaveEditorIntoSelected();
                const bool ok = m_SaveManager.SaveConfig();
                AddLog(ok ? "Configuration saved." : "Configuration could not be saved.", ok ? Good() : Bad());
            }
            if (ImGui::MenuItem("Refresh"))
            {
                m_SaveManager.RefreshBackups();
                AddLog("Backup list refreshed.", Accent());
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit")) PostMessage(m_MainWindow, WM_CLOSE, 0, 0);
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Game"))
        {
            if (ImGui::MenuItem("Add Game")) OpenCreateGameModal();
            if (ImGui::MenuItem("Edit Game", nullptr, false, m_SaveManager.CurrentGame() != nullptr)) OpenEditGameModal();
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Profile"))
        {
            if (ImGui::MenuItem("Add Profile", nullptr, false, m_SaveManager.CurrentGame() != nullptr)) OpenCreateProfileModal();
            if (ImGui::MenuItem("Edit Profile", nullptr, false, m_SaveManager.CurrentProfile() != nullptr)) OpenEditProfileModal();
            if (ImGui::MenuItem("Instant Switch", nullptr, false, m_SaveManager.CurrentGame() != nullptr)) ExecuteInstantProfileSwitch();
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Backup"))
        {
            if (ImGui::MenuItem("Create Backup", nullptr, false, m_SaveManager.CurrentProfile() != nullptr))
            {
                SaveEditorIntoSelected();
                const auto result = m_SaveManager.CreateBackupForCurrentProfile("Manual backup");
                AddLog(result.Message, result.Success ? Good() : Bad());
            }
            if (ImGui::MenuItem("Restore Selected", nullptr, false, m_SaveManager.CurrentBackup() != nullptr))
                m_OpenRestoreModal = true;
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View"))
        {
            ImGui::MenuItem("Navigator", nullptr, &m_ShowNavigator);
            ImGui::MenuItem("Profiles", nullptr, &m_ShowProfiles);
            ImGui::MenuItem("Game Config", nullptr, &m_ShowGameConfig);
            ImGui::MenuItem("Scope Rules", nullptr, &m_ShowScopeRules);
            ImGui::MenuItem("Backups", nullptr, &m_ShowBackups);
            ImGui::MenuItem("Backup Details", nullptr, &m_ShowBackupDetails);
            ImGui::MenuItem("Activity Log", nullptr, &m_ShowActivityLog);
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Help"))
        {
            if (ImGui::MenuItem("About"))
                m_OpenAboutPopup = true;

            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

    void SaveManagerApp::DrawNavigatorWindow()
    {
        if (!m_ShowNavigator) return;
        ImGui::Begin("Navigator", &m_ShowNavigator);

        if (ImGui::Button("New Game", ImVec2(-1.0f, 0.0f))) OpenCreateGameModal();
        ImGui::Separator();
        ImGui::TextUnformatted("Games");

        if (ImGui::BeginListBox("##GameList", ImVec2(-1.0f, 220.0f)))
        {
            const auto& games = m_SaveManager.Games();
            for (int i = 0; i < static_cast<int>(games.size()); ++i)
            {
                const bool selected = i == m_SaveManager.SelectedGame();
                const std::string label = games[i].DisplayName.empty() ? games[i].Id : games[i].DisplayName;
                if (ImGui::Selectable(label.c_str(), selected))
                {
                    SaveEditorIntoSelected();
                    m_SaveManager.SetSelectedGame(i);
                    LoadSelectedIntoEditor();
                }
            }
            ImGui::EndListBox();
        }

        const GameDefinition* game = m_SaveManager.CurrentGame();
        const GameProfile* profile = m_SaveManager.CurrentProfile();
        ImGui::Separator();
        ImGui::TextDisabled("Current Selection");
        if (game)
        {
            ImGui::Text("Game: %s", game->DisplayName.c_str());
            ImGui::Text("Scope: %s", ToString(game->ScopeMode));
        }
        if (profile) ImGui::Text("Profile: %s", profile->Name.c_str());

        ImGui::Separator();
        if (ImGui::Button("Create Backup", ImVec2(-1.0f, 0.0f)))
        {
            SaveEditorIntoSelected();
            auto result = m_SaveManager.CreateBackupForCurrentProfile("Manual backup");
            AddLog(result.Message, result.Success ? Good() : Bad());
        }
        if (ImGui::Button("Switch Profile", ImVec2(-1.0f, 0.0f))) ExecuteInstantProfileSwitch();

        ImGui::End();
    }

    void SaveManagerApp::DrawProfilesWindow()
    {
        if (!m_ShowProfiles) return;
        ImGui::Begin("Profiles", &m_ShowProfiles);

        GameDefinition* game = m_SaveManager.CurrentGame();
        if (!game)
        {
            ImGui::TextDisabled("No game selected.");
            ImGui::End();
            return;
        }

        if (ImGui::BeginTable("ProfilesLayout", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchProp))
        {
            ImGui::TableSetupColumn("List", ImGuiTableColumnFlags_WidthStretch, 0.90f);
            ImGui::TableSetupColumn("Editor", ImGuiTableColumnFlags_WidthStretch, 1.40f);
            ImGui::TableNextColumn();
            DrawProfileListPane(*game);
            ImGui::TableNextColumn();
            DrawProfileEditorPane();
            ImGui::EndTable();
        }
        ImGui::End();
    }

    void SaveManagerApp::DrawProfileListPane(GameDefinition& game)
    {
        if (ImGui::Button("New Profile", ImVec2(-1.0f, 0.0f))) OpenCreateProfileModal();
        if (ImGui::Button("Edit Selected Profile", ImVec2(-1.0f, 0.0f))) OpenEditProfileModal();
        if (ImGui::Button("Remove Selected Profile", ImVec2(-1.0f, 0.0f)))
        {
            SaveEditorIntoSelected();
            m_SaveManager.RemoveCurrentProfile();
            LoadSelectedIntoEditor();
            m_SaveManager.SaveConfig();
            AddLog("Profile removed.", Warn());
        }

        ImGui::Separator();
        if (ImGui::BeginListBox("##ProfileListPane", ImVec2(-1.0f, -1.0f)))
        {
            for (int i = 0; i < static_cast<int>(game.Profiles.size()); ++i)
            {
                const GameProfile& profile = game.Profiles[i];
                const bool selected = i == m_SaveManager.SelectedProfile();
                std::string label = profile.Favorite ? ("★ " + profile.Name) : profile.Name;
                label += "##" + profile.Id;
                if (ImGui::Selectable(label.c_str(), selected))
                {
                    SaveEditorIntoSelected();
                    m_SaveManager.SetSelectedProfile(i);
                    LoadSelectedIntoEditor();
                }
            }
            ImGui::EndListBox();
        }
    }

    void SaveManagerApp::DrawProfileEditorPane()
    {
        GameProfile* profile = m_SaveManager.CurrentProfile();
        if (!profile)
        {
            ImGui::TextDisabled("No profile selected.");
            return;
        }

        if (ImGui::Button("Edit Profile", ImVec2(-1.0f, 0.0f))) OpenEditProfileModal();
        ImGui::Separator();

        if (ImGui::BeginTabBar("ProfileEditorTabs"))
        {
            if (ImGui::BeginTabItem("General"))
            {
                ImGui::InputText("Profile Id", m_EditProfileId.data(), m_EditProfileId.size());
                ImGui::InputText("Profile Name", m_EditProfileName.data(), m_EditProfileName.size());
                ImGui::InputText("Description", m_EditProfileDescription.data(), m_EditProfileDescription.size());
                ImGui::Checkbox("Favorite", &profile->Favorite);
                ImGui::Checkbox("Auto Backup on Switch", &profile->AutoBackupOnSwitch);
                ImGui::SliderInt("Backup Limit", &profile->BackupLimit, 1, 100);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Notes"))
            {
                ImGui::InputTextMultiline("##ProfileNotes", m_EditProfileNotes.data(), m_EditProfileNotes.size(), ImVec2(-1.0f, -1.0f));
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }

        if (ImGui::Button("Apply Profile Changes", ImVec2(-1.0f, 0.0f)))
        {
            SaveEditorIntoSelected();
            const bool ok = m_SaveManager.SaveConfig();
            AddLog(ok ? "Profile updated." : "Profile could not be saved.", ok ? Good() : Bad());
        }
    }

    void SaveManagerApp::DrawGameConfigWindow()
    {
        if (!m_ShowGameConfig) return;
        ImGui::Begin("Game Config", &m_ShowGameConfig);

        GameDefinition* game = m_SaveManager.CurrentGame();
        if (!game)
        {
            ImGui::TextDisabled("No game selected.");
            if (ImGui::Button("Create Game", ImVec2(-1.0f, 0.0f))) OpenCreateGameModal();
            ImGui::End();
            return;
        }

        ImGui::Checkbox("Enabled", &game->Enabled);
        ImGui::InputText("Game Id", m_EditGameId.data(), m_EditGameId.size());
        ImGui::InputText("Display Name", m_EditGameName.data(), m_EditGameName.size());

        ImGui::TextUnformatted("Save Path");
        ImGui::SetNextItemWidth(-110.0f);
        ImGui::InputText("##SavePath", m_EditSavePath.data(), m_EditSavePath.size());
        ImGui::SameLine();
        if (ImGui::Button("Browse", ImVec2(100.0f, 0.0f)))
            if (auto folder = PickFolderDialog(m_MainWindow, L"Select Save Folder")) WriteToBuffer(m_EditSavePath, *folder);

        ImGui::InputText("Process Name", m_EditProcessName.data(), m_EditProcessName.size());
        ImGui::InputTextMultiline("Game Notes", m_EditGameNotes.data(), m_EditGameNotes.size(), ImVec2(-1.0f, 180.0f));

        const char* scopeNames[] = { "File Whitelist", "Folder Mode", "Hybrid Mode" };
        int scopeIndex = static_cast<int>(game->ScopeMode);
        if (ImGui::Combo("Save Scope", &scopeIndex, scopeNames, IM_ARRAYSIZE(scopeNames)))
            game->ScopeMode = static_cast<SaveScopeMode>(scopeIndex);

        if (ImGui::Button("Open Save Folder", ImVec2(-1.0f, 0.0f)))
        {
            const bool ok = OpenInExplorer(m_EditSavePath.data());
            AddLog(ok ? "Save path opened." : "Save path could not be opened.", ok ? Accent() : Bad());
        }
        if (ImGui::Button("Apply Game Changes", ImVec2(-1.0f, 0.0f)))
        {
            SaveEditorIntoSelected();
            const bool ok = m_SaveManager.SaveConfig();
            AddLog(ok ? "Game configuration updated." : "Game configuration could not be saved.", ok ? Good() : Bad());
        }

        ImGui::Separator();
        ImGui::BulletText("Save Path: %s", DirectoryExists(m_EditSavePath.data()) ? "OK" : "Missing");
        ImGui::End();
    }

    void SaveManagerApp::DrawScopeRulesWindow()
    {
        if (!m_ShowScopeRules) return;
        ImGui::Begin("Scope Rules", &m_ShowScopeRules);

        GameDefinition* game = m_SaveManager.CurrentGame();
        if (!game)
        {
            ImGui::TextDisabled("No game selected.");
            ImGui::End();
            return;
        }

        ImGui::TextDisabled("Current scope mode: %s", ToString(game->ScopeMode));
        ImGui::Separator();
        ImGui::InputText("New Rule", m_EditScopeRule.data(), m_EditScopeRule.size());
        if (ImGui::Button("Add File Rule", ImVec2(-1.0f, 0.0f)))
        {
            const std::string value = Trim(m_EditScopeRule.data());
            if (!value.empty())
            {
                game->Whitelist.push_back({ value, true });
                WriteToBuffer(m_EditScopeRule, "");
                m_SaveManager.SaveConfig();
                AddLog("File whitelist rule added.", Accent());
            }
        }
        if (ImGui::Button("Add Folder Rule", ImVec2(-1.0f, 0.0f)))
        {
            const std::string value = Trim(m_EditScopeRule.data());
            if (!value.empty())
            {
                game->FolderRules.push_back({ value, true });
                WriteToBuffer(m_EditScopeRule, "");
                m_SaveManager.SaveConfig();
                AddLog("Folder rule added.", Accent());
            }
        }

        ImGui::Separator();
        ImGui::TextUnformatted("File Whitelist");
        DrawScopeRuleTable(game->Whitelist, "WhitelistTable", 160.0f);
        ImGui::Spacing();
        ImGui::TextUnformatted("Folder Rules");
        DrawScopeRuleTable(game->FolderRules, "FolderRuleTable", 0.0f);
        ImGui::End();
    }

    void SaveManagerApp::DrawScopeRuleTable(std::vector<ScopeRule>& rules, const char* id, float height)
    {
        ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_ScrollY;
        if (ImGui::BeginTable(id, 3, flags, ImVec2(0.0f, height)))
        {
            ImGui::TableSetupColumn("Enabled", ImGuiTableColumnFlags_WidthFixed, 70.0f);
            ImGui::TableSetupColumn("Relative Path", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Action", ImGuiTableColumnFlags_WidthFixed, 80.0f);
            ImGui::TableHeadersRow();
            int removeIndex = -1;
            for (int i = 0; i < static_cast<int>(rules.size()); ++i)
            {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Checkbox((std::string("##enabled") + std::to_string(i) + id).c_str(), &rules[i].Enabled);
                ImGui::TableNextColumn();
                ImGui::TextWrapped("%s", rules[i].RelativePath.c_str());
                ImGui::TableNextColumn();
                if (ImGui::SmallButton((std::string("Remove##") + std::to_string(i) + id).c_str())) removeIndex = i;
            }
            if (removeIndex >= 0)
            {
                rules.erase(rules.begin() + removeIndex);
                m_SaveManager.SaveConfig();
                AddLog("Scope rule removed.", Warn());
            }
            ImGui::EndTable();
        }
    }

    void SaveManagerApp::DrawBackupsWindow()
    {
        if (!m_ShowBackups) return;
        ImGui::Begin("Backups", &m_ShowBackups);
        ImGui::Checkbox("Clear target before restore", &m_ClearBeforeRestore);
        ImGui::Checkbox("Auto backup before overwrite / restore", &m_BackupBeforeOverwrite);

        if (ImGui::BeginTable("BackupsTable", 5, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchProp))
        {
            ImGui::TableSetupColumn("Backup", ImGuiTableColumnFlags_WidthStretch, 1.1f);
            ImGui::TableSetupColumn("Created", ImGuiTableColumnFlags_WidthFixed, 145.0f);
            ImGui::TableSetupColumn("Files", ImGuiTableColumnFlags_WidthFixed, 60.0f);
            ImGui::TableSetupColumn("Size", ImGuiTableColumnFlags_WidthFixed, 85.0f);
            ImGui::TableSetupColumn("MD5", ImGuiTableColumnFlags_WidthStretch, 1.0f);
            ImGui::TableHeadersRow();

            const auto& backups = m_SaveManager.Backups();
            for (int i = 0; i < static_cast<int>(backups.size()); ++i)
            {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                if (ImGui::Selectable(backups[i].Name.c_str(), i == m_SaveManager.SelectedBackup(), ImGuiSelectableFlags_SpanAllColumns))
                    m_SaveManager.SetSelectedBackup(i);
                ImGui::TableNextColumn(); ImGui::TextUnformatted(backups[i].CreatedAtDisplay.c_str());
                ImGui::TableNextColumn(); ImGui::Text("%zu", backups[i].FileCount);
                ImGui::TableNextColumn(); ImGui::TextUnformatted(HumanSize(backups[i].TotalBytes).c_str());
                ImGui::TableNextColumn(); ImGui::TextUnformatted(backups[i].BackupHash.c_str());
            }
            ImGui::EndTable();
        }
        ImGui::End();
    }

    void SaveManagerApp::DrawBackupDetailsWindow()
    {
        if (!m_ShowBackupDetails) return;
        ImGui::Begin("Backup Details", &m_ShowBackupDetails);
        const BackupEntry* backup = m_SaveManager.CurrentBackup();
        if (!backup)
        {
            ImGui::TextDisabled("No backup selected.");
            ImGui::End();
            return;
        }

        ImGui::Text("Selected Backup: %s", backup->Name.c_str());
        if (!backup->Reason.empty()) ImGui::TextDisabled("Reason: %s", backup->Reason.c_str());
        ImGui::TextWrapped("%s", backup->FullPath.c_str());
        ImGui::TextWrapped("MD5: %s", backup->BackupHash.c_str());
        DrawBackupActionButtons(*backup);
        DrawDeleteBackupPopup();
        ImGui::Separator();

        if (ImGui::BeginTable("PreviewTable", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_ScrollY))
        {
            ImGui::TableSetupColumn("Relative Path", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Size", ImGuiTableColumnFlags_WidthFixed, 90.0f);
            ImGui::TableHeadersRow();
            for (const auto& file : backup->PreviewFiles)
            {
                ImGui::TableNextRow();
                ImGui::TableNextColumn(); ImGui::TextWrapped("%s", file.RelativePath.c_str());
                ImGui::TableNextColumn(); ImGui::TextUnformatted(HumanSize(file.Size).c_str());
            }
            ImGui::EndTable();
        }
        ImGui::End();
    }

    void SaveManagerApp::DrawBackupActionButtons(const BackupEntry& backup)
    {
        if (ImGui::Button("Restore Backup", ImVec2(-1.0f, 34.0f))) m_OpenRestoreModal = true;
        if (ImGui::Button("Open Backup", ImVec2(-1.0f, 34.0f)))
        {
            const bool ok = OpenInExplorer(backup.FullPath);
            AddLog(ok ? "Backup opened." : "Backup could not be opened.", ok ? Accent() : Bad());
        }
        if (ImGui::Button("Delete Backup", ImVec2(-1.0f, 34.0f))) ImGui::OpenPopup("DeleteBackupConfirm");
    }

    void SaveManagerApp::DrawActivityWindow()
    {
        if (!m_ShowActivityLog) return;
        ImGui::Begin("Activity Log", &m_ShowActivityLog);
        for (const auto& entry : m_Log) ImGui::TextColored(entry.Color, "%s", entry.Message.c_str());
        ImGui::End();
    }

    void SaveManagerApp::OpenCreateGameModal()
    {
        m_GameModalMode = 1;
        m_OpenGameModal = true;
        WriteToBuffer(m_ModalGameId, "new_game");
        WriteToBuffer(m_ModalGameName, "New Game");
        WriteToBuffer(m_ModalGameSavePath, "");
        WriteToBuffer(m_ModalGameProcessName, "");
        WriteToBuffer(m_ModalGameNotes, "");
        m_ModalGameEnabled = true;
        m_ModalGameScopeMode = static_cast<int>(SaveScopeMode::FolderMode);
    }

    void SaveManagerApp::OpenEditGameModal()
    {
        GameDefinition* game = m_SaveManager.CurrentGame();
        if (!game) return;
        m_GameModalMode = 2;
        m_OpenGameModal = true;
        WriteToBuffer(m_ModalGameId, game->Id);
        WriteToBuffer(m_ModalGameName, game->DisplayName);
        WriteToBuffer(m_ModalGameSavePath, game->SavePath);
        WriteToBuffer(m_ModalGameProcessName, game->ProcessName);
        WriteToBuffer(m_ModalGameNotes, game->Notes);
        m_ModalGameEnabled = game->Enabled;
        m_ModalGameScopeMode = static_cast<int>(game->ScopeMode);
    }

    void SaveManagerApp::OpenCreateProfileModal()
    {
        m_ProfileModalMode = 1;
        m_OpenProfileModal = true;
        WriteToBuffer(m_ModalProfileId, "profile");
        WriteToBuffer(m_ModalProfileName, "New Profile");
        WriteToBuffer(m_ModalProfileDescription, "");
        WriteToBuffer(m_ModalProfileNotes, "");
        m_ModalProfileFavorite = false;
        m_ModalProfileAutoBackup = true;
        m_ModalProfileBackupLimit = 20;
    }

    void SaveManagerApp::OpenEditProfileModal()
    {
        GameProfile* profile = m_SaveManager.CurrentProfile();
        if (!profile) return;
        m_ProfileModalMode = 2;
        m_OpenProfileModal = true;
        WriteToBuffer(m_ModalProfileId, profile->Id);
        WriteToBuffer(m_ModalProfileName, profile->Name);
        WriteToBuffer(m_ModalProfileDescription, profile->Description);
        WriteToBuffer(m_ModalProfileNotes, profile->Notes);
        m_ModalProfileFavorite = profile->Favorite;
        m_ModalProfileAutoBackup = profile->AutoBackupOnSwitch;
        m_ModalProfileBackupLimit = profile->BackupLimit;
    }

    void SaveManagerApp::DrawGameModal()
    {
        if (m_OpenGameModal) { ImGui::OpenPopup(m_GameModalMode == 1 ? "Create Game" : "Edit Game"); m_OpenGameModal = false; }
        const char* title = m_GameModalMode == 1 ? "Create Game" : "Edit Game";
        if (!ImGui::BeginPopupModal(title, nullptr, ImGuiWindowFlags_AlwaysAutoResize)) return;

        ImGui::InputText("Game Id", m_ModalGameId.data(), m_ModalGameId.size());
        ImGui::InputText("Display Name", m_ModalGameName.data(), m_ModalGameName.size());
        ImGui::InputText("Save Path", m_ModalGameSavePath.data(), m_ModalGameSavePath.size());
        ImGui::InputText("Process Name", m_ModalGameProcessName.data(), m_ModalGameProcessName.size());
        ImGui::Checkbox("Enabled", &m_ModalGameEnabled);
        const char* scopeNames[] = { "File Whitelist", "Folder Mode", "Hybrid Mode" };
        ImGui::Combo("Save Scope", &m_ModalGameScopeMode, scopeNames, IM_ARRAYSIZE(scopeNames));
        ImGui::InputTextMultiline("Notes", m_ModalGameNotes.data(), m_ModalGameNotes.size(), ImVec2(520.0f, 150.0f));

        if (ImGui::Button("Save", ImVec2(140.0f, 0.0f)))
        {
            if (m_GameModalMode == 1)
            {
                GameDefinition game;
                game.Id = SanitizeId(m_ModalGameId.data());
                game.DisplayName = m_ModalGameName.data();
                game.SavePath = m_ModalGameSavePath.data();
                game.ProcessName = m_ModalGameProcessName.data();
                game.Notes = m_ModalGameNotes.data();
                game.Enabled = m_ModalGameEnabled;
                game.ScopeMode = static_cast<SaveScopeMode>(m_ModalGameScopeMode);
                game.Profiles.push_back({ "vanilla", "Vanilla", "Default profile", "", true, true, 20 });
                game.ActiveProfileId = "vanilla";
                m_SaveManager.Games().push_back(std::move(game));
                m_SaveManager.SetSelectedGame(static_cast<int>(m_SaveManager.Games().size()) - 1);
                LoadSelectedIntoEditor();
                AddLog("Game created.", Good());
            }
            else if (GameDefinition* game = m_SaveManager.CurrentGame())
            {
                game->Id = SanitizeId(m_ModalGameId.data());
                game->DisplayName = m_ModalGameName.data();
                game->SavePath = m_ModalGameSavePath.data();
                game->ProcessName = m_ModalGameProcessName.data();
                game->Notes = m_ModalGameNotes.data();
                game->Enabled = m_ModalGameEnabled;
                game->ScopeMode = static_cast<SaveScopeMode>(m_ModalGameScopeMode);
                LoadSelectedIntoEditor();
                AddLog("Game updated.", Good());
            }
            m_SaveManager.SaveConfig();
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(140.0f, 0.0f))) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    void SaveManagerApp::DrawProfileModal()
    {
        if (m_OpenProfileModal) { ImGui::OpenPopup(m_ProfileModalMode == 1 ? "Create Profile" : "Edit Profile"); m_OpenProfileModal = false; }
        const char* title = m_ProfileModalMode == 1 ? "Create Profile" : "Edit Profile";
        if (!ImGui::BeginPopupModal(title, nullptr, ImGuiWindowFlags_AlwaysAutoResize)) return;

        ImGui::InputText("Profile Id", m_ModalProfileId.data(), m_ModalProfileId.size());
        ImGui::InputText("Profile Name", m_ModalProfileName.data(), m_ModalProfileName.size());
        ImGui::InputText("Description", m_ModalProfileDescription.data(), m_ModalProfileDescription.size());
        ImGui::Checkbox("Favorite", &m_ModalProfileFavorite);
        ImGui::Checkbox("Auto Backup on Switch", &m_ModalProfileAutoBackup);
        ImGui::SliderInt("Backup Limit", &m_ModalProfileBackupLimit, 1, 100);
        ImGui::InputTextMultiline("Notes", m_ModalProfileNotes.data(), m_ModalProfileNotes.size(), ImVec2(500.0f, 140.0f));

        if (ImGui::Button("Save", ImVec2(140.0f, 0.0f)))
        {
            GameDefinition* game = m_SaveManager.CurrentGame();
            if (game)
            {
                if (m_ProfileModalMode == 1)
                {
                    GameProfile profile;
                    profile.Id = SanitizeId(m_ModalProfileId.data());
                    profile.Name = m_ModalProfileName.data();
                    profile.Description = m_ModalProfileDescription.data();
                    profile.Notes = m_ModalProfileNotes.data();
                    profile.Favorite = m_ModalProfileFavorite;
                    profile.AutoBackupOnSwitch = m_ModalProfileAutoBackup;
                    profile.BackupLimit = m_ModalProfileBackupLimit;
                    game->Profiles.push_back(profile);
                    game->ActiveProfileId = profile.Id;
                    m_SaveManager.SyncSelectedProfileFromActive();
                }
                else if (GameProfile* profile = m_SaveManager.CurrentProfile())
                {
                    profile->Id = SanitizeId(m_ModalProfileId.data());
                    profile->Name = m_ModalProfileName.data();
                    profile->Description = m_ModalProfileDescription.data();
                    profile->Notes = m_ModalProfileNotes.data();
                    profile->Favorite = m_ModalProfileFavorite;
                    profile->AutoBackupOnSwitch = m_ModalProfileAutoBackup;
                    profile->BackupLimit = m_ModalProfileBackupLimit;
                    game->ActiveProfileId = profile->Id;
                }
                m_SaveManager.SaveConfig();
                LoadSelectedIntoEditor();
                AddLog("Profile saved.", Good());
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(140.0f, 0.0f))) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    void SaveManagerApp::DrawRestoreModal()
    {
        if (m_OpenRestoreModal) { ImGui::OpenPopup("Restore Backup"); m_OpenRestoreModal = false; }
        if (!ImGui::BeginPopupModal("Restore Backup", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) return;

        const BackupEntry* backup = m_SaveManager.CurrentBackup();
        if (!backup)
        {
            ImGui::TextDisabled("No backup selected.");
            if (ImGui::Button("Close", ImVec2(140.0f, 0.0f))) ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
            return;
        }

        ImGui::Text("Restore backup '%s'?", backup->Name.c_str());
        ImGui::Checkbox("Clear target before restore", &m_ClearBeforeRestore);
        ImGui::Checkbox("Auto backup before overwrite / restore", &m_BackupBeforeOverwrite);
        if (ImGui::Button("Restore", ImVec2(140.0f, 0.0f)))
        {
            SaveEditorIntoSelected();
            auto result = m_SaveManager.RestoreSelectedBackup(m_ClearBeforeRestore, m_BackupBeforeOverwrite);
            AddLog(result.Message, result.Success ? Good() : Bad());
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(140.0f, 0.0f))) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    void SaveManagerApp::DrawAboutPopup()
    {
        ImGui::SetNextWindowSize(ImVec2(550.0f, 610.0f), ImGuiCond_Appearing);

        if (m_OpenAboutPopup)
        {
            ImGui::OpenPopup("About Manifold");
            m_OpenAboutPopup = false;
        }

        if (!ImGui::BeginPopupModal("About Manifold", nullptr, ImGuiWindowFlags_NoResize))
            return;

        ImDrawList* dl = ImGui::GetWindowDrawList();
        const ImVec2 winPos = ImGui::GetWindowPos();
        const ImVec2 winSize = ImGui::GetWindowSize();

        const ImU32 lineCol = ImGui::GetColorU32(ImVec4(1, 1, 1, 0.06f));
        const ImU32 accentCol = ImGui::GetColorU32(ImVec4(0.15f, 0.82f, 0.88f, 0.85f));
        const ImU32 panelBorderCol = ImGui::GetColorU32(ImVec4(1, 1, 1, 0.06f));

        dl->AddRect(
            winPos,
            ImVec2(winPos.x + winSize.x, winPos.y + winSize.y),
            panelBorderCol,
            12.0f,
            0,
            1.0f
        );

        dl->AddLine(
            ImVec2(winPos.x + 14.0f, winPos.y + 1.0f),
            ImVec2(winPos.x + winSize.x - 14.0f, winPos.y + 1.0f),
            accentCol,
            2.0f
        );

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10.0f, 10.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.0f, 8.0f));

        ImGui::TextUnformatted("Manifold Save Manager");
        ImGui::TextDisabled("Creators, links, and project information");

        ImGui::Spacing();
        dl->AddLine(
            ImVec2(winPos.x + 16.0f, ImGui::GetCursorScreenPos().y),
            ImVec2(winPos.x + winSize.x - 16.0f, ImGui::GetCursorScreenPos().y),
            lineCol,
            1.0f
        );
        ImGui::Dummy(ImVec2(0.0f, 10.0f));

        const float columnWidth = 400.0f;
        const float animSize = 230.0f;
        const float buttonWidth = 110.0f;

        ImGui::BeginGroup();
        {
            ImGui::SetNextItemWidth(columnWidth);
            ImGui::TextUnformatted("Leunsel");
            ImGui::TextDisabled("Creator of Manifold");
            ImGui::Spacing();

            DrawAboutCubeAnimation(animSize);

            ImGui::Spacing();

            if (ImGui::Button("Website##Leunsel", ImVec2(buttonWidth, 0.0f)))
                OpenUrl("https://leunsel.com/");

            ImGui::SameLine();

            if (ImGui::Button("Patreon##Leunsel", ImVec2(buttonWidth, 0.0f)))
                OpenUrl("https://www.patreon.com/leunsel");

            ImGui::Spacing();
            ImGui::TextDisabled("Manifold / Framework / Tooling");
        }
        ImGui::EndGroup();

        ImGui::SameLine(0.0f, 30.0f);

        ImGui::BeginGroup();
        {
            ImGui::TextUnformatted("Cfemen");
            ImGui::TextDisabled("Contributor");
            ImGui::Spacing();

            DrawAboutCubeAnimation2(animSize);

            ImGui::Spacing();

            ImGui::BeginDisabled();
            if (ImGui::Button("Website##Cfemen", ImVec2(buttonWidth, 0.0f)))
                OpenUrl("");
            ImGui::EndDisabled();

            ImGui::SameLine();

            if (ImGui::Button("Patreon##Cfemen", ImVec2(buttonWidth, 0.0f)))
                OpenUrl("https://www.patreon.com/cfemen");

            ImGui::Spacing();
            ImGui::TextDisabled("Tooling");
        }
        ImGui::EndGroup();

        ImGui::Spacing();
        ImGui::Spacing();

        dl->AddLine(
            ImVec2(winPos.x + 16.0f, ImGui::GetCursorScreenPos().y),
            ImVec2(winPos.x + winSize.x - 16.0f, ImGui::GetCursorScreenPos().y),
            lineCol,
            1.0f
        );
        ImGui::Dummy(ImVec2(0.0f, 10.0f));

        ImGui::BeginGroup();
        {
            ImGui::TextDisabled("Project");
            ImGui::SameLine();
            ImGui::TextDisabled("|");
            ImGui::SameLine();
            ImGui::TextDisabled("Open source tooling and workflow UI");

            ImGui::Spacing();
        }
        ImGui::EndGroup();

        const float closeWidth = 120.0f;

        if (ImGui::Button("GitHub##Tool", ImVec2(closeWidth, 0.0f)))
            OpenUrl("https://github.com/your-org/your-repo");

        ImGui::SameLine();

        if (ImGui::Button("Close", ImVec2(closeWidth, 0.0f)))
            ImGui::CloseCurrentPopup();

        ImGui::PopStyleVar(2);
        ImGui::EndPopup();
    }

    void SaveManagerApp::DrawAboutCubeAnimation(float cubeSize)
    {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        if (!dl)
            return;

        ImVec2 start = ImGui::GetCursorScreenPos();
        ImVec2 region = ImVec2(cubeSize, cubeSize);

        ImGui::InvisibleButton("##AboutCubeCanvas", region);

        const ImVec2 canvasMin = start;
        const ImVec2 canvasMax(start.x + region.x, start.y + region.y);

        const float pad = 6.0f;
        const float sizef = (std::min)(region.x, region.y) - 2.0f * pad;
        Math::Vec2 center(
            canvasMin.x + region.x * 0.5f,
            canvasMin.y + region.y * 0.5f
        );

        const float t = static_cast<float>(ImGui::GetTime());
        const float rx = t * 0.6f;
        const float ry = t * 0.9f;

        const float fov = 1.1f;
        const float zOff = 3.2f;
        const float half = sizef * 0.5f;

        auto Proj = [&](const Math::Vec3& p) -> Math::Vec2
            {
                float z = p.z + zOff;
                float inv = (z > 0.01f) ? (fov / z) : (fov / 0.01f);
                return { center.x + p.x * inv * half, center.y + p.y * inv * half };
            };

        Math::Vec3 cube[8] = {
            {-1,-1,-1},{ 1,-1,-1},{-1, 1,-1},{ 1, 1,-1},
            {-1,-1, 1},{ 1,-1, 1},{-1, 1, 1},{ 1, 1, 1}
        };

        ImVec2 pts[8];
        for (int i = 0; i < 8; ++i)
        {
            Math::Vec3 v = Math::RotateVecX(
                Math::RotateVecY(cube[i], ry), rx
            );
            Math::Vec2 p = Proj(v);
            pts[i] = ImVec2(p.x, p.y);
        }

        static const int edges[12][2] = {
            {0,1},{1,3},{3,2},{2,0},
            {4,5},{5,7},{7,6},{6,4},
            {0,4},{1,5},{2,6},{3,7}
        };

        const ImU32 lineCol = ImGui::GetColorU32(ImVec4(1, 1, 1, 0.22f));
        for (const auto& e : edges)
            dl->AddLine(pts[e[0]], pts[e[1]], lineCol, 1.8f);

        static const int cornerOrder[8] = { 0, 1, 3, 2, 6, 7, 5, 4 };
        static const char* word = "Manifold";

        const ImU32 txtCol = ImGui::GetColorU32(ImVec4(1, 1, 1, 0.95f));
        const ImU32 txtShadow = ImGui::GetColorU32(ImVec4(0, 0, 0, 0.55f));
        const float outOffset = 8.0f;
        const float shadowOff = 1.0f;

        for (int i = 0; i < 8; ++i)
        {
            const int vi = cornerOrder[i];
            Math::Vec2 p(pts[vi].x, pts[vi].y);

            Math::Vec2 dir = (p - center).Normalized();
            if (dir.LengthSq() < 1e-6f)
                dir = Math::Vec2(0, -1);

            Math::Vec2 q = p + dir * outOffset;

            char c[2] = { word[i], 0 };
            ImVec2 ts = ImGui::CalcTextSize(c);

            dl->AddText(
                ImVec2(q.x - ts.x * 0.5f + shadowOff, q.y - ts.y * 0.5f + shadowOff),
                txtShadow,
                c
            );
            dl->AddText(
                ImVec2(q.x - ts.x * 0.5f, q.y - ts.y * 0.5f),
                txtCol,
                c
            );
        }

        // dl->AddRect(canvasMin, canvasMax, ImGui::GetColorU32(ImVec4(1, 1, 1, 0.05f)), 10.0f);
    }

    void SaveManagerApp::DrawAboutCubeAnimation2(float cubeSize)
    {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        if (!dl)
            return;

        ImVec2 start = ImGui::GetCursorScreenPos();
        ImVec2 region(cubeSize, cubeSize);

        ImGui::InvisibleButton("##AboutOctahedronCanvas", region);

        const ImVec2 canvasMin = start;
        const ImVec2 canvasMax(start.x + region.x, start.y + region.y);

        const float pad = 6.0f;
        const float sizef = (std::min)(region.x, region.y) - 2.0f * pad;

        Math::Vec2 center(
            canvasMin.x + region.x * 0.5f,
            canvasMin.y + region.y * 0.5f
        );

        const float t = static_cast<float>(ImGui::GetTime());

        const float rx = t * 0.45f;
        const float ry = t * 1.10f;
        const float rz = t * 0.30f;

        const float fov = 1.1f;
        const float zOff = 3.2f;
        const float half = sizef * 0.5f;

        const float shapeScale = 1.5f;

        auto Proj = [&](const Math::Vec3& p) -> Math::Vec2
            {
                float z = p.z + zOff;
                float inv = (z > 0.01f) ? (fov / z) : (fov / 0.01f);
                return { center.x + p.x * inv * half, center.y + p.y * inv * half };
            };

        auto Rotate = [&](const Math::Vec3& v) -> Math::Vec3
            {
                return Math::RotateVecZ(
                    Math::RotateVecX(
                        Math::RotateVecY(v, ry), rx
                    ),
                    rz
                );
            };

        Math::Vec3 verts[6] = {
            { 0,  0,  1},
            { 0,  0, -1},
            {-1,  0,  0},
            { 1,  0,  0},
            { 0,  1,  0},
            { 0, -1,  0}
        };

        ImVec2 pts[6];
        Math::Vec3 rotated[6];

        for (int i = 0; i < 6; ++i)
        {
            rotated[i] = Rotate(verts[i] * shapeScale);
            Math::Vec2 p = Proj(rotated[i]);
            pts[i] = ImVec2(p.x, p.y);
        }

        static const int edges[12][2] = {
            {0,2}, {0,3}, {0,4}, {0,5},
            {1,2}, {1,3}, {1,4}, {1,5},
            {2,4}, {4,3}, {3,5}, {5,2}
        };

        const ImU32 lineCol = ImGui::GetColorU32(ImVec4(1, 1, 1, 0.22f));
        for (const auto& e : edges)
            dl->AddLine(pts[e[0]], pts[e[1]], lineCol, 1.8f);

        static const char* word = "Cfemen";

        struct VertexLabel
        {
            ImVec2 pos;
            float depth;
            char ch;
        };

        VertexLabel labels[6];

        for (int i = 0; i < 6; ++i)
        {
            labels[i].pos = pts[i];
            labels[i].depth = rotated[i].z;
            labels[i].ch = word[i];
        }

        std::sort(std::begin(labels), std::end(labels),
            [](const VertexLabel& a, const VertexLabel& b)
            {
                return a.depth < b.depth;
            });

        const ImU32 txtCol = ImGui::GetColorU32(ImVec4(1, 1, 1, 0.96f));
        const ImU32 txtShadow = ImGui::GetColorU32(ImVec4(0, 0, 0, 0.55f));

        const float outOffset = 14.0f;
        const float shadowOff = 1.0f;

        for (const auto& label : labels)
        {
            Math::Vec2 p(label.pos.x, label.pos.y);
            Math::Vec2 dir = (p - center).Normalized();
            if (dir.LengthSq() < 1e-6f)
                dir = Math::Vec2(0.0f, -1.0f);

            Math::Vec2 q = p + dir * outOffset;

            char c[2] = { label.ch, 0 };
            ImVec2 ts = ImGui::CalcTextSize(c);
            ImVec2 textPos(q.x - ts.x * 0.5f, q.y - ts.y * 0.5f);

            dl->AddText(ImVec2(textPos.x + shadowOff, textPos.y + shadowOff), txtShadow, c);
            dl->AddText(textPos, txtCol, c);
        }

        // dl->AddRect(canvasMin, canvasMax, ImGui::GetColorU32(ImVec4(1, 1, 1, 0.05f)), 10.0f);
    }

    void SaveManagerApp::DrawDeleteBackupPopup()
    {
        if (!ImGui::BeginPopupModal("DeleteBackupConfirm", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) return;
        ImGui::TextUnformatted("Delete the selected backup?");
        if (ImGui::Button("Delete", ImVec2(120.0f, 0.0f)))
        {
            auto result = m_SaveManager.DeleteSelectedBackup();
            AddLog(result.Message, result.Success ? Warn() : Bad());
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120.0f, 0.0f))) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    void SaveManagerApp::ExecuteInstantProfileSwitch()
    {
        GameDefinition* game = m_SaveManager.CurrentGame();
        if (!game || game->Profiles.empty()) return;
        int nextIndex = m_SaveManager.SelectedProfile() + 1;
        if (nextIndex >= static_cast<int>(game->Profiles.size())) nextIndex = 0;
        auto result = m_SaveManager.SwitchToProfile(nextIndex, true);
        LoadSelectedIntoEditor();
        m_SaveManager.SaveConfig();
        AddLog(result.Message + " -> " + game->Profiles[nextIndex].Name, result.Success ? Good() : Bad());
    }

    void SaveManagerApp::LoadSelectedIntoEditor()
    {
        const GameDefinition* game = m_SaveManager.CurrentGame();
        const GameProfile* profile = m_SaveManager.CurrentProfile();
        if (game)
        {
            WriteToBuffer(m_EditGameId, game->Id);
            WriteToBuffer(m_EditGameName, game->DisplayName);
            WriteToBuffer(m_EditSavePath, game->SavePath);
            WriteToBuffer(m_EditProcessName, game->ProcessName);
            WriteToBuffer(m_EditGameNotes, game->Notes);
        }
        else
        {
            ClearGameEditor();
        }

        if (profile)
        {
            WriteToBuffer(m_EditProfileId, profile->Id);
            WriteToBuffer(m_EditProfileName, profile->Name);
            WriteToBuffer(m_EditProfileDescription, profile->Description);
            WriteToBuffer(m_EditProfileNotes, profile->Notes);
        }
        else
        {
            ClearProfileEditor();
        }
    }

    void SaveManagerApp::SaveEditorIntoSelected()
    {
        GameDefinition* game = m_SaveManager.CurrentGame();
        GameProfile* profile = m_SaveManager.CurrentProfile();
        if (game)
        {
            game->Id = SanitizeId(m_EditGameId.data());
            game->DisplayName = m_EditGameName.data();
            game->SavePath = m_EditSavePath.data();
            game->ProcessName = m_EditProcessName.data();
            game->Notes = m_EditGameNotes.data();
        }
        if (profile)
        {
            profile->Id = SanitizeId(m_EditProfileId.data());
            profile->Name = m_EditProfileName.data();
            profile->Description = m_EditProfileDescription.data();
            profile->Notes = m_EditProfileNotes.data();
            if (game) game->ActiveProfileId = profile->Id;
        }
    }

    void SaveManagerApp::ClearGameEditor()
    {
        m_EditGameId.fill('\0');
        m_EditGameName.fill('\0');
        m_EditSavePath.fill('\0');
        m_EditProcessName.fill('\0');
        m_EditGameNotes.fill('\0');
    }

    void SaveManagerApp::ClearProfileEditor()
    {
        m_EditProfileId.fill('\0');
        m_EditProfileName.fill('\0');
        m_EditProfileDescription.fill('\0');
        m_EditProfileNotes.fill('\0');
    }

    void SaveManagerApp::AddLog(std::string message, ImVec4 color)
    {
        const std::string stamped = '[' + FormatTime(std::time(nullptr)) + "] " + std::move(message);
        m_Log.push_back({ stamped, color });
        if (m_Log.size() > 150) m_Log.erase(m_Log.begin(), m_Log.begin() + static_cast<long long>(m_Log.size() - 150));
    }

    ImVec4 SaveManagerApp::Good() { return ImVec4(0.43f, 0.84f, 0.54f, 1.0f); }
    ImVec4 SaveManagerApp::Warn() { return ImVec4(0.95f, 0.73f, 0.31f, 1.0f); }
    ImVec4 SaveManagerApp::Bad() { return ImVec4(0.95f, 0.42f, 0.42f, 1.0f); }
    ImVec4 SaveManagerApp::Accent() { return ImVec4(0.47f, 0.72f, 1.0f, 1.0f); }
}
