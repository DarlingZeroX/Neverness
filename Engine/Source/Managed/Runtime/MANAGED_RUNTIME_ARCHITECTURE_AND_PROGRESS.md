# VisionGal Managed Runtime — 架構與總進度

本文檔描述 **Managed Runtime** 分層、當前完成度與後續規劃。**VisionGal 2026 主線原則與 P0–P2 實施路線**見 **§0**。實作細節以各子模組 `Docs/MODULE_ARCHITECTURE_AND_PROGRESS.md` 為準；Native **Engine Service ABI** 另見 [NNNativeEngineAPI](../Runtime/NNNativeEngineAPI/Docs/MODULE_ARCHITECTURE_AND_PROGRESS.md)（契约）、[NNRuntimeNativeEngineAPIStub](../Runtime/NNRuntimeNativeEngineAPIStub/Docs/MODULE_ARCHITECTURE_AND_PROGRESS.md)（Stub）。Native **Runtime** 全模組總覽見 [RUNTIME_ARCHITECTURE_AND_PROGRESS.md](../Runtime/RUNTIME_ARCHITECTURE_AND_PROGRESS.md)。

---

## 0. VisionGal 主線原則與路線（2026）

### 0.1 已確立原則

| 原則 | 說明 |
|------|------|
| **Lua Runtime 主線停止演進** | **sol2**、Lua Scene／Sequence／Gameplay／UI Bridge 等：**不再新增主線功能**，僅 **Legacy** 相容、資料遷移與 **`VISIONGAL_BUILD_LEGACY_GALGAME=ON`** 建置路徑。 |
| **RuntimeGalgame 徹底 Legacy 化** | 預設 **`VISIONGAL_BUILD_LEGACY_GALGAME=OFF`** 為**主線與舊產品線分界**；舊 Runtime **不再**承載新路線能力。 |
| **C++ 不承擔 Gameplay 產品邏輯** | **Native**：Kernel、RHI、Renderer、Asset IO、Threading、**Scene Runtime**、Host、ABI 等。**Managed C#**：Gameplay、Entity 邏輯、Graph、Sequence、Dialogue、Inspector、Editor、Asset Pipeline、GameFramework（對齊 **Unity / Godot Mono / Unreal C# Layer** 式分工）。 |

### 0.2 階段定位：後 ABI 穩定期

- **NNNativeEngineAPI** 與託管鏡像已可版本化遞進；當前重心轉為 **Runtime 真正 Kernel 化**（統一排程、實體子系統、場景執行時），而非僅「服務表聚合」。

### 0.3 第一階段主線：P0 Runtime Kernel 化

| 代號 | 方向 | Native（目標模組／能力） | Managed（目標模組／能力） | 備註 |
|------|------|--------------------------|---------------------------|------|
| **P0-1** | **Runtime 排程與 Pipeline** | **RuntimeScheduler**（**VGRuntimeScheduler** 首包：**RuntimeTickGroup**、**FixedUpdate** 累加上限、**IRuntimeSubsystem**、**RuntimeFrameContext**；**EntitySubsystem** 掛 **Update**；**LateUpdate** 末 **FlushMainThreadDelegates** 占位） | **VisionGal.Managed.RuntimeLoop**（**ManagedRuntimeScheduler** 與 Native 管線對稱；無 Engine 依賴） | **layoutVersion** 不變；**`getRuntimeTick`** 語義不變。後續：更多子系統掛載、Async 主線程隊列、**RuntimePipelineBuilder** 可配置化。 |
| **P0-2** | **VGEntitySystem 正式化** | **NNRuntimeScene** Phase 2–3 已落地（System、層級、**NN_FIELD** 字段反射、**VGSC** 二進制序列化；**VGEngineRuntime::EcsScene** + **Update** Tick）；後續 **SceneSubsystem** 橋接、C API | **EntityWorld** 與 Native **雙世界策略** | **非**全量鏡像；**NNEntity** 打包與 **EntityHandle** 一致，**仍無**自動同步／C# 組件綁定。**EntitySubsystem** + **§2.7.1** 為前置首包。 |
| **P0-3** | **VGSceneRuntime** | **NNRuntimeScene** 為 Native 存儲後端（Phase 4+：**NNPrefab**／**NNSceneRuntime** Streaming）；Phase 3 二進制快照已就緒；JSON 與 **VGManagedScene** DTO 邊界分離 | **VisionGal.Managed.Scene.Runtime** | 主線從 **JSON Scene DTO** 走向 **Runtime Scene**；JSON 再水合保留為工具／相容路徑。 |
| **P0-4** | **Managed 元件框架** | — | **VGManagedComponent**（ComponentMetadata、PropertyBag、ComponentSerializer、InspectorBinding、ComponentActivator、DefaultValuePipeline 等） | Inspector／Graph／Serialization／SaveGame／Editor 地基。 |
| **P0-5** | **Graph Runtime（Lua 替代核心）** | **不**實作 Native Graph VM | **VisionGal.Managed.Graph.Runtime**（GraphVM、NodeExecutor、FlowScheduler、SignalBus、Async／Latent Node、變數綁定等）**100% Managed** | 統一 Dialogue、Gameplay、Event、Cutscene、狀態機；**Native 不參與**，避免重蹈 **Lua Runtime** 無限膨脹。 |

### 0.4 第二至四階段（索引）

| 階段 | 內容 |
|------|------|
| **P1 Editor 產品化** | **VisionGal.Managed.Editor** 擴充（Dock、Selection、Workspace、UndoRedo、CommandBus、ToolContext）；**VisionGal.Managed.Inspector**；**VisionGal.Managed.Graph.Editor**；Asset Browser；Scene Editor。 |
| **P1 Asset Pipeline C# 化** | **VisionGal.Managed.AssetPipeline**（Importer、Metadata、Dependency、Cooker）；Native 僅 **IO／Streaming／Compression／GPU Upload**。 |
| **P2 Gameplay Framework** | **VisionGal.Managed.GameFramework**（GameInstance、World、Level、Subsystem、SaveGame、PlayerContext、GameMode 等）。 |
| **P2 熱重載與 Roslyn** | **AssemblyLoadContext**、Editor／PlayMode Domain、**VGManagedRoslyn** 等。 |

