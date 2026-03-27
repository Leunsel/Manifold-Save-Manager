#include "SaveManagerApp.Internal.hpp"

namespace manifold
{
    namespace
    {
        constexpr float kModalWidth = 620.0f;
        constexpr float kActionButtonHeight = 0.0f;

        void SetStandardModalSize(float height)
        {
            ImGui::SetNextWindowSize(ImVec2(kModalWidth, height), ImGuiCond_Appearing);
            ImGui::SetNextWindowSizeConstraints(ImVec2(460.0f, 0.0f), ImVec2(900.0f, FLT_MAX));
        }

        void DrawModalTitle(const char* title, const char* subtitle)
        {
            ImGui::TextUnformatted(title);
            if (subtitle != nullptr && subtitle[0] != '\0')
            {
                ImGui::TextDisabled("%s", subtitle);
            }
            // ImGui::Separator();
        }

        void DrawSectionLabel(const char* label)
        {
            ImGui::Spacing();
            ImGui::TextDisabled("%s", label);
        }

        bool DrawModalFooterButtons(const char* primaryLabel, const char* secondaryLabel, bool primaryEnabled = true)
        {
            const float spacing = ImGui::GetStyle().ItemSpacing.x;
            const float avail = ImGui::GetContentRegionAvail().x;
            const float buttonWidth = (avail - spacing) * 0.5f;

            bool primaryPressed = false;

            ImGui::BeginDisabled(!primaryEnabled);
            if (ImGui::Button(primaryLabel, ImVec2(buttonWidth, kActionButtonHeight)))
            {
                primaryPressed = true;
            }
            ImGui::EndDisabled();

            ImGui::SameLine();
            if (ImGui::Button(secondaryLabel, ImVec2(buttonWidth, kActionButtonHeight)))
            {
                ImGui::CloseCurrentPopup();
            }

            return primaryPressed;
        }

        bool HasText(const char* value)
        {
            return value != nullptr && value[0] != '\0';
        }
    }

    void SaveManagerApp::Impl::OpenCreateGameModal()
    {
        GameModal.LoadCreateDefaults();
    }

    void SaveManagerApp::Impl::OpenEditGameModal()
    {
        if (GameDefinition* game = SaveManager.CurrentGame())
        {
            GameModal.LoadFrom(*game);
        }
    }

    void SaveManagerApp::Impl::OpenCreateProfileModal()
    {
        ProfileModal.LoadCreateDefaults();
    }

    void SaveManagerApp::Impl::OpenEditProfileModal()
    {
        if (GameProfile* profile = SaveManager.CurrentProfile())
        {
            ProfileModal.LoadFrom(*profile);
        }
    }

