#include "SaveManagerApp.Internal.hpp"

namespace manifold
{
    using detail::BufferText;
    using detail::EditMode;
    using detail::ThemeRuntime;

    namespace detail
    {
        void GameEditorState::Clear()
        {
            BufferText::Clear(Id);
            BufferText::Clear(Name);
            BufferText::Clear(SavePath);
            BufferText::Clear(ProcessName);
            BufferText::Clear(Notes);
        }

        void GameEditorState::LoadFrom(const GameDefinition& game)
        {
            BufferText::Assign(Id, game.Id);
            BufferText::Assign(Name, game.DisplayName);
            BufferText::Assign(SavePath, game.SavePath);
            BufferText::Assign(ProcessName, game.ProcessName);
            BufferText::Assign(Notes, game.Notes);
        }

        void GameEditorState::ApplyTo(GameDefinition& game) const
        {
            game.Id = SanitizeId(Id.data());
            game.DisplayName = Name.data();
            game.SavePath = SavePath.data();
            game.ProcessName = ProcessName.data();
            game.Notes = Notes.data();
        }

        void ProfileEditorState::Clear()
        {
            BufferText::Clear(Id);
            BufferText::Clear(Name);
            BufferText::Clear(Description);
            BufferText::Clear(Notes);
            BufferText::Clear(ScopeRule);
        }

        void ProfileEditorState::LoadFrom(const GameProfile& profile)
        {
            BufferText::Assign(Id, profile.Id);
            BufferText::Assign(Name, profile.Name);
            BufferText::Assign(Description, profile.Description);
            BufferText::Assign(Notes, profile.Notes);
        }

        void ProfileEditorState::ApplyTo(GameProfile& profile) const
        {
            profile.Id = SanitizeId(Id.data());
            profile.Name = Name.data();
            profile.Description = Description.data();
            profile.Notes = Notes.data();
        }

        void GameModalState::LoadCreateDefaults()
        {
            BufferText::Assign(Id, "new_game");
            BufferText::Assign(Name, "New Game");
            BufferText::Assign(SavePath, "");
            BufferText::Assign(ProcessName, "");
            BufferText::Assign(Notes, "");
            Enabled = true;
            ScopeMode = static_cast<int>(SaveScopeMode::FolderMode);
            Mode = EditMode::Create;
            OpenRequested = true;
        }

        void GameModalState::LoadFrom(const GameDefinition& game)
        {
            BufferText::Assign(Id, game.Id);
            BufferText::Assign(Name, game.DisplayName);
            BufferText::Assign(SavePath, game.SavePath);
            BufferText::Assign(ProcessName, game.ProcessName);
            BufferText::Assign(Notes, game.Notes);
            Enabled = game.Enabled;
            ScopeMode = static_cast<int>(game.ScopeMode);
            Mode = EditMode::Edit;
            OpenRequested = true;
        }

        void GameModalState::ApplyTo(GameDefinition& game) const
        {
            game.Id = SanitizeId(Id.data());
            game.DisplayName = Name.data();
            game.SavePath = SavePath.data();
            game.ProcessName = ProcessName.data();
            game.Notes = Notes.data();
            game.Enabled = Enabled;
            game.ScopeMode = static_cast<SaveScopeMode>(ScopeMode);
        }

        const char* GameModalState::PopupTitle() const
        {
            return Mode == EditMode::Create ? "Create Game" : "Edit Game";
        }

        void ProfileModalState::LoadCreateDefaults()
        {
            BufferText::Assign(Id, "profile");
            BufferText::Assign(Name, "New Profile");
            BufferText::Assign(Description, "");
            BufferText::Assign(Notes, "");
            Favorite = false;
            AutoBackup = true;
            BackupLimit = 20;
            Mode = EditMode::Create;
            OpenRequested = true;
        }

        void ProfileModalState::LoadFrom(const GameProfile& profile)
        {
            BufferText::Assign(Id, profile.Id);
            BufferText::Assign(Name, profile.Name);
            BufferText::Assign(Description, profile.Description);
            BufferText::Assign(Notes, profile.Notes);
            Favorite = profile.Favorite;
            AutoBackup = profile.AutoBackupOnSwitch;
            BackupLimit = profile.BackupLimit;
            Mode = EditMode::Edit;
            OpenRequested = true;
        }

