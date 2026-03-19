#pragma once

#include <string>
#include <vector>

#include "Models.hpp"

namespace manifold
{
    class SaveManager
    {
    public:
        void Load();
        bool SaveConfig() const;

        std::vector<GameDefinition>& Games();
        const std::vector<GameDefinition>& Games() const;
        const std::vector<BackupEntry>& Backups() const;

        int SelectedGame() const;
        int SelectedBackup() const;
        int SelectedProfile() const;

        void SetSelectedGame(int index);
        void SetSelectedProfile(int index);
        void SetSelectedBackup(int index);

        GameDefinition* CurrentGame();
        const GameDefinition* CurrentGame() const;
        GameProfile* CurrentProfile();
        const GameProfile* CurrentProfile() const;
        const BackupEntry* CurrentBackup() const;

        void RefreshBackups();
        void AddGame();
        void RemoveCurrentGame();
        void AddProfileToCurrentGame();
        void RemoveCurrentProfile();
        void SyncSelectedProfileFromActive();

        OperationResult CreateBackupForCurrentProfile(const std::string& reason);
        OperationResult RestoreSelectedBackup(bool clearDestinationFirst, bool backupBeforeOverwrite);
        OperationResult SwitchToProfile(int newProfileIndex, bool createAutoBackup);
        OperationResult DeleteSelectedBackup();

    private:
        bool PersistState();
        void ResetSelections();
        void NormalizeSelectionState();

        std::string MakeUniqueGameId(const std::string& base) const;
        std::string MakeUniqueProfileId(const GameDefinition& game, const std::string& base) const;
        void EnforceBackupLimit(const GameDefinition& game, const GameProfile& profile);

    private:
        std::vector<GameDefinition> m_Games;
        std::vector<BackupEntry> m_Backups;
        int m_SelectedGame = -1;
        int m_SelectedProfile = -1;
        int m_SelectedBackup = -1;
    };
}