### 0.5 與本文件既有章節之關係

- **§2.7.1**：**Kernel 首包**（**layout v5**、**EntitySubsystem**、**`entity.*`** 轉發）屬 **P0-2** 前置；**完整 VGEntitySystem** 與 **雙世界策略** 仍為 **P0-2** 本體與文檔化工作。
- **Native NNRuntimeScene Phase 2–3**（2026-05-17）：見 [NNRuntimeScene/Docs/MODULE_ARCHITECTURE_AND_PROGRESS.md](../Runtime/NNRuntimeScene/Docs/MODULE_ARCHITECTURE_AND_PROGRESS.md)；字段反射與 **VGSC** 序列化首包已落地；與 **EntityHandle** 打包對齊，**不**擴展 **NNNativeEngineAPI** layout，**無** C# 自動綁定。
- **§5.1**：Phase 6 Native（Gameplay／存檔）可與 **P0** 並行規劃，須**分別 ABI 評審**與 **layoutVersion** 同步。

---

## 1. 分層總覽

```mermaid
flowchart TB
  subgraph native [Native C++]
    Host[NNRuntimeManagedHost]
    Core[NNRuntimeManaged SHARED]
    EngABI[NNNativeEngineAPI]
    EngRt[VGEngineRuntime]
    EngRtSvc[VGEngineRuntimeServices]
    Future[VGEngine等未接線]
    Host --> Core
    Core --> EngABI
    Core -.->|可選| EngRtSvc
    EngRtSvc --> EngRt
    EngABI -.->|Stub 或 Runtime 表| EngRtSvc
    EngABI -.->|Adapter| Future
  end
  subgraph managed [Managed C#]
    MRuntime[VisionGal.Managed.Runtime]
    MCore[VisionGal.Managed.Core]
    MEng[VisionGal.Managed.Engine]
    MEngRt[VisionGal.Managed.Engine.Runtime]
    MObj[VisionGal.Managed.Object]
    MRefl[VisionGal.Managed.Reflection]
    MSer[VisionGal.Managed.Serialization]
    MGameplay[VisionGal.Managed.Gameplay]
    MEntity[VisionGal.Managed.Entity]
    MRuntime --> MCore
    MRuntime --> MEng
    MRuntime --> MEngRt
    MRuntime --> MObj
    MRuntime --> MEntity
    MObj --> MEng
    MRefl --> MObj
    MSer --> MRefl
    MEngRt --> MEng
    MGameplay --> MEng
  end
  MCore -->|"VGNativeAPI v2"| Core
  MEng -->|"NNNativeEngineAPI 鏡像"| EngABI
  Host -->|"CoreCLR + UCO"| MRuntime
```

| 層級 | 模組 / 程式集 | 職責 |
|------|----------------|------|
| **Native Runtime Host（可選）** | **NNRuntimeManagedHost** | CoreCLR 生命週期、nethost/hostfxr、`load_assembly_and_get_function_pointer`；**`VISIONGAL_ENABLE_MANAGED_HOST`** 預設 **OFF**；見 [NNRuntimeManagedHost/Docs](../../Runtime/NNRuntimeManagedHost/Docs/MODULE_ARCHITECTURE_AND_PROGRESS.md)。 |
| **Managed Runtime Foundation（Native）** | **NNRuntimeManaged**（`NevernessRuntime-Managed` SHARED） | **`VGNativeAPI`**、預設 Native 實作、**`VGNativeApi_GetDefaultTable`** 從 DLL 導出；**v2** 起掛載 **`engineServices`**。 |
| **Managed Runtime Foundation（C#）** | **NevernessRuntimeManaged-Core** | 託管鏡像與 **無 DllImport** 之 **`NativeApiBootstrap`**；見 [Core/Docs](Core/Docs/MODULE_ARCHITECTURE_AND_PROGRESS.md)。 |
| **Managed Engine SDK** | **NNNativeEngineAPI**（Native）+ **VisionGal.Managed.Engine** | **僅** Engine Service 函數表 ABI 與託管鏡像、Handle 型別、Stub / 未來 Adapter 接線點；**layout v4** 起含 **`NNEntityAPI`** 子表；**layout v5** 起尾部 **`getRuntimeTick`**（**§2.7.1** Kernel 首包）。**不含** Gameplay 產品邏輯。 |
| **Engine Runtime（Native）** | **VGEngineRuntime** + **VGEngineRuntimeServices** | 行程級 **Timing / Async / Scene（擴充）/ Object / AssetRegistry**、**EntitySubsystem**（**`entity.*`** Runtime 轉發；**layout v5**）等狀態機；**不**鏈結完整 **VGEngine**。 |
| **Managed Engine Runtime 薄封裝** | **VisionGal.Managed.Engine.Runtime** | 讀已安裝 ABI 之 **EngineTime** 等；依賴 **VisionGal.Managed.Engine**，**不**含 Gameplay。 |
| **Managed Engine Foundation** | **VGManagedObject**、**VGManagedReflection**、**VGManagedSerialization**、**VGManagedAssets**、**VGManagedScene** 等 | Unity 式地基：註冊表、元資料、序列化、資產 GUID、場景再水合；Editor/Inspector/Graph 為資料模型 Shell。 |
| **Managed Gameplay** | **VGManagedGameplay** + **VisionGal.Managed.Gameplay** | Galgame 變數表、**GameplaySessionSnapshot**、對白、**SequenceRunner**（含 slice 5：**BranchOnVariableSequenceStep**、**WaitForVariableSequenceStep**、**Advance**）；與 Legacy 分離。 |
| **Managed Entity（P0 首包 + slice 2–5）** | **VGManagedEntity** + **VisionGal.Managed.Entity** | 託管 **EntityWorld** / **EntityHandle** / **VGComponent** 與基礎元件；泛型與 **Type** 鍵讀寫查詢（含 **GetComponent**／**RemoveComponent** 非泛型重載）、**GetComponentCount**。Native 側 **§2.7.1 Kernel 首包**：**`NNEntityAPI`**（**layout v5**、**`getServiceAbiToken`** + **`getRuntimeTick`**）由 **VGEngineRuntimeServices** 轉發 **EntitySubsystem**；與託管 **EntityWorld** **並存、非自動同步**；完整 **VGEntitySystem**／資料鏡像仍見 **§2.7**／**§2.7.1** 殘留。 |
| **託管入口程式集** | **VisionGal.Managed.Runtime** | `[UnmanagedCallersOnly]` 匯出、Bootstrap **Core + Engine + Foundation**；專案參考 **VisionGal.Managed.RuntimeLoop**（**P0-1**）。 |
| **託管 Runtime Loop（P0-1）** | **VisionGal.Managed.RuntimeLoop** | 與 Native **RuntimeScheduler** 對稱之 **ManagedRuntimeScheduler**；無 Engine 依賴；單元測試與純 C# 宿主可演練 **TickGroup** 順序。 |
| **Legacy（可選建置）** | `Engine/Source/Legacy/**` | Galgame 執行時與編輯器；**`VISIONGAL_BUILD_LEGACY_GALGAME`** 預設 **OFF**。 |

