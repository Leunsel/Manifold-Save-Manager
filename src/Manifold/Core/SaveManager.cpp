#include "SaveManager.hpp"

#include <algorithm>
#include <fstream>
#include <system_error>

#include "Utils.hpp"

namespace manifold
{
    bool SaveManager::PersistState()
    {
        return SaveConfig();
    }

    void SaveManager::ResetSelections()
    {
        m_SelectedGame = -1;
        m_SelectedProfile = -1;
        m_SelectedBackup = -1;
        m_Backups.clear();
    }

    void SaveManager::NormalizeSelectionState()
    {
        if (m_Games.empty())
        {
            ResetSelections();
            return;
        }

        m_SelectedGame = std::clamp(m_SelectedGame, 0, static_cast<int>(m_Games.size()) - 1);

        GameDefinition* game = CurrentGame();
        if (!game)
        {
            ResetSelections();
            return;
        }

        if (game->Profiles.empty())
        {
            GameProfile profile;
            profile.Id = "vanilla";
            profile.Name = "Vanilla";
            profile.Description = "Default profile";
            profile.AutoBackupOnSwitch = true;
            profile.BackupLimit = 20;
            game->Profiles.push_back(std::move(profile));
        }

        bool activeFound = false;
        for (int i = 0; i < static_cast<int>(game->Profiles.size()); ++i)
        {
            if (game->Profiles[i].Id == game->ActiveProfileId)
            {
                m_SelectedProfile = i;
                activeFound = true;
                break;
            }
        }

        if (!activeFound)
        {
            m_SelectedProfile = 0;
            game->ActiveProfileId = game->Profiles[0].Id;
        }

        m_SelectedProfile = std::clamp(m_SelectedProfile, 0, static_cast<int>(game->Profiles.size()) - 1);

        if (m_SelectedBackup >= static_cast<int>(m_Backups.size()))
            m_SelectedBackup = -1;
    }

    void SaveManager::Load()
    {
        m_Games = LoadConfigFile();

        if (m_Games.empty())
        {
            GameDefinition game;
            game.Id = "new_game";
            game.DisplayName = "New Game";
            game.ScopeMode = SaveScopeMode::FolderMode;
            game.Profiles.push_back({ "vanilla", "Vanilla", "Default profile", "", true, true, 20 });
            game.ActiveProfileId = "vanilla";
            m_Games.push_back(std::move(game));
        }

        m_SelectedGame = 0;
        NormalizeSelectionState();
        RefreshBackups();
        PersistState();
    }

    bool SaveManager::SaveConfig() const { return SaveConfigFile(m_Games); }
    std::vector<GameDefinition>& SaveManager::Games() { return m_Games; }
    const std::vector<GameDefinition>& SaveManager::Games() const { return m_Games; }
    const std::vector<BackupEntry>& SaveManager::Backups() const { return m_Backups; }
    int SaveManager::SelectedGame() const { return m_SelectedGame; }
    int SaveManager::SelectedBackup() const { return m_SelectedBackup; }
    int SaveManager::SelectedProfile() const { return m_SelectedProfile; }

    void SaveManager::SetSelectedGame(int index)
    {
        if (index < 0 || index >= static_cast<int>(m_Games.size()))
            return;

        if (m_SelectedGame == index)
            return;

        m_SelectedGame = index;
        m_SelectedBackup = -1;
        SyncSelectedProfileFromActive();
        RefreshBackups();
    }

    void SaveManager::SetSelectedProfile(int index)
    {
        GameDefinition* game = CurrentGame();
        if (!game || index < 0 || index >= static_cast<int>(game->Profiles.size()))
            return;

        if (m_SelectedProfile == index && game->ActiveProfileId == game->Profiles[index].Id)
            return;

        m_SelectedProfile = index;
        game->ActiveProfileId = game->Profiles[index].Id;
        m_SelectedBackup = -1;
        RefreshBackups();
        PersistState();
    }

    void SaveManager::SetSelectedBackup(int index)
    {
        if (index >= -1 && index < static_cast<int>(m_Backups.size()))
            m_SelectedBackup = index;
    }

