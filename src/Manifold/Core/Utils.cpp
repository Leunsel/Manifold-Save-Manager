#include "Utils.hpp"

#include <algorithm>
#include <bcrypt.h>
#include <cctype>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <map>
#include <shellapi.h>
#include <shlobj.h>
#include <sstream>
#include <system_error>
#include <wrl/client.h>

#pragma comment(lib, "Shell32.lib")
#pragma comment(lib, "Ole32.lib")
#pragma comment(lib, "Comdlg32.lib")
#pragma comment(lib, "bcrypt.lib")

using Microsoft::WRL::ComPtr;

namespace manifold
{
    namespace
    {
        std::string BytesToHex(const std::vector<unsigned char>& bytes)
        {
            std::ostringstream oss;
            oss << std::hex << std::setfill('0');
            for (unsigned char byte : bytes)
            {
                oss << std::setw(2) << static_cast<int>(byte);
            }
            return oss.str();
        }

        std::optional<std::vector<unsigned char>> ComputeHashBytes(const std::vector<unsigned char>& data, LPCWSTR algorithmName)
        {
            BCRYPT_ALG_HANDLE algorithm = nullptr;
            BCRYPT_HASH_HANDLE hash = nullptr;
            PUCHAR hashObject = nullptr;
            DWORD hashObjectLength = 0;
            DWORD hashLength = 0;
            DWORD resultLength = 0;

            auto cleanup = [&]()
            {
                if (hash) BCryptDestroyHash(hash);
                if (hashObject) HeapFree(GetProcessHeap(), 0, hashObject);
                if (algorithm) BCryptCloseAlgorithmProvider(algorithm, 0);
            };

            if (!BCRYPT_SUCCESS(BCryptOpenAlgorithmProvider(&algorithm, algorithmName, nullptr, 0)))
            {
                cleanup();
                return std::nullopt;
            }

            if (!BCRYPT_SUCCESS(BCryptGetProperty(algorithm, BCRYPT_OBJECT_LENGTH, reinterpret_cast<PUCHAR>(&hashObjectLength), sizeof(hashObjectLength), &resultLength, 0)))
                return cleanup(), std::nullopt;
            if (!BCRYPT_SUCCESS(BCryptGetProperty(algorithm, BCRYPT_HASH_LENGTH, reinterpret_cast<PUCHAR>(&hashLength), sizeof(hashLength), &resultLength, 0)))
                return cleanup(), std::nullopt;

            hashObject = static_cast<PUCHAR>(HeapAlloc(GetProcessHeap(), 0, hashObjectLength));
            if (!hashObject)
                return cleanup(), std::nullopt;

            std::vector<unsigned char> hashBuffer(hashLength);
            if (!BCRYPT_SUCCESS(BCryptCreateHash(algorithm, &hash, hashObject, hashObjectLength, nullptr, 0, 0)))
                return cleanup(), std::nullopt;

            if (!data.empty())
            {
                if (!BCRYPT_SUCCESS(BCryptHashData(hash, const_cast<PUCHAR>(data.data()), static_cast<ULONG>(data.size()), 0)))
                    return cleanup(), std::nullopt;
            }

            if (!BCRYPT_SUCCESS(BCryptFinishHash(hash, hashBuffer.data(), hashLength, 0)))
                return cleanup(), std::nullopt;

            cleanup();
            return hashBuffer;
        }
    }

