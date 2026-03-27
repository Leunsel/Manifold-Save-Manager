#include "SaveManagerApp.Internal.hpp"

namespace manifold
{
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

        if (!ImGui::BeginPopupModal(GameModal.PopupTitle(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            return;
        }

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
            else if (GameModal.Mode == detail::EditMode::Create)
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
            else
            {
                if (GameDefinition* game = SaveManager.CurrentGame())
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
        }

        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(140.0f, 0.0f)))
        {
            ImGui::CloseCurrentPopup();
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

        if (!ImGui::BeginPopupModal(ProfileModal.PopupTitle(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            return;
        }

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
            if (game != nullptr)
            {
                if (ProfileModal.Mode == detail::EditMode::Create)
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

                SaveManager.SaveConfig();
                SyncEditorFromSelection();
                AddLog("Profile saved.", Good());
            }
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(140.0f, 0.0f)))
        {
            ImGui::CloseCurrentPopup();
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

        if (!ImGui::BeginPopupModal("Restore Backup", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            return;
        }

        const BackupEntry* backup = SaveManager.CurrentBackup();
        if (backup == nullptr)
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
            SyncSelectionFromEditor();
            const auto result = SaveManager.RestoreSelectedBackup(ClearBeforeRestore, BackupBeforeOverwrite);
            AddLog(result.Message, result.Success ? Good() : Bad());
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(140.0f, 0.0f)))
        {
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

        if (!ImGui::BeginPopupModal("DeleteGameConfirm", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            return;
        }

        const GameDefinition* game = SaveManager.CurrentGame();
        if (game == nullptr)
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
            SyncEditorFromSelection();
            AddLog("Game removed.", Warn());
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_TIMES " Cancel", ImVec2(140.0f, 0.0f)))
        {
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

        if (!ImGui::BeginPopupModal("DeleteBackupConfirm", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            return;
        }

        ImGui::TextUnformatted("Delete the selected backup?");
        if (ImGui::Button("Delete", ImVec2(120.0f, 0.0f)))
        {
            const auto result = SaveManager.DeleteSelectedBackup();
            AddLog(result.Message, result.Success ? Warn() : Bad());
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120.0f, 0.0f)))
        {
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
        ImGui::SetNextWindowSize(ImVec2(550.0f, 515.0f), ImGuiCond_Appearing);

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

        const ImU32 lineCol = ImGui::GetColorU32(ImVec4(1, 1, 1, 0.06f));
        const ImU32 accentCol = ImGui::GetColorU32(ImVec4(0.15f, 0.82f, 0.88f, 0.85f));
        const ImU32 panelBorderCol = ImGui::GetColorU32(ImVec4(1, 1, 1, 0.06f));

        dl->AddRect(winPos, ImVec2(winPos.x + winSize.x, winPos.y + winSize.y), panelBorderCol, 12.0f, 0, 1.0f);
        dl->AddLine(ImVec2(winPos.x + 14.0f, winPos.y + 1.0f), ImVec2(winPos.x + winSize.x - 14.0f, winPos.y + 1.0f), accentCol, 2.0f);

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10.0f, 10.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.0f, 8.0f));

        ImGui::TextUnformatted("Manifold Save Manager");
        ImGui::TextDisabled("Creators, links, and project information");

        ImGui::Dummy(ImVec2(0.0f, 10.0f));

        const float animSize = 230.0f;
        const float buttonWidth = 110.0f;

        ImGui::BeginGroup();
        {
            ImGui::TextUnformatted("Leunsel");
            ImGui::TextDisabled("Creator of Manifold");
            ImGui::Spacing();

            DrawAboutCubeAnimation(animSize);

            ImGui::Spacing();
            if (ImGui::Button("Website##Leunsel", ImVec2(buttonWidth, 0.0f)))
            {
                OpenUrl("https://leunsel.com/");
            }
            ImGui::SameLine();
            if (ImGui::Button("Patreon##Leunsel", ImVec2(buttonWidth, 0.0f)))
            {
                OpenUrl("https://www.patreon.com/leunsel");
            }
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
            {
                OpenUrl("");
            }
            ImGui::EndDisabled();

            ImGui::SameLine();
            if (ImGui::Button("Patreon##Cfemen", ImVec2(buttonWidth, 0.0f)))
            {
                OpenUrl("https://www.patreon.com/cfemen");
            }
        }
        ImGui::EndGroup();

        ImGui::Dummy(ImVec2(0.0f, 10.0f));

        const float closeWidth = 120.0f;
        if (ImGui::Button("GitHub##Tool", ImVec2(closeWidth, 0.0f)))
        {
            OpenUrl("https://github.com/Leunsel/Manifold-Save-Manager");
        }

        ImGui::SameLine();
        if (ImGui::Button("Close", ImVec2(closeWidth, 0.0f)))
        {
            ImGui::CloseCurrentPopup();
        }

        ImGui::PopStyleVar(2);
        ImGui::EndPopup();
    }
}
