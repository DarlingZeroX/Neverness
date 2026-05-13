# VGGalgameCore include guard（Phase 7.7）
# 在仓库根目录执行：pwsh -File Engine/Scripts/check_vggalgame_core_includes.ps1
$ErrorActionPreference = "Stop"
$core = Join-Path $PSScriptRoot "../Source/Runtime/VGGalgameCore"
$bad = @("VGGalgameNodeGraph/", "VGGalgameScriptSequence/", "VGEditorGalgame", "/Editor/")
$files = Get-ChildItem -Path $core -Recurse -Include *.h,*.cpp
$exit = 0
foreach ($f in $files) {
  $t = Get-Content -Raw $f.FullName
  foreach ($p in $bad) {
    if ($t -match [regex]::Escape($p)) {
      Write-Host "FORBIDDEN include pattern '$p' in $($f.FullName)"
      $exit = 1
    }
  }
}
exit $exit
