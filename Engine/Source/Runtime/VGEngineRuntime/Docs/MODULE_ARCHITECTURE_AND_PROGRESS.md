# VGEngineRuntime — 行程級 Runtime Facade（Phase 4）

## 1. 定位

| 項目 | 說明 |
|------|------|
| **職責** | 提供 **`VGEngineRuntime`** 單例：`Initialize` / `Tick` / `Shutdown`；內建 **TimingSystem**（`std::chrono` 語意之累積時間與帧序）、**AsyncSystem**（背景 `std::thread` + 可輪詢完成）、**SceneSubsystem** / **AssetSubsystem** 空殼（未接 **VGEngine** / **VGAsset** 前回傳明確無效值）。 |
| **不負責** | 不鏈結 **VGEngine**、**VGRHI**、**VGUI**；不提供 Gameplay / 對白。 |
| **CMake 目標** | **`VGEngineRuntime`**（**`STATIC`**） |
| **依賴** | **`VGNativeEngineAPI`**（僅 **EngineHandles** 等標頭）。 |

---

## 2. 執行緒與生命週期

- **`Tick`** 須由與 **`Initialize` / `Shutdown` 相同之控制執行緒**呼叫（與未來 game loop 對齊）。
- **`Shutdown`** 會 **join** 尚未 **`releaseWait`** 之背景執行緒；請勿於 Async 回呼內呼叫 **`Shutdown`**。

---

## 3. 開發進展

| 日期 | 進展 |
|------|------|
| **2026-05-14** | Phase 4 首包：新增本靜態庫與子系統骨架；供 **VGEngineRuntimeServices** 轉發 ABI。 |