    std::wstring Utf8ToWide(const std::string& text)
    {
        if (text.empty()) return {};
        const int size = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), static_cast<int>(text.size()), nullptr, 0);
        std::wstring result(static_cast<size_t>(size), L'\0');
        MultiByteToWideChar(CP_UTF8, 0, text.c_str(), static_cast<int>(text.size()), result.data(), size);
        return result;
    }

    std::string WideToUtf8(const std::wstring& text)
    {
        if (text.empty()) return {};
        const int size = WideCharToMultiByte(CP_UTF8, 0, text.c_str(), static_cast<int>(text.size()), nullptr, 0, nullptr, nullptr);
        std::string result(static_cast<size_t>(size), '\0');
        WideCharToMultiByte(CP_UTF8, 0, text.c_str(), static_cast<int>(text.size()), result.data(), size, nullptr, nullptr);
        return result;
    }

    std::string Trim(std::string value)
    {
        auto isNotSpace = [](unsigned char c) { return !std::isspace(c); };
        value.erase(value.begin(), std::find_if(value.begin(), value.end(), isNotSpace));
        value.erase(std::find_if(value.rbegin(), value.rend(), isNotSpace).base(), value.end());
        return value;
    }

    std::string SanitizeId(std::string value)
    {
        value = Trim(value);
        for (char& c : value)
        {
            if (!(std::isalnum(static_cast<unsigned char>(c)) || c == '_' || c == '-'))
                c = '_';
        }
        if (value.empty()) value = "Item";
        return value;
    }

    std::string FormatTime(std::time_t value)
    {
        if (value == 0) return "Unknown";
        std::tm tmLocal{};
        localtime_s(&tmLocal, &value);
        char buffer[64]{};
        std::strftime(buffer, sizeof(buffer), "%d.%m.%Y %H:%M:%S", &tmLocal);
        return std::string(buffer);
    }

    std::string NowStamp()
    {
        std::time_t now = std::time(nullptr);
        std::tm tmLocal{};
        localtime_s(&tmLocal, &now);
        char buffer[64]{};
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d_%H-%M-%S", &tmLocal);
        return std::string(buffer);
    }

    std::string HumanSize(std::uintmax_t bytes)
    {
        static constexpr const char* suffixes[] = { "B", "KB", "MB", "GB", "TB" };
        double size = static_cast<double>(bytes);
        size_t suffix = 0;
        while (size >= 1024.0 && suffix < std::size(suffixes) - 1)
        {
            size /= 1024.0;
            ++suffix;
        }

        std::ostringstream oss;
        oss << std::fixed << std::setprecision(size < 10.0 && suffix > 0 ? 1 : 0) << size << ' ' << suffixes[suffix];
        return oss.str();
    }

    std::string EscapePipe(std::string value)
    {
        for (char& c : value)
        {
            if (c == '|') c = '/';
            if (c == '\n' || c == '\r') c = ' ';
        }
        return value;
    }

    bool DirectoryExists(const std::string& path)
    {
        std::error_code ec;
        const fs::path p = Utf8ToWide(path);
        return fs::exists(p, ec) && fs::is_directory(p, ec);
    }

    bool FileExists(const fs::path& path)
    {
        std::error_code ec;
        return fs::exists(path, ec) && fs::is_regular_file(path, ec);
    }

    std::string GetAppRoot()
    {
        PWSTR knownFolder = nullptr;
        if (FAILED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &knownFolder)) || knownFolder == nullptr)
            return ".\\ManifoldSaveManager";

        fs::path root = fs::path(knownFolder) / L"Manifold" / L"SaveManager";
        CoTaskMemFree(knownFolder);

        std::error_code ec;
        fs::create_directories(root, ec);
        return WideToUtf8(root.wstring());
    }

    fs::path GetConfigPath()
    {
        return fs::path(Utf8ToWide(GetAppRoot())) / L"manifold_save_manager.cfg";
    }

    fs::path GetGamesRoot()
    {
        fs::path root = fs::path(Utf8ToWide(GetAppRoot())) / L"Games";
        std::error_code ec;
        fs::create_directories(root, ec);
        return root;
    }

    fs::path BuildGameRoot(const std::string& gameId)
    {
        fs::path root = GetGamesRoot() / Utf8ToWide(gameId);
        std::error_code ec;
        fs::create_directories(root / L"Profiles", ec);
        return root;
    }

    fs::path BuildProfileRoot(const std::string& gameId, const std::string& profileId)
    {
        fs::path root = BuildGameRoot(gameId) / L"Profiles" / Utf8ToWide(profileId);
        std::error_code ec;
        fs::create_directories(root / L"Backups", ec);
        return root;
    }

    fs::path BuildProfileBackupRoot(const std::string& gameId, const std::string& profileId)
    {
        fs::path root = BuildProfileRoot(gameId, profileId) / L"Backups";
        std::error_code ec;
        fs::create_directories(root, ec);
        return root;
    }

    bool OpenInExplorer(const std::string& path)
    {
        if (path.empty()) return false;
        const HINSTANCE result = ShellExecuteW(nullptr, L"open", Utf8ToWide(path).c_str(), nullptr, nullptr, SW_SHOWDEFAULT);
        return reinterpret_cast<INT_PTR>(result) > 32;
    }

    std::optional<std::string> PickFolderDialog(HWND owner, const std::wstring& title)
    {
        ComPtr<IFileDialog> dialog;
        if (FAILED(CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&dialog))))
            return std::nullopt;

        DWORD options = 0;
        dialog->GetOptions(&options);
        dialog->SetOptions(options | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM | FOS_PATHMUSTEXIST);
        dialog->SetTitle(title.c_str());

        if (FAILED(dialog->Show(owner)))
            return std::nullopt;

        ComPtr<IShellItem> item;
        if (FAILED(dialog->GetResult(&item)))
            return std::nullopt;

        PWSTR folderPath = nullptr;
        if (FAILED(item->GetDisplayName(SIGDN_FILESYSPATH, &folderPath)) || folderPath == nullptr)
            return std::nullopt;

        std::wstring widePath(folderPath);
        CoTaskMemFree(folderPath);
        return WideToUtf8(widePath);
    }

    std::optional<std::string> ComputeFileMd5(const fs::path& filePath)
    {
        std::ifstream file(filePath, std::ios::binary);
        if (!file.is_open()) return std::nullopt;

        BCRYPT_ALG_HANDLE algorithm = nullptr;
        BCRYPT_HASH_HANDLE hash = nullptr;
        PUCHAR hashObject = nullptr;
        DWORD hashObjectLength = 0;
        DWORD hashLength = 0;
        DWORD resultLength = 0;

        auto cleanup = [&]()
        {
            if (hash) BCryptDestroyHash(hash);
            if (hashObject) HeapFree(GetProcessHeap(), 0, hashObject);
            if (algorithm) BCryptCloseAlgorithmProvider(algorithm, 0);
        };

        if (!BCRYPT_SUCCESS(BCryptOpenAlgorithmProvider(&algorithm, BCRYPT_MD5_ALGORITHM, nullptr, 0)))
            return cleanup(), std::nullopt;
        if (!BCRYPT_SUCCESS(BCryptGetProperty(algorithm, BCRYPT_OBJECT_LENGTH, reinterpret_cast<PUCHAR>(&hashObjectLength), sizeof(hashObjectLength), &resultLength, 0)))
            return cleanup(), std::nullopt;
        if (!BCRYPT_SUCCESS(BCryptGetProperty(algorithm, BCRYPT_HASH_LENGTH, reinterpret_cast<PUCHAR>(&hashLength), sizeof(hashLength), &resultLength, 0)))
            return cleanup(), std::nullopt;

        hashObject = static_cast<PUCHAR>(HeapAlloc(GetProcessHeap(), 0, hashObjectLength));
        if (!hashObject) return cleanup(), std::nullopt;

        std::vector<unsigned char> hashBuffer(hashLength);
        if (!BCRYPT_SUCCESS(BCryptCreateHash(algorithm, &hash, hashObject, hashObjectLength, nullptr, 0, 0)))
            return cleanup(), std::nullopt;

        std::vector<unsigned char> buffer(64 * 1024);
        while (file)
        {
            file.read(reinterpret_cast<char*>(buffer.data()), static_cast<std::streamsize>(buffer.size()));
            const std::streamsize bytesRead = file.gcount();
            if (bytesRead > 0)
            {
                if (!BCRYPT_SUCCESS(BCryptHashData(hash, buffer.data(), static_cast<ULONG>(bytesRead), 0)))
                    return cleanup(), std::nullopt;
            }
        }

        if (!BCRYPT_SUCCESS(BCryptFinishHash(hash, hashBuffer.data(), hashLength, 0)))
            return cleanup(), std::nullopt;

        cleanup();
        return BytesToHex(hashBuffer);
    }

    std::pair<std::uintmax_t, std::size_t> CalculateDirectoryStats(const fs::path& path)
    {
        std::uintmax_t totalBytes = 0;
        std::size_t fileCount = 0;
        std::error_code ec;
        if (!fs::exists(path, ec)) return { 0, 0 };

        for (const auto& entry : fs::recursive_directory_iterator(path, ec))
        {
            if (ec) break;
            if (entry.is_regular_file(ec))
            {
                ++fileCount;
                totalBytes += entry.file_size(ec);
            }
        }
        return { totalBytes, fileCount };
    }

    std::vector<fs::path> GatherSaveItems(const GameDefinition& game)
    {
        std::vector<fs::path> result;
        std::error_code ec;
        fs::path saveRoot = Utf8ToWide(game.SavePath);
        if (!fs::exists(saveRoot, ec)) return result;

        auto addIfExists = [&](const std::string& rel)
        {
            fs::path p = saveRoot / Utf8ToWide(rel);
            if (fs::exists(p, ec)) result.push_back(p);
        };

        if (game.ScopeMode == SaveScopeMode::FolderMode)
        {
            result.push_back(saveRoot);
            return result;
        }

        if (game.ScopeMode == SaveScopeMode::FileWhitelist || game.ScopeMode == SaveScopeMode::HybridMode)
        {
            for (const ScopeRule& rule : game.Whitelist)
                if (rule.Enabled && !Trim(rule.RelativePath).empty()) addIfExists(rule.RelativePath);
        }

        if (game.ScopeMode == SaveScopeMode::HybridMode)
        {
            for (const ScopeRule& rule : game.FolderRules)
                if (rule.Enabled && !Trim(rule.RelativePath).empty()) addIfExists(rule.RelativePath);
        }

        std::sort(result.begin(), result.end());
        result.erase(std::unique(result.begin(), result.end()), result.end());
        return result;
    }

    OperationResult CopyItemIntoContainer(const fs::path& sourceRoot, const fs::path& sourceItem, const fs::path& containerRoot, bool overwrite)
    {
        std::error_code ec;
        fs::path relative = fs::relative(sourceItem, sourceRoot, ec);
        if (ec) return { false, "Relative path could not be resolved." };

        fs::path target = containerRoot / relative;
        if (fs::is_directory(sourceItem, ec))
        {
            fs::create_directories(target, ec);
            if (ec) return { false, "Target folder could not be created." };

            const auto options = overwrite ? (fs::copy_options::recursive | fs::copy_options::overwrite_existing) : fs::copy_options::recursive;
            fs::copy(sourceItem, target, options, ec);
            return ec ? OperationResult{ false, "Directory copy failed: " + ec.message() } : OperationResult{ true, "Directory copied." };
        }

        fs::create_directories(target.parent_path(), ec);
        if (ec) return { false, "Target parent folder could not be created." };

        fs::copy_file(sourceItem, target, overwrite ? fs::copy_options::overwrite_existing : fs::copy_options::none, ec);
        return ec ? OperationResult{ false, "File copy failed: " + ec.message() } : OperationResult{ true, "File copied." };
    }

    OperationResult ClearDirectoryContents(const fs::path& directory)
    {
        std::error_code ec;
        if (!fs::exists(directory, ec))
        {
            fs::create_directories(directory, ec);
            return ec ? OperationResult{ false, "Save folder could not be created." } : OperationResult{ true, "Save folder created." };
        }

        for (const auto& entry : fs::directory_iterator(directory, ec))
        {
            if (ec) return { false, "Save folder could not be enumerated." };
            fs::remove_all(entry.path(), ec);
            if (ec) return { false, "Old save contents could not be removed." };
        }

        return { true, "Save folder cleared." };
    }

    std::optional<std::string> ComputeDirectoryAggregateMd5(const fs::path& directory)
    {
        std::error_code ec;
        if (!fs::exists(directory, ec)) return std::nullopt;

        std::vector<fs::path> files;
        for (const auto& entry : fs::recursive_directory_iterator(directory, ec))
        {
            if (ec) return std::nullopt;
            if (entry.is_regular_file(ec) && entry.path().filename() != L".backup.md5")
                files.push_back(entry.path());
        }

        std::sort(files.begin(), files.end());

        std::ostringstream manifest;
        for (const fs::path& file : files)
        {
            const auto fileHash = ComputeFileMd5(file);
            if (!fileHash.has_value()) return std::nullopt;

            const fs::path relative = fs::relative(file, directory, ec);
            if (ec) return std::nullopt;

            const auto fileSize = fs::file_size(file, ec);
            if (ec) return std::nullopt;

            manifest << relative.generic_string() << '|' << fileSize << '|' << *fileHash << '\n';
        }

        const std::string manifestText = manifest.str();
        std::vector<unsigned char> bytes(manifestText.begin(), manifestText.end());
        const auto aggregate = ComputeHashBytes(bytes, BCRYPT_MD5_ALGORITHM);
        return aggregate.has_value() ? std::optional<std::string>(BytesToHex(*aggregate)) : std::nullopt;
    }

    std::vector<BackupFilePreview> BuildPreviewFiles(const fs::path& root)
    {
        std::vector<BackupFilePreview> files;
        std::error_code ec;
        for (const auto& entry : fs::recursive_directory_iterator(root, ec))
        {
            if (ec) break;
            if (!entry.is_regular_file(ec)) continue;
            if (entry.path().filename() == L".backup.md5") continue;

            fs::path rel = fs::relative(entry.path(), root, ec);
            if (ec) continue;
            files.push_back({ rel.generic_string(), entry.file_size(ec) });
        }

        std::sort(files.begin(), files.end(), [](const BackupFilePreview& a, const BackupFilePreview& b)
        {
            return a.RelativePath < b.RelativePath;
        });

        if (files.size() > 48) files.resize(48);
        return files;
    }

    bool SaveConfigFile(const std::vector<GameDefinition>& games)
    {
        std::ofstream file(GetConfigPath(), std::ios::trunc);
        if (!file.is_open()) return false;

        file << "# GAME|gameId|displayName|savePath|processName|notes|enabled|scopeMode|activeProfileId\n";
        file << "# PROFILE|gameId|profileId|name|description|notes|favorite|autoBackupOnSwitch|backupLimit\n";
        file << "# WFILE|gameId|relativePath|enabled\n";
        file << "# WFOLDER|gameId|relativePath|enabled\n";

        for (const auto& game : games)
        {
            file << "GAME|" << EscapePipe(game.Id) << '|' << EscapePipe(game.DisplayName) << '|' << EscapePipe(game.SavePath)
                 << '|' << EscapePipe(game.ProcessName) << '|' << EscapePipe(game.Notes) << '|' << (game.Enabled ? '1' : '0')
                 << '|' << static_cast<int>(game.ScopeMode) << '|' << EscapePipe(game.ActiveProfileId) << '\n';

            for (const auto& rule : game.Whitelist)
                file << "WFILE|" << EscapePipe(game.Id) << '|' << EscapePipe(rule.RelativePath) << '|' << (rule.Enabled ? '1' : '0') << '\n';
            for (const auto& rule : game.FolderRules)
                file << "WFOLDER|" << EscapePipe(game.Id) << '|' << EscapePipe(rule.RelativePath) << '|' << (rule.Enabled ? '1' : '0') << '\n';
            for (const auto& profile : game.Profiles)
                file << "PROFILE|" << EscapePipe(game.Id) << '|' << EscapePipe(profile.Id) << '|' << EscapePipe(profile.Name) << '|' << EscapePipe(profile.Description)
                     << '|' << EscapePipe(profile.Notes) << '|' << (profile.Favorite ? '1' : '0') << '|' << (profile.AutoBackupOnSwitch ? '1' : '0')
                     << '|' << profile.BackupLimit << '\n';
        }
        return true;
    }

    std::vector<GameDefinition> LoadConfigFile()
    {
        std::vector<GameDefinition> games;
        std::ifstream file(GetConfigPath());
        if (!file.is_open()) return games;

        std::string line;
        std::map<std::string, size_t> gameIndex;
        while (std::getline(file, line))
        {
            line = Trim(line);
            if (line.empty() || line.starts_with("#")) continue;

            std::vector<std::string> parts;
            std::stringstream ss(line);
            std::string segment;
            while (std::getline(ss, segment, '|')) parts.push_back(segment);
            if (parts.empty()) continue;

            if (parts[0] == "GAME" && parts.size() >= 9)
            {
                GameDefinition game;
                game.Id = parts[1];
                game.DisplayName = parts[2];
                game.SavePath = parts[3];
                game.ProcessName = parts[4];
                game.Notes = parts[5];
                game.Enabled = parts[6] == "1";
                game.ScopeMode = static_cast<SaveScopeMode>(std::clamp(std::atoi(parts[7].c_str()), 0, 2));
                game.ActiveProfileId = parts[8];
                gameIndex[game.Id] = games.size();
                games.push_back(std::move(game));
            }
            else if (parts[0] == "WFILE" && parts.size() >= 4)
            {
                auto it = gameIndex.find(parts[1]);
                if (it != gameIndex.end()) games[it->second].Whitelist.push_back({ parts[2], parts[3] == "1" });
            }
            else if (parts[0] == "WFOLDER" && parts.size() >= 4)
            {
                auto it = gameIndex.find(parts[1]);
                if (it != gameIndex.end()) games[it->second].FolderRules.push_back({ parts[2], parts[3] == "1" });
            }
            else if (parts[0] == "PROFILE" && parts.size() >= 9)
            {
                auto it = gameIndex.find(parts[1]);
                if (it != gameIndex.end())
                {
                    GameProfile profile;
                    profile.Id = parts[2];
                    profile.Name = parts[3];
                    profile.Description = parts[4];
                    profile.Notes = parts[5];
                    profile.Favorite = parts[6] == "1";
                    profile.AutoBackupOnSwitch = parts[7] == "1";
                    profile.BackupLimit = max(1, std::atoi(parts[8].c_str()));
                    games[it->second].Profiles.push_back(std::move(profile));
                }
            }
        }

        for (auto& game : games)
        {
            if (game.Profiles.empty())
                game.Profiles.push_back({ "vanilla", "Vanilla", "Default profile", "", true, true, 20 });
            if (game.ActiveProfileId.empty())
                game.ActiveProfileId = game.Profiles.front().Id;
        }
        return games;
    }

    std::vector<BackupEntry> LoadBackupsForProfile(const GameDefinition& game, const GameProfile& profile)
    {
        std::vector<BackupEntry> backups;
        const fs::path root = BuildProfileBackupRoot(game.Id, profile.Id);
        std::error_code ec;
        if (!fs::exists(root, ec)) return backups;

        for (const auto& entry : fs::directory_iterator(root, ec))
        {
            if (ec || !entry.is_directory(ec)) continue;

            BackupEntry backup;
            backup.Id = WideToUtf8(entry.path().filename().wstring());
            backup.Name = backup.Id;
            backup.GameId = game.Id;
            backup.ProfileId = profile.Id;
            backup.FullPath = WideToUtf8(entry.path().wstring());

            const auto stats = CalculateDirectoryStats(entry.path());
            backup.TotalBytes = stats.first;
            backup.FileCount = stats.second;
            backup.PreviewFiles = BuildPreviewFiles(entry.path());

            auto writeTime = fs::last_write_time(entry.path(), ec);
            if (!ec)
            {
                using namespace std::chrono;
                const auto systemTime = time_point_cast<system_clock::duration>(writeTime - fs::file_time_type::clock::now() + system_clock::now());
                backup.CreatedAt = system_clock::to_time_t(systemTime);
                backup.CreatedAtDisplay = FormatTime(backup.CreatedAt);
            }
            else
            {
                backup.CreatedAtDisplay = "Unknown";
            }

            const fs::path hashFile = entry.path() / L".backup.md5";
            if (FileExists(hashFile))
            {
                std::ifstream in(hashFile);
                std::getline(in, backup.BackupHash);
                backup.BackupHash = Trim(backup.BackupHash);
            }
            else
            {
                backup.BackupHash = ComputeDirectoryAggregateMd5(entry.path()).value_or("Unavailable");
            }

            const fs::path reasonFile = entry.path() / L".reason.txt";
            if (FileExists(reasonFile))
            {
                std::ifstream in(reasonFile);
                std::getline(in, backup.Reason);
                backup.Reason = Trim(backup.Reason);
            }

            backups.push_back(std::move(backup));
        }

        std::sort(backups.begin(), backups.end(), [](const BackupEntry& a, const BackupEntry& b)
        {
            return a.CreatedAt > b.CreatedAt;
        });
        return backups;
    }

    bool OpenUrl(const char* url)
    {
        const HINSTANCE result = ShellExecuteA(nullptr, "open", url, nullptr, nullptr, SW_SHOWNORMAL);
        return reinterpret_cast<intptr_t>(result) > 32;
    }
}