---

## 2. Phase 總覽

| Phase | 名稱 | 狀態 | 說明 |
|-------|------|------|------|
| **1** | Native Runtime Host | **已完成** | C++ → C# 單向 UCO。 |
| **2** | Managed ABI Foundation | **已完成** | **NNRuntimeManaged** + **`VGNativeAPI`** + **NevernessRuntimeManaged-Core**。 |
| **3** | Managed Engine Runtime Foundation | **已完成** | **NNNativeEngineAPI**、**`engineServices`**、**VisionGal.Managed.Engine**。 |
| **4** | Engine Runtime Service Integration | **已完成（首包）** | **VGEngineRuntime** / **VGEngineRuntimeServices**、layout v2+、Host 測試 Tick/timing/async。 |
| **5** | Managed Engine Infrastructure | **已完成（首包 + 加固 + 5.3）** | Foundation 模組樹、**BootstrapEngineFoundation**、**SceneRehydrator** 實體再水合。 |
| **5.3** | Scene Entity Rehydration | **已完成** | **`ApplyEntryProperties`**、**`RehydrateFromJson`**、Bootstrap 演練。 |
| **6** | VGManagedGameplay | **進行中（託管 slice 2–5 已落地；Native 子項待定）** | **（託管）** 首包 + slice 2–5：**GameplayVariableStore**、**GameplaySessionSnapshot**、**DialoguePresenter**、**SequenceRunner**（**SceneRehydrator**、slice 5：**BranchOnVariableSequenceStep**、**WaitForVariableSequenceStep**、**SequenceRunner.Advance**）、**`BootstrapGameplay`** + **FlagGameplay**。**（Phase 6 Native 子項）** **Gameplay／存檔** 服務表與檔案 I/O 仍待 ABI 評審後實作。 |
| **7** | Managed Editor 產品化 | **未開始** | 真實 UI / 工具鏈（當前 Shell 資料模型）。 |
| **8** | Hot Reload / ALC | **未開始** | **VGManagedScripting** 脚手架；生產化在 Phase 8。 |
| **9** | VGManagedRoslyn | **未開始** | Roslyn 編譯管線。 |

### 2.5 Managed Runtime 總體狀態（2026-05-17，對齊 P0 **§2.7.1 Kernel** + **NNRuntimeScene Phase 2–3**）

| 維度 | 說明 |
|------|------|
| **完成度** | Phase **1–5.3** 已閉環；Native **[NNRuntimeScene](../Runtime/NNRuntimeScene/Docs/MODULE_ARCHITECTURE_AND_PROGRESS.md) Phase 2–3**（System、層級、字段反射、**VGSC** 序列化；**Engine** **Update** Tick 適配）已合入，**不**改 **NNNativeEngineAPI** layout、**不**自動同步 **EntityWorld**、**無** C# 組件自動綁定；**Phase 6 託管子階段**（slice **2–5**：變數 JSON、場景再水合序列步、會話快照、條件分支與 **Advance** 可恢復等待；見 §2.3–§2.6.1）**已落地**；**Phase 6 整體**仍標「進行中」係因 **Native Gameplay／存檔 ABI** 尚未納表。**P0 託管 Entity** 首包與 **slice 2–5** 已納 publish 與 **Foundation.Tests**。**§2.7.1 Kernel 首包**：**`NNEntityAPI`** **layout v5**（**`getServiceAbiToken`**、**`getRuntimeTick`**）、**EntitySubsystem**、**`BuildRuntime`** 覆寫 **`entity.*`**、託管鏡像與測試已落地；完整 **VGEntitySystem**、**EntityWorld** 資料鏡像、**VGSceneRuntime**、Graph Runtime、Editor 產品化仍未完成。 |
| **開發進展** | **VisionGal.Managed.Gameplay**／**Entity** 同上；Native **NevernessRuntime-Scene** 與 [RUNTIME_ARCHITECTURE_AND_PROGRESS.md](../Runtime/RUNTIME_ARCHITECTURE_AND_PROGRESS.md) **§5.2** 同步；**VisionGal.Managed.Engine**：**layout v5** 鏡像、**`NNEntityApi`**、**`EngineNativeApiBootstrap.ExerciseStubInteropPath`** 擴充 **Entity** 冒煙（含 **`GetRuntimeTick`**）；**NativeEngineApiEntityServiceTests** 校驗 **LayoutVersion** 與可選已安裝路徑；**NNNativeEngineAPI** Stub 與 **VGEngineRuntimeServices** **`BuildRuntime`** 覆寫 **`entity.*`** 轉發 **EntitySubsystem**；**VGManagedHostTest** 斷言 **layoutVersion**、魔數與（Runtime 路徑下）**`getRuntimeTick`**；各 MODULE 與 [RUNTIME_ARCHITECTURE_AND_PROGRESS.md](../Runtime/RUNTIME_ARCHITECTURE_AND_PROGRESS.md) **§5.2** 同步。 |
| **未完成項（索引）** | **§2.7** 表格：完整 **VGEntitySystem**、**NNRuntimeScene** Phase 4+（**Prefab**／Streaming／**SceneSubsystem** 橋接／C API）、**EntityWorld**／Native **資料鏡像**、Native **Gameplay／存檔** ABI、**VGSceneRuntime** 產品化、Graph.Runtime、Editor Phase 7、GameFramework、AssetPipeline、Hot Reload／Roslyn、Lua 遷出等。（**`NNEntityAPI`** Kernel 首包見 **§2.7.1**；**第一階段 Kernel 化** 五項索引見 **§0.3**。） |
| **未來規劃** | 短期：Phase 6 **Native** 子項見 **§5.1**；**P0+** 接續完整 **VGEntitySystem** 與 **EntityWorld** 資料策略實作（**不**承諾跨 ABI 自動灌入）。**第一階段 Kernel 化路線**（排程器、Scene Runtime、Graph.Runtime、Managed Component）見 **§0.3**–**§0.4**。中期：**VGSceneRuntime**、Graph、Editor；長期：ALC、Roslyn、Lua 移除。 |

