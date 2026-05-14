# VGNativeEngineAPI — Native Engine Service ABI（Phase 3）

## 1. 定位

| 項目 | 說明 |
|------|------|
| **職責** | 僅承載 **Engine Runtime 服務層** 之 C 可互操作 **函數表聚合**（`VGNativeEngineAPI`）：Render、UI、Audio、Asset、Input、Scene、Timing、AsyncWait。**不包含** Gameplay、對白、變數、存檔、Sequence、Editor。 |
| **不負責** | 不連結 **VGEngine** / **VGRHI** / **VGUI**（RmlUi）；本 Phase 僅提供 **Stub** 實作與診斷計數，真實後端由後續 Adapter 目標接線。 |
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
| `VGNativeEngineApi_GetDefaultTable` | 行程內單例唯讀表指標。 |
| `VGNativeEngineApi_GetStubInvokeCount` | 測試用：Stub 被呼叫之累計次數。 |
| **Handle typedef** | `VGTextureHandle`、`VGRenderTargetHandle`、`VGElementHandle`、`VGAudioHandle`、`VGAssetHandle`、`VGAsyncWaitHandle` 皆為 `uint64_t`；**0** 表無效。 |

**`extern "C"` 政策**：僅模組邊界導出上述建表 / 取表 / 診斷符號；業務能力一律經由 **函數表欄位** 間接呼叫。

---

## 4. 與 VGManagedCore 的關係

- **`VGNativeApiTable_BuildDefault`**（VGManagedCore）將 **`VGNativeAPI.engineServices`** 設為 **`VGNativeEngineApi_GetDefaultTable()`** 之回傳值。
- **`VG_NATIVE_API_VERSION`** 已遞增至 **2**，表尾追加 **`engineServices`** 指標欄位（不重排舊欄位）。

---

## 5. Phase 路線圖（本模組）

| Phase | 內容 |
|-------|------|
| **3（當前）** | 子 API 頭檔拆分、聚合體、Stub、`GetStubInvokeCount`、與 **VisionGal.Managed.Engine** 鏡像對齊。 |
| **4+** | Adapter 靜態/動態庫：將 Stub 替換為呼叫 **VGEngine** / **VGRHI** / **VGUI**；維持 ABI 佈局或遞增 `layoutVersion`。 |

---

## 6. 開發進展

| 日期 | 進展 |
|------|------|
| **2026-05-14** | **Phase 3 落地**：新增本模組、預設 Stub、GTest 經託管 **ExerciseStubInteropPath** 驗證 Stub 計數遞增。 |
