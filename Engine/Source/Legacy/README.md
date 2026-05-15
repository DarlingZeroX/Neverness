# Legacy — Galgame 產品線（凍結）

**狀態：Deprecated / Frozen / Compatibility Only（Phase 5）**

本目錄包含已凍結之 Galgame 執行時與編輯器模組（Lua / Dialogue / Sequence / Story 等）。主線 **Managed Engine Foundation** 不再擴展此處程式碼。

## 建置

根 CMake 選項 **`VISIONGAL_BUILD_LEGACY_GALGAME`**（預設 **OFF**）。設為 **ON** 後才會編譯本目錄下目標。

## 目錄

| 路徑 | 說明 |
|------|------|
| `RuntimeGalgame/` | Galgame 執行時 DLL 與契約 |
| `Editor/VGEditorGalgame*` | Galgame 編輯器模組 |
| `Tests/` | Legacy 單元測試 |