#### 2.5.1 P0 託管 Entity 切片（補強）

| 項目 | 狀態 |
|------|------|
| **程式集** | **VisionGal.Managed.Entity**（`net10.0`，僅 BCL） |
| **API** | **EntityHandle**、**EntityWorld**（泛型與 **Type** 鍵：**HasComponent**／**GetComponent**／**TryGetComponent**／**RemoveComponent**／**GetComponentCount**）、**VGComponent**、基礎元件、**ComponentRegistry**、**ComponentPool** 骨架、**EntityArchetype** 占位 |
| **測試** | **EntityWorldTests**（Spawn/掛件/Destroy、工廠路徑、泛型與非泛型查詢／**GetComponent**／**RemoveComponent**、**GetComponentCount**、世代防護） |
| **發佈** | **VisionGal.Managed.Runtime** 專案參考；**visiongal_managed_runtime_publish** 產出含 **VisionGal.Managed.Entity.dll** |
| **文檔** | [VGManagedEntity/Docs/MODULE_ARCHITECTURE_AND_PROGRESS.md](VGManagedEntity/Docs/MODULE_ARCHITECTURE_AND_PROGRESS.md)；本檔 **§2.7**（未完成路線圖） |

#### 2.5.2 P0 託管 Entity slice 2（查詢 API，2026-05-19）

| 類別 | 內容 |
|------|------|
| **API** | **EntityWorld.HasComponent{T}**、**GetComponent{T}**（實體無效／未掛載時拋 **InvalidOperationException**，與 **AddComponent** 之失敗語意對齊）；**EntityWorld** 類 **remarks** 補充與 Native **NNEntityHandle** 無自動映射。 |
| **測試** | **EntityWorldTests**：**HasComponent** 掛載前後與銷毀後、**GetComponent** 與 **TryGet** 引用一致、缺件／已銷毀拋錯、偽造世代 Handle。 |

#### 2.5.3 P0 託管 Entity slice 3（元件計數與註解，2026-05-20）

| 類別 | 內容 |
|------|------|
| **API** | **EntityWorld.GetComponentCount**（僅存活實體回傳字典鍵數量，否則 0；不暴露可變集合）。 |
| **註解** | **Spawn**／**Destroy**／**IsAlive**／**AddComponent**／**RemoveComponent**／**ClearForTesting** 行內中文補強；類 **remarks** 納入 **GetComponentCount**。 |
| **測試** | **EntityWorldTests**：空實體、掛載／移除、銷毀後計數、偽造世代 Handle 為 0。 |

#### 2.5.4 P0 託管 Entity slice 4（非泛型 **Type** 鍵查詢，2026-05-21）

| 類別 | 內容 |
|------|------|
| **API** | **EntityWorld.HasComponent(EntityHandle, Type)**、**TryGetComponent(EntityHandle, Type, out VGComponent?)**；與 **ComponentRegistry.TryCreate** 之精確 **Type** 鍵一致；非法型別拋 **ArgumentException**、null 拋 **ArgumentNullException**。 |
| **註解** | **EntityWorld** 類 **remarks** 補泛型／非泛型雙路徑說明；私有 **ThrowIfInvalidComponentLookupType** 附 XML。 |
| **測試** | **EntityWorldTests**：與泛型引用一致、缺件／銷毀／偽造世代、null 與非 **VGComponent** 型別參數校驗。 |

#### 2.5.5 P0 託管 Entity slice 5（非泛型 **Type** 鍵 **GetComponent**／**RemoveComponent**，2026-05-15）

| 類別 | 內容 |
|------|------|
| **API** | **EntityWorld.GetComponent(EntityHandle, Type)**（與泛型 **GetComponent{T}** 例外語意一致；成功回傳非 null **VGComponent**）；**EntityWorld.RemoveComponent(EntityHandle, Type)**（與泛型 **RemoveComponent{T}** 一致：已死靜默返回；存活則以精確鍵移除）。 |
| **註解** | **EntityWorld** 類 **remarks** 補 **Type** 鍵與泛型成對、供資料驅動／反射路徑；**ThrowIfInvalidComponentLookupType** 擴及 **Get**／**Remove** 非泛型路徑。 |
| **測試** | **EntityWorldTests**：非泛型 **Get**／**Remove** 與泛型引用與訊息一致、**GetComponentCount** 遞減、參數校驗。 |

### 2.1 Phase 5.3 摘要（2026-05-15）

| 類別 | 內容 |
|------|------|
| **API** | **`SceneSerializer.ApplyEntryProperties`**；**`SceneRehydrator`** / **`Scene.RehydrateFromJson`**。 |
| **Bootstrap** | **`BootstrapEngineFoundation`** 在 JSON 往返後清空註冊表並再水合，驗證 **DisplayName** 與 Native **IsAlive**。 |
| **測試** | **SceneRehydrationTests**（屬性套用 + 條件式 Engine 整合）；**VersionToleranceTests**（場景 DTO 含未知根欄位仍可反序列化）。 |

### 2.2 Phase 6 首包摘要（2026-05-15）

