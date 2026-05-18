# VGManagedAssets — 資產 GUID 與登記表（VisionGal.Managed.Assets）

## 1. 定位

| 項目 | 說明 |
|------|------|
| **職責** | **GUID**（128-bit，與 Native NNGuid 互操作）、**AssetDatabase**、**ImportPipeline**、**DependencyTracking**。 |
| **程式集** | **VisionGal.Managed.Assets**（`net10.0`，`AllowUnsafeBlocks`） |
| **依賴** | **VisionGal.Managed.Object**、**VisionGal.Managed.Engine** |

## 2. 匯入回退（Phase 5 加固）

- 優先 **`AssetRegistry.importAsset`**（Native）。
- Native 回傳零 GUID 時，使用 **`GUID.FromDeterministicPath`**（FNV-1a 穩定雜湊），**禁止**對虛擬路徑呼叫 **`GUID.Parse`**。
- **`AssetDatabase.ClearForTesting`**：清空託管快取（不呼叫 Native 登記表）。

## 3. Phase 5 進展

| 日期 | 進展 |
|------|------|
| **2026-05-15** | 初始模組：GUID、資產登記、匯入管線、依賴追蹤。 |
| **2026-05-15** | **加固**：確定性路徑 GUID；`ImportPipeline` 註解與測試；Bootstrap 演練 **Import** + **TryResolveGuid**。 |