    void SaveManagerApp::Impl::DrawGameModal()
    {
        if (GameModal.OpenRequested)
        {
            ImGui::OpenPopup(GameModal.PopupTitle());
            GameModal.OpenRequested = false;
        }

        SetStandardModalSize(560.0f);
        if (!ImGui::BeginPopupModal(GameModal.PopupTitle(), nullptr, ImGuiWindowFlags_NoResize))
        {
            return;
        }

        const bool creating = GameModal.Mode == detail::EditMode::Create;
        const std::string sanitizedId = SanitizeId(GameModal.Id.data());
        const bool hasValidId = !sanitizedId.empty();
        const bool hasName = HasText(GameModal.Name.data());
        const bool canSave = hasValidId && hasName;

        DrawModalTitle(
            creating ? "Create Game" : "Edit Game",
            creating ? "Add a new game entry and define where its save data lives."
            : "Update the selected game and keep the setup consistent.");

        DrawSectionLabel("Identity");
        ImGui::InputText("Game Id", GameModal.Id.data(), GameModal.Id.size());
        if (!hasValidId)
        {
            ImGui::TextDisabled("Use a stable internal id. It must not be empty.");
        }
        ImGui::InputText("Display Name", GameModal.Name.data(), GameModal.Name.size());

        DrawSectionLabel("Save Setup");
        ImGui::InputText("Save Path", GameModal.SavePath.data(), GameModal.SavePath.size());
        ImGui::InputText("Process Name", GameModal.ProcessName.data(), GameModal.ProcessName.size());
        ImGui::Checkbox("Enabled", &GameModal.Enabled);

        const char* scopeNames[] = { "File Whitelist", "Folder Mode", "Hybrid Mode" };
        ImGui::Combo("Save Scope", &GameModal.ScopeMode, scopeNames, IM_ARRAYSIZE(scopeNames));

        DrawSectionLabel("Notes");
        ImGui::InputTextMultiline("##GameNotes", GameModal.Notes.data(), GameModal.Notes.size(), ImVec2(-1.0f, 150.0f));

        ImGui::Spacing();
        if (DrawModalFooterButtons(creating ? ICON_FA_SAVE " Create Game" : ICON_FA_SAVE " Save Changes", ICON_FA_TIMES " Cancel", canSave))
        {
            if (!hasValidId)
            {
                AddLog("Game Id must not be empty.", Bad());
            }
            else if (!hasName)
            {
                AddLog("Display name must not be empty.", Bad());
            }
            else if (creating)
            {
                SaveManager.AddGame();
                if (GameDefinition* game = SaveManager.CurrentGame())
                {
                    GameModal.ApplyTo(*game);
                    const bool ok = SaveManager.SaveConfig();
                    AddLog(ok ? "Game created." : "Game created, but config could not be saved.", ok ? Good() : Bad());
                    SyncEditorFromSelection();
                    ImGui::CloseCurrentPopup();
                }
                else
                {
                    AddLog("Game could not be created.", Bad());
                }
            }
            else if (GameDefinition* game = SaveManager.CurrentGame())
            {
                GameModal.ApplyTo(*game);
                const bool ok = SaveManager.SaveConfig();
                AddLog(ok ? "Game updated." : "Game updated, but config could not be saved.", ok ? Good() : Bad());
                SyncEditorFromSelection();
                ImGui::CloseCurrentPopup();
            }
            else
            {
                AddLog("No game selected for editing.", Bad());
            }
        }

        ImGui::EndPopup();
    }

    void SaveManagerApp::Impl::DrawProfileModal()
    {
        if (ProfileModal.OpenRequested)
        {
            ImGui::OpenPopup(ProfileModal.PopupTitle());
            ProfileModal.OpenRequested = false;
        }

        SetStandardModalSize(520.0f);
        if (!ImGui::BeginPopupModal(ProfileModal.PopupTitle(), nullptr, ImGuiWindowFlags_NoResize))
        {
            return;
        }

        const bool creating = ProfileModal.Mode == detail::EditMode::Create;
        const bool hasValidId = !SanitizeId(ProfileModal.Id.data()).empty();
        const bool hasName = HasText(ProfileModal.Name.data());
        const bool hasGame = SaveManager.CurrentGame() != nullptr;
        const bool canSave = hasGame && hasValidId && hasName;

        DrawModalTitle(
            creating ? "Create Profile" : "Edit Profile",
            creating ? "Create a profile for different saves, rules, or backup behavior."
            : "Adjust the selected profile and its backup preferences.");

        if (!hasGame)
        {
            ImGui::TextDisabled("No game is currently selected. Select a game before creating or editing a profile.");
            ImGui::Spacing();
            DrawModalFooterButtons(ICON_FA_SAVE " Save", ICON_FA_TIMES " Close", false);
            ImGui::EndPopup();
            return;
        }

        DrawSectionLabel("Identity");
        ImGui::InputText("Profile Id", ProfileModal.Id.data(), ProfileModal.Id.size());
        if (!hasValidId)
        {
            ImGui::TextDisabled("Use a stable profile id. It must not be empty.");
        }
        ImGui::InputText("Profile Name", ProfileModal.Name.data(), ProfileModal.Name.size());
        ImGui::InputText("Description", ProfileModal.Description.data(), ProfileModal.Description.size());

        DrawSectionLabel("Behavior");
        ImGui::Checkbox("Favorite", &ProfileModal.Favorite);
        ImGui::Checkbox("Auto Backup on Switch", &ProfileModal.AutoBackup);
        ImGui::SliderInt("Backup Limit", &ProfileModal.BackupLimit, 1, 100);

        DrawSectionLabel("Notes");
        ImGui::InputTextMultiline("##ProfileNotes", ProfileModal.Notes.data(), ProfileModal.Notes.size(), ImVec2(-1.0f, 120.0f));

        ImGui::Spacing();
        if (DrawModalFooterButtons(creating ? ICON_FA_SAVE " Create Profile" : ICON_FA_SAVE " Save Changes", ICON_FA_TIMES " Cancel", canSave))
        {
            GameDefinition* game = SaveManager.CurrentGame();
            if (game == nullptr)
            {
                AddLog("No game selected for profile changes.", Bad());
            }
            else if (!hasValidId)
            {
                AddLog("Profile Id must not be empty.", Bad());
            }
            else if (!hasName)
            {
                AddLog("Profile name must not be empty.", Bad());
            }
            else
            {
                if (creating)
                {
                    GameProfile profile{};
                    ProfileModal.ApplyTo(profile);
                    game->Profiles.push_back(profile);
                    game->ActiveProfileId = profile.Id;
                    SaveManager.SyncSelectedProfileFromActive();
                }
                else if (GameProfile* profile = SaveManager.CurrentProfile())
                {
                    ProfileModal.ApplyTo(*profile);
                    game->ActiveProfileId = profile->Id;
                }
                else
                {
                    AddLog("No profile selected for editing.", Bad());
                    ImGui::EndPopup();
                    return;
                }

                const bool ok = SaveManager.SaveConfig();
                SyncEditorFromSelection();
                AddLog(ok ? "Profile saved." : "Profile updated, but config could not be saved.", ok ? Good() : Bad());
                ImGui::CloseCurrentPopup();
            }
        }

        ImGui::EndPopup();
    }