| 類別 | 內容 |
|------|------|
| **模組** | **VGManagedGameplay** / **VisionGal.Managed.Gameplay**。 |
| **API** | **GameplayVariableStore**；**DialoguePresenter**（`NNUIApi.SetDialogueText`，未安裝 ABI 時 no-op）。 |
| **測試** | **GameplayVariableStoreTests**、**DialoguePresenterTests**。 |
| **發佈** | **VisionGal.Managed.Runtime** 專案參考 **VisionGal.Managed.Gameplay**，**ManagedRuntimePublish** 含 **VisionGal.Managed.Gameplay.dll**；**VGManagedHostTest** 校驗該檔。 |

### 2.3 Phase 6 slice 2（2026-05-15）

| 類別 | 內容 |
|------|------|
| **API** | **GameplayVariableStore.ToJson** / **TryParseFromJson**（`formatVersion` + `entries`，型別 `string\|bool\|int64\|double`）；**SequenceRunner**、**SetVariableSequenceStep**、**PresentDialogueSequenceStep**。 |
| **UCO** | **`Entry.BootstrapGameplay`**（須在 **`BootstrapNativeApi`** 之後）；**`GetBootstrapFlags`** 新增 **`FlagGameplay` = 1<<4**。 |
| **測試** | **GameplayVariableStoreJsonTests**、**SequenceRunnerTests**；**VGManagedHostTest** 斷言 Gameplay 旗標。 |

### 2.4 Phase 6 slice 3（2026-05-15）

| 類別 | 內容 |
|------|------|
| **API** | **SequenceContext.ActiveScene**；**RehydrateSceneSequenceStep**、**SyncFirstEntityDisplayNameToVariableSequenceStep**（場景 JSON → 再水合 → 首實體 **DisplayName** 寫入變數表）。 |
| **依賴** | **VisionGal.Managed.Gameplay** 新增 **ProjectReference** → **VisionGal.Managed.Scene**。 |
| **Bootstrap** | **`Entry.BootstrapGameplay`**：清空註冊表後建場景快照 → 再清空 → 序列內再水合並校驗 **rehydratedTitle** 與 **gpBootstrap**。 |
| **測試** | **SequenceRunnerTests.Run_RehydrateSceneThenSyncFirstDisplayName_WritesVariable**（僅在 **EngineNativeApiBootstrap.IsInstalled** 時執行，與 **SceneRehydrationTests** 一致）；既有 **VGManagedHostTest** 仍斷言 **FlagGameplay**。 |

### 2.6 Phase 6 slice 4（2026-05-15）

| 類別 | 內容 |
|------|------|
| **API** | **GameplaySessionSnapshot**（`formatVersion`、`variableStoreJson`、`sceneJson?`）、**Capture** / **ToJson** / **TryParseFromJson** / **ApplyTo**；**GameplayVariableStore.CopyFrom**（供快照載入覆寫變數表）。 |
| **Bootstrap** | **`Entry.BootstrapGameplay`**：序列成功後組裝快照（含當前場景 JSON）→ 根 JSON 往返 → **ApplyTo** 新表並校驗 **gpBootstrap** / **rehydratedTitle**。 |
| **測試** | **GameplaySessionSnapshotTests**（純託管往返、根層未知欄位、可選場景字串；真實場景 JSON 仍受 Engine 安裝門檻約束）。 |

### 2.6.1 Phase 6 slice 5（2026-05-15）

| 類別 | 內容 |
|------|------|
| **API** | **BranchOnVariableSequenceStep**（變數寬鬆相等則執行 then/else 子序列）；**WaitForVariableSequenceStep**（真值門闩或與期望值相等）；**SequenceMachineState**、**SequenceRunner.Advance** / **SequenceAdvanceKind**（等待步阻塞時 **Waiting**，不推進下標）。 |
| **語義** | 含等待步之劇本應以 **Advance** 輪詢；線性 **Run** 在變數條件未滿足時對等待步返回 **false**；無 Native Input / Sequence 新 ABI。 |
| **測試** | **SequenceRunnerTests**（分支 then/else、**Advance** 之 **Waiting→Completed**、**Run** 與變數門檻）。 |

### 2.7 未完成與優先路線（P0–P2）

| 優先級 | 項目 | 狀態 | 說明 |
|--------|------|------|------|
| **P0** | **VisionGal.Managed.Entity**（託管） | **首包 + slice 2–5 已落地** | **EntityWorld**（Spawn / Destroy；泛型與 **Type** 鍵 **HasComponent**／**TryGetComponent**／**GetComponent**／**RemoveComponent**；**GetComponentCount**；泛型 **TryGet**）；**VGComponent** / 基礎元件；與 **SceneEntity** 並存。Native **§2.7.1 Kernel 首包**（**`NNEntityAPI`** **layout v5**）已接線；與 **EntityWorld** **並存、非自動同步**；資料鏡像見 **§2.7.1** 殘留。 |
| **P0** | **NNEntityAPI**（Native） | **Kernel 首包已落地／完整 ECS 子系統未開始** | **`EntityAPI.h`**、**`getServiceAbiToken`**、**`getRuntimeTick`**（尾部追加）、**`NN_NATIVE_ENGINE_API_LAYOUT_VERSION` = 5**；**EntitySubsystem**（經 **RuntimeScheduler** 之 **Update** 階段遞增 **`runtimeTick`**）；**`VGEngineRuntimeServices::BuildRuntime`** 覆寫 **`entity.*`**；Stub 供 **`BuildDefault`**／OFF 路徑；**VGManagedHostTest**、**NativeEngineApiEntityServiceTests**。真實 **VGEntitySystem**、與託管 **EntityWorld** 之資料鏡像仍待後續。 |
| **P0** | **Legacy RuntimeGalgame** | **凍結** | 停止新功能；與新 C# 路線並行直至遷移。 |
| **P0** | **VGEngineRuntime** Kernel 化 | **進行中（P0-1 Scheduler 首包 + §2.7.1 Entity）** | **Timing / Async / Scene / Object / AssetRegistry** 等轉發已落地；**§2.7.1** 起 **EntitySubsystem**；**P0-1** 起 **RuntimeScheduler** 統一 **IRuntimeSubsystem** 管線（**EntitySubsystem** 於 **Update** 組；**FixedUpdate**／**LateUpdate** 占位與累加器上限與託管 **ManagedRuntimeScheduler** 對齊）。完整 **VGEntitySystem** ECS、**VGAsset** 真實管線仍為後續擴充項。 |
| **P1** | **VGSceneRuntime** + **VGSceneRuntimeAPI** + **VisionGal.Managed.Scene.Runtime** | **未開始** | Load/Unload、Prefab Runtime、Streaming、Async Scene。 |
| **P1** | **VisionGal.Managed.Graph.Runtime**（GraphVM 等） | **未開始** | 全 C#；取代 Lua Sequence 之長期目標。 |
| **P1** | **Phase 7 Editor**（Core / Graph.Editor / Inspector / AssetBrowser / SceneEditor） | **未開始** | 當前 **VGManagedEditor** 為 Shell。 |
| **P2** | **VisionGal.Managed.GameFramework** | **未開始** | GameInstance、World、Level、Subsystem、SaveGame 等。 |
| **P2** | **VisionGal.Managed.AssetPipeline** | **未開始** | Importer、Meta、Cooker、Build。 |
| **P2** | **Hot Reload / ALC / Roslyn / Managed PlayMode** | **未開始** | 對齊 Phase 8–9 與 **VGManagedScripting** 深化。 |
| **長期** | **移除 Lua**（sol2、Lua UI/Sequence/Script） | **未開始** | 依賴 Graph Runtime + Gameplay + Editor 成熟後執行。 |

