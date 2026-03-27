$scriptDir   = Split-Path -Parent $MyInvocation.MyCommand.Path
$counterFile = Join-Path $scriptDir "build_number.txt"
$headerFile  = Join-Path $scriptDir "BuildNumber.generated.hpp"

if (!(Test-Path $counterFile)) {
    Set-Content -Path $counterFile -Value "0" -Encoding ascii
}

[int]$count = Get-Content $counterFile
$count++
Set-Content -Path $counterFile -Value $count -Encoding ascii

@"
#pragma once
#define MANIFOLD_BUILD_NUMBER $count
"@ | Set-Content -Path $headerFile -Encoding ascii