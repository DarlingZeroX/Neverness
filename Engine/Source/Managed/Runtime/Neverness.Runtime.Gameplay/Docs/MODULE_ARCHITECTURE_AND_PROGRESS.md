# VGManagedGameplay — Galgame 執行時（VisionGal.Managed.Gameplay）

## 1. 定位

| 項目 | 說明 |
|------|------|
| **職責** | Phase 6：**GameplayVariableStore**（託管變數表 + **JSON 快照** `ToJson` / `TryParseFromJson`）、**GameplaySessionSnapshot**（會話根 JSON：`variableStoreJson` + 可選 `sceneJson`）、**DialoguePresenter**、**SequenceRunner** / **ISequenceStep**（SetVariable / PresentDialogue / **RehydrateScene** / **SyncFirstEntityDisplayNameToVariable**；slice 5：**BranchOnVariableSequenceStep**、**WaitForVariableSequenceStep**、**SequenceMachineState**、**SequenceRunner.Advance**）。 |
| **程式集** | **VisionGal.Managed.Gameplay**（`net10.0`，`AllowUnsafeBlocks`） |
| **依賴** | **VisionGal.Managed.Core**、**VisionGal.Managed.Engine**、**VisionGal.Managed.Scene**、**VisionGal.Managed.Serialization**（**VersionTolerance** 選項） |
| **消費方** | **VisionGal.Managed.Runtime**（專案參考，使 `dotnet publish` 輸出含本程式集）；Foundation 單元測試。 |
| **不負責** | Native 劇本驅動 / Sequence **ABI**、Native 專用存檔 I/O、Legacy Galgame、Roslyn / Hot Reload。長期：**Lua Sequence** 由 **VisionGal.Managed.Graph.Runtime**（**§0.3** **P0-5**，**100% Managed**）替代，見總覽 **§0**。 |

## 2. 完成度與進度總覽（2026-05-17）

| 區塊 | 狀態 |
|------|------|
| **變數與持久化** | **GameplayVariableStore**（記憶體 + **ToJson** / **TryParseFromJson**）、**CopyFrom**；**GameplaySessionSnapshot**（根 JSON + 嵌套變子表 + 可選場景）已落地並有單測。 |
| **對白** | **DialoguePresenter** 經 Engine 服務表；ABI 未安裝時 no-op。 |
| **序列編排** | **SequenceRunner**：線性 **Run**；slice 3 場景再水合步驟；slice 5 **BranchOnVariableSequenceStep**、**WaitForVariableSequenceStep**、**SequenceMachineState**、**Advance**（**Waiting** 不推進下標）。實作分檔：**SequenceRunner.cs**、**SequenceFlow.cs**。 |
| **測試** | **VisionGal.Managed.Foundation.Tests**：變數表、對白、JSON、線性序列、場景聯動（條件式）、快照、分支／等待步驟／狀態機。 |
| **與總覽 Phase 6** | 與 [MANAGED_RUNTIME_ARCHITECTURE_AND_PROGRESS.md](../../MANAGED_RUNTIME_ARCHITECTURE_AND_PROGRESS.md) **§2** 一致：**託管** slice 2–5 已落地；**Native Gameplay／存檔** 為 Phase 6 **未開始子項**，見 **§2.7** 與總覽 **§5.1** 推進順序草案。 |
| **與 Native** | 本模組不擴充 **NNNativeEngineAPI** layout；後續 **Gameplay／存檔** ABI 見總覽 **§5.1**、**§2.7**。 |

## 3. Phase 6 進展

| 日期 | 進展 |
|------|------|
| **2026-05-15** | **首包**：變數表 API、對白 Presenter（ABI 未安裝時 no-op）；**VisionGal.Managed.Foundation.Tests** 覆蓋。 |
| **2026-05-15** | **Runtime 納入 publish**：**VisionGal.Managed.Runtime** 專案參考本模組；**VGManagedHostTest** 斷言 **VisionGal.Managed.Gameplay.dll** 存在於 publish 根目錄。 |
| **2026-05-15** | **Phase 6 slice 2**：變數表 JSON（`string|bool|int64|double`）、**SequenceRunner**、**Entry.BootstrapGameplay** UCO；**GetBootstrapFlags** 新增 **FlagGameplay (1<<4)**；**VisionGal.Managed.Foundation.Tests** 覆蓋 JSON 與序列。 |
| **2026-05-15** | **Phase 6 slice 3**：**SequenceContext.ActiveScene**；**RehydrateSceneSequenceStep** / **SyncFirstEntityDisplayNameToVariableSequenceStep** 與 **SceneRehydrator** 聯動；**Entry.BootstrapGameplay** 演練；程式集新增 **VisionGal.Managed.Scene** 參考；程式碼補充中文註解。 |
| **2026-05-15** | **Phase 6 slice 4**：**GameplaySessionSnapshot**（根 JSON + 嵌套變子文檔 + 可選場景 JSON）、**GameplayVariableStore.CopyFrom**；**BootstrapGameplay** 末尾演練快照往返；**GameplaySessionSnapshotTests**。 |
| **2026-05-15** | **Phase 6 slice 5**：託管序列**條件分支**與**可恢復等待**（**Advance** + **Waiting**）；**SequenceFlow.cs**（**SequenceMachineState**、**SequenceAdvanceKind**）；**SequenceRunnerTests** 擴充；公開 API 補充簡體中文 XML。 |
| **2026-05-16** | MODULE 增 **§2 完成度與進度總覽**；與 **RUNTIME** 根 **§5.2**、**MANAGED_RUNTIME §2.5** 對齊；**SequenceRunner.cs** 步驟型別之簡體中文 XML 補強。 |
| **2026-05-17** | **§2** 對齊總覽 **§2** Phase 6「託管已落地／Native 子項待定」表述；**SequenceFlow.cs** 註解補強；驗證矩陣見總覽 **§2.5**。 |
| **2026-05-18** | Native **Gameplay／存檔** 推進順序見總覽 [MANAGED_RUNTIME_ARCHITECTURE_AND_PROGRESS.md](../../MANAGED_RUNTIME_ARCHITECTURE_AND_PROGRESS.md) **§5.1**；**GameplaySessionSnapshot** 註解補充與 Native 對接說明。 |
| **2026-05-15** | **§0**：**不負責** 擴表列出 **VisionGal.Managed.Graph.Runtime**（**P0-5**）；長期 **Lua Sequence** 遷出主線見總覽 **§0**。 |

## 4. 後續

- Native Gameplay / 存檔 **ABI**（若凍結）與實際檔案 I/O；**實施順序草案**見總覽 [MANAGED_RUNTIME_ARCHITECTURE_AND_PROGRESS.md](../../MANAGED_RUNTIME_ARCHITECTURE_AND_PROGRESS.md) **§5.1**。
- 多實體條件步驟、跨 **Advance** 邊界之巢狀等待（子序列內變數門檻）等進階編排。