#### 2.7.1 Native **NNEntityAPI** 與 Kernel 首包（索引）— **§2.7.1 已落檔（含 layout v5）**

以下涵蓋 **ABI 前置**與 **Kernel 首包**；**完整 VGEntitySystem** 與 **EntityWorld 資料鏡像**仍列殘留。

| 原條目 | 狀態 | 說明 |
|--------|------|------|
| **ABI 評審** | **已落檔** | 採 **`NNNativeEngineAPI` 聚合體尾部**獨立 **`NNEntityAPI entity`** 子表（不擴展 **`NNSceneAPI`**）；子表僅允許**尾部追加**欄位；**`NN_NATIVE_ENGINE_API_LAYOUT_VERSION`** 已遞增至 **5**（**`getRuntimeTick`**）；與 **VisionGal.Managed.Engine** **`LayoutVersion`**、**`NNNativeEngineApi`** 欄位順序同步。 |
| **Kernel 首包** | **已落地** | **`EntityAPI.h`**；**`NNNativeEngineApiStubs.cpp`** 之 **`getServiceAbiToken`** / **`getRuntimeTick`**；**`EntitySubsystem`**（**`RuntimeScheduler`** 在 **Update** 階段驅動 **`runtimeTick`** 遞增；**`Shutdown`** 經調度器 **`ShutdownRegistered`** 觸發 **`Reset`**）；**`VGEngineRuntimeServices::BuildRuntime`** 覆寫 **`outTable->entity.*`** 轉發至 **`VGEngineRuntime::Instance().Entity()`**；**`EngineNativeApiBootstrap.ExerciseStubInteropPath`**；**VGManagedHostTest**、**`NativeEngineApiEntityServiceTests`**。 |
| **與 SceneAPI 之邊界** | **已文件化** | **`SceneAPI.h` / `EngineHandles.h` / `EntityAPI.h`** 與 [NNNativeEngineAPI MODULE](../Runtime/NNNativeEngineAPI/Docs/MODULE_ARCHITECTURE_AND_PROGRESS.md)：**`NNEntityHandle`**＝場景圖；託管 **`EntityHandle`**＝純 C# ECS；**不承諾**數值互通；**`getRuntimeTick`** 僅供觀測 Runtime 已驅動子表，**不代表** **EntityWorld** 已鏡像。 |

**EntityWorld／Gameplay 資料策略（文件契約）**：託管 **EntityWorld** 與 Native **Entity** 路徑 **並存**、**無跨 ABI 自動同步**；未來若需對齊，僅經 **Gameplay 變數**、顯式 Facade 或專用橋接 API（本階段**不**實作自動灌入）。

**殘留（後續）**：完整 **VGEntitySystem**（Native ECS 等）、**EntityWorld** 與 Native **資料結構鏡像**、Gameplay 與引擎實體之產品級對齊（索引仍見 **§2.7** 優先級表）。

---

## 3. 關鍵設計決策（摘要）

1. **函數表優先**：託管端經 **`VGNativeAPI*`** 與 **`NNNativeEngineAPI*`** 間接呼叫引擎能力，避免散落 `DllImport`。
2. **版本欄位**：**`VG_NATIVE_API_VERSION`** / **`NN_NATIVE_ENGINE_API_LAYOUT_VERSION`** 與託管常數同步；破壞性佈局變更必須遞增。
3. **Host 不膨脹**：hostfxr 封裝在 **NNRuntimeManagedHost**（可選建置）；宿主 ABI 在 **NNRuntimeManaged**（SHARED）；Engine 服務 ABI 契約在 **NNNativeEngineAPI**，Stub 在 **NNRuntimeNativeEngineAPIStub**。
4. **雙 DLL 部署**：啟用 Host 時需 **NevernessRuntime-ManagedHost.dll** + **NevernessRuntime-Managed.dll**；可選 **NevernessRuntime-EngineServices** 鏈入建表路徑。
5. **Handle 邊界**：資源以 **uint64** Handle 暴露；禁止託管層持有 C++ 物件指標穿越 ABI。
6. **Object 生命週期**：**`createObject`** 成功即 ref=1，**`Dispose`** 遞減至 0 後 **destroy**；再水合一律建立**新** Native 控制代碼。

---

## 4. 建置與測試（Windows / MSVC）

前提：vcpkg **`nethost`**、**.NET 10 SDK**、**`VISIONGAL_ENABLE_MANAGED_HOST=ON`**、**`ENABLE_TESTS=ON`**。

