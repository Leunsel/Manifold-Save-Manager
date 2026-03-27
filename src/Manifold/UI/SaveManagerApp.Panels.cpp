#include "SaveManagerApp.Internal.hpp"

namespace manifold
{
    void SaveManagerApp::Impl::DrawNavigatorWindow()
    {
        if (!Panels.Navigator)
        {
            return;
        }

        if (!BeginRetroWindow(ICON_FA_GAMEPAD " Navigator###Navigator", &Panels.Navigator))
        {
            EndRetroWindow();
            return;
        }

        const bool hasGame = SaveManager.CurrentGame() != nullptr;

        if (RetroButton(ICON_FA_PLUS "##AddGame")) OpenCreateGameModal();
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Create game");

        ImGui::SameLine();
        ImGui::BeginDisabled(!hasGame);
        if (RetroButton(ICON_FA_EDIT "##EditGame")) OpenEditGameModal();
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Edit selected game");

        ImGui::SameLine();
        if (RetroButton(ICON_FA_TRASH_ALT "##DeleteGame")) Popups.DeleteGame = true;
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Delete selected game");
        ImGui::EndDisabled();

        ImGui::SameLine();
        ImGui::SetNextItemWidth(-1.0f);
        ImGui::InputTextWithHint("##GameFilter", ICON_FA_SEARCH " Filter games...", Filters.Game, IM_ARRAYSIZE(Filters.Game));
        ImGui::Separator();

        if (ImGui::BeginListBox("##GameList", ImVec2(-1.0f, -100.0f)))
        {
            const auto& games = SaveManager.Games();
            const std::string filter = Trim(Filters.Game);

            for (int i = 0; i < static_cast<int>(games.size()); ++i)
            {
                const bool selected = i == SaveManager.SelectedGame();
                const std::string display = games[i].DisplayName.empty() ? games[i].Id : games[i].DisplayName;

                if (!filter.empty())
                {
                    const std::string haystack = ToLowerCopy(display + " " + games[i].Id + " " + games[i].SavePath);
                    if (haystack.find(ToLowerCopy(filter)) == std::string::npos)
                    {
                        continue;
                    }
                }

                ImGui::PushID(i);
                if (ImGui::Selectable(display.c_str(), selected))
                {
                    SyncSelectionFromEditor();
                    SaveManager.SetSelectedGame(i);
                    SyncEditorFromSelection();
                }

                if (ImGui::BeginPopupContextItem("GameContext"))
                {
                    if (ImGui::MenuItem(ICON_FA_EDIT " Edit")) OpenEditGameModal();
                    if (ImGui::MenuItem(ICON_FA_TRASH_ALT " Delete")) Popups.DeleteGame = true;
                    ImGui::Separator();
                    if (ImGui::MenuItem(ICON_FA_ARCHIVE " Create Backup", nullptr, false, SaveManager.CurrentProfile() != nullptr))
                    {
                        SyncSelectionFromEditor();
                        const auto result = SaveManager.CreateBackupForCurrentProfile("Manual backup");
                        AddLog(result.Message, result.Success ? Good() : Bad());
                    }
                    ImGui::EndPopup();
                }
                ImGui::PopID();
            }
            ImGui::EndListBox();
        }

        if (const GameProfile* profile = SaveManager.CurrentProfile())
        {
            ImGui::Separator();
            ImGui::TextDisabled("Active Profile: %s", profile->Name.c_str());
        }

        EndRetroWindow();
    }