    void SaveManagerApp::Impl::DrawRestoreModal()
    {
        if (Popups.RestoreBackup)
        {
            ImGui::OpenPopup("Restore Backup");
            Popups.RestoreBackup = false;
        }

        SetStandardModalSize(470.0f);
        if (!ImGui::BeginPopupModal("Restore Backup", nullptr, ImGuiWindowFlags_NoResize))
        {
            return;
        }

        const BackupEntry* backup = SaveManager.CurrentBackup();
        DrawModalTitle("Restore Backup", "This replaces the current target files with the selected backup.");

        if (backup == nullptr)
        {
            ImGui::TextDisabled("No backup selected. Choose a backup first, then open restore again.");
            ImGui::Spacing();
            DrawModalFooterButtons(ICON_FA_UNDO " Restore", ICON_FA_TIMES " Close", false);
            ImGui::EndPopup();
            return;
        }

        DrawSectionLabel("Selected Backup");
        ImGui::TextWrapped("%s", backup->Name.c_str());
        if (!backup->CreatedAtDisplay.empty())
        {
            ImGui::TextDisabled("Created: %s", backup->CreatedAtDisplay.c_str());
        }
        ImGui::TextDisabled("Files: %zu", backup->FileCount);
        if (!backup->FullPath.empty())
        {
            ImGui::TextWrapped("Location: %s", backup->FullPath.c_str());
        }

        DrawSectionLabel("Restore Options");
        ImGui::Checkbox("Clear target before restore", &ClearBeforeRestore);
        ImGui::Checkbox("Auto backup before overwrite / restore", &BackupBeforeOverwrite);

        ImGui::Spacing();
        if (DrawModalFooterButtons(ICON_FA_UNDO " Restore Backup", ICON_FA_TIMES " Cancel", true))
        {
            SyncSelectionFromEditor();
            const auto result = SaveManager.RestoreSelectedBackup(ClearBeforeRestore, BackupBeforeOverwrite);
            AddLog(result.Message, result.Success ? Good() : Bad());
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    void SaveManagerApp::Impl::DrawDeleteGamePopup()
    {
        if (Popups.DeleteGame)
        {
            ImGui::OpenPopup("DeleteGameConfirm");
            Popups.DeleteGame = false;
        }

        SetStandardModalSize(360.0f);
        if (!ImGui::BeginPopupModal("DeleteGameConfirm", nullptr, ImGuiWindowFlags_NoResize))
        {
            return;
        }

        DrawModalTitle("Delete Game", "Remove the selected game entry from Manifold.");

        const GameDefinition* game = SaveManager.CurrentGame();
        if (game == nullptr)
        {
            ImGui::TextDisabled("No game selected.");
            ImGui::Spacing();
            DrawModalFooterButtons(ICON_FA_TRASH_ALT " Delete", ICON_FA_TIMES " Close", false);
            ImGui::EndPopup();
            return;
        }

        const std::string label = game->DisplayName.empty() ? game->Id : game->DisplayName;
        ImGui::TextWrapped("You are about to remove '%s'.", label.c_str());
        ImGui::TextDisabled("This deletes only the configuration entry, not the actual save files.");

        ImGui::Spacing();
        if (DrawModalFooterButtons(ICON_FA_TRASH_ALT " Delete Game", ICON_FA_TIMES " Cancel", true))
        {
            SaveManager.RemoveCurrentGame();
            SyncEditorFromSelection();
            AddLog("Game removed.", Warn());
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    void SaveManagerApp::Impl::DrawDeleteBackupPopup()
    {
        if (Popups.DeleteBackup)
        {
            ImGui::OpenPopup("DeleteBackupConfirm");
            Popups.DeleteBackup = false;
        }

        SetStandardModalSize(360.0f);
        if (!ImGui::BeginPopupModal("DeleteBackupConfirm", nullptr, ImGuiWindowFlags_NoResize))
        {
            return;
        }

        DrawModalTitle("Delete Backup", "Remove the selected backup from disk.");

        const BackupEntry* backup = SaveManager.CurrentBackup();
        if (backup == nullptr)
        {
            ImGui::TextDisabled("No backup selected.");
            ImGui::Spacing();
            DrawModalFooterButtons(ICON_FA_TRASH_ALT " Delete", ICON_FA_TIMES " Close", false);
            ImGui::EndPopup();
            return;
        }

        ImGui::TextWrapped("Delete backup '%s'?", backup->Name.c_str());
        ImGui::TextDisabled("This cannot be undone from inside Manifold.");

        ImGui::Spacing();
        if (DrawModalFooterButtons(ICON_FA_TRASH_ALT " Delete Backup", ICON_FA_TIMES " Cancel", true))
        {
            const auto result = SaveManager.DeleteSelectedBackup();
            AddLog(result.Message, result.Success ? Warn() : Bad());
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    void SaveManagerApp::Impl::DrawAboutCubeAnimation(float cubeSize)
    {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        if (dl == nullptr)
        {
            return;
        }

        const ImVec2 start = ImGui::GetCursorScreenPos();
        const ImVec2 region(cubeSize, cubeSize);
        ImGui::InvisibleButton("##AboutCubeCanvas", region);

        const ImVec2 canvasMin = start;
        const float pad = 6.0f;
        const float sizef = (std::min)(region.x, region.y) - 2.0f * pad;
        const Math::Vec2 center(canvasMin.x + region.x * 0.5f, canvasMin.y + region.y * 0.5f);

        const float t = static_cast<float>(ImGui::GetTime());
        const float rx = t * 0.6f;
        const float ry = t * 0.9f;
        const float fov = 1.1f;
        const float zOff = 3.2f;
        const float half = sizef * 0.5f;

        auto project = [&](const Math::Vec3& p) -> Math::Vec2
            {
                const float z = p.z + zOff;
                const float inv = (z > 0.01f) ? (fov / z) : (fov / 0.01f);
                return { center.x + p.x * inv * half, center.y + p.y * inv * half };
            };

        Math::Vec3 cube[8] = {
            {-1, -1, -1}, {1, -1, -1}, {-1, 1, -1}, {1, 1, -1},
            {-1, -1, 1},  {1, -1, 1},  {-1, 1, 1},  {1, 1, 1}
        };

        ImVec2 pts[8];
        for (int i = 0; i < 8; ++i)
        {
            const Math::Vec3 v = Math::RotateVecX(Math::RotateVecY(cube[i], ry), rx);
            const Math::Vec2 p = project(v);
            pts[i] = ImVec2(p.x, p.y);
        }

        static const int edges[12][2] = {
            {0, 1}, {1, 3}, {3, 2}, {2, 0},
            {4, 5}, {5, 7}, {7, 6}, {6, 4},
            {0, 4}, {1, 5}, {2, 6}, {3, 7}
        };

        const ImU32 lineCol = ImGui::GetColorU32(ImVec4(1, 1, 1, 0.22f));
        for (const auto& edge : edges)
        {
            dl->AddLine(pts[edge[0]], pts[edge[1]], lineCol, 1.8f);
        }

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
            {
                dir = Math::Vec2(0.0f, -1.0f);
            }

            const Math::Vec2 q = p + dir * outOffset;
            char c[2] = { word[i], 0 };
            const ImVec2 ts = ImGui::CalcTextSize(c);

            dl->AddText(ImVec2(q.x - ts.x * 0.5f + shadowOff, q.y - ts.y * 0.5f + shadowOff), txtShadow, c);
            dl->AddText(ImVec2(q.x - ts.x * 0.5f, q.y - ts.y * 0.5f), txtCol, c);
        }
    }

    void SaveManagerApp::Impl::DrawAboutCubeAnimation2(float cubeSize)
    {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        if (dl == nullptr)
        {
            return;
        }

        const ImVec2 start = ImGui::GetCursorScreenPos();
        const ImVec2 region(cubeSize, cubeSize);
        ImGui::InvisibleButton("##AboutOctahedronCanvas", region);

        const ImVec2 canvasMin = start;
        const float pad = 6.0f;
        const float sizef = (std::min)(region.x, region.y) - 2.0f * pad;
        const Math::Vec2 center(canvasMin.x + region.x * 0.5f, canvasMin.y + region.y * 0.5f);

        const float t = static_cast<float>(ImGui::GetTime());
        const float rx = t * 0.45f;
        const float ry = t * 1.10f;
        const float rz = t * 0.30f;
        const float fov = 1.1f;
        const float zOff = 3.2f;
        const float half = sizef * 0.5f;
        const float shapeScale = 1.5f;

        auto project = [&](const Math::Vec3& p) -> Math::Vec2
            {
                const float z = p.z + zOff;
                const float inv = (z > 0.01f) ? (fov / z) : (fov / 0.01f);
                return { center.x + p.x * inv * half, center.y + p.y * inv * half };
            };

        auto rotate = [&](const Math::Vec3& v) -> Math::Vec3
            {
                return Math::RotateVecZ(Math::RotateVecX(Math::RotateVecY(v, ry), rx), rz);
            };

        Math::Vec3 verts[6] = { {0, 0, 1}, {0, 0, -1}, {-1, 0, 0}, {1, 0, 0}, {0, 1, 0}, {0, -1, 0} };
        ImVec2 pts[6];
        Math::Vec3 rotated[6];

        for (int i = 0; i < 6; ++i)
        {
            rotated[i] = rotate(verts[i] * shapeScale);
            const Math::Vec2 p = project(rotated[i]);
            pts[i] = ImVec2(p.x, p.y);
        }

        static const int edges[12][2] = {
            {0, 2}, {0, 3}, {0, 4}, {0, 5},
            {1, 2}, {1, 3}, {1, 4}, {1, 5},
            {2, 4}, {4, 3}, {3, 5}, {5, 2}
        };

        const ImU32 lineCol = ImGui::GetColorU32(ImVec4(1, 1, 1, 0.22f));
        for (const auto& edge : edges)
        {
            dl->AddLine(pts[edge[0]], pts[edge[1]], lineCol, 1.8f);
        }

        static const char* word = "Cfemen";

        struct VertexLabel
        {
            ImVec2 Pos;
            float Depth;
            char Character;
        };

        VertexLabel labels[6];
        for (int i = 0; i < 6; ++i)
        {
            labels[i].Pos = pts[i];
            labels[i].Depth = rotated[i].z;
            labels[i].Character = word[i];
        }

        std::sort(std::begin(labels), std::end(labels), [](const VertexLabel& a, const VertexLabel& b)
            {
                return a.Depth < b.Depth;
            });

        const ImU32 txtCol = ImGui::GetColorU32(ImVec4(1, 1, 1, 0.96f));
        const ImU32 txtShadow = ImGui::GetColorU32(ImVec4(0, 0, 0, 0.55f));
        const float outOffset = 14.0f;
        const float shadowOff = 1.0f;

        for (const auto& label : labels)
        {
            Math::Vec2 p(label.Pos.x, label.Pos.y);
            Math::Vec2 dir = (p - center).Normalized();
            if (dir.LengthSq() < 1e-6f)
            {
                dir = Math::Vec2(0.0f, -1.0f);
            }

            const Math::Vec2 q = p + dir * outOffset;
            char c[2] = { label.Character, 0 };
            const ImVec2 ts = ImGui::CalcTextSize(c);
            const ImVec2 textPos(q.x - ts.x * 0.5f, q.y - ts.y * 0.5f);

            dl->AddText(ImVec2(textPos.x + shadowOff, textPos.y + shadowOff), txtShadow, c);
            dl->AddText(textPos, txtCol, c);
        }
    }

    void SaveManagerApp::Impl::DrawAboutPopup()
    {
        ImGui::SetNextWindowSize(ImVec2(660.0f, 520.0f), ImGuiCond_Appearing);

        if (Popups.About)
        {
            ImGui::OpenPopup("About Manifold");
            Popups.About = false;
        }

        if (!ImGui::BeginPopupModal("About Manifold", nullptr, ImGuiWindowFlags_NoResize))
        {
            return;
        }

        ImDrawList* dl = ImGui::GetWindowDrawList();
        const ImVec2 winPos = ImGui::GetWindowPos();
        const ImVec2 winSize = ImGui::GetWindowSize();

        const ImU32 accentCol = ImGui::GetColorU32(ImVec4(0.15f, 0.82f, 0.88f, 0.85f));
        const ImU32 panelBorderCol = ImGui::GetColorU32(ImVec4(1, 1, 1, 0.06f));

        dl->AddRect(winPos, ImVec2(winPos.x + winSize.x, winPos.y + winSize.y), panelBorderCol, 12.0f, 0, 1.0f);
        dl->AddLine(ImVec2(winPos.x + 14.0f, winPos.y + 1.0f), ImVec2(winPos.x + winSize.x - 14.0f, winPos.y + 1.0f), accentCol, 2.0f);

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10.0f, 10.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.0f, 8.0f));

        DrawModalTitle("Manifold Save Manager", "Creators, links, and project information.");

        const float cardWidth = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) * 0.5f;
        const float animSize = cardWidth - 70.0f;
        const float linkSpacing = ImGui::GetStyle().ItemSpacing.x;
        const float linkWidth = (cardWidth - 24.0f - linkSpacing) * 0.5f;

        auto drawPersonCard = [&](const char* name, const char* role, bool second, const char* websiteUrl, const char* patreonUrl)
            {
                ImGui::BeginChild(second ? "##AboutCardCfemen" : "##AboutCardLeunsel", ImVec2(cardWidth, 370.0f), true);
                ImGui::TextUnformatted(name);
                ImGui::TextDisabled("%s", role);
                ImGui::Spacing();

                if (second) DrawAboutCubeAnimation2(animSize);
                else DrawAboutCubeAnimation(animSize);

                ImGui::Spacing();
                if (websiteUrl != nullptr && websiteUrl[0] != '\0')
                {
                    if (ImGui::Button((std::string("Website##") + name).c_str(), ImVec2(linkWidth, 0.0f)))
                    {
                        OpenUrl(websiteUrl);
                    }
                }
                else
                {
                    ImGui::BeginDisabled();
                    ImGui::Button((std::string("Website##") + name).c_str(), ImVec2(linkWidth, 0.0f));
                    ImGui::EndDisabled();
                }

                ImGui::SameLine();
                if (ImGui::Button((std::string("Patreon##") + name).c_str(), ImVec2(linkWidth, 0.0f)))
                {
                    OpenUrl(patreonUrl);
                }
                ImGui::EndChild();
            };

        drawPersonCard("Leunsel", "Creator of Manifold", false, "https://leunsel.com/", "https://www.patreon.com/leunsel");
        ImGui::SameLine();
        drawPersonCard("Cfemen", "Contributor", true, "", "https://www.patreon.com/cfemen");

        ImGui::Spacing();
        if (DrawModalFooterButtons("GitHub", "Close", true))
        {
            OpenUrl("https://github.com/Leunsel/Manifold-Save-Manager");
        }

        ImGui::PopStyleVar(2);
        ImGui::EndPopup();
    }
}