```bat
cmake -B build -DCMAKE_TOOLCHAIN_FILE=<vcpkg>/scripts/buildsystems/vcpkg.cmake -DENABLE_TESTS=ON -DVISIONGAL_ENABLE_MANAGED_HOST=ON
cmake --build build --config Debug --target VGManagedHostTest visiongal_managed_runtime_publish visiongal_managed_foundation_tests
ctest -C Debug -R VGManagedHost --output-on-failure
dotnet test Engine/Source/Managed/Tests/VisionGal.Managed.Foundation.Tests/VisionGal.Managed.Foundation.Tests.csproj -c Debug
```

`dotnet publish` 輸出目錄預設：**`${CMAKE_BINARY_DIR}/ManagedRuntimePublish`**。

---

## 5. 未來規劃（Phase 6+）

| Phase | 目標 | 依賴 |
|-------|------|------|
| **6** | Native **Gameplay / 存檔** ABI 等（**Phase 6 Native 子項**） | **託管** slice 2–5 與 **Entity** 首包 + **§2.5.2–2.5.5**（查詢／計數／**Type** 鍵讀寫）已落地；**Native** 服務表／檔案 I/O 仍待 ABI 評審（與 **§2** Phase 6 說明欄一致）。 |
| **P0+** | Native **VGEntitySystem**（實體子系統）、**VGSceneRuntime**、**Graph.Runtime** | 見 §2.7（**NNEntityAPI** Kernel 首包已 **layout v5**；完整系統本體仍待） |
| **7** | Editor 產品化：Inspector/Graph 與真實 UI | Phase 6 或並行 Shell 深化 |
| **8** | **AssemblyLoadContext**、Hot Reload 卸載 | 宿主多程序集策略 |
| **9** | **Roslyn** 腳本編譯（`VISIONGAL_ENABLE_ROSLYN`） | ABI 凍結、ALC 就緒 |
| **長期** | **VGEngine** 全量 Adapter 替換 Stub | 各 Engine Service 子表逐項接線 |

### 5.1 Phase 6 Native 子項（Gameplay／存檔）推進順序（草案）

以下為 **不改 layout 前** 的建議順序，實作前須完成 **ABI 佈局評審** 並同步 **VisionGal.Managed.Engine** 鏡像常數；細節仍以 **§2.7** 優先級表為準。

| 順序 | 工作項 | 說明 |
|------|--------|------|
| **1** | **佈局與版本** | 在 **NNNativeEngineAPI** 定義 **Gameplay／Save** 子函數表（或擴展既有服務表）；遞增 **`NN_NATIVE_ENGINE_API_LAYOUT_VERSION`**；託管端補齊讀表與 **Stub** 演練。 |
| **2** | **最小 I/O 語義** | 明確存檔路徑／slot、錯誤碼、同步或異步回調；與 **GameplaySessionSnapshot** 託管 JSON 之對應（可選：Native 僅管檔案、內容仍由託管序列化）。 |
| **3** | **Runtime 轉發** | 在 **VGEngineRuntimeServices**（或專用服務）掛接實表；**VGManagedHostTest** + **Foundation.Tests** 擴充跨 ABI 演練。 |
| **4** | **產品化** | 與 **Phase 7** Editor、**Graph.Runtime** 長期路線對齊；不與 Legacy Galgame 新路徑耦合。 |

---

## 6. 變更記錄