    void SaveManagerApp::Impl::DrawProfilesWindow()
    {
        if (!Panels.Profiles)
        {
            return;
        }

        if (!BeginRetroWindow(ICON_FA_USERS " Profiles###Profiles", &Panels.Profiles))
        {
            EndRetroWindow();
            return;
        }

        GameDefinition* game = SaveManager.CurrentGame();
        if (game == nullptr)
        {
            ImGui::TextDisabled("No game selected.");
            EndRetroWindow();
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

        EndRetroWindow();
    }

    void SaveManagerApp::Impl::DrawProfileListPane(GameDefinition& game)
    {
        const bool hasProfile = SaveManager.CurrentProfile() != nullptr;
        const bool canRemove = hasProfile && game.Profiles.size() > 1;

        if (RetroButton(ICON_FA_PLUS "##AddProfile")) OpenCreateProfileModal();
        ImGui::SameLine();

        ImGui::BeginDisabled(!hasProfile);
        if (RetroButton(ICON_FA_EDIT "##EditProfile")) OpenEditProfileModal();
        ImGui::SameLine();

        ImGui::BeginDisabled(!canRemove);
        if (RetroButton(ICON_FA_TRASH_ALT "##RemoveProfile"))
        {
            SyncSelectionFromEditor();
            SaveManager.RemoveCurrentProfile();
            SyncEditorFromSelection();
            SaveManager.SaveConfig();
            AddLog("Profile removed.", Warn());
        }
        ImGui::EndDisabled();
        ImGui::EndDisabled();

        ImGui::Separator();

        if (ImGui::BeginListBox("##ProfileListPane", ImVec2(-1.0f, -1.0f)))
        {
            for (int i = 0; i < static_cast<int>(game.Profiles.size()); ++i)
            {
                const GameProfile& profile = game.Profiles[i];
                const bool selected = i == SaveManager.SelectedProfile();
                std::string label = profile.Favorite ? std::string(ICON_FA_STAR " ") : std::string();
                label += profile.Name + "##" + profile.Id;

                ImGui::PushID(i);
                if (ImGui::Selectable(label.c_str(), selected))
                {
                    SyncSelectionFromEditor();
                    SaveManager.SetSelectedProfile(i);
                    SyncEditorFromSelection();
                }

                if (ImGui::BeginPopupContextItem("ProfileContext"))
                {
                    if (ImGui::MenuItem(ICON_FA_EDIT " Edit")) OpenEditProfileModal();
                    if (ImGui::MenuItem(ICON_FA_EXCHANGE_ALT " Activate"))
                    {
                        SyncSelectionFromEditor();
                        SaveManager.SetSelectedProfile(i);
                        SyncEditorFromSelection();
                        SaveManager.SaveConfig();
                        AddLog("Profile activated.", Good());
                    }
                    ImGui::Separator();
                    if (ImGui::MenuItem(ICON_FA_TRASH_ALT " Remove", nullptr, false, game.Profiles.size() > 1))
                    {
                        SyncSelectionFromEditor();
                        SaveManager.SetSelectedProfile(i);
                        SaveManager.RemoveCurrentProfile();
                        SyncEditorFromSelection();
                        SaveManager.SaveConfig();
                        AddLog("Profile removed.", Warn());
                    }
                    ImGui::EndPopup();
                }
                ImGui::PopID();
            }
            ImGui::EndListBox();
        }
    }

    void SaveManagerApp::Impl::DrawProfileEditorPane()
    {
        GameProfile* profile = SaveManager.CurrentProfile();
        if (profile == nullptr)
        {
            ImGui::TextDisabled("No profile selected.");
            return;
        }

        ImGui::TextDisabled("Selected profile settings");
        ImGui::Separator();

        if (ImGui::BeginTabBar("ProfileEditorTabs"))
        {
            if (ImGui::BeginTabItem("General"))
            {
                ImGui::InputText("Profile Id", ProfileEditor.Id.data(), ProfileEditor.Id.size());
                ImGui::InputText("Profile Name", ProfileEditor.Name.data(), ProfileEditor.Name.size());
                ImGui::InputText("Description", ProfileEditor.Description.data(), ProfileEditor.Description.size());
                ImGui::Checkbox("Favorite", &profile->Favorite);
                ImGui::Checkbox("Auto Backup on Switch", &profile->AutoBackupOnSwitch);
                ImGui::SliderInt("Backup Limit", &profile->BackupLimit, 1, 100);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Notes"))
            {
                ImGui::InputTextMultiline("##ProfileNotes", ProfileEditor.Notes.data(), ProfileEditor.Notes.size(), ImVec2(-1.0f, -35.0f));
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }

        if (RetroButton("Apply Profile Changes", ImVec2(-1.0f, 0.0f)))
        {
            SyncSelectionFromEditor();
            const bool ok = SaveManager.SaveConfig();
            AddLog(ok ? "Profile updated." : "Profile could not be saved.", ok ? Good() : Bad());
        }
    }

    void SaveManagerApp::Impl::DrawGameConfigWindow()
    {
        if (!Panels.GameConfig)
        {
            return;
        }

        if (!BeginRetroWindow(ICON_FA_COG " Game Config###Game Config", &Panels.GameConfig))
        {
            EndRetroWindow();
            return;
        }

        GameDefinition* game = SaveManager.CurrentGame();
        if (game == nullptr)
        {
            ImGui::TextDisabled("No game selected.");
            if (RetroButton("Create Game", ImVec2(-1.0f, 0.0f))) OpenCreateGameModal();
            EndRetroWindow();
            return;
        }

        ImGui::Checkbox("Enabled", &game->Enabled);
        ImGui::InputText("Game Id", GameEditor.Id.data(), GameEditor.Id.size());
        ImGui::InputText("Display Name", GameEditor.Name.data(), GameEditor.Name.size());

        ImGui::TextUnformatted("Save Path");
        ImGui::SetNextItemWidth(-110.0f);
        ImGui::InputText("##SavePath", GameEditor.SavePath.data(), GameEditor.SavePath.size());
        ImGui::SameLine();
        if (RetroButton("Browse", ImVec2(100.0f, 0.0f)))
        {
            if (auto folder = PickFolderDialog(MainWindow, L"Select Save Folder"))
            {
                WriteToBuffer(GameEditor.SavePath, *folder);
            }
        }

        ImGui::InputText("Process Name", GameEditor.ProcessName.data(), GameEditor.ProcessName.size());
        ImGui::InputTextMultiline("Game Notes", GameEditor.Notes.data(), GameEditor.Notes.size(), ImVec2(-100.0f, 180.0f));

        const char* scopeNames[] = { "File Whitelist", "Folder Mode", "Hybrid Mode" };
        int scopeIndex = static_cast<int>(game->ScopeMode);
        if (ImGui::Combo("Save Scope", &scopeIndex, scopeNames, IM_ARRAYSIZE(scopeNames)))
        {
            game->ScopeMode = static_cast<SaveScopeMode>(scopeIndex);
        }

        ImGui::Spacing();
        if (RetroButton(ICON_FA_FOLDER_OPEN " Open Folder", ImVec2(160.0f, 0.0f)))
        {
            const bool ok = OpenInExplorer(GameEditor.SavePath.data());
            AddLog(ok ? "Save path opened." : "Save path could not be opened.", ok ? Accent() : Bad());
        }

        ImGui::SameLine();
        if (RetroButton(ICON_FA_SAVE " Save Changes", ImVec2(170.0f, 0.0f)))
        {
            SyncSelectionFromEditor();
            const bool ok = SaveManager.SaveConfig();
            AddLog(ok ? "Game configuration updated." : "Game configuration could not be saved.", ok ? Good() : Bad());
        }

        ImGui::Separator();
        ImGui::BulletText("Save Path: %s", DirectoryExists(GameEditor.SavePath.data()) ? "OK" : "Missing");
        EndRetroWindow();
    }

    void SaveManagerApp::Impl::DrawScopeRulesWindow()
    {
        if (!Panels.ScopeRules)
        {
            return;
        }

        if (!BeginRetroWindow(ICON_FA_FILTER " Scope Rules###Scope Rules", &Panels.ScopeRules))
        {
            EndRetroWindow();
            return;
        }

        GameDefinition* game = SaveManager.CurrentGame();
        if (game == nullptr)
        {
            ImGui::TextDisabled("No game selected.");
            EndRetroWindow();
            return;
        }

        ImGui::TextDisabled("Scope Mode");
        ImGui::SameLine();
        ImGui::TextUnformatted(ToString(game->ScopeMode));
        ImGui::Separator();

        static int newRuleType = 0;
        const char* ruleKinds[] = { "File Rule", "Folder Rule" };
        ImGui::SetNextItemWidth(150.0f);
        ImGui::Combo("##RuleType", &newRuleType, ruleKinds, IM_ARRAYSIZE(ruleKinds));
        ImGui::SameLine();
        ImGui::SetNextItemWidth(-110.0f);
        ImGui::InputTextWithHint("##NewScopeRule", "Relative path, e.g. SaveData.sav or Saves\\Slot1", ProfileEditor.ScopeRule.data(), ProfileEditor.ScopeRule.size());
        ImGui::SameLine();

        if (RetroButton(ICON_FA_PLUS " Add", ImVec2(90.0f, 0.0f)))
        {
            const std::string value = Trim(ProfileEditor.ScopeRule.data());
            if (!value.empty())
            {
                if (newRuleType == 0)
                {
                    game->Whitelist.push_back({ value, true });
                    AddLog("File whitelist rule added.", Accent());
                }
                else
                {
                    game->FolderRules.push_back({ value, true });
                    AddLog("Folder rule added.", Accent());
                }
                WriteToBuffer(ProfileEditor.ScopeRule, "");
                SaveManager.SaveConfig();
            }
            else
            {
                AddLog("Scope rule must not be empty.", Bad());
            }
        }

        ImGui::Separator();
        ImGui::TextDisabled("File Whitelist");
        ImGui::SameLine();
        ImGui::Text("(%d)", static_cast<int>(game->Whitelist.size()));
        DrawScopeRuleTable(game->Whitelist, "WhitelistTable", 180.0f);

        ImGui::Spacing();
        ImGui::TextDisabled("Folder Rules");
        ImGui::SameLine();
        ImGui::Text("(%d)", static_cast<int>(game->FolderRules.size()));
        DrawScopeRuleTable(game->FolderRules, "FolderRuleTable", 0.0f);

        EndRetroWindow();
    }

    void SaveManagerApp::Impl::DrawScopeRuleTable(std::vector<ScopeRule>& rules, const char* id, float height)
    {
        const ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
            ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_ScrollY;

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
                if (ImGui::SmallButton((std::string("Remove##") + std::to_string(i) + id).c_str()))
                {
                    removeIndex = i;
                }
            }

            if (removeIndex >= 0)
            {
                rules.erase(rules.begin() + removeIndex);
                SaveManager.SaveConfig();
                AddLog("Scope rule removed.", Warn());
            }
            ImGui::EndTable();
        }
    }

    void SaveManagerApp::Impl::DrawBackupsWindow()
    {
        if (!Panels.Backups)
        {
            return;
        }

        if (!BeginRetroWindow(ICON_FA_ARCHIVE " Backups###Backups", &Panels.Backups))
        {
            EndRetroWindow();
            return;
        }

        ImGui::Checkbox("Clear target before restore", &ClearBeforeRestore);
        ImGui::Checkbox("Auto backup before overwrite / restore", &BackupBeforeOverwrite);

        if (ImGui::BeginTable("BackupsTable", 5, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV |
            ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchProp))
        {
            ImGui::TableSetupColumn("Backup", ImGuiTableColumnFlags_WidthStretch, 1.1f);
            ImGui::TableSetupColumn("Created", ImGuiTableColumnFlags_WidthFixed, 145.0f);
            ImGui::TableSetupColumn("Files", ImGuiTableColumnFlags_WidthFixed, 60.0f);
            ImGui::TableSetupColumn("Size", ImGuiTableColumnFlags_WidthFixed, 85.0f);
            ImGui::TableSetupColumn("MD5", ImGuiTableColumnFlags_WidthStretch, 1.0f);
            ImGui::TableHeadersRow();

            const auto& backups = SaveManager.Backups();
            for (int i = 0; i < static_cast<int>(backups.size()); ++i)
            {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                if (ImGui::Selectable(backups[i].Name.c_str(), i == SaveManager.SelectedBackup(), ImGuiSelectableFlags_SpanAllColumns))
                {
                    SaveManager.SetSelectedBackup(i);
                }
                ImGui::TableNextColumn();
                ImGui::TextUnformatted(backups[i].CreatedAtDisplay.c_str());
                ImGui::TableNextColumn();
                ImGui::Text("%zu", backups[i].FileCount);
                ImGui::TableNextColumn();
                ImGui::TextUnformatted(HumanSize(backups[i].TotalBytes).c_str());
                ImGui::TableNextColumn();
                ImGui::TextUnformatted(backups[i].BackupHash.c_str());
            }
            ImGui::EndTable();
        }

        EndRetroWindow();
    }

    void SaveManagerApp::Impl::DrawBackupDetailsWindow()
    {
        if (!Panels.BackupDetails)
        {
            return;
        }

        if (!BeginRetroWindow(ICON_FA_INFO_CIRCLE " Backup Details###Backup Details", &Panels.BackupDetails))
        {
            EndRetroWindow();
            return;
        }

        const BackupEntry* backup = SaveManager.CurrentBackup();
        if (backup == nullptr)
        {
            ImGui::TextDisabled("No backup selected.");
            EndRetroWindow();
            return;
        }

        ImGui::TextUnformatted(backup->Name.c_str());
        if (!backup->Reason.empty())
        {
            ImGui::TextDisabled("Reason");
            ImGui::SameLine();
            ImGui::TextWrapped("%s", backup->Reason.c_str());
        }

        ImGui::Spacing();
        if (ImGui::BeginTable("BackupMetaTable", 2, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_BordersInnerV))
        {
            ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, 90.0f);
            ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::TextDisabled("Hash");
            ImGui::TableNextColumn();
            if (backup->BackupHash.empty()) ImGui::TextDisabled("Unavailable");
            else ImGui::TextWrapped("%s", backup->BackupHash.c_str());

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::TextDisabled("Files");
            ImGui::TableNextColumn();
            ImGui::Text("%d", static_cast<int>(backup->PreviewFiles.size()));
            ImGui::EndTable();
        }

        ImGui::Spacing();
        DrawBackupActionButtons(*backup);
        ImGui::Separator();
        ImGui::TextDisabled("Preview");
        ImGui::SameLine();
        ImGui::Text("(%d files)", static_cast<int>(backup->PreviewFiles.size()));
        ImGui::Spacing();

        if (backup->PreviewFiles.empty())
        {
            ImGui::TextDisabled("No preview files available for this backup.");
            EndRetroWindow();
            return;
        }

        if (ImGui::BeginTable("PreviewTable", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
            ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_ScrollY, ImVec2(0.0f, 0.0f)))
        {
            ImGui::TableSetupColumn("Relative Path", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Size", ImGuiTableColumnFlags_WidthFixed, 100.0f);
            ImGui::TableHeadersRow();

            for (const auto& file : backup->PreviewFiles)
            {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::TextWrapped("%s", file.RelativePath.c_str());
                ImGui::TableNextColumn();
                ImGui::TextUnformatted(HumanSize(file.Size).c_str());
            }
            ImGui::EndTable();
        }

        EndRetroWindow();
    }

    void SaveManagerApp::Impl::DrawBackupActionButtons(const BackupEntry& backup)
    {
        const float buttonWidth = 120.0f;
        if (RetroButton(ICON_FA_UNDO " Restore", ImVec2(buttonWidth, 0.0f))) Popups.RestoreBackup = true;
        ImGui::SameLine();
        if (RetroButton(ICON_FA_FOLDER_OPEN " Open", ImVec2(buttonWidth, 0.0f)))
        {
            const bool ok = OpenInExplorer(backup.FullPath);
            AddLog(ok ? "Backup opened." : "Backup could not be opened.", ok ? Accent() : Bad());
        }
        ImGui::SameLine();
        if (RetroButton(ICON_FA_TRASH_ALT " Delete", ImVec2(buttonWidth, 0.0f))) Popups.DeleteBackup = true;
    }

    void SaveManagerApp::Impl::DrawActivityWindow()
    {
        if (!Panels.ActivityLog)
        {
            return;
        }

        if (!BeginRetroWindow(ICON_FA_STREAM " Activity Log###Activity Log", &Panels.ActivityLog))
        {
            EndRetroWindow();
            return;
        }

        for (const auto& entry : Log)
        {
            ImGui::TextColored(entry.Color, "%s", entry.Message.c_str());
        }

        EndRetroWindow();
    }

    void SaveManagerApp::Impl::ExecuteInstantProfileSwitch()
    {
        GameDefinition* game = SaveManager.CurrentGame();
        if (game == nullptr || game->Profiles.empty())
        {
            return;
        }

        int nextIndex = SaveManager.SelectedProfile() + 1;
        if (nextIndex >= static_cast<int>(game->Profiles.size()))
        {
            nextIndex = 0;
        }

        const auto result = SaveManager.SwitchToProfile(nextIndex, true);
        SyncEditorFromSelection();
        SaveManager.SaveConfig();
        AddLog(result.Message + " -> " + game->Profiles[nextIndex].Name, result.Success ? Good() : Bad());
    }
}
