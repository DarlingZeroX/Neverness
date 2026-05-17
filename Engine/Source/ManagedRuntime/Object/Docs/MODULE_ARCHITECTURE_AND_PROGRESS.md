# VGManagedObject — 託管物件生命週期（VisionGal.Managed.Object）

## 1. 定位

| 項目 | 說明 |
|------|------|
| **職責** | 託管 **VGObject** 抽象基底、**VGObjectId** 識別、靜態 **ObjectRegistry**、經 **EngineApi.Object** 之 **NativeHandleBridge**、**LifetimeSystem** retain/release 協調。 |
| **程式集** | **VisionGal.Managed.Object**（`net10.0`，`AllowUnsafeBlocks`） |
| **依賴** | **VisionGal.Managed.Engine** |

## 2. 生命週期契約（Phase 5 加固）

- **`createObject`** 成功時 Native 引用計數為 **1**；**`CreateAndRegister`** 不再額外 `Retain`。
- **`Dispose`** → **`Release`**；引用歸零後 **`DestroyObject`**。
- **`ObjectRegistry.ClearForTesting`**：先對所有已註冊物件呼叫 **`Dispose`**，再清空表與 Id 計數器。
- **`CreateAndRegister<T>`**：以反射匹配 `(VGObjectId, VGObjectHandle)` 或三參數 `(…, string typeName)` 建構子。

## 3. 進展

| 日期 | 進展 |
|------|------|
| **2026-05-15** | 初始模組：Id / Object / Registry / Native 橋接 / 生命週期系統。 |
| **2026-05-15** | **Phase 5 加固**：ref-count、`ClearForTesting`、反射建構 **SceneEntity**。 |
| **2026-05-15** | **Phase 5.3**：**SceneRehydrator** 經 **`LifetimeSystem`** 再水合場景實體（新 Native 控制代碼）。 |

## 4. 後續

- 執行緒安全之讀寫鎖（若多執行緒存取註冊表）。
