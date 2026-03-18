#include "Models.hpp"

namespace manifold
{
    const char* ToString(SaveScopeMode mode)
    {
        switch (mode)
        {
        case SaveScopeMode::FileWhitelist: return "File Whitelist";
        case SaveScopeMode::FolderMode:    return "Folder Mode";
        case SaveScopeMode::HybridMode:    return "Hybrid Mode";
        default:                           return "Unknown";
        }
    }
}
