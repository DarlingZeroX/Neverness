# VGEngineRuntimeServices — Engine Service ABI Adapter（Phase 4）

## 1. 定位

| 項目 | 說明 |
|------|------|
| **職責** | 實作 **`VGNativeEngineApiTable_BuildRuntime`**：以 **`VGNativeEngineApiTable_BuildDefault`** 為基底，覆寫 **Timing**、**AsyncWait**、**Scene** 擴充欄位、**Asset** 擴充欄位，轉發至 **`VGEngineRuntime`**；並提供 **`VGNativeEngineApi_GetRuntimeTable`**、宿主輔助 **`VGEngineRuntimeHost_*`**。 |
| **不負責** | 不取代 **Stub** 目標（**`VGNativeEngineApi_GetDefaultTable`** 仍供純 Stub 測試）；Render / UI / Audio / Input 等仍沿用 Stub 函數指標。 |
| **CMake 目標** | **`VGEngineRuntimeServices`**（**`STATIC`**） |
| **依賴** | **`VGNativeEngineAPI`**、**`VGEngineRuntime`**。 |

---

## 2. 與 VGManagedCore 的關係

- CMake 選項 **`VISIONGAL_USE_ENGINE_RUNTIME_SERVICES`**（預設 **ON**）：**`VGNativeApiTable_BuildDefault`** 將 **`engineServices`** 設為 **`VGNativeEngineApi_GetRuntimeTable()`**；**OFF** 時仍使用 **`VGNativeEngineApi_GetDefaultTable()`**。

---

## 3. 開發進展

| 日期 | 進展 |
|------|------|
| **2026-05-14** | Phase 4 首包：新增本適配層與 **Runtime** 建表路徑。 |
