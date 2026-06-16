# VFS 新增 C# 可用 API：AddFileSystem / RemoveFileSystem / HasFileSystem / UnregisterAlias

## Context

当前 VFSService 只暴露了 `MountFileSystem(alias, IFileSystemPtr)` 这一个 C++ 内部方法给挂载用，C# 侧无法从托管代码创建/管理文件系统。需要将底层 VirtualFileSystem 的文件系统管理操作通过 C ABI 函数表暴露给 C#。

**核心难题**：`IFileSystemPtr` 是 C++ `shared_ptr<IFileSystem>`，不能直接跨 ABI。方案是用 **handle（uint64）** 作不透明句柄，Native 侧维护 handle→IFileSystemPtr 的映射表。

---

## 修改文件清单（7 个文件）

### 0. 枚举定义（C++ / C# 各一份）

```cpp
// C++ — 放在 VfsAPI.h
enum NNVfsFileSystemType : uint32_t {
    NNVfsFS_Native  = 0,  // NativeFileSystem（磁盘目录）
    NNVfsFS_Zip     = 1,  // ZipFileSystem（.zip/.pak）
    NNVfsFS_Memory  = 2,  // MemoryFileSystem（内存）
};
```

```csharp
// C# — 放在 NNNativeEngineApiTypes.cs
public enum NNVfsFileSystemType : uint
{
    Native = 0,
    Zip    = 1,
    Memory = 2,
}
```

### 1. `Engine/Source/Runtime/NNRuntimeVFS/Include/VFSService.h`
在 `VFSService` struct 中追加 5 个静态方法：

```cpp
/** 创建并挂载文件系统，返回不透明 handle（0 失败）。
 *  type: 0=Native, 1=Zip, 2=Memory。
 *  path: Native/Zip 时为磁盘路径；Memory 时忽略。
 */
static uint64_t AddFileSystem(const std::string& alias, uint32_t type, const std::string& path);

/** 根据 handle 精确移除指定文件系统。 */
static bool RemoveFileSystem(uint64_t handle);

/** 查询 handle 对应的文件系统是否仍挂载在 VFS 中。 */
static bool HasFileSystem(uint64_t handle);

/** 移除 alias 下全部文件系统。 */
static void UnregisterAlias(const std::string& alias);

/** 查询 alias 是否有任何文件系统已注册。 */
static bool IsAliasRegistered(const std::string& alias);
```

### 2. `Engine/Source/Runtime/NNRuntimeVFS/Source/VFSService.cpp`
实现上述 5 个方法：

- 内部维护 `static std::unordered_map<uint64_t, std::pair<std::string, IFileSystemPtr>>` handle 注册表 + `static uint64_t s_NextHandle = 1` 自增 ID。
- `AddFileSystem`：根据 type 创建对应 FS → `fs->Initialize()` → `GetInstance()->AddFileSystem(alias, fs)` → 存入注册表 → 返回 handle。
- `RemoveFileSystem(handle)`：从注册表查找 → `GetInstance()->RemoveFileSystem(alias, fs)` → 从注册表删除。
- `HasFileSystem(handle)`：从注册表查找 → `GetInstance()->HasFileSystem(alias, fs)`。
- `UnregisterAlias`：`GetInstance()->UnregisterAlias(alias)` → 删除注册表中该 alias 的所有条目。
- `IsAliasRegistered`：`GetInstance()->IsAliasRegistered(alias)`。

### 3. `Engine/Source/Runtime/NNNativeEngineAPI/Include/VfsAPI.h`
在 `NNVfsAPI` 结构体 **尾部追加** 5 个函数指针（兼容现有布局，不破坏 ABI）：

