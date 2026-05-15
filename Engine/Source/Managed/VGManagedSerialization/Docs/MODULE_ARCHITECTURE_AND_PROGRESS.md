# VGManagedSerialization — JSON 序列化（VisionGal.Managed.Serialization）

## 1. 定位

| 項目 | 說明 |
|------|------|
| **職責** | **VersionTolerance** 格式版本、**SceneSerializer** / **AssetSerializer** / **GraphSerializer**（System.Text.Json）。 |
| **程式集** | **VisionGal.Managed.Serialization**（`net10.0`） |
| **依賴** | **VisionGal.Managed.Reflection** |

## 2. 版本策略

- 寫入時強制 **`FormatVersion = CurrentFormatVersion`**（當前 **1**）。
- 讀取時 **`PropertyNameCaseInsensitive`** + 忽略未知欄位（寬鬆模式）；未來可於反序列化後顯式校驗版本號。

## 3. 進展

| 日期 | 進展 |
|------|------|
| **2026-05-15** | 初始模組：場景/資產/圖 JSON 序列化與版本容忍。 |
| **2026-05-15** | **Phase 5 加固**：**GraphSerializer** 中文註解；**formatVersion** 單元測試。 |
| **2026-05-15** | **Phase 5.3**：**`SceneSerializer.ApplyEntryProperties`**（DTO 屬性寫回託管實例）。 |
| **2026-05-15** | **VersionTolerance**：**`VersionToleranceTests`** 驗證根層未知 JSON 欄位不阻斷反序列化。 |
