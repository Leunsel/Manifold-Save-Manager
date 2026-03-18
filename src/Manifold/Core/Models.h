#pragma once

#include <cstddef>
#include <cstdint>
#include <ctime>
#include <string>
#include <vector>

#include "../../ImGui/imgui.h"

namespace manifold
{
    enum class SaveScopeMode
    {
        FileWhitelist = 0,
        FolderMode = 1,
        HybridMode = 2,
    };

    const char* ToString(SaveScopeMode mode);

    struct ScopeRule
    {
        std::string RelativePath;
        bool Enabled = true;
    };

    struct LogEntry
    {
        std::string Message;
        ImVec4 Color{};
    };

    struct GameProfile
    {
        std::string Id;
        std::string Name;
        std::string Description;
        std::string Notes;
        bool Favorite = false;
        bool AutoBackupOnSwitch = true;
        int BackupLimit = 20;
    };

    struct GameDefinition
    {
        std::string Id;
        std::string DisplayName;
        std::string SavePath;
        std::string ProcessName;
        std::string Notes;
        bool Enabled = true;
        SaveScopeMode ScopeMode = SaveScopeMode::FolderMode;
        std::vector<ScopeRule> Whitelist;
        std::vector<ScopeRule> FolderRules;
        std::vector<GameProfile> Profiles;
        std::string ActiveProfileId;
    };

    struct BackupFilePreview
    {
        std::string RelativePath;
        std::uintmax_t Size = 0;
    };

    struct BackupEntry
    {
        std::string Id;
        std::string Name;
        std::string GameId;
        std::string ProfileId;
        std::string FullPath;
        std::string CreatedAtDisplay;
        std::string BackupHash;
        std::string Reason;
        std::uintmax_t TotalBytes = 0;
        std::size_t FileCount = 0;
        std::time_t CreatedAt = 0;
        std::vector<BackupFilePreview> PreviewFiles;
    };

    struct OperationResult
    {
        bool Success = false;
        std::string Message;
    };
}
