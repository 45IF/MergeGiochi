param([string]$SdkPath = (Join-Path $PSScriptRoot '../../strumenti/plugin-sdk'))
$ErrorActionPreference = 'Stop'
$repo = Split-Path -Parent $PSScriptRoot
$env:PLUGIN_SDK_DIR = (Resolve-Path -LiteralPath $SdkPath).Path
$locator = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio/Installer/vswhere.exe'
$msbuild = & $locator -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -find 'MSBuild/**/Bin/MSBuild.exe' | Select-Object -First 1
if (-not $msbuild) { throw 'Visual Studio C++ non disponibile.' }
& (Join-Path $env:PLUGIN_SDK_DIR 'tools/premake/premake5.exe') "--file=$repo/premake5.lua" vs2022
if ($LASTEXITCODE -ne 0) { throw 'Generazione progetto fallita.' }
& $msbuild (Join-Path $repo 'build/MergeGiochi.sln') /m:2 /p:Configuration=Release /verbosity:minimal
if ($LASTEXITCODE -ne 0) { throw 'Compilazione fallita.' }
Get-FileHash (Join-Path $repo 'build/bin/ProvaASI.asi') -Algorithm SHA256
