# VGManagedEntity — 託管實體／元件世界（VisionGal.Managed.Entity）

## 1. 定位

| 項目 | 說明 |
|------|------|
| **職責** | P0：**EntityHandle**（Index + Generation）、**EntityWorld**（Spawn / Destroy / IsAlive；泛型與 **Type** 鍵 **HasComponent**／**TryGetComponent**／**GetComponent**／**RemoveComponent**；**GetComponentCount**；泛型 **TryGet**）、**VGComponent** 基底與 **Transform / Name / Active / Hierarchy** 元件、**ComponentPool** 骨架、**ComponentRegistry** 工廠表、**EntityArchetype** 枚舉占位。 |
| **程式集** | **VisionGal.Managed.Entity**（`net10.0`） |
| **依賴** | 無（僅 BCL） |
| **消費方** | **VisionGal.Managed.Runtime**（專案參考，納入 publish）；Foundation 單元測試。 |
| **不負責** | Native **VGEntitySystem** 完整實作；與 **SceneEntity** JSON 再水合無自動對應（並存，遷移另議）。**`NNEntityAPI`** Kernel 首包（**layout v5**、**`BuildRuntime`** 覆寫 **`entity.*`**）見 **VisionGal.Managed.Engine** 鏡像與 MANAGED **§2.7.1**；**EntityWorld** 與 Native **並存、非自動同步**（見 **§2.7.1** 資料策略）。 |

### 1.1 雙世界策略（2026 主線，MANAGED **§0.3** P0-2）

| 世界 | 職責 |
|------|------|
| **Native（目標）** | Runtime 實體句柄、場景圖、流式載入、**Transform**、渲染附掛等（**VGEntitySystem**／**VGSceneRuntime** 演進中）。 |
| **Managed（本程式集）** | **Gameplay ECS**：**EntityWorld** 純 C# 元件表與查詢；**不**承諾與 Kernel 資料結構自動鏡像；對齊透 **Gameplay 變數**、顯式 Facade 或未來窄橋接。 |

## 2. 與 VGManagedScene 之關係

- **SceneEntity**（`VisionGal.Managed.Scene`）仍為 VGObject 生命週期與序列化路徑；**EntityWorld** 為純託管 ECS 風格內核首包。
- 未來可選：劇本層橋接、或 **VGSceneRuntime** 載入後向 **EntityWorld** 顯式灌入元件（見總覽 **§0.3** **P0-3**；**非**自動同步）。

## 3. 完成度與進度總覽（2026-05-15）

| 區塊 | 狀態 |
|------|------|
| **核心型別** | **EntityHandle**、**EntityWorld**、**VGComponent** 已落地；**slice 2** 泛型 **HasComponent**／**GetComponent**；**slice 3** **GetComponentCount** 與行內中文註解；**slice 4** **HasComponent(handle, Type)**／**TryGetComponent(handle, Type, out)**（與 **ComponentRegistry.TryCreate** 鍵一致）；**slice 5** **GetComponent(handle, Type)**／**RemoveComponent(handle, Type)**（與泛型面對稱）；公開成員與 **EntityWorld** 已補中文 XML／行內註解。 |
| **基礎元件** | **Transform / Name / Active / Hierarchy** 首包欄位就緒；無自動與 Scene 同步。 |
| **擴展點** | **ComponentRegistry**（靜態工廠）、**ComponentPool**（骨架）、**EntityArchetype**（占位）。 |
| **測試** | **EntityWorldTests**（掛件／Destroy、工廠路徑、泛型與非泛型查詢／**Get**／**Remove**、**GetComponentCount**、世代偽造 Handle）；見 **VisionGal.Managed.Foundation.Tests**。 |
| **發佈鏈** | **VisionGal.Managed.Runtime** → publish；**VGManagedHost** CMake **DEPENDS**；**VGManagedHostTest** 斷言 **VisionGal.Managed.Entity.dll**。 |

## 4. 開發進展（變更記錄）

| 日期 | 進展 |
|------|------|
| **2026-05-15** | **首包**：託管實體世界與基礎元件；Runtime 參考；CMake `DEPENDS`；**VGManagedHostTest**；**EntityWorldTests**。 |
| **2026-05-15** | **補強**：程式集內公開 API 與 **EntityWorldTests** 補充詳細中文註解；本 MODULE 增「完成度與進度總覽」表；總覽 **MANAGED_RUNTIME** **§2.5 / §2.5.1** 同步。 |
| **2026-05-19** | **slice 2**：**EntityWorld.HasComponent** / **GetComponent**；**EntityWorld** 類 **remarks**（與 Native **NNEntityHandle** 無自動映射）；**EntityWorldTests** 擴充；總覽 **§2.5.2**、**§2.7.1** 交叉。 |
| **2026-05-20** | **slice 3**：**EntityWorld.GetComponentCount**；**Spawn**／**Destroy**／**IsAlive** 等行內中文註解補強；**EntityWorldTests** 計數與偽造世代；總覽 **§2.5.3**。 |
| **2026-05-21** | **slice 4**：**HasComponent(EntityHandle, Type)**、**TryGetComponent(EntityHandle, Type, out)**；**ThrowIfInvalidComponentLookupType**；**EntityWorldTests** 參數校驗與與泛型引用一致；總覽 **§2.5.4**。 |
| **2026-05-15** | **slice 5**：**GetComponent(EntityHandle, Type)**、**RemoveComponent(EntityHandle, Type)**；**EntityWorldTests** 對稱閉環；總覽 **§2.5.5**。 |
| **2026-05-15** | **§2.7.1 首包（跨棧）**：Native **`NNEntityAPI`** 子表骨架（**`getServiceAbiToken`**）與本模組邊界文檔更新；**EntityWorld** 仍純 C#。 |
| **2026-05-15** | **§2.7.1 Kernel 首包（layout v5）**：**`getRuntimeTick`**、**EntitySubsystem**、**`BuildRuntime`** **`entity.*`** 轉發；本 MODULE 更新殘留項與 **EntityWorld** 資料策略（並存、非自動同步）。 |
| **2026-05-15** | **P0 總覽對齊**：**EntityWorld** 類 **remarks** 明示 **NNEntityAPI**／**getRuntimeTick** 與託管資料無鏡像承諾；與 MANAGED **§2.7.1**／**§2.5** 同步。 |
| **2026-05-15** | **§0 雙世界**：新增 **§1.1** 表；與 MANAGED **§0.3** **P0-2** 對齊。 |

## 5. 未完成與後續

- 完整 **VGEntitySystem** 實作、**EntityWorld** 與 Native **資料鏡像**之顯式橋接（**不**在本模組單方承諾自動灌入）；**`NNEntityAPI`** Kernel 首包（**layout v5**）見 MANAGED **§2.7.1**。
- Archetype 批次分配、SoA **ComponentStorage**、多執行緒查表與查詢 API。
- 獨立 **VisionGal.Managed.Component**（序列化／反射元資料）與本模組拆分評審。

## 6. 近期規劃（對齊總覽 §2.7）

| 優先級 | 方向 |
|--------|------|
| **P0 下一跳** | **VGEntitySystem** 本體、**EntityWorld**／Native **顯式橋接**與產品級 Facade（**§2.7.1** Kernel 首包已 **layout v5**）；**總路線**見 MANAGED **§0.3** **P0-2**。 |
| **P1** | **VGSceneRuntime** 與託管 **VisionGal.Managed.Scene.Runtime**（**§0.3** **P0-3**）。 |
| **長期** | 與 **Graph.Runtime**、Gameplay 序列步驟之資料邊對齊（不本模組單方承諾時程）。 |
