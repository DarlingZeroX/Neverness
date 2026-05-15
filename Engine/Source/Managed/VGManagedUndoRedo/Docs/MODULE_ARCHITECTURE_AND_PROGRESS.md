# VGManagedUndoRedo — 撤銷/重做（VisionGal.Managed.UndoRedo）

## 1. 定位

| 項目 | 說明 |
|------|------|
| **職責** | **IUndoableCommand**、**UndoStack** 雙棧、**PropertyChangeCommand** 屬性變更撤銷。 |
| **程式集** | **VisionGal.Managed.UndoRedo**（`net10.0`） |
| **依賴** | **VisionGal.Managed.Editor**（經 Inspector 引用鏈接 Reflection） |

## 2. Phase 5 進展

| 日期 | 進展 |
|------|------|
| **2026-05-15** | 初始模組：Undo/Redo 棧與屬性變更命令。 |
| **2026-05-15** | **加固**：首包切片；與 Editor Phase 7 聯動規劃。 |
