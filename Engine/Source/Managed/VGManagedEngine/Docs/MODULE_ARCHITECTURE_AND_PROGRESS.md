# VGManagedEngine — Managed Engine SDK（VisionGal.Managed.Engine）

## 1. 定位

| 項目 | 說明 |
|------|------|
| **職責** | **僅**託管端 **Engine Service** 之 ABI 鏡像：`VGNativeEngineAPI` 與子表之 `[StructLayout(Sequential)]` 結構（含 **layout v4** 之 **`VGEntityApi`**、**layout v5** 尾部 **`GetRuntimeTick`**）、`EngineNativeApiBootstrap` 安裝與 Stub 演練路徑、**Handle** 型別封裝。**不包含** Gameplay、對白、存檔、Sequence。 |
| **不負責** | CoreCLR 宿主、**`VGNativeAPI`** 宿主級欄位定義（見 **VisionGal.Managed.Core**）。 |
| **程式集** | **`VisionGal.Managed.Engine`**（`net10.0`，`AllowUnsafeBlocks`）、**`VisionGal.Managed.Engine.Runtime`**（薄封裝） |
| **依賴** | **VisionGal.Managed.Core**（取得 `VGNativeApi` / `VGNativeApiConstants`）。**Engine.Runtime** 另依賴 **VisionGal.Managed.Engine**。 |

---

## 2. 目錄結構

```
Engine/Source/Managed/VGManagedEngine/
├── Docs/
│   └── MODULE_ARCHITECTURE_AND_PROGRESS.md   ← 本文件
└── Managed/
    ├── VisionGal.Managed.Engine/
    │   ├── VisionGal.Managed.Engine.csproj
    │   ├── VGNativeEngineApiConstants.cs
    │   ├── VGNativeHandles.cs
    │   ├── VGNativeEngineApiTypes.cs
    │   └── EngineNativeApiBootstrap.cs
    └── VisionGal.Managed.Engine.Runtime/
        ├── VisionGal.Managed.Engine.Runtime.csproj
        └── EngineTime.cs
```

---

## 3. 公開 API 摘要

| 類型 / 成員 | 說明 |
|-------------|------|
| `VGNativeEngineApiConstants.LayoutVersion` | 與 Native `VG_NATIVE_ENGINE_API_LAYOUT_VERSION` 對齊（**4** 起含 **`VGEntityAPI`**；**5** 起含 **`GetRuntimeTick`**）。 |
| `VGNativeEngineApiConstants.EntityServiceAbiToken` | 與 Native `VG_ENTITY_SERVICE_ABI_TOKEN`（`EntityAPI.h`）一致之服務魔數。 |
| `VGNativeEngineApi` 等 | 與 C 頭 `EngineAPIRegistry.h` 欄位順序一致之鏡像（末尾 **`Entity`** 對應 **`VGEntityAPI`**）。 |
| `EngineNativeApiBootstrap.InstallFromNativeApiTable` | 由 `VGNativeAPI*` 解析 `engineServices` 並按值快取函數指標。 |
| `EngineNativeApiBootstrap.ExerciseStubInteropPath` | 測試用：演練 **Timing**（含 Phase 4 擴充欄位）、**AsyncWait** 三件套，及 **layout v5** 之 **`Entity.GetServiceAbiToken`** / **`GetRuntimeTick`** 冒煙。 |
| **`EngineTime`**（**VisionGal.Managed.Engine.Runtime**） | 讀取已安裝 ABI 之 **DeltaTime** / **TotalTime** / **FrameIndex**。 |

---

## 4. 與 VisionGal.Managed.Runtime 的關係

- **`Entry.BootstrapNativeApi`** 在 **`NativeApiBootstrap.Install`** 之後呼叫 **`EngineNativeApiBootstrap.InstallFromNativeApiTable`** 與 **`ExerciseStubInteropPath`**。
- **`VisionGal.Managed.Runtime.csproj`** 以 `ProjectReference` 引用 **Engine** 與 **Engine.Runtime**；**`VGManagedHost/CMakeLists.txt`** 之 `dotnet publish` 依賴列表已納入兩目錄之 `*.cs`。

---

## 5. Phase 路線圖（本模組）

| Phase | 內容 |
|-------|------|
| **3** | 鏡像 Stub 表、Bootstrap、演練路徑。 |
| **4（當前）** | **`LayoutVersion` = 5** 鏡像（含 **`VGEntityApi`**、**`GetRuntimeTick`**）、**Engine.Runtime** 薄封裝。 |
| **5+** | 依子系統新增 **Service 封裝類**（如正式 `VGRenderService`），仍禁止在此層撰寫 Gameplay。 |

---

## 6. 開發進展

| 日期 | 進展 |
|------|------|
| **2026-05-14** | 新增 **VisionGal.Managed.Engine** 與本模組文檔；與 **VGNativeEngineAPI** 完成跨邊界 Stub 驗證。 |
| **2026-05-14** | 新增 **VisionGal.Managed.Engine.Runtime**（**EngineTime**）與 ABI **layout v2** 鏡像欄位。 |
| **2026-05-15** | **layout v4**：**`VGEntityApi`**、**`EntityServiceAbiToken`**、**`ExerciseStubInteropPath`** 擴充；與 **VGNativeEngineAPI**／MANAGED **§2.7.1** 對齊。 |
| **2026-05-15** | **layout v5**：**`GetRuntimeTick`**、**`LayoutVersion` = 5**；**ExerciseStubInteropPath** 擴充；MANAGED **§2.7.1 Kernel 首包** 對齊。 |
| **2026-05-15** | **P0 註解收口**：**`VGNativeEngineApiConstants`** 補充 **layout v5** 語義與與 Native 版本不一致時之拒絕安裝行為說明（對齊 **InstallFromNativeApiTable**）。 |
| **2026-05-15** | **P0 對齊審計**：**`VGEntityApi`** **remarks** 擴充 **GetRuntimeTick** 與 **EntityWorld** 無鏡像承諾、欄位順序與 C 一致；**merge_docs** 刷新 **MERGED**。 |
