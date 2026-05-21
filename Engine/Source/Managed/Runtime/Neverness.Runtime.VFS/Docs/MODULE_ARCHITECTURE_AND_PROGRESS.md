# Neverness.Runtime.VFS

## 职责

- 封装 **`NNVfsAPI`** 函数表（禁止 `DllImport`）
- 对外提供 **`VFS.ReadText` / `WriteText` / `ReadBytes` / `GetAbsolutePath` / `GetRelativePath` / `RebuildNativeFileSystemFiles`**
- UTF-8 路径编组；Native 缓冲区经 **`freeBuffer`** 释放

## 不负责

- VFS Mount（Native **EditorInitializer** / C++ 宿主）
- Asset / Scene 高层 IO

## 依赖

- `Neverness.Runtime.Engine`（`NNVfsApi` 镜像）
- `Neverness.Runtime.Interop`（`EngineNativeApiBootstrap`）

## 进度

- [x] Phase 1：`VFSHost` + `VFS` 门面
- [x] layout v10 与 Native `NNBuildVfsRuntimeApi` 对齐
