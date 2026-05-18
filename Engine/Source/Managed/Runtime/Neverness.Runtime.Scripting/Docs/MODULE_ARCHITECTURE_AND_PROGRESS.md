# VGManagedScripting — 腳本載入與 Roslyn（VisionGal.Managed.Scripting）

## 1. 定位

| 項目 | 說明 |
|------|------|
| **職責** | **ManagedAssemblyLoadContextHost**、**HotReloadCoordinator**、**RoslynScriptCompiler**（`VISIONGAL_ENABLE_ROSLYN` / Microsoft.CodeAnalysis.CSharp 4.14.0）。 |
| **程式集** | **VisionGal.Managed.Scripting**（`net10.0`） |
| **依賴** | **VisionGal.Managed.Core** |

## 2. Phase 5 進展

| 日期 | 進展 |
|------|------|
| **2026-05-15** | 初始模組：ALC 宿主、熱重載、Roslyn 條件編譯編譯器。 |
| **2026-05-15** | **加固**：標記為 **脚手架**（非生產）；**VISIONGAL_ENABLE_ROSLYN** 未接線；Phase 8/9。 |
