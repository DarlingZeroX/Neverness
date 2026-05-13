$ErrorActionPreference = "Stop"
$core = Join-Path (Split-Path $PSScriptRoot -Parent) "Source/Runtime/VGGalgameCore"
New-Item -ItemType Directory -Force -Path (Join-Path $core "Interface"), (Join-Path $core "Include") | Out-Null

function Write-Shim($RelativePath, $IncludeLine) {
    $p = Join-Path $core $RelativePath
    $dir = Split-Path $p -Parent
    if (-not (Test-Path $dir)) { New-Item -ItemType Directory -Force -Path $dir | Out-Null }
    @(
        "#pragma once"
        "#include `"$IncludeLine`""
        ""
    ) | Set-Content -LiteralPath $p -Encoding utf8
}

Write-Shim "VGGalCoreConfig.h" "VGGalgameContract/VGGalCoreConfig.h"

foreach ($n in @(
    "IGameEngine", "ISubsystemBus", "IGalGameContext", "IStoryScriptSystem", "IStoryScriptExecutor",
    "IStoryExecutionAdapter", "IAudioSubsystem", "IArchiveSubsystem", "IUISubsystem", "IScriptSubsystem",
    "ISceneSubsystem", "IDialogueSubsystem", "IGalRuntimeSession", "IExecutionScheduler", "IRuntimeEventPipeline",
    "ILuaRuntimeBridge", "IRuntimeLayerGraph", "IRuntimeSnapshotProvider"
)) {
    Write-Shim "Interface/$n.h" "VGGalgameContract/Interface/$n.h"
}

foreach ($n in @("IGameSystem", "IGameObject", "IStoryScript")) {
    Write-Shim "Interface/$n.h" "VGGalgameRuntimeCore/Interface/$n.h"
}

foreach ($n in @(
    "SaveArchive", "ArchiveDataContainer", "GalGameContext", "GalGameRuntimeState", "GalGameEvent",
    "Components", "GalGameLayoutUtils", "GalGameEngineAccess", "GalGameContextSnapshot", "SubsystemBusGuard",
    "GalExecutionLifecycle", "VGGalgameCore_Deprecated"
)) {
    Write-Shim "Include/$n.h" "VGGalgameRuntimeCore/Include/$n.h"
}

Write-Shim "Include/SubsystemBusSnapshot.h" "VGGalgameContract/Include/SubsystemBusSnapshot.h"