        void ProfileModalState::ApplyTo(GameProfile& profile) const
        {
            profile.Id = SanitizeId(Id.data());
            profile.Name = Name.data();
            profile.Description = Description.data();
            profile.Notes = Notes.data();
            profile.Favorite = Favorite;
            profile.AutoBackupOnSwitch = AutoBackup;
            profile.BackupLimit = BackupLimit;
        }

        const char* ProfileModalState::PopupTitle() const
        {
            return Mode == EditMode::Create ? "Create Profile" : "Edit Profile";
        }
    }

    SaveManagerApp::Impl::Impl(HWND hwnd) : MainWindow(hwnd) { }

    SaveManagerApp::SaveManagerApp(HWND hwnd) : m_Impl(std::make_unique<Impl>(hwnd))
    {
        m_Impl->Initialize();
    }

    SaveManagerApp::~SaveManagerApp() = default;
    SaveManagerApp::SaveManagerApp(SaveManagerApp&&) noexcept = default;
    SaveManagerApp& SaveManagerApp::operator=(SaveManagerApp&&) noexcept = default;

    void SaveManagerApp::Render()
    {
        m_Impl->Render();
    }

    void SaveManagerApp::Impl::Initialize()
    {
        LoadFonts();
        ApplyThemeIfNeeded();
        SaveManager.Load();
        SyncEditorFromSelection();
        AddLog("SaveManager state loaded.", Accent());
    }

    void SaveManagerApp::Impl::LoadFonts()
    {
        ImGuiIO& io = ImGui::GetIO();
        if (io.Fonts->Fonts.Size > 0)
        {
            return;
        }

        ImFontConfig baseConfig{};
        baseConfig.OversampleH = 1;
        baseConfig.OversampleV = 1;
        baseConfig.PixelSnapH = false;
        io.Fonts->AddFontDefault(&baseConfig);

        static const ImWchar iconRanges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };

        ImFontConfig iconConfig{};
        iconConfig.MergeMode = true;
        iconConfig.PixelSnapH = true;
        iconConfig.GlyphMinAdvanceX = 16.0f;
        iconConfig.FontDataOwnedByAtlas = false;
        io.Fonts->AddFontFromMemoryTTF(rawData, sizeof(rawData), 10.0f, &iconConfig, iconRanges);
    }

    void SaveManagerApp::Impl::Render()
    {
        DrawRootWindow();
    }

    void SaveManagerApp::Impl::SyncEditorFromSelection()
    {
        if (const GameDefinition* game = SaveManager.CurrentGame())
        {
            GameEditor.LoadFrom(*game);
        }
        else
        {
            GameEditor.Clear();
        }

        if (const GameProfile* profile = SaveManager.CurrentProfile())
        {
            ProfileEditor.LoadFrom(*profile);
        }
        else
        {
            ProfileEditor.Clear();
        }
    }

    void SaveManagerApp::Impl::SyncSelectionFromEditor()
    {
        GameDefinition* game = SaveManager.CurrentGame();
        GameProfile* profile = SaveManager.CurrentProfile();

        if (game != nullptr)
        {
            GameEditor.ApplyTo(*game);
        }

        if (profile != nullptr)
        {
            ProfileEditor.ApplyTo(*profile);
            if (game != nullptr)
            {
                game->ActiveProfileId = profile->Id;
            }
        }
    }

    void SaveManagerApp::Impl::AddLog(std::string message, ImVec4 color)
    {
        const std::string stamped = '[' + FormatTime(std::time(nullptr)) + "] " + std::move(message);
        Log.push_back({ stamped, color });
        if (Log.size() > 150)
        {
            Log.erase(Log.begin(), Log.begin() + static_cast<long long>(Log.size() - 150));
        }
    }

    ImVec4 SaveManagerApp::Impl::Good() { return ImVec4(0.25f, 0.85f, 0.45f, 1.0f); }
    ImVec4 SaveManagerApp::Impl::Warn() { return ImVec4(0.95f, 0.75f, 0.20f, 1.0f); }
    ImVec4 SaveManagerApp::Impl::Bad() { return ImVec4(0.92f, 0.28f, 0.25f, 1.0f); }
    ImVec4 SaveManagerApp::Impl::Accent() { return ImVec4(0.18f, 0.74f, 0.82f, 1.0f); }
}
