#pragma once
#include <string>
#include <string_view>

#ifdef __has_include
#if __has_include("BuildNumber.generated.hpp")
#include "BuildNumber.generated.hpp"
#endif
#endif

#ifndef MANIFOLD_BUILD_NUMBER
#define MANIFOLD_BUILD_NUMBER 0
#endif

namespace manifold::version
{
    inline constexpr int Major = 0;
    inline constexpr int Minor = 1;
    inline constexpr int Patch = 0;
    inline constexpr int Build = MANIFOLD_BUILD_NUMBER;

    inline constexpr bool IsPrerelease = true;
    inline constexpr std::string_view PrereleaseTag = "PREVIEW";

    std::string ToString();
    std::string ToDetailedString();
}