    GameDefinition* SaveManager::CurrentGame()
    {
        if (m_SelectedGame < 0 || m_SelectedGame >= static_cast<int>(m_Games.size()))
            return nullptr;
        return &m_Games[m_SelectedGame];
    }

    const GameDefinition* SaveManager::CurrentGame() const
    {
        if (m_SelectedGame < 0 || m_SelectedGame >= static_cast<int>(m_Games.size()))
            return nullptr;
        return &m_Games[m_SelectedGame];
    }

    GameProfile* SaveManager::CurrentProfile()
    {
        GameDefinition* game = CurrentGame();
        if (!game || m_SelectedProfile < 0 || m_SelectedProfile >= static_cast<int>(game->Profiles.size()))
            return nullptr;
        return &game->Profiles[m_SelectedProfile];
    }

    const GameProfile* SaveManager::CurrentProfile() const
    {
        const GameDefinition* game = CurrentGame();
        if (!game || m_SelectedProfile < 0 || m_SelectedProfile >= static_cast<int>(game->Profiles.size()))
            return nullptr;
        return &game->Profiles[m_SelectedProfile];
    }

    const BackupEntry* SaveManager::CurrentBackup() const
    {
        if (m_SelectedBackup < 0 || m_SelectedBackup >= static_cast<int>(m_Backups.size()))
            return nullptr;
        return &m_Backups[m_SelectedBackup];
    }

    void SaveManager::RefreshBackups()
    {
        m_Backups.clear();

        const GameDefinition* game = CurrentGame();
        const GameProfile* profile = CurrentProfile();

        if (game && profile)
            m_Backups = LoadBackupsForProfile(*game, *profile);

        if (m_SelectedBackup >= static_cast<int>(m_Backups.size()))
            m_SelectedBackup = -1;
    }

    void SaveManager::AddGame()
    {
        GameDefinition game;
        game.Id = MakeUniqueGameId("new_game");
        game.DisplayName = "New Game";
        game.ScopeMode = SaveScopeMode::FolderMode;
        game.Profiles.push_back({ "vanilla", "Vanilla", "Default profile", "", true, true, 20 });
        game.ActiveProfileId = "vanilla";

        m_Games.push_back(std::move(game));
        m_SelectedGame = static_cast<int>(m_Games.size()) - 1;
        SyncSelectedProfileFromActive();
        RefreshBackups();
        PersistState();
    }

    void SaveManager::RemoveCurrentGame()
    {
        if (m_SelectedGame < 0 || m_SelectedGame >= static_cast<int>(m_Games.size()))
            return;

        m_Games.erase(m_Games.begin() + m_SelectedGame);

        if (m_Games.empty())
        {
            ResetSelections();
            PersistState();
            return;
        }

        m_SelectedGame = std::clamp(m_SelectedGame, 0, static_cast<int>(m_Games.size()) - 1);
        SyncSelectedProfileFromActive();
        RefreshBackups();
        PersistState();
    }

    void SaveManager::AddProfileToCurrentGame()
    {
        GameDefinition* game = CurrentGame();
        if (!game)
            return;

        GameProfile profile;
        profile.Id = MakeUniqueProfileId(*game, "profile");
        profile.Name = "New Profile";
        profile.Description = "Additional save profile";
        profile.AutoBackupOnSwitch = true;
        profile.BackupLimit = 20;

        game->Profiles.push_back(profile);
        game->ActiveProfileId = profile.Id;

        SyncSelectedProfileFromActive();
        RefreshBackups();
        PersistState();
    }

    void SaveManager::RemoveCurrentProfile()
    {
        GameDefinition* game = CurrentGame();
        if (!game ||
            m_SelectedProfile < 0 ||
            m_SelectedProfile >= static_cast<int>(game->Profiles.size()) ||
            game->Profiles.size() <= 1)
        {
            return;
        }

        game->Profiles.erase(game->Profiles.begin() + m_SelectedProfile);
        m_SelectedProfile = std::clamp(m_SelectedProfile, 0, static_cast<int>(game->Profiles.size()) - 1);
        game->ActiveProfileId = game->Profiles[m_SelectedProfile].Id;

        RefreshBackups();
        PersistState();
    }

