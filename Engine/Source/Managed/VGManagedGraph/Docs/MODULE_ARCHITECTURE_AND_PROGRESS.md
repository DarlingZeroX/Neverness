# VGManagedGraph — 節點圖（VisionGal.Managed.Graph）

## 1. 定位

| 項目 | 說明 |
|------|------|
| **職責** | **Graph** / **GraphNode** / **GraphEdge** / **GraphPort**、**GraphValidator** 結構驗證。 |
| **程式集** | **VisionGal.Managed.Graph**（`net10.0`） |
| **依賴** | **VisionGal.Managed.Reflection** |
| **主線（2026）** | 當前為**資料模型**與驗證；執行時目標程式集 **VisionGal.Managed.Graph.Runtime**（GraphVM、NodeExecutor 等，**100% Managed**，見 [MANAGED 總覽 §0.3 P0-5](../../MANAGED_RUNTIME_ARCHITECTURE_AND_PROGRESS.md)）。**不**做 Native Graph VM。 |

## 2. Phase 5 進展

| 日期 | 進展 |
|------|------|
| **2026-05-15** | 初始模組：節點圖資料模型與驗證器。 |
| **2026-05-15** | **加固**：**GraphSerializer** 屬性註解；未納入 Bootstrap 路徑。 |
| **2026-05-15** | **§0 主線**：補充 **Graph.Runtime** 與 MANAGED **§0.3** **P0-5** 索引。 |
