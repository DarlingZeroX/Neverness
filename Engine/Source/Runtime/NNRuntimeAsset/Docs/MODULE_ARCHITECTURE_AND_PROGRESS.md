# NNRuntimeAsset 模組架構與進度

## 概述

Runtime 資產系統核心模組。提供 .nnasset 格式定義、資產管理器、Handle 系統、快取、串流管理。

**模組類型：** STATIC
**CMake 目標：** `NevernessRuntime-Asset`
**依賴：** `NevernessRuntime-NativeEngineAPI`

## 目錄結構

```
NNRuntimeAsset/
├── CMakeLists.txt
├── Include/
│   ├── NNAssetManager.h       # 資產管理器（核心）
│   ├── NNAssetHandle.h        # 類型化 Handle + HandleTable
│   ├── NNAssetFormat.h        # .nnasset 二進位格式定義
│   ├── NNAssetTypes.h         # 型別註冊系統
│   ├── NNAssetCache.h         # LRU 快取
│   ├── NNAssetRef.h           # RAII 引用計數智慧指標
│   └── NNStreamingManager.h   # 異步串流管理器
├── Private/
│   ├── NNAssetManager.cpp
│   ├── NNHandleTable.cpp
│   ├── NNAssetCache.cpp
│   ├── NNStreamingManager.cpp
│   ├── NNAssetFormat.cpp      # 檔案讀寫
│   └── NNAssetManagerApi.cpp  # C ABI 橋接
└── Docs/
    └── MODULE_ARCHITECTURE_AND_PROGRESS.md
```

## 核心設計

### NNAssetHandleT<T>
- 模板類型化 Handle（8 位元組）
- uint64 編碼：低 32 位 = 索引，高 32 位 = generation
- HandleTable 支援 free list 復用 + generation 防 ABA

### NNAssetManager
- 單例模式
- GUID → Handle 映射
- 同步/異步載入
- .nnasset 檔案解析
- 引用計數管理
- 包掛載（預留）
- Hot Reload 支援

### NNAssetCache
- LRU 驅逐策略
- 可設定記憶體預算
- 釘選機制（不被驅逐）

### NNStreamingManager
- IO 執行緒池 + 解碼執行緒池
- 優先級佇列
- 框架已建立，完整實作在 Phase 5

### .nnasset 格式
- Header 固定 64 位元組
- 依賴 GUID 表
- Blob 描述符表
- 對齊填充
- 連續 payload

## 狀態

**Phase 1：完成** ✅

- [x] 目錄結構建立
- [x] CMakeLists.txt
- [x] NNAssetFormat.h/.cpp — .nnasset 格式定義 + 檔案讀寫
- [x] NNHandleTable — Handle 分配/釋放/查詢
- [x] NNAssetHandle.h — 類型化 Handle 模板
- [x] NNAssetTypes.h — 型別註冊系統
- [x] NNAssetManager.h/.cpp — 資產管理器核心
- [x] NNAssetCache.h/.cpp — LRU 快取
- [x] NNStreamingManager.h/.cpp — 串流管理器框架
- [x] NNAssetRef.h — RAII 引用計數
- [x] NNAssetManagerApi.cpp — C ABI 橋接

**後續 Phase：**
- Phase 5：完善異步 IO（Overlapped IO / IOCP）
- Phase 5：完善解碼管線
- Phase 6：包管理完整實作
- Phase 7：Hot Reload 整合

## 介面

### C ABI
- `NNAssetManagerAPI` — 在 `AssetManagerAPI.h` 中定義
- 透過 `NNBuildAssetManagerRuntimeApi()` 接線

### C++
- `NNAssetManager::Instance()` — 單例存取
- `NNAssetHandleT<T>` — 類型化 Handle
- `NNAssetRef<T>` — RAII 引用計數
