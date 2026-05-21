# NNRuntimeVFS

## 职责

- 虚拟文件系统实现（`VirtualFileSystem`、`VFSService`）
- **NNVfsAPI** Runtime 导出（`ABI/VfsRuntimeApi.cpp` → `NNBuildVfsRuntimeApi`）
- Phase 1：`readText` / `writeText` / `readBytes` / `getRelativePath` / `getAbsolutePath` / `rebuildNativeFileSystemFiles` + `freeBuffer`

## 不负责

- ABI 契约定义（见 **NNNativeEngineAPI** / `VfsAPI.h`）
- Mount 路径配置（由 **EditorInitializer** / 宿主在 C++ 侧完成）

## 依赖

- `NevernessRuntime-NativeEngineAPI`（契约头）
- `NevernessCore-Core`、`NevernessCore-FileSystem`

## 关键文件

| 文件 | 说明 |
|------|------|
| `Include/VFSService.h` | C++ 服务入口 |
| `ABI/VfsRuntimeApi.cpp` | C ABI 跳板 + 内存分配 |
| `Include/VfsApiExport.h` | `NNBuildVfsRuntimeApi` |

## 进度

- [x] Phase 1：VFS → C# 函数表桥接（layout v10 `NNNativeEngineAPI.vfs`）
- [ ] Phase 2+：Mount API、异步 IO（不在 Phase 1 范围）
