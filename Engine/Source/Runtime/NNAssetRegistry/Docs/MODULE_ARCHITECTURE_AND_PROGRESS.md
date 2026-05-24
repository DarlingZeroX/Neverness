# NNAssetRegistry 模組架構與進度

## 概述

Runtime 資產註冊表。提供 GUID ↔ 虛擬路徑雙向映射和依賴關係圖（前向/反向/環檢測）。

**模組類型：** STATIC
**CMake 目標：** `NevernessRuntime-AssetRegistry`
**依賴：** `NevernessRuntime-NativeEngineAPI`

## 目錄結構

```
NNAssetRegistry/
├── CMakeLists.txt
├── Include/
│   ├── NNAssetRegistry.h      # 註冊表主介面
│   ├── NNGuidTable.h          # GUID ↔ Path 雙向索引
│   └── NNDependencyTable.h    # 依賴關係圖
├── Private/
│   ├── NNAssetRegistry.cpp    # 註冊表實作
│   ├── NNGuidTable.cpp        # GUID 表實作
│   ├── NNDependencyTable.cpp  # 依賴圖實作（含 DFS 環檢測）
│   └── NNAssetRegistryApi.cpp # C ABI 橋接（增強版）
└── Docs/
    └── MODULE_ARCHITECTURE_AND_PROGRESS.md
```

## 核心設計

### NNGuidTable
- `unordered_map` 雙向索引
- GUID.low → path, path → GUID
- 註冊時自動清理舊映射

### NNDependencyTable
- 前向依賴：asset → [dep1, dep2, ...]
- 反向依賴：dep → [asset1, asset2, ...]
- DFS + 三色標記環檢測
- SetDependencies / AddDependency / RemoveDependency

### NNAssetRegistry
- Thread-safe（內部互斥）
- 整合 NNGuidTable + NNDependencyTable
- 合成 GUID 使用 'NNAS' 魔數前綴

## 與舊 AssetRegistrySubsystem 的關係

| 功能 | 舊 AssetRegistrySubsystem | 新 NNAssetRegistry |
|------|--------------------------|-------------------|
| GUID ↔ Path | ✅ | ✅ |
| 前向依賴查詢 | ✅ 基本 | ✅ 完整 |
| 反向依賴查詢 | ❌ | ✅ |
| 環檢測 | ❌ | ✅ |
| setDependencies | ❌ | ✅ |
| addDependency | ❌ | ✅ |
| removeDependency | ❌ | ✅ |
| 邊數統計 | ❌ | ✅ |

新模組與舊 `AssetRegistrySubsystem` 並存。舊模組由 `NNEngineRuntime` 使用，
新模組透過獨立 C ABI 函數表暴露（`NNBuildAssetRegistryEnhancedRuntimeApi`）。

## 狀態

**Phase 1：完成** ✅

- [x] 目錄結構建立
- [x] CMakeLists.txt
- [x] NNAssetRegistry.h/.cpp — 註冊表核心
- [x] NNGuidTable.h/.cpp — GUID ↔ Path 雙向索引
- [x] NNDependencyTable.h/.cpp — 依賴圖 + 環檢測
- [x] NNAssetRegistryApi.cpp — C ABI 橋接（增強版，8 個新函數）

**後續 Phase：**
- Phase 2：序列化（Library/Cache/Dependency.cache）
- Phase 4：與 EditorAssetDatabase 整合
- Phase 4：髒傳播演算法

## 介面

### C ABI
- `NNAssetRegistryAPI` — 增強版（原有 8 函數 + 新增 8 函數）
- 透過 `NNBuildAssetRegistryEnhancedRuntimeApi()` 接線

### C++
- `NNAssetRegistry::Instance()` — 單例存取
