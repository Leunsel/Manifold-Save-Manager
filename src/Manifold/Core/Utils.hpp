#pragma once

#include <array>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>
#include <windows.h>

#include "Models.hpp"

namespace manifold
{
    namespace fs = std::filesystem;

    std::wstring Utf8ToWide(const std::string& text);
    std::string WideToUtf8(const std::wstring& text);

    std::string Trim(std::string value);
    std::string SanitizeId(std::string value);
    std::string FormatTime(std::time_t value);
    std::string NowStamp();
    std::string HumanSize(std::uintmax_t bytes);
    std::string EscapePipe(std::string value);

    bool DirectoryExists(const std::string& path);
    bool FileExists(const fs::path& path);
    bool OpenInExplorer(const std::string& path);

    std::optional<std::string> PickFolderDialog(HWND owner, const std::wstring& title);

    std::string GetAppRoot();
    fs::path GetConfigPath();
    fs::path GetGamesRoot();
    fs::path BuildGameRoot(const std::string& gameId);
    fs::path BuildProfileRoot(const std::string& gameId, const std::string& profileId);
    fs::path BuildProfileBackupRoot(const std::string& gameId, const std::string& profileId);

    std::optional<std::string> ComputeFileMd5(const fs::path& filePath);
    std::optional<std::string> ComputeDirectoryAggregateMd5(const fs::path& directory);
    std::pair<std::uintmax_t, std::size_t> CalculateDirectoryStats(const fs::path& path);
    std::vector<fs::path> GatherSaveItems(const GameDefinition& game);
    OperationResult CopyItemIntoContainer(const fs::path& sourceRoot, const fs::path& sourceItem, const fs::path& containerRoot, bool overwrite);
    OperationResult ClearDirectoryContents(const fs::path& directory);
    std::vector<BackupFilePreview> BuildPreviewFiles(const fs::path& root);

    bool SaveConfigFile(const std::vector<GameDefinition>& games);
    std::vector<GameDefinition> LoadConfigFile();
    std::vector<BackupEntry> LoadBackupsForProfile(const GameDefinition& game, const GameProfile& profile);

    bool OpenUrl(const char* url);

    template <size_t N>
    void WriteToBuffer(std::array<char, N>& buffer, const std::string& text)
    {
        std::snprintf(buffer.data(), buffer.size(), "%s", text.c_str());
    }
}