| 日期 | 說明 |
|------|------|
| **2026-05-14** | **Phase 2–4** 落地；本總覽首版。 |
| **2026-05-15** | **Phase 5 首包**：layout v3、Legacy 凍結、Foundation 模組樹、**BootstrapEngineFoundation**。 |
| **2026-05-15** | **Phase 5 加固**：生命週期/匯入/場景 DTO 往返/反射修正；Foundation.Tests + **GetBootstrapFlags**。 |
| **2026-05-15** | **Phase 5.3 + Phase 6 首包**：**SceneRehydrator**、**ApplyEntryProperties**；**VGManagedGameplay**；文檔與測試同步。 |
| **2026-05-15** | **Runtime ↔ Gameplay publish**：Runtime 專案參考 Gameplay；GTest 斷言 publish 含 **VisionGal.Managed.Gameplay.dll**；**VersionToleranceTests**（未知 JSON 根欄位）。 |
| **2026-05-15** | **Phase 6 slice 3**：**SequenceRunner** 場景再水合步驟、**Gameplay→Scene** 專案參考、**BootstrapGameplay** 擴充；**MANAGED_RUNTIME** / 模組文檔與中文註解同步。 |
| **2026-05-15** | **Phase 6 slice 4**：**GameplaySessionSnapshot**、**CopyFrom**、**BootstrapGameplay** 快照演練、**GameplaySessionSnapshotTests**；文檔與 **merge_docs** 同步。 |
| **2026-05-15** | **P0 託管 Entity 首包**：**VGManagedEntity** / **VisionGal.Managed.Entity**；Runtime 參考；CMake **DEPENDS**；**VGManagedHostTest** 斷言 **VisionGal.Managed.Entity.dll**；**EntityWorldTests**；**§2.7** 路線圖與 **merge_docs**。 |
| **2026-05-15** | **P0 Entity 補強**：**VisionGal.Managed.Entity** 與 **EntityWorldTests** 補充詳細中文註解；**MANAGED_RUNTIME §2.5** 擴充總體完成度／未完成項索引／**§2.5.1** 切片表；**VGManagedEntity** MODULE 進度表更新；**merge_docs**。 |
| **2026-05-15** | **Phase 6 slice 5**：**BranchOnVariableSequenceStep**、**WaitForVariableSequenceStep**、**SequenceMachineState**、**SequenceRunner.Advance**；**SequenceRunnerTests** 擴充；**VGManagedGameplay** MODULE 與本總覽 **§2.6.1**／Phase 表同步；**merge_docs**。 |
| **2026-05-15** | **§2.5** 總體狀態更新（slice 5 納入完成度／未完成項仍指向 §2.7）；**VGManagedGameplay** MODULE 完成度表；**RUNTIME** 根 **§5.2** 與 **NNNativeEngineAPI** / **VGEngineRuntime** MODULE 進展列；**SequenceRunner** 類步驟之簡體中文 XML 補強；**merge_docs**。 |
| **2026-05-15** | **§2 Phase 6** 狀態欄與說明欄拆清「託管 slice 2–5 已落地」vs「Native Gameplay／存檔子項」；**§2.5** 同步完成度語義與 **Foundation.Tests**／**VGManagedHostTest** 表述；**SequenceFlow.cs** XML；**RUNTIME §5.1** 里程碑；**merge_docs**。 |
| **2026-05-15** | **§5.1** 增 **Phase 6 Native 子項**（Gameplay／存檔）推進順序草案；**§2.5** 未來規劃指向 **§5.1**；**RUNTIME** 增 **§6** 未來規劃；**NNNativeEngineAPI**／**VGManagedGameplay** MODULE 與 **GameplaySessionSnapshot** 註解交叉；**merge_docs**；**Foundation.Tests**。 |
| **2026-05-15** | **P0 託管 Entity slice 2**：**EntityWorld.HasComponent** / **GetComponent**、**EntityWorldTests** 擴充、**EntityWorld** 類 **remarks**；**§2.5.1**／新增 **§2.5.2**；**§2.7** 託管 Entity 狀態更新與新增 **§2.7.1** Native **NNEntityAPI** 前置索引；**RUNTIME** 根 **§5.1**／**§5.2**、**NNNativeEngineAPI**／**VGEngineRuntime**／**VGManagedEntity** MODULE 交叉；**merge_docs**；**Foundation.Tests**。 |
| **2026-05-15** | **P0 託管 Entity slice 3**：**EntityWorld.GetComponentCount**、關鍵路徑行內中文註解；新增 **§2.5.3**；**§2.5**／**§2.7**／Phase 6 依賴列同步；**VGManagedEntity** MODULE、**RUNTIME** 根 **§5.1**／**§5.2**／**§6.1**、**VGEngineRuntime** MODULE；**merge_docs**；**Foundation.Tests**。 |
| **2026-05-21** | **P0 託管 Entity slice 4**：**EntityWorld.HasComponent(handle, Type)**、**TryGetComponent(handle, Type, out)**；**EntityWorldTests** 非泛型路徑；新增 **§2.5.4**；**§2.5**／**§2.7**／§1 分層表／Phase 6 依賴列；**VGManagedEntity** MODULE、**RUNTIME** 根、**NNNativeEngineAPI**／**VGEngineRuntime** MODULE；**merge_docs**；**Foundation.Tests**。 |
| **2026-05-15** | **P0 託管 Entity slice 5**：**EntityWorld.GetComponent(handle, Type)**、**RemoveComponent(handle, Type)**；**EntityWorldTests** 對稱閉環；新增 **§2.5.5**；**§2.5**／**§2.5.1**／**§2.7**／§1 分層表／Phase 6 依賴列；**VGManagedEntity** MODULE、**RUNTIME** 根 **§5.1**／**§5.2**／**§6.1**、**NNNativeEngineAPI**／**VGEngineRuntime** MODULE；**merge_docs**；**Foundation.Tests**。 |
| **2026-05-15** | **§2.7.1 首包**：**layout v4**、**`EntityAPI.h`**、**`NNEntityAPI`** Stub 與託管鏡像；**SceneAPI**／**EngineHandles**／**EntityAPI** 邊界註釋；**VGEngineRuntimeServices** 繼承 Stub 註釋；**VGManagedHostTest**／**NativeEngineApiEntityServiceTests**；**§2.7**／**§2.7.1**／**RUNTIME**／各 MODULE 與 **merge_docs**。 |
| **2026-05-15** | **§2.7.1 Kernel 首包（layout v5）**：**`getRuntimeTick`**、**EntitySubsystem**、**`BuildRuntime`** 覆寫 **`entity.*`**；託管鏡像與 **ExerciseStubInteropPath**／測試擴充；**§207**／**§2.7**／**RUNTIME**／各 MODULE；**EntityWorld** 並存與非自動同步策略文件化；**merge_docs**。 |
| **2026-05-15** | **P0 文檔與註解收口**：**VGEngineRuntime**／**EntitySubsystem** 源碼補充詳細簡體中文文件頭與契約說明；**NNNativeEngineApiConstants** XML 擴充；**Tests/CMakeLists.txt** 使 **visiongal_managed_foundation_tests** 依賴 **visiongal_managed_runtime_publish** 以避免並行 **dotnet** 競態；**§2.5**／**§2.7** Kernel 化狀態欄更新；**RUNTIME §5.2** 日期對齊；相關 MODULE 變更列；**merge_docs**；**Foundation.Tests**。 |
| **2026-05-15** | **P0 對齊審計（MERGED 刷新）**：核對 Native **layout v5**、**BuildRuntime** **`entity.*`**、託管鏡像與測試與 **§2.7.1** 一致；**VGEngineRuntimeServices.cpp**／**NNNativeEngineApiTypes.cs** 補強中文註解；**NNNativeEngineAPI**／**VGEngineRuntimeServices**／**VGManagedHost** MODULE 與 **merge_docs**；無行為變更。 |
| **2026-05-15** | **§0 VisionGal 主線（2026）**：Lua／Legacy 分界、**C++ 不承擔 Gameplay**、**P0 Runtime Kernel 化**（P0-1～P0-5）與 **P1／P2** 索引表；與 **§2.5**／**§2.7** 關聯說明；各相關 MODULE 交叉更新；**merge_docs**。 |
| **2026-05-15** | **P0-1 RuntimeScheduler**：Native **RuntimeScheduler**／**IRuntimeSubsystem**／**EntitySubsystem** 掛 **Update**；託管 **VisionGal.Managed.RuntimeLoop**；**VGManagedHost** CMake 與 **Foundation.Tests**；**layoutVersion** 不變；文檔 **§0.3**／**§2.7**／**§2.7.1**、**VGEngineRuntime**／**VGManagedRuntimeLoop** MODULE、**RUNTIME** 根；**merge_docs**。 |

---

## 7. 文檔合併

在 `Engine/Source/Managed` 執行 `python merge_docs.py` 可重新生成 **MERGED_ARCHITECTURE_AND_PROGRESS.md**（自動生成檔，請勿手改正文）。
