#include "Version.hpp"
#include <format>

namespace manifold::version
{
    std::string ToString()
    {
        if (IsPrerelease)
        {
            return std::format(
                "v{}.{}.{}-{}+BUILD.{}",
                Major,
                Minor,
                Patch,
                PrereleaseTag,
                Build);
        }

        return std::format(
            "v{}.{}.{}+BUILD.{}",
            Major,
            Minor,
            Patch,
            Build);
    }

    std::string ToDetailedString()
    {
        return std::format(
            "{} ({})",
            ToString(),
            __DATE__ " " __TIME__);
    }
}