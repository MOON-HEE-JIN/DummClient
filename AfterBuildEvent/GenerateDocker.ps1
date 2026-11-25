param(
    [Parameter(Mandatory)][string]$TargetPath,
    [Parameter(Mandatory)][string]$Configuration,
    [Parameter(Mandatory)][string]$Platform,
    [Parameter(Mandatory)][string]$RedistPath
)

$ErrorActionPreference = 'Stop'
$outputDir = Join-Path (Split-Path $PSScriptRoot) "DockerBuild\$Platform\$Configuration"
$targetDir = Split-Path $TargetPath
$exeName = Split-Path $TargetPath -Leaf
$crt = Get-ChildItem -Path "$RedistPath\Microsoft.VC*.CRT\*.dll" -File
if (!$crt) { throw "MSVC runtime DLLs not found in $RedistPath" }

New-Item -ItemType Directory -Force -Path $outputDir | Out-Null
Copy-Item -LiteralPath $TargetPath -Destination $outputDir -Force
Get-ChildItem -LiteralPath $targetDir -File |
    Where-Object { $_.Extension -in '.dll', '.pdb' } |
    Copy-Item -Destination $outputDir -Force
$crt | Copy-Item -Destination $outputDir -Force

@"
FROM mcr.microsoft.com/windows/servercore:ltsc2019

WORKDIR C:/app
COPY . C:/app/
ENTRYPOINT ["C:/app/$exeName"]
"@ | Set-Content -LiteralPath (Join-Path $outputDir 'Dockerfile') -Encoding ASCII
Write-Host "Docker files generated: $outputDir"
