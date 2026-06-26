# Ninja + MSVC 中文环境下增量编译失效修复

**日期**: 2026-06-26
**状态**: 已修复（临时方案），需永久修复

---

## 问题现象

修改 C++ 头文件后，Ninja 不会重新编译引用该头文件的 `.cpp` 文件，增量编译完全失效。

## 根因分析

### 核心问题：`msvc_deps_prefix` 编码不匹配

Ninja 通过解析 MSVC 的 `/showIncludes` 输出来追踪头文件依赖。MSVC 输出格式为：

```
注: 包含文件: G:\Neverness\Engine\Source\Runtime\NNRuntimeEngineServices\Include\ManagedRuntimeServices.h
```

Ninja 需要知道前缀是什么，才能提取后面的文件路径。这个前缀存储在 `rules.ninja` 中：

```ninja
msvc_deps_prefix = 注: 包含文件:
```

**问题链条**：

1. **MSVC 输出**：中文 Windows 下 `/showIncludes` 输出 **GBK 编码**的 `注: 包含文件:`
2. **CMake 写入**：CMake 将这个字符串写入 `rules.ninja`，文件是 **UTF-8 编码**
3. **编码损坏**：GBK 字节 `\xd7\xa2\xd2\xe2` 被当作 UTF-8 存储，变成乱码 `ע��: �����ļ�:`
4. **匹配失败**：Ninja 用损坏的 UTF-8 字符串去匹配 MSVC 的 GBK 输出 → 匹配失败
5. **依赖丢失**：所有目标的 `#deps` 都是 **0**，头文件依赖完全丢失

### 验证方法

```bash
# 检查 deps 数据库
cd Build/VS/x64-debug
ninja -t deps

# 如果看到 #deps 0，说明依赖追踪坏了
# 正常应该是 #deps 5, deps mtime xxx (VALID)
```

```bash
# 检查 msvc_deps_prefix 编码
python3 -c "
with open('CMakeFiles/rules.ninja', 'rb') as f:
    content = f.read()
    idx = content.find(b'msvc_deps_prefix')
    line = content[idx:idx+60]
    print('Raw bytes:', line)
    # 如果看到 \xd7\xa2 这样的 GBK 字节，说明有编码问题
"
```

## 已执行的修复（临时方案）

### 1. 修复 `rules.ninja` 中的前缀

```bash
cd Build/VS/x64-debug
python3 -c "
with open('CMakeFiles/rules.ninja', 'rb') as f:
    content = f.read()
prefix = b'msvc_deps_prefix = '
idx = content.find(prefix)
end = content.find(b'\n', idx)
new_line = prefix + b'note: including file:'
content = content[:idx] + new_line + content[end:]
with open('CMakeFiles/rules.ninja', 'wb') as f:
    f.write(content)
"
```

### 2. 清理依赖数据库

```bash
rm -f .ninja_deps .ninja_log
```

### 3. 重新完整编译

```bash
ninja -j8
```

编译完成后，依赖关系会被正确建立，以后增量编译恢复正常。

## 永久修复方案

### 方案 A：Windows 系统级 UTF-8（推荐）

1. 设置 → 时间和语言 → 区域 → 管理语言设置 → 更改系统区域设置
2. 勾选「Beta: 使用 Unicode UTF-8 提供全球语言支持」
3. 重启电脑

这样 MSVC 的 `/showIncludes` 会输出 UTF-8 英文前缀，与 Ninja 文件编码一致。

**注意**：这可能影响某些旧程序的兼容性。

### 方案 B：CMake 构建脚本自动修复

在构建脚本中添加后处理步骤，每次运行 CMake 后自动修复 `rules.ninja`：

```powershell
# build.ps1 或类似构建脚本
cmake -G Ninja -B Build/VS/x64-debug -S .
python3 fix_ninja_deps_prefix.py Build/VS/x64-debug
ninja -C Build/VS/x64-debug
```

### 方案 C：等待 CMake 修复

CMake 3.28+ 已经改进了 `msvc_deps_prefix` 的编码处理。当前项目使用的是 CMake 4.3/4.4，理论上应该已经修复。如果问题仍然存在，可能需要向 CMake 报告 bug。

## 影响范围

- **所有**使用 Ninja + MSVC + 中文 Windows 的编译都会受影响
- 不仅是 NNRuntimeEngineServices，所有模块的头文件依赖都是 `#deps 0`
- 重新完整编译一次后，依赖关系会被正确建立

## 复发条件

每次运行 `cmake -G Ninja` 重新生成构建文件时，`rules.ninja` 会被重新生成，`msvc_deps_prefix` 又会变回 GBK 编码的中文，问题会复发。

**解决方案**：在 CMake 重新生成后，重新执行修复脚本。

---

## 相关链接

- [CMake Issue Tracker](https://gitlab.kitware.com/cmake/cmake/-/issues) - 搜索 `msvc_deps_prefix`
- [Ninja Issue Tracker](https://github.com/ninja-build/ninja/issues) - 搜索 CJK encoding
- [CMake 源码](https://github.com/Kitware/CMake) - `Modules/Platform/Windows-MSVC.cmake`
- [Ninja 源码](https://github.com/ninja-build/ninja) - `src/msvc_helper-win32.cc`
