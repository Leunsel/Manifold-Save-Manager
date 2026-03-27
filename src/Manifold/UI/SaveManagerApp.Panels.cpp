#include "SaveManagerApp.Internal.hpp"

namespace manifold
{
    namespace
    {
        void DrawEmptyState(const char* title, const char* description)
        {
            ImGui::Spacing();
            ImGui::TextDisabled("%s", title);
            ImGui::TextWrapped("%s", description);
        }

        void DrawStatusLine(const char* label, const char* value)
        {
            ImGui::TextDisabled("%s", label);
            ImGui::SameLine();
            ImGui::TextWrapped("%s", value);
        }
    }

    void SaveManagerApp::Impl::DrawNavigatorWindow()
    {
        if (!Panels.Navigator)
        {
            return;
        }

        if (!BeginRetroWindow(ICON_FA_GAMEPAD " Games###Navigator", &Panels.Navigator))
        {
            EndRetroWindow();
            return;
        }

        GameDefinition* game = SaveManager.CurrentGame();
        const GameProfile* profile = SaveManager.CurrentProfile();
        const bool hasGame = game != nullptr;

        if (RetroButton(ICON_FA_PLUS " New Game", ImVec2(120.0f, 0.0f)))
        {
            OpenCreateGameModal();
        }
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Create a new game entry");

        ImGui::SameLine();
        ImGui::BeginDisabled(!hasGame);
        if (RetroButton(ICON_FA_EDIT " Edit", ImVec2(90.0f, 0.0f)))
        {
            OpenEditGameModal();
        }
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Edit the selected game");

        ImGui::SameLine();
        if (RetroButton(ICON_FA_TRASH_ALT " Delete", ImVec2(90.0f, 0.0f)))
        {
            Popups.DeleteGame = true;
        }
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Delete the selected game");
        ImGui::EndDisabled();

        ImGui::SetNextItemWidth(-1.0f);
        ImGui::InputTextWithHint("##GameFilter", ICON_FA_SEARCH " Search games...", Filters.Game, IM_ARRAYSIZE(Filters.Game));
        ImGui::Separator();

        if (ImGui::BeginListBox("##GameList", ImVec2(-1.0f, -140.0f)))
        {
            const auto& games = SaveManager.Games();
            const std::string filter = ToLowerCopy(Trim(Filters.Game));

            for (int i = 0; i < static_cast<int>(games.size()); ++i)
            {
                const bool selected = i == SaveManager.SelectedGame();
                const std::string display = games[i].DisplayName.empty() ? games[i].Id : games[i].DisplayName;

                if (!filter.empty())
                {
                    const std::string haystack = ToLowerCopy(display + " " + games[i].Id + " " + games[i].SavePath);
                    if (haystack.find(filter) == std::string::npos)
                    {
                        continue;
                    }
                }

                std::string label = display;
                if (!games[i].Enabled)
                {
                    label += "  [Disabled]";
                }

                ImGui::PushID(i);
                if (ImGui::Selectable(label.c_str(), selected))
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

        ImGui::Separator();
        if (!hasGame)
        {
            DrawEmptyState("No game selected.", "Choose a game from the list or create a new one to start setting up profiles, rules, and backups.");
            EndRetroWindow();
            return;
        }

        const std::string displayName = game->DisplayName.empty() ? game->Id : game->DisplayName;
        DrawStatusLine("Selected:", displayName.c_str());
        DrawStatusLine("Game Id:", game->Id.c_str());
        DrawStatusLine("Save Path:", game->SavePath.empty() ? "Not configured" : game->SavePath.c_str());
        DrawStatusLine("Profiles:", std::to_string(game->Profiles.size()).c_str());
        DrawStatusLine("Active Profile:", profile != nullptr ? profile->Name.c_str() : "None");

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
            DrawEmptyState("No game selected.", "Pick a game first. Profiles belong to the currently selected game.");
            EndRetroWindow();
            return;
        }

        if (ImGui::BeginTable("ProfilesLayout", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchProp))
        {
            ImGui::TableSetupColumn("List", ImGuiTableColumnFlags_WidthStretch, 0.85f);
            ImGui::TableSetupColumn("Editor", ImGuiTableColumnFlags_WidthStretch, 1.35f);
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

        const float spacing = ImGui::GetStyle().ItemSpacing.x;
        const float avail = ImGui::GetContentRegionAvail().x;

        const float threeButtonWidth = (avail - spacing * 3.0f) / 3.0f;

        // Schalte bei wenig Platz auf reine Icons um.
        // Den Schwellenwert kannst du nach Geschmack anpassen.
        const bool compactButtons = threeButtonWidth < 110.0f;

        const char* newLabel = compactButtons ? ICON_FA_PLUS : ICON_FA_PLUS " New Profile";
        const char* renameLabel = compactButtons ? ICON_FA_EDIT : ICON_FA_EDIT " Rename";
        const char* activateLabel = compactButtons ? ICON_FA_EXCHANGE_ALT : ICON_FA_EXCHANGE_ALT " Activate";
        const char* removeLabel = compactButtons ? ICON_FA_TRASH_ALT : ICON_FA_TRASH_ALT " Remove";

        if (RetroButton(newLabel, ImVec2(threeButtonWidth, 0.0f)))
        {
            OpenCreateProfileModal();
        }
        if (compactButtons && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
        {
            ImGui::SetTooltip("New Profile");
        }

        ImGui::SameLine();

        ImGui::BeginDisabled(!hasProfile);
        if (RetroButton(renameLabel, ImVec2(threeButtonWidth, 0.0f)))
        {
            OpenEditProfileModal();
        }
        if (compactButtons && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
        {
            ImGui::SetTooltip("Rename");
        }

        ImGui::SameLine();

        if (RetroButton(activateLabel, ImVec2(threeButtonWidth, 0.0f)))
        {
            SyncSelectionFromEditor();
            SyncEditorFromSelection();
            SaveManager.SaveConfig();
            AddLog("Profile activated.", Good());
        }
        if (compactButtons && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
        {
            ImGui::SetTooltip("Activate");
        }
        ImGui::EndDisabled();

        ImGui::BeginDisabled(!canRemove);
        if (RetroButton(removeLabel, ImVec2(avail - spacing, 0.0f)))
        {
            SyncSelectionFromEditor();
            SaveManager.RemoveCurrentProfile();
            SyncEditorFromSelection();
            SaveManager.SaveConfig();
            AddLog("Profile removed.", Warn());
        }
        if (compactButtons && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
        {
            ImGui::SetTooltip("Remove");
        }
        ImGui::EndDisabled();

        ImGui::Separator();
        ImGui::TextDisabled("Profiles for this game");
        ImGui::Spacing();

        if (ImGui::BeginListBox("##ProfileListPane", ImVec2(-1.0f, -1.0f)))
        {
            for (int i = 0; i < static_cast<int>(game.Profiles.size()); ++i)
            {
                const GameProfile& profile = game.Profiles[i];
                const bool selected = i == SaveManager.SelectedProfile();

                std::string label;
                if (profile.Favorite)
                {
                    label += ICON_FA_STAR " ";
                }
                if (selected)
                {
                    label += "[Active] ";
                }
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
                    if (ImGui::MenuItem(ICON_FA_EDIT " Rename")) OpenEditProfileModal();
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
            DrawEmptyState("No profile selected.", "Choose a profile on the left to edit its behavior and notes.");
            return;
        }

        ImGui::TextDisabled("Profile Details");
        ImGui::Separator();

        ImGui::InputText("Profile Id", ProfileEditor.Id.data(), ProfileEditor.Id.size());
        ImGui::InputText("Name", ProfileEditor.Name.data(), ProfileEditor.Name.size());
        ImGui::InputText("Description", ProfileEditor.Description.data(), ProfileEditor.Description.size());

        ImGui::Spacing();
        ImGui::TextDisabled("Behavior");
        ImGui::Separator();
        ImGui::Checkbox("Mark as favorite", &profile->Favorite);
        ImGui::Checkbox("Create backup when switching to this profile", &profile->AutoBackupOnSwitch);
        ImGui::SliderInt("Keep backups", &profile->BackupLimit, 1, 100);

        ImGui::Spacing();
        ImGui::TextDisabled("Notes");
        ImGui::Separator();
        ImGui::InputTextMultiline("##ProfileNotes", ProfileEditor.Notes.data(), ProfileEditor.Notes.size(), ImVec2(-1.0f, 180.0f));

        ImGui::Spacing();
        if (RetroButton(ICON_FA_SAVE " Save Profile Changes", ImVec2(190.0f, 0.0f)))
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

        if (!BeginRetroWindow(ICON_FA_COG " Game Setup###Game Config", &Panels.GameConfig))
        {
            EndRetroWindow();
            return;
        }

        GameDefinition* game = SaveManager.CurrentGame();
        if (game == nullptr)
        {
            DrawEmptyState("No game selected.", "Create or pick a game to configure save path, process name, and save behavior.");
            if (RetroButton(ICON_FA_PLUS " Create Game", ImVec2(-1.0f, 0.0f))) OpenCreateGameModal();
            EndRetroWindow();
            return;
        }

        ImGui::Checkbox("Enabled", &game->Enabled);
        ImGui::InputText("Game Id", GameEditor.Id.data(), GameEditor.Id.size());
        ImGui::InputText("Display Name", GameEditor.Name.data(), GameEditor.Name.size());
        ImGui::InputText("Process Name", GameEditor.ProcessName.data(), GameEditor.ProcessName.size());

        ImGui::TextDisabled("Save Folder");
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

        const char* scopeNames[] = { "File Whitelist", "Folder Mode", "Hybrid Mode" };
        int scopeIndex = static_cast<int>(game->ScopeMode);
        if (ImGui::Combo("Save Scope", &scopeIndex, scopeNames, IM_ARRAYSIZE(scopeNames)))
        {
            game->ScopeMode = static_cast<SaveScopeMode>(scopeIndex);
        }

        ImGui::TextDisabled("Game Notes");
        ImGui::InputTextMultiline("##GameNotes", GameEditor.Notes.data(), GameEditor.Notes.size(), ImVec2(-1.0f, 160.0f));

        ImGui::Separator();
        DrawStatusLine("Folder Status:", DirectoryExists(GameEditor.SavePath.data()) ? "Available" : "Missing");
        DrawStatusLine("Scope Mode:", ToString(game->ScopeMode));

        if (RetroButton(ICON_FA_FOLDER_OPEN " Open Save Folder", ImVec2(180.0f, 0.0f)))
        {
            const bool ok = OpenInExplorer(GameEditor.SavePath.data());
            AddLog(ok ? "Save path opened." : "Save path could not be opened.", ok ? Accent() : Bad());
        }
        ImGui::SameLine();
        if (RetroButton(ICON_FA_SAVE " Save Game Setup", ImVec2(180.0f, 0.0f)))
        {
            SyncSelectionFromEditor();
            const bool ok = SaveManager.SaveConfig();
            AddLog(ok ? "Game configuration updated." : "Game configuration could not be saved.", ok ? Good() : Bad());
        }

        EndRetroWindow();
    }

    void SaveManagerApp::Impl::DrawScopeRulesWindow()
    {
        if (!Panels.ScopeRules)
        {
            return;
        }

        if (!BeginRetroWindow(ICON_FA_FILTER " Save Rules###Scope Rules", &Panels.ScopeRules))
        {
            EndRetroWindow();
            return;
        }

        GameDefinition* game = SaveManager.CurrentGame();
        if (game == nullptr)
        {
            DrawEmptyState("No game selected.", "Pick a game first. Rules define which files and folders belong to that game's save data.");
            EndRetroWindow();
            return;
        }

        static char newFileRule[512] = {};
        static char newFolderRule[512] = {};

        DrawStatusLine("Current Mode:", ToString(game->ScopeMode));
        ImGui::Separator();

        ImGui::TextDisabled("Files to include");
        ImGui::SetNextItemWidth(-120.0f);
        ImGui::InputTextWithHint("##NewFileRule", "Example: SaveData.sav", newFileRule, IM_ARRAYSIZE(newFileRule));
        ImGui::SameLine();
        if (RetroButton(ICON_FA_PLUS " Add File", ImVec2(110.0f, 0.0f)))
        {
            const std::string value = Trim(newFileRule);
            if (!value.empty())
            {
                game->Whitelist.push_back({ value, true });
                newFileRule[0] = '\0';
                SaveManager.SaveConfig();
                AddLog("File whitelist rule added.", Accent());
            }
            else
            {
                AddLog("File rule must not be empty.", Bad());
            }
        }

        DrawScopeRuleTable(game->Whitelist, "WhitelistTable", 170.0f);

        ImGui::Spacing();
        ImGui::TextDisabled("Folders to include");
        ImGui::SetNextItemWidth(-120.0f);
        ImGui::InputTextWithHint("##NewFolderRule", "Example: Saves\\Slot1", newFolderRule, IM_ARRAYSIZE(newFolderRule));
        ImGui::SameLine();
        if (RetroButton(ICON_FA_PLUS " Add Folder", ImVec2(110.0f, 0.0f)))
        {
            const std::string value = Trim(newFolderRule);
            if (!value.empty())
            {
                game->FolderRules.push_back({ value, true });
                newFolderRule[0] = '\0';
                SaveManager.SaveConfig();
                AddLog("Folder rule added.", Accent());
            }
            else
            {
                AddLog("Folder rule must not be empty.", Bad());
            }
        }

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
            ImGui::TableSetupColumn("Action", ImGuiTableColumnFlags_WidthFixed, 90.0f);
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

        if (!BeginRetroWindow(ICON_FA_ARCHIVE " Backup Center###Backups", &Panels.Backups))
        {
            EndRetroWindow();
            return;
        }

        const GameProfile* profile = SaveManager.CurrentProfile();
        if (profile == nullptr)
        {
            DrawEmptyState(
                "No profile selected.",
                "Choose a game and profile first. Backups are created and restored for the active profile.");
            EndRetroWindow();
            return;
        }

        // Top actions
        if (RetroButton(ICON_FA_ARCHIVE " Create Backup Now", ImVec2(190.0f, 0.0f)))
        {
            SyncSelectionFromEditor();
            const auto result = SaveManager.CreateBackupForCurrentProfile("Manual backup");
            AddLog(result.Message, result.Success ? Good() : Bad());
        }

        ImGui::Spacing();
        ImGui::Checkbox("Clear target before restore", &ClearBeforeRestore);
        ImGui::Checkbox("Auto backup before overwrite / restore", &BackupBeforeOverwrite);

        ImGui::Separator();

        // Backup list
        ImGui::TextDisabled("Available Backups");
        if (ImGui::BeginTable(
            "BackupsTable",
            4,
            ImGuiTableFlags_RowBg |
            ImGuiTableFlags_BordersInnerV |
            ImGuiTableFlags_ScrollY |
            ImGuiTableFlags_Resizable |
            ImGuiTableFlags_SizingStretchProp,
            ImVec2(0.0f, 180.0f)))
        {
            ImGui::TableSetupColumn("Backup", ImGuiTableColumnFlags_WidthStretch, 1.10f);
            ImGui::TableSetupColumn("Created", ImGuiTableColumnFlags_WidthFixed, 145.0f);
            ImGui::TableSetupColumn("Files", ImGuiTableColumnFlags_WidthFixed, 60.0f);
            ImGui::TableSetupColumn("Size", ImGuiTableColumnFlags_WidthFixed, 85.0f);
            ImGui::TableHeadersRow();

            const auto& backups = SaveManager.Backups();
            for (int i = 0; i < static_cast<int>(backups.size()); ++i)
            {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();

                if (ImGui::Selectable(
                    backups[i].Name.c_str(),
                    i == SaveManager.SelectedBackup(),
                    ImGuiSelectableFlags_SpanAllColumns))
                {
                    SaveManager.SetSelectedBackup(i);
                }

                ImGui::TableNextColumn();
                ImGui::TextUnformatted(backups[i].CreatedAtDisplay.c_str());

                ImGui::TableNextColumn();
                ImGui::Text("%zu", backups[i].FileCount);

                ImGui::TableNextColumn();
                ImGui::TextUnformatted(HumanSize(backups[i].TotalBytes).c_str());
            }

            ImGui::EndTable();
        }

        ImGui::Separator();

        // Backup details
        const BackupEntry* backup = SaveManager.CurrentBackup();
        if (backup == nullptr)
        {
            DrawEmptyState(
                "No backup selected.",
                "Select a backup above to review its details, preview its files, or restore it.");
            EndRetroWindow();
            return;
        }

        ImGui::TextDisabled("Selected Backup");
        ImGui::TextUnformatted(backup->Name.c_str());

        if (!backup->Reason.empty())
        {
            DrawStatusLine("Reason:", backup->Reason.c_str());
        }

        DrawStatusLine("Hash:", backup->BackupHash.empty() ? "Unavailable" : backup->BackupHash.c_str());
        DrawStatusLine("Files:", std::to_string(backup->PreviewFiles.size()).c_str());
        DrawStatusLine("Location:", backup->FullPath.c_str());

        ImGui::Spacing();
        DrawBackupActionButtons(*backup);

        ImGui::Separator();

        // Preview files
        ImGui::TextDisabled("Preview Files");

        if (backup->PreviewFiles.empty())
        {
            ImGui::TextDisabled("No preview files available for this backup.");
        }
        else if (ImGui::BeginTable(
            "PreviewTable",
            2,
            ImGuiTableFlags_Borders |
            ImGuiTableFlags_RowBg |
            ImGuiTableFlags_SizingStretchProp |
            ImGuiTableFlags_ScrollY,
            ImVec2(0.0f, 0.0f)))
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

    // void SaveManagerApp::Impl::DrawBackupDetailsWindow()
    // {
    //     if (!Panels.BackupDetails)
    //     {
    //         return;
    //     }
    // 
    //     if (!BeginRetroWindow(ICON_FA_INFO_CIRCLE " Backup Details###Backup Details", &Panels.BackupDetails))
    //     {
    //         EndRetroWindow();
    //         return;
    //     }
    // 
    //     const BackupEntry* backup = SaveManager.CurrentBackup();
    //     if (backup == nullptr)
    //     {
    //         DrawEmptyState("No backup selected.", "Use the Backup Center to select a backup. This panel now acts as a compact detail view.");
    //         EndRetroWindow();
    //         return;
    //     }
    // 
    //     ImGui::TextUnformatted(backup->Name.c_str());
    //     DrawStatusLine("Created:", backup->CreatedAtDisplay.c_str());
    //     DrawStatusLine("Hash:", backup->BackupHash.empty() ? "Unavailable" : backup->BackupHash.c_str());
    //     DrawStatusLine("Files:", std::to_string(backup->PreviewFiles.size()).c_str());
    //     DrawStatusLine("Size:", HumanSize(backup->TotalBytes).c_str());
    // 
    //     if (!backup->Reason.empty())
    //     {
    //         DrawStatusLine("Reason:", backup->Reason.c_str());
    //     }
    // 
    //     ImGui::Spacing();
    //     DrawBackupActionButtons(*backup);
    //     EndRetroWindow();
    // }

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

        if (RetroButton(ICON_FA_TRASH_ALT " Clear Log", ImVec2(120.0f, 0.0f)))
        {
            Log.clear();
        }

        ImGui::Separator();
        if (ImGui::BeginChild("##ActivityLogScroll", ImVec2(0.0f, 0.0f), false, ImGuiWindowFlags_HorizontalScrollbar))
        {
            for (const auto& entry : Log)
            {
                ImGui::TextColored(entry.Color, "%s", entry.Message.c_str());
            }

            if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY() || ImGui::GetScrollMaxY() <= 0.0f)
            {
                ImGui::SetScrollHereY(1.0f);
            }
            ImGui::EndChild();
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
