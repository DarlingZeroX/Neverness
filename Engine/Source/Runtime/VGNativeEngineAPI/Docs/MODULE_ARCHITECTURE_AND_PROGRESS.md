# VGNativeEngineAPI — Native Engine Service ABI（Phase 3～4）

## 1. 定位

| 項目 | 說明 |
|------|------|
| **職責** | 僅承載 **Engine Runtime 服務層** 之 C 可互操作 **函數表聚合**（`VGNativeEngineAPI`）：Render、UI、Audio、Asset、Input、Scene、Timing、AsyncWait。**不包含** Gameplay、對白、變數、存檔、Sequence、Editor。 |
| **不負責** | 不連結 **VGEngine** / **VGRHI** / **VGUI**（RmlUi）；本模組提供 **Stub** 實作與診斷計數；**Timing / Async** 等之 **Runtime** 轉發由 **VGEngineRuntimeServices**（可選 CMake）覆寫函數指標。 |
| **CMake 目標** | **`VGNativeEngineAPI`**（**`STATIC`**） |
| **依賴** | 僅 C++ 標準庫。 |

---

## 2. 目錄結構

```
Engine/Source/Runtime/VGNativeEngineAPI/
├── CMakeLists.txt
├── Docs/
│   └── MODULE_ARCHITECTURE_AND_PROGRESS.md   ← 本文件
├── Include/VGNativeEngineAPI/
│   ├── NativeInterop.h
│   ├── EngineHandles.h
│   ├── RenderAPI.h
│   ├── UIAPI.h
│   ├── AudioAPI.h
│   ├── AssetAPI.h
│   ├── InputAPI.h
│   ├── SceneAPI.h
│   ├── TimingAPI.h
│   ├── AsyncWaitAPI.h
│   ├── EngineAPIRegistry.h
│   ├── NativeEngineAPI.h
│   └── VGNativeEngineApiConfig.h
└── Private/
    └── VGNativeEngineApiStubs.cpp
```

---

## 3. 公開 ABI 摘要

| 符號 / 概念 | 說明 |
|-------------|------|
| `VG_NATIVE_ENGINE_API_LAYOUT_VERSION` | 聚合體佈局版本；與託管 `VGNativeEngineApiConstants.LayoutVersion` 對齊。 |
| `VGNativeEngineAPI` | 子表：`render`、`ui`、`audio`、`asset`、`input`、`scene`、`timing`、`asyncWait`。 |
| `VGNativeEngineApiTable_BuildDefault` | 填充 Stub 函數指標。 |
| `VGNativeEngineApi_GetDefaultTable` | 行程內單例唯讀表指標（純 Stub）。 |
| `VGNativeEngineApi_GetStubInvokeCount` | 測試用：Stub 被呼叫之累計次數。 |
| **Handle typedef** | `VGTextureHandle`、`VGRenderTargetHandle`、`VGElementHandle`、`VGAudioHandle`、`VGAssetHandle`、`VGAsyncWaitHandle`、`VGEntityHandle` 皆為 `uint64_t`；**0** 表無效。 |
| **`VG_NATIVE_ENGINE_API_LAYOUT_VERSION`** | Phase 4 起為 **2**（Timing / Scene / Asset 子表尾擴充）。 |

**`extern "C"` 政策**：僅模組邊界導出上述建表 / 取表 / 診斷符號；業務能力一律經由 **函數表欄位** 間接呼叫。

---

## 4. 與 VGManagedCore 的關係

- **`VGNativeApiTable_BuildDefault`**（VGManagedCore）依 **`VISIONGAL_USE_ENGINE_RUNTIME_SERVICES`**：將 **`VGNativeAPI.engineServices`** 設為 **`VGNativeEngineApi_GetRuntimeTable()`** 或 **`VGNativeEngineApi_GetDefaultTable()`**。
- **`VG_NATIVE_API_VERSION`** 已遞增至 **2**，表尾追加 **`engineServices`** 指標欄位（不重排舊欄位）。

---

## 5. Phase 路線圖（本模組）

| Phase | 內容 |
|-------|------|
| **3** | 子 API 頭檔拆分、聚合體、Stub、`GetStubInvokeCount`、與 **VisionGal.Managed.Engine** 鏡像對齊。 |
| **4（當前）** | **`layoutVersion` = 2**：Timing（`getTotalTime` / `getFrameIndex`）、Scene 擴充、Asset 擴充；Runtime 轉發見 **VGEngineRuntimeServices**。 |
| **5+** | Adapter：將其餘 Stub 替換為呼叫 **VGEngine** / **VGRHI** / **VGUI**；維持 ABI 佈局或遞增 `layoutVersion`。 |

---

## 6. 開發進展

| 日期 | 進展 |
|------|------|
| **2026-05-14** | **Phase 3 落地**：新增本模組、預設 Stub、GTest 經託管 **ExerciseStubInteropPath** 驗證 Stub 計數遞增。 |
| **2026-05-14** | **Phase 4**：`layoutVersion` 遞增至 **2**；擴充子表欄位；與 **VGEngineRuntimeServices** 對齊之 Runtime 路徑。 |
