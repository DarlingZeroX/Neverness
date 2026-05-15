# VGManagedScene — 場景與 Prefab（VisionGal.Managed.Scene）

## 1. 定位

| 項目 | 說明 |
|------|------|
| **職責** | **Scene** 容器、**SceneEntity**（VGObject 衍生）、**Prefab** 實例化、與 **SceneSerializer** JSON 往返、**SceneRehydrator** 實體再水合。 |
| **程式集** | **VisionGal.Managed.Scene**（`net10.0`） |
| **依賴** | **VisionGal.Managed.Object**、**VisionGal.Managed.Serialization** |

## 2. JSON 語意

| API | 說明 |
|-----|------|
| **`ToJson`** | 序列化為含 `formatVersion` 之 DTO JSON。 |
| **`FromJson`** | 僅還原 **`SceneDocument` DTO**，不重建 **`SceneEntity`**。 |
| **`ValidateRoundTripDocument`** | 驗證名稱、實體數與 **`DisplayName`** 等屬性 payload。 |
| **`RestoreFromDocument`** | 還原僅託管 **`Scene`** 容器（不含實體）。 |
| **`RehydrateFromJson`** / **`SceneRehydrator`** | 完整再水合：經 **`LifetimeSystem`** 建立新 Native 控制代碼並套用 DTO 屬性。 |

**刻意未接線**：Native **`VGSceneAPI`**（spawn / setParent 等）之 C# 橋接層。

## 3. 進展

| 日期 | 進展 |
|------|------|
| **2026-05-15** | 初始模組：場景實體、Prefab、序列化整合。 |
| **2026-05-15** | **Phase 5 加固**：`ValidateRoundTripDocument` / `RestoreFromDocument`（僅容器）。 |
| **2026-05-15** | **Phase 5.3**：**`SceneRehydrator`**、**`RehydrateFromJson`**；Bootstrap 演練 JSON→實體往返。 |
