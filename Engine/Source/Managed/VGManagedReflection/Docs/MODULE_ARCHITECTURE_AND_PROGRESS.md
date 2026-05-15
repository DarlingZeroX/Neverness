# VGManagedReflection — 託管反射元資料（VisionGal.Managed.Reflection）

## 1. 定位

| 項目 | 說明 |
|------|------|
| **職責** | Inspector / 序列化共用之屬性標記（SerializeField、HideInInspector、Range）、**TypeMetadata** / **PropertyMetadata**、**ReflectionRegistry** 快取。 |
| **程式集** | **VisionGal.Managed.Reflection**（`net10.0`） |
| **依賴** | **VisionGal.Managed.Object** |

## 2. 掃描規則（Phase 5 加固）

屬性納入序列化掃描當且僅當：

- 標記 **`[SerializeField]`**，或
- 具 **public setter** 之可讀寫屬性。

（已修正運算子優先級導致 public setter 誤判之問題。）

## 3. Phase 5 進展

| 日期 | 進展 |
|------|------|
| **2026-05-15** | 初始模組：屬性標記、型別/屬性元資料、註冊表快取。 |
| **2026-05-15** | **加固**：`TypeMetadata.ScanMembers` 邏輯修正；單元測試覆蓋 **SceneEntity.DisplayName** 與 fixture 欄位。 |
