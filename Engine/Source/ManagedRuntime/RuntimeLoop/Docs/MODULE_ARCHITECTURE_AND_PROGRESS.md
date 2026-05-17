# VGManagedRuntimeLoop — 託管 Runtime Loop API

## 1. 定位與邊界

| 項目 | 說明 |
|------|------|
| **程式集** | **VisionGal.Managed.RuntimeLoop**（`net10.0`） |
| **職責** | 提供與 Native **`visiongal::engine::RuntimeScheduler`** 對稱之 **`ManagedRuntimeScheduler`**、**`RuntimeTickGroup`**、**`ManagedRuntimeFrameContext`**、**`IManagedRuntimeSubsystem`**；**無** **DllImport**、**無** **VisionGal.Managed.Engine** 依賴，供純 C# 宿主或單元測試演練 **PlayerLoop** 式順序。 |
| **不負責** | 不驅動 **VGEngineRuntime**、不讀寫 **VGNativeEngineAPI**；與 Native 帧之對齊僅能由產品層自行約定（未來可選 P/Invoke／Host 回調，**本模組不提供**）。 |
| **對稱關係** | 見 [VGEngineRuntime MODULE](../../../Runtime/VGEngineRuntime/Docs/MODULE_ARCHITECTURE_AND_PROGRESS.md) **RuntimeScheduler**；總覽 **P0-1** 見 [MANAGED_RUNTIME_ARCHITECTURE_AND_PROGRESS.md](../../MANAGED_RUNTIME_ARCHITECTURE_AND_PROGRESS.md) **§0.3**。 |
| **消費方** | **VisionGal.Managed.Runtime**（專案參考，納入 publish）；**VisionGal.Managed.Foundation.Tests**。 |

## 2. 目錄與公開 API

- `RuntimeTickGroup.cs` — 與 Native **RuntimeTickGroup** 語義一致。
- `ManagedRuntimeFrameContext.cs` — 只讀帧上下文。
- `IManagedRuntimeSubsystem.cs` — 子系統介面。
- `ManagedRuntimeScheduler.cs` — 註冊、**InitializeRegistered** / **ShutdownRegistered**、**Tick**（含 **FixedUpdate** 累加與每帧最大步數上限）。

## 3. 開發進展

| 日期 | 進展 |
|------|------|
| **2026-05-15** | **P0-1 首包**：新程式集與 **ManagedRuntimeScheduler**；**VisionGal.Managed.Runtime** 專案參考；**VGManagedHost** CMake **DEPENDS**；**Foundation.Tests** 順序與 **FixedUpdate** 上限用例；與 Native **RuntimeScheduler** 管線對齊說明。 |

## 4. 相關鏈接

- [MANAGED_RUNTIME_ARCHITECTURE_AND_PROGRESS.md](../../MANAGED_RUNTIME_ARCHITECTURE_AND_PROGRESS.md) **§0.3**
- [VGEngineRuntime MODULE](../../../Runtime/VGEngineRuntime/Docs/MODULE_ARCHITECTURE_AND_PROGRESS.md)
