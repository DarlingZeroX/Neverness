# VGManagedGameplay — Galgame 執行時（VisionGal.Managed.Gameplay）

## 1. 定位

| 項目 | 說明 |
|------|------|
| **職責** | Phase 6 首包：**GameplayVariableStore**（託管變數表）、**DialoguePresenter**（經 `VGUIApi.SetDialogueText` 呈現對白）。 |
| **程式集** | **VisionGal.Managed.Gameplay**（`net10.0`，`AllowUnsafeBlocks`） |
| **依賴** | **VisionGal.Managed.Core**、**VisionGal.Managed.Engine** |
| **消費方** | **VisionGal.Managed.Runtime**（專案參考，使 `dotnet publish` 輸出含本程式集）；Foundation 單元測試。 |
| **不負責** | Sequence 編排、存檔、Legacy Galgame、Roslyn / Hot Reload |

## 2. Phase 6 進展

| 日期 | 進展 |
|------|------|
| **2026-05-15** | **首包**：變數表 API、對白 Presenter（ABI 未安裝時 no-op）；**VisionGal.Managed.Foundation.Tests** 覆蓋。 |
| **2026-05-15** | **Runtime 納入 publish**：**VisionGal.Managed.Runtime** 專案參考本模組；**VGManagedHostTest** 斷言 **VisionGal.Managed.Gameplay.dll** 存在於 publish 根目錄。 |

## 3. 後續

- **SequenceRunner** 與場景再水合聯動。
- 變數表與序列化 / Native ABI 對接。
- 與 **VGManagedHost** UCO Bootstrap 演練（可選）。