    OperationResult SaveManager::CreateBackupForCurrentProfile(const std::string& reason)
    {
        GameDefinition* game = CurrentGame();
        GameProfile* profile = CurrentProfile();

        if (!game) return { false, "No game selected." };
        if (!profile) return { false, "No profile selected." };
        if (game->SavePath.empty()) return { false, "Save path is empty." };
        if (!DirectoryExists(game->SavePath)) return { false, "Save path does not exist." };

        const auto items = GatherSaveItems(*game);
        if (items.empty()) return { false, "No save items matched the current scope." };

        const fs::path saveRoot = Utf8ToWide(game->SavePath);
        const fs::path backupRoot = BuildProfileBackupRoot(game->Id, profile->Id);
        const fs::path destination = backupRoot / Utf8ToWide(NowStamp());

        std::error_code ec;
        fs::create_directories(destination, ec);
        if (ec) return { false, "Backup folder could not be created: " + ec.message() };

        for (const fs::path& item : items)
        {
            OperationResult result = CopyItemIntoContainer(saveRoot, item, destination, false);
            if (!result.Success)
                return result;
        }

        if (const auto hash = ComputeDirectoryAggregateMd5(destination); hash.has_value())
        {
            std::ofstream hashFile(destination / L".backup.md5", std::ios::trunc);
            hashFile << *hash << '\n';
        }

        if (!reason.empty())
        {
            std::ofstream reasonFile(destination / L".reason.txt", std::ios::trunc);
            reasonFile << reason << '\n';
        }

        EnforceBackupLimit(*game, *profile);
        RefreshBackups();
        return { true, "Backup created." };
    }

    OperationResult SaveManager::RestoreSelectedBackup(bool clearDestinationFirst, bool backupBeforeOverwrite)
    {
        GameDefinition* game = CurrentGame();
        GameProfile* profile = CurrentProfile();
        const BackupEntry* selectedBackup = CurrentBackup();

        if (!game) return { false, "No game selected." };
        if (!profile) return { false, "No profile selected." };
        if (!selectedBackup) return { false, "No backup selected." };
        if (game->SavePath.empty()) return { false, "Save path is empty." };

        const std::string backupFullPath = selectedBackup->FullPath;
        const fs::path source = Utf8ToWide(backupFullPath);
        const fs::path destination = Utf8ToWide(game->SavePath);

        std::error_code ec;

        if (!fs::exists(source, ec) || ec)
            return { false, "Selected backup path does not exist." };

        if (!fs::is_directory(source, ec) || ec)
            return { false, "Selected backup path is not a directory." };

        bool hasPayload = false;
        for (fs::recursive_directory_iterator it(source, ec), end; !ec && it != end; ++it)
        {
            const auto name = it->path().filename();
            if (name == L".backup.md5" || name == L".reason.txt")
                continue;

            hasPayload = true;
            break;
        }

        if (ec)
            return { false, "Backup enumeration failed: " + ec.message() };

        if (!hasPayload)
            return { false, "Selected backup is empty." };

        if (backupBeforeOverwrite)
        {
            auto result = CreateBackupForCurrentProfile("Auto backup before restore");
            if (!result.Success)
                return { false, "Restore aborted because the pre-restore backup failed: " + result.Message };
        }

        if (clearDestinationFirst)
        {
            auto clearResult = ClearDirectoryContents(destination);
            if (!clearResult.Success)
                return clearResult;
        }

        fs::create_directories(destination, ec);
        if (ec)
            return { false, "Destination folder could not be created: " + ec.message() };

        for (fs::recursive_directory_iterator it(source, ec), end; !ec && it != end; ++it)
        {
            const auto& entry = *it;
            const auto name = entry.path().filename();

            if (name == L".backup.md5" || name == L".reason.txt")
                continue;

            fs::path relative = fs::relative(entry.path(), source, ec);
            if (ec)
                return { false, "Relative restore path could not be resolved: " + ec.message() };

            fs::path target = destination / relative;

            if (entry.is_directory(ec))
            {
                if (ec)
                    return { false, "Restore directory check failed: " + ec.message() };

                fs::create_directories(target, ec);
                if (ec)
                    return { false, "Target restore directory could not be created: " + ec.message() };
            }
            else if (entry.is_regular_file(ec))
            {
                if (ec)
                    return { false, "Restore file check failed: " + ec.message() };

                fs::create_directories(target.parent_path(), ec);
                if (ec)
                    return { false, "Target restore parent could not be created: " + ec.message() };

                fs::copy_file(entry.path(), target, fs::copy_options::overwrite_existing, ec);
                if (ec)
                    return { false, "Restore file copy failed: " + ec.message() };
            }
        }

        if (ec)
            return { false, "Backup enumeration failed: " + ec.message() };

        return { true, "Backup restored to active save path." };
    }

