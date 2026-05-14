# VGManagedEngine — Managed Engine SDK（VisionGal.Managed.Engine）

## 1. 定位

| 項目 | 說明 |
|------|------|
| **職責** | **僅**託管端 **Engine Service** 之 ABI 鏡像：`VGNativeEngineAPI` 與子表之 `[StructLayout(Sequential)]` 結構、`EngineNativeApiBootstrap` 安裝與 Stub 演練路徑、**Handle** 型別封裝。**不包含** Gameplay、對白、存檔、Sequence。 |
| **不負責** | CoreCLR 宿主、**`VGNativeAPI`** 宿主級欄位定義（見 **VisionGal.Managed.Core**）。 |
| **程式集** | **`VisionGal.Managed.Engine`**（`net10.0`，`AllowUnsafeBlocks`） |
| **依賴** | **VisionGal.Managed.Core**（取得 `VGNativeApi` / `VGNativeApiConstants`）。 |

---

## 2. 目錄結構

```
Engine/Source/Managed/VGManagedEngine/
├── Docs/
│   └── MODULE_ARCHITECTURE_AND_PROGRESS.md   ← 本文件
└── Managed/VisionGal.Managed.Engine/
    ├── VisionGal.Managed.Engine.csproj
    ├── VGNativeEngineApiConstants.cs
    ├── VGNativeHandles.cs
    ├── VGNativeEngineApiTypes.cs
    └── EngineNativeApiBootstrap.cs
```

---

## 3. 公開 API 摘要

| 類型 / 成員 | 說明 |
|-------------|------|
| `VGNativeEngineApiConstants.LayoutVersion` | 與 Native `VG_NATIVE_ENGINE_API_LAYOUT_VERSION` 對齊。 |
| `VGNativeEngineApi` 等 | 與 C 頭 `EngineAPIRegistry.h` 欄位順序一致之鏡像。 |
| `EngineNativeApiBootstrap.InstallFromNativeApiTable` | 由 `VGNativeAPI*` 解析 `engineServices` 並按值快取函數指標。 |
| `EngineNativeApiBootstrap.ExerciseStubInteropPath` | 測試用：呼叫 `GetDeltaTime` 與 AsyncWait 三件套。 |

---

## 4. 與 VisionGal.Managed.Runtime 的關係

- **`Entry.BootstrapNativeApi`** 在 **`NativeApiBootstrap.Install`** 之後呼叫 **`EngineNativeApiBootstrap.InstallFromNativeApiTable`** 與 **`ExerciseStubInteropPath`**。
- **`VisionGal.Managed.Runtime.csproj`** 以 `ProjectReference` 引用本程式集；**`VGManagedHost/CMakeLists.txt`** 之 `dotnet publish` 依賴列表已納入本目錄 `*.cs`。

---

## 5. Phase 路線圖（本模組）

| Phase | 內容 |
|-------|------|
| **3（當前）** | 鏡像 Stub 表、Bootstrap、演練路徑。 |
| **4+** | 依子系統新增 **Service 封裝類**（如正式 `VGRenderService`），仍禁止在此層撰寫 Gameplay。 |

---

## 6. 開發進展

| 日期 | 進展 |
|------|------|
| **2026-05-14** | 新增 **VisionGal.Managed.Engine** 與本模組文檔；與 **VGNativeEngineAPI** 完成跨邊界 Stub 驗證。 |
