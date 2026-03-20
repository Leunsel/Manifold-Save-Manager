
#include "Theme.hpp"
#include "../Core/Math.hpp"
#include "../Core/Utils.hpp"
#include "SaveManagerApp.hpp"
#include "../Core/SaveManager.hpp"

#include <array>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <windows.h>
#include <shellapi.h>

#include "../Font/IconsFontAwesome5.hpp"
#include "../Font/Font.hpp"

#include "../../Imgui/imgui.h"
#include "../../ImGui/imgui_internal.h"

namespace manifold
{
    struct GameEditorState
    {
        std::array<char, 128>  Id{};
        std::array<char, 128>  Name{};
        std::array<char, 512>  SavePath{};
        std::array<char, 128>  ProcessName{};
        std::array<char, 2048> Notes{};
    };

    struct ProfileEditorState
    {
        std::array<char, 128>  Id{};
        std::array<char, 128>  Name{};
        std::array<char, 256>  Description{};
        std::array<char, 2048> Notes{};
        std::array<char, 260>  ScopeRule{};
    };

    struct GameModalState
    {
        std::array<char, 128>  Id{};
        std::array<char, 128>  Name{};
        std::array<char, 512>  SavePath{};
        std::array<char, 128>  ProcessName{};
        std::array<char, 2048> Notes{};
        bool Enabled = true;
        int ScopeMode = 0;
    };

    struct ProfileModalState
    {
        std::array<char, 128>  Id{};
        std::array<char, 128>  Name{};
        std::array<char, 256>  Description{};
        std::array<char, 2048> Notes{};
        bool Favorite = false;
        bool AutoBackup = true;
        int BackupLimit = 20;
    };

    struct SaveManagerApp::Impl
    {
        explicit Impl(HWND hwnd)
            : MainWindow(hwnd)
        {
        }

        HWND MainWindow = nullptr;
        SaveManager SaveManager;

        bool ClearBeforeRestore = true;
        bool BackupBeforeOverwrite = true;
        bool DockLayoutBuilt = false;

        bool ShowNavigator = true;
        bool ShowProfiles = true;
        bool ShowGameConfig = true;
        bool ShowScopeRules = true;
        bool ShowBackups = true;
        bool ShowBackupDetails = true;
        bool ShowActivityLog = true;

        bool OpenAboutPopup = false;
        bool OpenProfileModal = false;
        bool OpenGameModal = false;
        bool OpenRestoreModal = false;

        bool OpenDeleteGamePopup = false;
        bool OpenDeleteProfilePopup = false;
        char GameFilter[128]{};
        char BackupFilter[128]{};

        int ProfileModalMode = 0;
        int GameModalMode = 0;

        std::vector<LogEntry> Log;

        GameEditorState GameEditor;
        ProfileEditorState ProfileEditor;
        GameModalState GameModal;
        ProfileModalState ProfileModal;

        void Render();

        void DrawRootWindow();
        void InitializeDefaultDockLayout(ImGuiID dockspaceId);
        void DrawMainMenuBar();

        void DrawNavigatorWindow();
        void DrawDeleteGamePopup();
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
        void DrawAboutPopup();
        void DrawDeleteBackupPopup();

        void DrawAboutCubeAnimation(float cubeSize);
        void DrawAboutCubeAnimation2(float cubeSize);

        void OpenCreateGameModal();
        void OpenEditGameModal();
        void OpenCreateProfileModal();
        void OpenEditProfileModal();
        void ExecuteInstantProfileSwitch();

        void LoadSelectedIntoEditor();
        void SaveEditorIntoSelected();
        void ClearGameEditor();
        void ClearProfileEditor();

        void LoadManifoldFonts();

        void AddLog(std::string message, ImVec4 color);

        static ImVec4 Good();
        static ImVec4 Warn();
        static ImVec4 Bad();
        static ImVec4 Accent();
    };

    SaveManagerApp::SaveManagerApp(HWND hwnd) : m_Impl(std::make_unique<Impl>(hwnd))
    {
        m_Impl->LoadManifoldFonts();
        ApplyManifoldTheme();

        m_Impl->SaveManager.Load();
        m_Impl->LoadSelectedIntoEditor();
        m_Impl->AddLog("SaveManager state loaded.", Impl::Accent());
    }

    SaveManagerApp::~SaveManagerApp() = default;
    SaveManagerApp::SaveManagerApp(SaveManagerApp&&) noexcept = default;
    SaveManagerApp& SaveManagerApp::operator=(SaveManagerApp&&) noexcept = default;

    void SaveManagerApp::Render()
    {
        m_Impl->Render();
    }

    void SaveManagerApp::Impl::Render()
    {
        DrawRootWindow();
    }

    void SaveManagerApp::Impl::AddLog(std::string message, ImVec4 color)
    {
        const std::string stamped = '[' + FormatTime(std::time(nullptr)) + "] " + std::move(message);
        Log.push_back({ stamped, color });
        if (Log.size() > 150)
            Log.erase(Log.begin(), Log.begin() + static_cast<long long>(Log.size() - 150));
    }

    ImVec4 SaveManagerApp::Impl::Good()
    {
        return ImVec4(0.25f, 0.85f, 0.45f, 1.0f);
    }