    OperationResult SaveManager::SwitchToProfile(int newProfileIndex, bool createAutoBackup)
    {
        GameDefinition* game = CurrentGame();
        if (!game) return { false, "No game selected." };
        if (newProfileIndex < 0 || newProfileIndex >= static_cast<int>(game->Profiles.size()))
            return { false, "Invalid target profile." };
        if (newProfileIndex == m_SelectedProfile)
            return { true, "Profile already active." };

        GameProfile* current = CurrentProfile();
        if (current && createAutoBackup && current->AutoBackupOnSwitch)
        {
            auto backupResult = CreateBackupForCurrentProfile("Auto backup before profile switch");
            if (!backupResult.Success)
                return { false, "Profile switch aborted because the automatic backup failed: " + backupResult.Message };
        }

        m_SelectedProfile = newProfileIndex;
        game->ActiveProfileId = game->Profiles[m_SelectedProfile].Id;
        RefreshBackups();
        PersistState();

        return { true, "Active profile switched." };
    }

    OperationResult SaveManager::DeleteSelectedBackup()
    {
        const BackupEntry* backup = CurrentBackup();
        if (!backup) return { false, "No backup selected." };

        const std::string fullPath = backup->FullPath;

        std::error_code ec;
        fs::remove_all(Utf8ToWide(fullPath), ec);
        if (ec)
            return { false, "Backup could not be deleted: " + ec.message() };

        RefreshBackups();
        m_SelectedBackup = -1;
        return { true, "Backup deleted." };
    }

    void SaveManager::SyncSelectedProfileFromActive()
    {
        GameDefinition* game = CurrentGame();
        if (!game)
        {
            m_SelectedProfile = -1;
            return;
        }

        if (game->Profiles.empty())
        {
            m_SelectedProfile = -1;
            game->ActiveProfileId.clear();
            return;
        }

        for (int i = 0; i < static_cast<int>(game->Profiles.size()); ++i)
        {
            if (game->Profiles[i].Id == game->ActiveProfileId)
            {
                m_SelectedProfile = i;
                return;
            }
        }

        m_SelectedProfile = 0;
        game->ActiveProfileId = game->Profiles[0].Id;
    }

    std::string SaveManager::MakeUniqueGameId(const std::string& base) const
    {
        std::string id = SanitizeId(base);
        if (id.empty())
            id = "game";

        int suffix = 1;
        auto exists = [&](const std::string& value)
            {
                return std::any_of(m_Games.begin(), m_Games.end(),
                    [&](const GameDefinition& g) { return g.Id == value; });
            };

        std::string candidate = id;
        while (exists(candidate))
            candidate = id + "_" + std::to_string(suffix++);

        return candidate;
    }

    std::string SaveManager::MakeUniqueProfileId(const GameDefinition& game, const std::string& base) const
    {
        std::string id = SanitizeId(base);
        if (id.empty())
            id = "profile";

        int suffix = 1;
        auto exists = [&](const std::string& value)
            {
                return std::any_of(game.Profiles.begin(), game.Profiles.end(),
                    [&](const GameProfile& p) { return p.Id == value; });
            };

        std::string candidate = id;
        while (exists(candidate))
            candidate = id + "_" + std::to_string(suffix++);

        return candidate;
    }

    void SaveManager::EnforceBackupLimit(const GameDefinition& game, const GameProfile& profile)
    {
        auto backups = LoadBackupsForProfile(game, profile);
        if (profile.BackupLimit <= 0 || static_cast<int>(backups.size()) <= profile.BackupLimit)
            return;

        for (size_t i = static_cast<size_t>(profile.BackupLimit); i < backups.size(); ++i)
        {
            std::error_code ec;
            fs::remove_all(Utf8ToWide(backups[i].FullPath), ec);
        }
    }
}