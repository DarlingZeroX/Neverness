# VGManagedCore — Managed Runtime 基礎層（Phase 2/3：Native/Managed ABI）

## 1. 定位

| 項目 | 說明 |
|------|------|
| **職責** | 定義並實作 **VisionGal Native ↔ Managed 共享 ABI**：**`VGNativeAPI`** 函數表、預設 Native 實作（如 **`LogInfo`**）、預設 API 表單例、建表服務（**`VGNativeApiTable_BuildDefault`**）。**Phase 3**：表尾掛載 **`engineServices`** → **`VGNativeEngineAPI`**（見 Runtime 模組 **VGNativeEngineAPI**）。**不包含** CoreCLR 啟動、hostfxr、程式集載入（見 **VGManagedHost**）。 |
| **不負責** | Gameplay、對白、Editor、Hot Reload、Roslyn、反射式 Invoke、擴展元資料。 |
| **CMake 目標** | **`VGManagedCore`**（**`STATIC`**） |
| **依賴** | **`VGNativeEngineAPI`**（**`PUBLIC`** 鏈接，傳遞標頭與靜態庫使用需求）；C++ 標準庫；**不**鏈接 `nethost`。 |

---

## 2. 目錄結構

```
Engine/Source/Managed/VGManagedCore/
├── CMakeLists.txt
├── Docs/
│   └── MODULE_ARCHITECTURE_AND_PROGRESS.md   ← 本文件
├── Include/VGManagedCore/
│   ├── VGManagedCoreConfig.h
│   ├── NativeAPI.h            ← VGNativeAPI、VG_NATIVE_API_VERSION、預設 LogInfo、engineServices
│   ├── ManagedHandle.h
│   ├── ManagedABI.h
│   ├── ManagedRuntimeServices.h
│   └── ManagedExports.h       ← VGNativeApi_GetDefaultTable
├── Private/
│   ├── NativeAPI.cpp
│   ├── ManagedRuntimeServices.cpp
│   └── ManagedExports.cpp
└── Managed/VisionGal.Managed.Core/
    ├── VisionGal.Managed.Core.csproj
    ├── VGNativeApiConstants.cs
    ├── VGNativeApi.cs
    └── NativeApiBootstrap.cs
```

---

## 3. 公開 C ABI 摘要

| 符號 | 說明 |
|------|------|
| `VG_NATIVE_API_VERSION` | 與託管 `VGNativeApiConstants.ApiVersion` 對齊（**當前為 2**）。 |
| `VGNativeAPI` | `apiVersion`、`reserved0`、`logInfo`、**`engineServices`**（`const VGNativeEngineAPI*`，可為 nullptr；預設表由建表函式填寫）。 |
| `VGNativeApi_DefaultLogInfo` | 預設 `logInfo`：寫 stderr + 內部診斷計數。 |
| `VGNativeApi_GetLogInfoCallCount` | 診斷：累計 `DefaultLogInfo` 呼叫次數。 |
| `VGNativeApiTable_BuildDefault` | 填充預設表並掛載 **`VGNativeEngineApi_GetDefaultTable()`**。 |
| `VGNativeApi_RegisterLogInfoOverride` | Phase 2 占位（未改變預設表）。 |
| `VGNativeApi_GetDefaultTable` | 回傳行程內唯讀單例表指標。 |

託管鏡像與安裝邏輯見 **`VisionGal.Managed.Core`**；引擎子表鏡像見 **`VisionGal.Managed.Engine`**（由 **`VisionGal.Managed.Runtime`** 引用）。

---

## 4. 與 VGManagedHost 的邊界

- **VGManagedHost**：載入 CoreCLR、解析 **`[UnmanagedCallersOnly]`**，將 **`VGNativeApi_GetDefaultTable()`** 的指標傳給託管 **`Entry.BootstrapNativeApi`**。
- **VGManagedCore**：提供表與 Native 側函式實作；**不**包含 hostfxr。

---

## 5. Phase 路線圖（本模組）

| Phase | 內容 |
|-------|------|
| **2** | **`VGNativeAPI`** v1、`LogInfo` 閉環、託管 **`NativeApiBootstrap`**。 |
| **3（當前）** | **`VG_NATIVE_API_VERSION` = 2**、**`engineServices`** 指標、與 **VGNativeEngineAPI** 建表串接。 |
| **4+** | 擴充表欄位（版本遞增）、**`RegisterLogInfoOverride`** 真正接線、與 Gameplay 合約對齊之共享 struct（遷入 **`ManagedABI.h`** 需評審）。 |

---

## 6. 開發進展

| 日期 | 進展 |
|------|------|
| **2026-05-14** | **Phase 2 落地**：新增 **VGManagedCore** 靜態庫、`VGNativeAPI`、預設 Log、單例表、**VisionGal.Managed.Core**、Runtime **`BootstrapNativeApi`**、擴展 **VGManagedHostTest**。 |
| **2026-05-14** | **Phase 3**：遞增 **`VG_NATIVE_API_VERSION`**、掛載 **VGNativeEngineAPI**、託管 **VisionGal.Managed.Engine**、Stub 跨邊界測試。 |
| **2026-05-15** | **Phase 5 加固**：Foundation Bootstrap 經 **`VGNativeApi_GetDefaultTable`** 與 **GetBootstrapFlags** 驗證。 |