    ImVec4 SaveManagerApp::Impl::Warn()
    {
        return ImVec4(0.95f, 0.75f, 0.20f, 1.0f);
    }

    ImVec4 SaveManagerApp::Impl::Bad()
    {
        return ImVec4(0.92f, 0.28f, 0.25f, 1.0f);
    }

    ImVec4 SaveManagerApp::Impl::Accent()
    {
        return ImVec4(0.18f, 0.74f, 0.82f, 1.0f);
    }

    void SaveManagerApp::Impl::DrawRootWindow()
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
        DrawDeleteGamePopup();
        DrawDeleteBackupPopup();
    }

    void SaveManagerApp::Impl::InitializeDefaultDockLayout(ImGuiID dockspaceId)
    {
        if (DockLayoutBuilt) return;
        DockLayoutBuilt = true;

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

    void SaveManagerApp::Impl::DrawMainMenuBar()
    {
        if (!ImGui::BeginMenuBar()) return;

        if (ImGui::BeginMenu(ICON_FA_FILE_ALT " File"))
        {
            if (ImGui::MenuItem(ICON_FA_SAVE " Save Config"))
            {
                SaveEditorIntoSelected();
                const bool ok = SaveManager.SaveConfig();
                AddLog(ok ? "Configuration saved." : "Configuration could not be saved.", ok ? Good() : Bad());
            }

            if (ImGui::MenuItem(ICON_FA_SYNC " Refresh"))
            {
                SaveEditorIntoSelected();
                SaveManager.Load();
                LoadSelectedIntoEditor();
                AddLog("Configuration and backups refreshed.", Accent());
            }

            ImGui::Separator();

            if (ImGui::MenuItem(ICON_FA_SIGN_OUT_ALT " Exit"))
                PostMessage(MainWindow, WM_CLOSE, 0, 0);

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu(ICON_FA_GAMEPAD " Game"))
        {
            if (ImGui::MenuItem(ICON_FA_PLUS " Add Game"))
                OpenCreateGameModal();

            if (ImGui::MenuItem(ICON_FA_EDIT " Edit Game", nullptr, false, SaveManager.CurrentGame() != nullptr))
                OpenEditGameModal();

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu(ICON_FA_USERS " Profile"))
        {
            if (ImGui::MenuItem(ICON_FA_PLUS " Add Profile", nullptr, false, SaveManager.CurrentGame() != nullptr))
                OpenCreateProfileModal();

            if (ImGui::MenuItem(ICON_FA_EDIT " Edit Profile", nullptr, false, SaveManager.CurrentProfile() != nullptr))
                OpenEditProfileModal();

            if (ImGui::MenuItem(ICON_FA_EXCHANGE_ALT " Instant Switch", nullptr, false, SaveManager.CurrentGame() != nullptr))
                ExecuteInstantProfileSwitch();

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu(ICON_FA_ARCHIVE " Backup"))
        {
            if (ImGui::MenuItem(ICON_FA_SAVE " Create Backup", nullptr, false, SaveManager.CurrentProfile() != nullptr))
            {
                SaveEditorIntoSelected();
                const auto result = SaveManager.CreateBackupForCurrentProfile("Manual backup");
                AddLog(result.Message, result.Success ? Good() : Bad());
            }

            if (ImGui::MenuItem(ICON_FA_UNDO " Restore Selected", nullptr, false, SaveManager.CurrentBackup() != nullptr))
                OpenRestoreModal = true;

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu(ICON_FA_EYE " View"))
        {
            ImGui::MenuItem(ICON_FA_GAMEPAD " Navigator", nullptr, &ShowNavigator);
            ImGui::MenuItem(ICON_FA_USERS " Profiles", nullptr, &ShowProfiles);
            ImGui::MenuItem(ICON_FA_COG " Game Config", nullptr, &ShowGameConfig);
            ImGui::MenuItem(ICON_FA_FILTER " Scope Rules", nullptr, &ShowScopeRules);
            ImGui::MenuItem(ICON_FA_ARCHIVE " Backups", nullptr, &ShowBackups);
            ImGui::MenuItem(ICON_FA_INFO_CIRCLE " Backup Details", nullptr, &ShowBackupDetails);
            ImGui::MenuItem(ICON_FA_STREAM " Activity Log", nullptr, &ShowActivityLog);
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu(ICON_FA_QUESTION_CIRCLE " Help"))
        {
            if (ImGui::MenuItem(ICON_FA_INFO_CIRCLE " About"))
                OpenAboutPopup = true;

            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

    void SaveManagerApp::Impl::DrawNavigatorWindow()
    {
        if (!ShowNavigator) return;
        ImGui::Begin(ICON_FA_GAMEPAD " Navigator###Navigator", &ShowNavigator);

        const bool hasGame = SaveManager.CurrentGame() != nullptr;

        if (ImGui::Button(ICON_FA_PLUS "##AddGame"))
            OpenCreateGameModal();
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Create game");

        ImGui::SameLine();

        ImGui::BeginDisabled(!hasGame);
        if (ImGui::Button(ICON_FA_EDIT "##EditGame"))
            OpenEditGameModal();
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Edit selected game");

        ImGui::SameLine();

        if (ImGui::Button(ICON_FA_TRASH_ALT "##DeleteGame"))
            OpenDeleteGamePopup = true;
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Delete selected game");
        ImGui::EndDisabled();

        ImGui::SameLine();
        ImGui::SetNextItemWidth(-1.0f);
        ImGui::InputTextWithHint("##GameFilter", ICON_FA_SEARCH " Filter games...", GameFilter, IM_ARRAYSIZE(GameFilter));

        ImGui::Separator();

        if (ImGui::BeginListBox("##GameList", ImVec2(-1.0f, -100.0f)))
        {
            const auto& games = SaveManager.Games();
            const std::string filter = Trim(GameFilter);

            for (int i = 0; i < static_cast<int>(games.size()); ++i)
            {
                const bool selected = i == SaveManager.SelectedGame();
                const std::string display = games[i].DisplayName.empty() ? games[i].Id : games[i].DisplayName;

                if (!filter.empty())
                {
                    std::string hay = ToLowerCopy(display + " " + games[i].Id + " " + games[i].SavePath);
                    if (hay.find(ToLowerCopy(filter)) == std::string::npos)
                        continue;
                }

                ImGui::PushID(i);
                if (ImGui::Selectable(display.c_str(), selected))
                {
                    SaveEditorIntoSelected();
                    SaveManager.SetSelectedGame(i);
                    LoadSelectedIntoEditor();
                }

                if (ImGui::BeginPopupContextItem("GameContext"))
                {
                    if (ImGui::MenuItem(ICON_FA_EDIT " Edit"))
                        OpenEditGameModal();

                    if (ImGui::MenuItem(ICON_FA_TRASH_ALT " Delete"))
                        OpenDeleteGamePopup = true;

                    ImGui::Separator();

                    if (ImGui::MenuItem(ICON_FA_ARCHIVE " Create Backup", nullptr, false, SaveManager.CurrentProfile() != nullptr))
                    {
                        SaveEditorIntoSelected();
                        auto result = SaveManager.CreateBackupForCurrentProfile("Manual backup");
                        AddLog(result.Message, result.Success ? Good() : Bad());
                    }

                    ImGui::EndPopup();
                }
                ImGui::PopID();
            }

            ImGui::EndListBox();
        }

        const GameDefinition* game = SaveManager.CurrentGame();
        const GameProfile* profile = SaveManager.CurrentProfile();

        ImGui::Separator();
        ImGui::TextDisabled("Selection");
        // if (game)
        // {
        //     ImGui::TextUnformatted((game->DisplayName.empty() ? game->Id : game->DisplayName).c_str());
        //     ImGui::TextDisabled("%s", game->SavePath.empty() ? "No save path configured" : game->SavePath.c_str());
        // }
        if (profile)
            ImGui::TextDisabled("Active Profile: %s", profile->Name.c_str());

        DrawDeleteGamePopup();

        ImGui::End();
    }

    void SaveManagerApp::Impl::DrawDeleteGamePopup()
    {
        if (OpenDeleteGamePopup)
        {
            ImGui::OpenPopup("DeleteGameConfirm");
            OpenDeleteGamePopup = false;
        }

        if (!ImGui::BeginPopupModal("DeleteGameConfirm", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
            return;

        const GameDefinition* game = SaveManager.CurrentGame();
        if (!game)
        {
            ImGui::TextDisabled("No game selected.");
        }
        else
        {
            const std::string label = game->DisplayName.empty() ? game->Id : game->DisplayName;
            ImGui::Text("Delete game '%s'?", label.c_str());
            ImGui::TextDisabled("This removes the configuration entry from Manifold.");
        }

        ImGui::Spacing();

        if (ImGui::Button(ICON_FA_TRASH_ALT " Delete", ImVec2(140.0f, 0.0f)))
        {
            SaveManager.RemoveCurrentGame();
            LoadSelectedIntoEditor();
            AddLog("Game removed.", Warn());
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        if (ImGui::Button(ICON_FA_TIMES " Cancel", ImVec2(140.0f, 0.0f)))
            ImGui::CloseCurrentPopup();

        ImGui::EndPopup();
    }

    void SaveManagerApp::Impl::DrawProfilesWindow()
    {
        if (!ShowProfiles) return;
        ImGui::Begin(ICON_FA_USERS " Profiles###Profiles", &ShowProfiles);

        GameDefinition* game = SaveManager.CurrentGame();
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

    void SaveManagerApp::Impl::DrawProfileListPane(GameDefinition& game)
    {
        const bool hasProfile = SaveManager.CurrentProfile() != nullptr;
        const bool canRemove = game.Profiles.size() > 1 && hasProfile;

        if (ImGui::Button(ICON_FA_PLUS "##AddProfile"))
            OpenCreateProfileModal();
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Create profile");

        ImGui::SameLine();

        ImGui::BeginDisabled(!hasProfile);
        if (ImGui::Button(ICON_FA_EDIT "##EditProfile"))
            OpenEditProfileModal();
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Edit selected profile");

        ImGui::SameLine();

        ImGui::BeginDisabled(!canRemove);
        if (ImGui::Button(ICON_FA_TRASH_ALT "##RemoveProfile"))
        {
            SaveEditorIntoSelected();
            SaveManager.RemoveCurrentProfile();
            LoadSelectedIntoEditor();
            SaveManager.SaveConfig();
            AddLog("Profile removed.", Warn());
        }
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Remove selected profile");
        ImGui::EndDisabled();
        ImGui::EndDisabled();

        ImGui::Separator();

        if (ImGui::BeginListBox("##ProfileListPane", ImVec2(-1.0f, -1.0f)))
        {
            for (int i = 0; i < static_cast<int>(game.Profiles.size()); ++i)
            {
                const GameProfile& profile = game.Profiles[i];
                const bool selected = i == SaveManager.SelectedProfile();

                std::string label;
                if (profile.Favorite)
                    label += ICON_FA_STAR " ";
                label += profile.Name;
                label += "##";
                label += profile.Id;

                ImGui::PushID(i);

                if (ImGui::Selectable(label.c_str(), selected))
                {
                    SaveEditorIntoSelected();
                    SaveManager.SetSelectedProfile(i);
                    LoadSelectedIntoEditor();
                }

                if (ImGui::BeginPopupContextItem("ProfileContext"))
                {
                    if (ImGui::MenuItem(ICON_FA_EDIT " Edit"))
                        OpenEditProfileModal();

                    if (ImGui::MenuItem(ICON_FA_EXCHANGE_ALT " Activate"))
                    {
                        SaveEditorIntoSelected();
                        SaveManager.SetSelectedProfile(i);
                        LoadSelectedIntoEditor();
                        SaveManager.SaveConfig();
                        AddLog("Profile activated.", Good());
                    }

                    ImGui::Separator();

                    if (ImGui::MenuItem(ICON_FA_TRASH_ALT " Remove", nullptr, false, game.Profiles.size() > 1))
                    {
                        SaveEditorIntoSelected();
                        SaveManager.SetSelectedProfile(i);
                        SaveManager.RemoveCurrentProfile();
                        LoadSelectedIntoEditor();
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
        if (!profile)
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

        if (ImGui::Button("Apply Profile Changes", ImVec2(-1.0f, 0.0f)))
        {
            SaveEditorIntoSelected();
            const bool ok = SaveManager.SaveConfig();
            AddLog(ok ? "Profile updated." : "Profile could not be saved.", ok ? Good() : Bad());
        }
    }

    void SaveManagerApp::Impl::DrawGameConfigWindow()
    {
        if (!ShowGameConfig) return;
        ImGui::Begin(ICON_FA_COG " Game Config###Game Config", &ShowGameConfig);

        GameDefinition* game = SaveManager.CurrentGame();
        if (!game)
        {
            ImGui::TextDisabled("No game selected.");
            if (ImGui::Button("Create Game", ImVec2(-1.0f, 0.0f))) OpenCreateGameModal();
            ImGui::End();
            return;
        }

        ImGui::Checkbox("Enabled", &game->Enabled);
        ImGui::InputText("Game Id", GameEditor.Id.data(), GameEditor.Id.size());
        ImGui::InputText("Display Name", GameEditor.Name.data(), GameEditor.Name.size());

        ImGui::TextUnformatted("Save Path");
        ImGui::SetNextItemWidth(-110.0f);
        ImGui::InputText("##SavePath", GameEditor.SavePath.data(), GameEditor.SavePath.size());
        ImGui::SameLine();
        if (ImGui::Button("Browse", ImVec2(100.0f, 0.0f)))
            if (auto folder = PickFolderDialog(MainWindow, L"Select Save Folder")) WriteToBuffer(GameEditor.SavePath, *folder);

        ImGui::InputText("Process Name", GameEditor.ProcessName.data(), GameEditor.ProcessName.size());
        ImGui::InputTextMultiline("Game Notes", GameEditor.Notes.data(), GameEditor.Notes.size(), ImVec2(-100.0f, 180.0f));

        const char* scopeNames[] = { "File Whitelist", "Folder Mode", "Hybrid Mode" };
        int scopeIndex = static_cast<int>(game->ScopeMode);
        if (ImGui::Combo("Save Scope", &scopeIndex, scopeNames, IM_ARRAYSIZE(scopeNames)))
            game->ScopeMode = static_cast<SaveScopeMode>(scopeIndex);

        ImGui::Spacing();
        if (ImGui::Button(ICON_FA_FOLDER_OPEN " Open Folder", ImVec2(160.0f, 0.0f)))
        {
            const bool ok = OpenInExplorer(GameEditor.SavePath.data());
            AddLog(ok ? "Save path opened." : "Save path could not be opened.", ok ? Accent() : Bad());
        }

        ImGui::SameLine();

        if (ImGui::Button(ICON_FA_SAVE " Save Changes", ImVec2(170.0f, 0.0f)))
        {
            SaveEditorIntoSelected();
            const bool ok = SaveManager.SaveConfig();
            AddLog(ok ? "Game configuration updated." : "Game configuration could not be saved.", ok ? Good() : Bad());
        }

        ImGui::Separator();
        ImGui::BulletText("Save Path: %s", DirectoryExists(GameEditor.SavePath.data()) ? "OK" : "Missing");
        ImGui::End();
    }

    void SaveManagerApp::Impl::DrawScopeRulesWindow()
    {
        if (!ShowScopeRules) return;

        ImGui::Begin(ICON_FA_FILTER " Scope Rules###Scope Rules", &ShowScopeRules);

        GameDefinition* game = SaveManager.CurrentGame();
        if (!game)
        {
            ImGui::TextDisabled("No game selected.");
            ImGui::Separator();
            ImGui::TextDisabled("Select a game to manage whitelist and folder rules.");
            ImGui::End();
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
        ImGui::InputTextWithHint(
            "##NewScopeRule",
            "Relative path, e.g. SaveData.sav or Saves\\Slot1",
            ProfileEditor.ScopeRule.data(),
            ProfileEditor.ScopeRule.size());

        ImGui::SameLine();

        if (ImGui::Button(ICON_FA_PLUS " Add", ImVec2(90.0f, 0.0f)))
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

        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Add the entered rule to the selected rule group");

        ImGui::Spacing();
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

        ImGui::End();
    }

    void SaveManagerApp::Impl::DrawScopeRuleTable(std::vector<ScopeRule>& rules, const char* id, float height)
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
                SaveManager.SaveConfig();
                AddLog("Scope rule removed.", Warn());
            }
            ImGui::EndTable();
        }
    }

    void SaveManagerApp::Impl::DrawBackupsWindow()
    {
        if (!ShowBackups) return;
        ImGui::Begin(ICON_FA_ARCHIVE " Backups###Backups", &ShowBackups);
        ImGui::Checkbox("Clear target before restore", &ClearBeforeRestore);
        ImGui::Checkbox("Auto backup before overwrite / restore", &BackupBeforeOverwrite);

        if (ImGui::BeginTable("BackupsTable", 5, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchProp))
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
                    SaveManager.SetSelectedBackup(i);
                ImGui::TableNextColumn(); ImGui::TextUnformatted(backups[i].CreatedAtDisplay.c_str());
                ImGui::TableNextColumn(); ImGui::Text("%zu", backups[i].FileCount);
                ImGui::TableNextColumn(); ImGui::TextUnformatted(HumanSize(backups[i].TotalBytes).c_str());
                ImGui::TableNextColumn(); ImGui::TextUnformatted(backups[i].BackupHash.c_str());
            }
            ImGui::EndTable();
        }
        ImGui::End();
    }

    void SaveManagerApp::Impl::DrawBackupDetailsWindow()
    {
        if (!ShowBackupDetails) return;

        ImGui::Begin(ICON_FA_INFO_CIRCLE " Backup Details###Backup Details", &ShowBackupDetails);

        const BackupEntry* backup = SaveManager.CurrentBackup();
        if (!backup)
        {
            ImGui::TextDisabled("No backup selected.");
            ImGui::Separator();
            ImGui::TextDisabled("Select a backup from the Backups window to inspect its details.");
            ImGui::End();
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
            // ImGui::TextDisabled("Path");
            // ImGui::TableNextColumn();
            // const std::string displayPath = StripPathForDisplay(backup->FullPath);
            // ImGui::TextWrapped("%s", displayPath.c_str());
            // if (ImGui::IsItemHovered())
            //     ImGui::SetTooltip("%s", backup->FullPath.c_str());

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::TextDisabled("Hash");
            ImGui::TableNextColumn();
            if (backup->BackupHash.empty())
                ImGui::TextDisabled("Unavailable");
            else
                ImGui::TextWrapped("%s", backup->BackupHash.c_str());

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::TextDisabled("Files");
            ImGui::TableNextColumn();
            ImGui::Text("%d", static_cast<int>(backup->PreviewFiles.size()));

            ImGui::EndTable();
        }

        ImGui::Spacing();
        DrawBackupActionButtons(*backup);
        DrawDeleteBackupPopup();

        ImGui::Separator();

        ImGui::TextDisabled("Preview");
        ImGui::SameLine();
        ImGui::Text("(%d files)", static_cast<int>(backup->PreviewFiles.size()));

        ImGui::Spacing();

        if (backup->PreviewFiles.empty())
        {
            ImGui::TextDisabled("No preview files available for this backup.");
            ImGui::End();
            return;
        }

        if (ImGui::BeginTable("PreviewTable", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_ScrollY, ImVec2(0.0f, 0.0f)))
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

        ImGui::End();
    }

    void SaveManagerApp::Impl::DrawBackupActionButtons(const BackupEntry& backup)
    {
        const float buttonWidth = 120.0f;

        if (ImGui::Button(ICON_FA_UNDO " Restore", ImVec2(buttonWidth, 0.0f)))
            OpenRestoreModal = true;
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Restore the selected backup");

        ImGui::SameLine();

        if (ImGui::Button(ICON_FA_FOLDER_OPEN " Open", ImVec2(buttonWidth, 0.0f)))
        {
            const bool ok = OpenInExplorer(backup.FullPath);
            AddLog(ok ? "Backup opened." : "Backup could not be opened.", ok ? Accent() : Bad());
        }
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Open the backup folder");

        ImGui::SameLine();

        if (ImGui::Button(ICON_FA_TRASH_ALT " Delete", ImVec2(buttonWidth, 0.0f)))
            ImGui::OpenPopup("DeleteBackupConfirm");
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Delete the selected backup");
    }

    void SaveManagerApp::Impl::DrawActivityWindow()
    {
        if (!ShowActivityLog) return;
        ImGui::Begin(ICON_FA_STREAM " Activity Log###Activity Log", &ShowActivityLog);
        for (const auto& entry : Log) ImGui::TextColored(entry.Color, "%s", entry.Message.c_str());
        ImGui::End();
    }

    void SaveManagerApp::Impl::OpenCreateGameModal()
    {
        GameModalMode = 1;
        OpenGameModal = true;
        WriteToBuffer(GameModal.Id, "new_game");
        WriteToBuffer(GameModal.Name, "New Game");
        WriteToBuffer(GameModal.SavePath, "");
        WriteToBuffer(GameModal.ProcessName, "");
        WriteToBuffer(GameModal.Notes, "");
        GameModal.Enabled = true;
        GameModal.ScopeMode = static_cast<int>(SaveScopeMode::FolderMode);
    }

    void SaveManagerApp::Impl::OpenEditGameModal()
    {
        GameDefinition* game = SaveManager.CurrentGame();
        if (!game) return;
        GameModalMode = 2;
        OpenGameModal = true;
        WriteToBuffer(GameModal.Id, game->Id);
        WriteToBuffer(GameModal.Name, game->DisplayName);
        WriteToBuffer(GameModal.SavePath, game->SavePath);
        WriteToBuffer(GameModal.ProcessName, game->ProcessName);
        WriteToBuffer(GameModal.Notes, game->Notes);
        GameModal.Enabled = game->Enabled;
        GameModal.ScopeMode = static_cast<int>(game->ScopeMode);
    }

    void SaveManagerApp::Impl::OpenCreateProfileModal()
    {
        ProfileModalMode = 1;
        OpenProfileModal = true;
        WriteToBuffer(ProfileModal.Id, "profile");
        WriteToBuffer(ProfileModal.Name, "New Profile");
        WriteToBuffer(ProfileModal.Description, "");
        WriteToBuffer(ProfileModal.Notes, "");
        ProfileModal.Favorite = false;
        ProfileModal.AutoBackup = true;
        ProfileModal.BackupLimit = 20;
    }

    void SaveManagerApp::Impl::OpenEditProfileModal()
    {
        GameProfile* profile = SaveManager.CurrentProfile();
        if (!profile) return;
        ProfileModalMode = 2;
        OpenProfileModal = true;
        WriteToBuffer(ProfileModal.Id, profile->Id);
        WriteToBuffer(ProfileModal.Name, profile->Name);
        WriteToBuffer(ProfileModal.Description, profile->Description);
        WriteToBuffer(ProfileModal.Notes, profile->Notes);
        ProfileModal.Favorite = profile->Favorite;
        ProfileModal.AutoBackup = profile->AutoBackupOnSwitch;
        ProfileModal.BackupLimit = profile->BackupLimit;
    }

    void SaveManagerApp::Impl::DrawGameModal()
    {
        if (OpenGameModal)
        {
            ImGui::OpenPopup(GameModalMode == 1 ? "Create Game" : "Edit Game");
            OpenGameModal = false;
        }
        const char* title = GameModalMode == 1 ? "Create Game" : "Edit Game";
        if (!ImGui::BeginPopupModal(title, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
            return;
        ImGui::InputText("Game Id", GameModal.Id.data(), GameModal.Id.size());
        ImGui::InputText("Display Name", GameModal.Name.data(), GameModal.Name.size());
        ImGui::InputText("Save Path", GameModal.SavePath.data(), GameModal.SavePath.size());
        ImGui::InputText("Process Name", GameModal.ProcessName.data(), GameModal.ProcessName.size());
        ImGui::Checkbox("Enabled", &GameModal.Enabled);
        const char* scopeNames[] = { "File Whitelist", "Folder Mode", "Hybrid Mode" };
        ImGui::Combo("Save Scope", &GameModal.ScopeMode, scopeNames, IM_ARRAYSIZE(scopeNames));
        ImGui::InputTextMultiline("Notes", GameModal.Notes.data(), GameModal.Notes.size(), ImVec2(520.0f, 150.0f));
        if (ImGui::Button("Save", ImVec2(140.0f, 0.0f)))
        {
            const std::string sanitizedId = SanitizeId(GameModal.Id.data());
            if (sanitizedId.empty())
            {
                AddLog("Game Id must not be empty.", Bad());
            }
            else if (GameModalMode == 1)
            {
                SaveManager.AddGame();
                if (GameDefinition* game = SaveManager.CurrentGame())
                {
                    game->Id = sanitizedId;
                    game->DisplayName = GameModal.Name.data();
                    game->SavePath = GameModal.SavePath.data();
                    game->ProcessName = GameModal.ProcessName.data();
                    game->Notes = GameModal.Notes.data();
                    game->Enabled = GameModal.Enabled;
                    game->ScopeMode = static_cast<SaveScopeMode>(GameModal.ScopeMode);
                    if (!SaveManager.SaveConfig())
                        AddLog("Game created, but config could not be saved.", Bad());
                    else
                        AddLog("Game created.", Good());
                    LoadSelectedIntoEditor();
                    ImGui::CloseCurrentPopup();
                }
                else
                {
                    AddLog("Game could not be created.", Bad());
                }
            }
            else
            {
                if (GameDefinition* game = SaveManager.CurrentGame())
                {
                    game->Id = sanitizedId;
                    game->DisplayName = GameModal.Name.data();
                    game->SavePath = GameModal.SavePath.data();
                    game->ProcessName = GameModal.ProcessName.data();
                    game->Notes = GameModal.Notes.data();
                    game->Enabled = GameModal.Enabled;
                    game->ScopeMode = static_cast<SaveScopeMode>(GameModal.ScopeMode);
                    if (!SaveManager.SaveConfig())
                        AddLog("Game updated, but config could not be saved.", Bad());
                    else
                        AddLog("Game updated.", Good());
                    LoadSelectedIntoEditor();
                    ImGui::CloseCurrentPopup();
                }
                else
                {
                    AddLog("No game selected for editing.", Bad());
                }
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(140.0f, 0.0f)))
            ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    void SaveManagerApp::Impl::DrawProfileModal()
    {
        if (OpenProfileModal) { ImGui::OpenPopup(ProfileModalMode == 1 ? "Create Profile" : "Edit Profile"); OpenProfileModal = false; }
        const char* title = ProfileModalMode == 1 ? "Create Profile" : "Edit Profile";
        if (!ImGui::BeginPopupModal(title, nullptr, ImGuiWindowFlags_AlwaysAutoResize)) return;

        ImGui::InputText("Profile Id", ProfileModal.Id.data(), ProfileModal.Id.size());
        ImGui::InputText("Profile Name", ProfileModal.Name.data(), ProfileModal.Name.size());
        ImGui::InputText("Description", ProfileModal.Description.data(), ProfileModal.Description.size());
        ImGui::Checkbox("Favorite", &ProfileModal.Favorite);
        ImGui::Checkbox("Auto Backup on Switch", &ProfileModal.AutoBackup);
        ImGui::SliderInt("Backup Limit", &ProfileModal.BackupLimit, 1, 100);
        ImGui::InputTextMultiline("Notes", ProfileModal.Notes.data(), ProfileModal.Notes.size(), ImVec2(500.0f, 140.0f));

        if (ImGui::Button("Save", ImVec2(140.0f, 0.0f)))
        {
            GameDefinition* game = SaveManager.CurrentGame();
            if (game)
            {
                if (ProfileModalMode == 1)
                {
                    GameProfile profile;
                    profile.Id = SanitizeId(ProfileModal.Id.data());
                    profile.Name = ProfileModal.Name.data();
                    profile.Description = ProfileModal.Description.data();
                    profile.Notes = ProfileModal.Notes.data();
                    profile.Favorite = ProfileModal.Favorite;
                    profile.AutoBackupOnSwitch = ProfileModal.AutoBackup;
                    profile.BackupLimit = ProfileModal.BackupLimit;
                    game->Profiles.push_back(profile);
                    game->ActiveProfileId = profile.Id;
                    SaveManager.SyncSelectedProfileFromActive();
                }
                else if (GameProfile* profile = SaveManager.CurrentProfile())
                {
                    profile->Id = SanitizeId(ProfileModal.Id.data());
                    profile->Name = ProfileModal.Name.data();
                    profile->Description = ProfileModal.Description.data();
                    profile->Notes = ProfileModal.Notes.data();
                    profile->Favorite = ProfileModal.Favorite;
                    profile->AutoBackupOnSwitch = ProfileModal.AutoBackup;
                    profile->BackupLimit = ProfileModal.BackupLimit;
                    game->ActiveProfileId = profile->Id;
                }
                SaveManager.SaveConfig();
                LoadSelectedIntoEditor();
                AddLog("Profile saved.", Good());
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(140.0f, 0.0f))) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    void SaveManagerApp::Impl::DrawRestoreModal()
    {
        if (OpenRestoreModal) { ImGui::OpenPopup("Restore Backup"); OpenRestoreModal = false; }
        if (!ImGui::BeginPopupModal("Restore Backup", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) return;

        const BackupEntry* backup = SaveManager.CurrentBackup();
        if (!backup)
        {
            ImGui::TextDisabled("No backup selected.");
            if (ImGui::Button("Close", ImVec2(140.0f, 0.0f))) ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
            return;
        }

        ImGui::Text("Restore backup '%s'?", backup->Name.c_str());
        ImGui::Checkbox("Clear target before restore", &ClearBeforeRestore);
        ImGui::Checkbox("Auto backup before overwrite / restore", &BackupBeforeOverwrite);
        if (ImGui::Button("Restore", ImVec2(140.0f, 0.0f)))
        {
            SaveEditorIntoSelected();
            auto result = SaveManager.RestoreSelectedBackup(ClearBeforeRestore, BackupBeforeOverwrite);
            AddLog(result.Message, result.Success ? Good() : Bad());
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(140.0f, 0.0f))) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    void SaveManagerApp::Impl::DrawAboutPopup()
    {
        ImGui::SetNextWindowSize(ImVec2(550.0f, 610.0f), ImGuiCond_Appearing);

        if (OpenAboutPopup)
        {
            ImGui::OpenPopup("About Manifold");
            OpenAboutPopup = false;
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
            OpenUrl("https://github.com/Leunsel/Manifold-Save-Manager");

        ImGui::SameLine();

        if (ImGui::Button("Close", ImVec2(closeWidth, 0.0f)))
            ImGui::CloseCurrentPopup();

        ImGui::PopStyleVar(2);
        ImGui::EndPopup();
    }

    void SaveManagerApp::Impl::DrawAboutCubeAnimation(float cubeSize)
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

    void SaveManagerApp::Impl::DrawAboutCubeAnimation2(float cubeSize)
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

    void SaveManagerApp::Impl::DrawDeleteBackupPopup()
    {
        if (!ImGui::BeginPopupModal("DeleteBackupConfirm", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) return;
        ImGui::TextUnformatted("Delete the selected backup?");
        if (ImGui::Button("Delete", ImVec2(120.0f, 0.0f)))
        {
            auto result = SaveManager.DeleteSelectedBackup();
            AddLog(result.Message, result.Success ? Warn() : Bad());
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120.0f, 0.0f))) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    void SaveManagerApp::Impl::ExecuteInstantProfileSwitch()
    {
        GameDefinition* game = SaveManager.CurrentGame();
        if (!game || game->Profiles.empty()) return;
        int nextIndex = SaveManager.SelectedProfile() + 1;
        if (nextIndex >= static_cast<int>(game->Profiles.size())) nextIndex = 0;
        auto result = SaveManager.SwitchToProfile(nextIndex, true);
        LoadSelectedIntoEditor();
        SaveManager.SaveConfig();
        AddLog(result.Message + " -> " + game->Profiles[nextIndex].Name, result.Success ? Good() : Bad());
    }

    void SaveManagerApp::Impl::LoadSelectedIntoEditor()
    {
        const GameDefinition* game = SaveManager.CurrentGame();
        const GameProfile* profile = SaveManager.CurrentProfile();
        if (game)
        {
            WriteToBuffer(GameEditor.Id, game->Id);
            WriteToBuffer(GameEditor.Name, game->DisplayName);
            WriteToBuffer(GameEditor.SavePath, game->SavePath);
            WriteToBuffer(GameEditor.ProcessName, game->ProcessName);
            WriteToBuffer(GameEditor.Notes, game->Notes);
        }
        else
        {
            ClearGameEditor();
        }

        if (profile)
        {
            WriteToBuffer(ProfileEditor.Id, profile->Id);
            WriteToBuffer(ProfileEditor.Name, profile->Name);
            WriteToBuffer(ProfileEditor.Description, profile->Description);
            WriteToBuffer(ProfileEditor.Notes, profile->Notes);
        }
        else
        {
            ClearProfileEditor();
        }
    }

    void SaveManagerApp::Impl::SaveEditorIntoSelected()
    {
        GameDefinition* game = SaveManager.CurrentGame();
        GameProfile* profile = SaveManager.CurrentProfile();
        if (game)
        {
            game->Id = SanitizeId(GameEditor.Id.data());
            game->DisplayName = GameEditor.Name.data();
            game->SavePath = GameEditor.SavePath.data();
            game->ProcessName = GameEditor.ProcessName.data();
            game->Notes = GameEditor.Notes.data();
        }
        if (profile)
        {
            profile->Id = SanitizeId(ProfileEditor.Id.data());
            profile->Name = ProfileEditor.Name.data();
            profile->Description = ProfileEditor.Description.data();
            profile->Notes = ProfileEditor.Notes.data();
            if (game) game->ActiveProfileId = profile->Id;
        }
    }

    void SaveManagerApp::Impl::ClearGameEditor()
    {
        GameEditor.Id.fill('\0');
        GameEditor.Name.fill('\0');
        GameEditor.SavePath.fill('\0');
        GameEditor.ProcessName.fill('\0');
        GameEditor.Notes.fill('\0');
    }

    void SaveManagerApp::Impl::ClearProfileEditor()
    {
        ProfileEditor.Id.fill('\0');
        ProfileEditor.Name.fill('\0');
        ProfileEditor.Description.fill('\0');
        ProfileEditor.Notes.fill('\0');
    }

    void SaveManagerApp::Impl::LoadManifoldFonts()
    {
        ImGuiIO& io = ImGui::GetIO();

        if (io.Fonts->Fonts.Size > 0)
            return;

        ImFontConfig baseConfig;
        baseConfig.OversampleH = 1;
        baseConfig.OversampleV = 1;
        baseConfig.PixelSnapH = false;

        io.Fonts->AddFontDefault(&baseConfig);

        static const ImWchar iconRanges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };

        ImFontConfig iconConfig;
        iconConfig.MergeMode = true;
        iconConfig.PixelSnapH = true;
        iconConfig.GlyphMinAdvanceX = 16.0f;
        iconConfig.FontDataOwnedByAtlas = false; // Prevent Deallocation Errors thrown by ImGui.

        io.Fonts->AddFontFromMemoryTTF(rawData, sizeof(rawData), 10.0f, &iconConfig, iconRanges);
    }
}