```cpp
typedef uint64_t(NN_ENGINE_ABI_STDCALL* NNVfsAddFileSystemFn)(const char* aliasUtf8, NNVfsFileSystemType type, const char* pathUtf8);
typedef int(NN_ENGINE_ABI_STDCALL* NNVfsRemoveFileSystemFn)(uint64_t handle);
typedef int(NN_ENGINE_ABI_STDCALL* NNVfsHasFileSystemFn)(uint64_t handle);
typedef void(NN_ENGINE_ABI_STDCALL* NNVfsUnregisterAliasFn)(const char* aliasUtf8);
typedef int(NN_ENGINE_ABI_STDCALL* NNVfsIsAliasRegisteredFn)(const char* aliasUtf8);
```

`NNVfsAPI` 结构体尾部追加对应 5 个字段。

### 4. `Engine/Source/Runtime/NNRuntimeVFS/ABI/VfsRuntimeApi.cpp`
实现 5 个 C ABI 函数，转发到 `VFSService` 的对应方法。在 `NNBuildVfsRuntimeApi` 中填充新字段。

### 5. `Engine/Source/Managed/Runtime/Neverness.Runtime.Engine/NNNativeEngineApiTypes.cs`
在 `NNVfsApi` 结构体 **尾部追加** 5 个 `delegate* unmanaged` 字段，与 C++ `NNVfsAPI` 逐字节对齐。同文件添加 `NNVfsFileSystemType` 枚举。

### 6. `Engine/Source/Managed/Runtime/Neverness.Runtime.VFS/Private/VFSHost.cs`
追加 5 个内部方法：`AddFileSystem`、`RemoveFileSystem`、`HasFileSystem`、`UnregisterAlias`、`IsAliasRegistered`。路径字符串编组复用现有 UTF-8 模式。

### 7. `Engine/Source/Managed/Runtime/Neverness.Runtime.VFS/Public/VFS.cs`
追加 5 个公共 API，委托给 VFSHost。

---

## C# 公共 API 最终签名

```csharp
public static class VFS
{
    // 新增 ↓

    /// <summary>创建并挂载文件系统，返回 handle（0 失败）。</summary>
    /// <param name="alias">VFS 别名（如 "/"）。</param>
    /// <param name="type">文件系统类型。</param>
    /// <param name="path">Native/Zip 时为磁盘路径；Memory 时传 null 或空串。</param>
    public static ulong AddFileSystem(string alias, NNVfsFileSystemType type, string? path);

    /// <summary>根据 handle 精确移除文件系统。</summary>
    public static bool RemoveFileSystem(ulong handle);

    /// <summary>查询 handle 对应的文件系统是否仍在 VFS 中。</summary>
    public static bool HasFileSystem(ulong handle);

    /// <summary>移除 alias 下全部文件系统。</summary>
    public static void UnregisterAlias(string alias);

    /// <summary>查询 alias 是否有任何文件系统已注册。</summary>
    public static bool IsAliasRegistered(string alias);
}
```

---

## ABI 安全性

- **只追加不删除**：新函数指针追加到 `NNVfsAPI` 末尾，`size` 字段仍为旧值，旧代码不会访问新字段。
- **handle = uint64**：blittable，C#/C++ 零拷贝。
- **type = uint32**：blittable 枚举。
- **路径字符串**：UTF-8 + NUL 结尾，复用现有编组模式。
- **错误返回**：handle 类返回 0 表示失败；bool 类返回 0/1。

---

## 验证

1. 编译 NNRuntimeVFS、NNNativeEngineAPI、Neverness.Runtime.VFS、Neverness.Runtime.Engine 四个模块。
2. C# 测试：
   - `VFS.AddFileSystem("/", NNVfsFileSystemType.Native, "C:\\temp")` → `VFS.IsAliasRegistered("/")` = true
   - `VFS.AddFileSystem("/", NNVfsFileSystemType.Memory, null)` → `VFS.HasFileSystem(handle)` = true
   - `VFS.RemoveFileSystem(handle)` → `VFS.HasFileSystem(handle)` = false
   - `VFS.UnregisterAlias("/")` → `VFS.IsAliasRegistered("/")` = false
