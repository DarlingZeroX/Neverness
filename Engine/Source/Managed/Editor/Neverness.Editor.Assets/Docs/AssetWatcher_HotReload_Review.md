# 资产变更监视 & 热重载系统 — 审查报告

> 审查日期: 2026-06-25
> 审查范围: `AssetWatcher` / `HotReloadCoordinator` / `ImportPipeline` / `ImportStateCache` / `DependencyGraph`
> 状态: **已修复**（Phase 1-4 全部完成）

---

## 一、现有系统总览

资产变更监视和热重载**已经实现**，由 5 个组件协作：

```
┌──────────────┐  事件入队   ┌───────────────────────┐  ProcessEvent  ┌───────────────┐
│ AssetWatcher │────────────→│ HotReloadCoordinator  │──────────────→│ ImportPipeline│
│ (FSWatcher)  │  200ms 防抖 │ (事件队列 + Tick)      │                │ (8 阶段导入)   │
│ + SHA256 过滤 │             └───────────┬───────────┘                └───────┬───────┘
└──────────────┘                         │ MarkForReload                      │
                                         ▼                                    ▼
                              ┌──────────────────┐               ┌────────────────────┐
                              │ AssetHandle       │               │ EditorAssetDatabase │
                              │ .ReloadMarked()   │               │ + DependencyGraph   │
                              └──────────────────┘               └────────────────────┘
                                         │
                                         ▼ EditorEventBus
                              ┌──────────────────┐
                              │ AssetReloaded     │
                              │ → ContentBrowser  │
                              │ → Inspector       │
                              └──────────────────┘
```

**数据流:**
1. `AssetWatcher` 监视 `Assets/` 目录，FileSystemWatcher + 200ms 防抖 + SHA256 内容哈希过滤
2. 事件入队到 `HotReloadCoordinator._pendingEvents`（Timer 线程安全）
3. `Tick()` 每帧从主线程出队，调用 `ProcessEvent` 分发处理
4. `ImportPipeline.Reimport()` 重新导入 + `DependencyGraph` 脏传播
5. 批量收集 GUID，`FlushRuntimeReloads()` 调用 `AssetHandle.MarkForReload()` + `ReloadMarkedAssets()`
6. 通过 `EditorEventBus` 发送 `AssetReloaded` 事件通知 UI 层

**现有能力:**
- [x] 文件变更检测（SHA256 内容哈希，避免重复触发）
- [x] 文件创建/删除/重命名事件
- [x] 200ms 防抖合并
- [x] 增量导入（`ImportStateCache` 跳过未变化文件）
- [x] 依赖图脏传播（A 变化 → 所有引用 A 的资产 reimport）
- [x] Runtime 热重载通知（每帧 Tick 批量 MarkForReload）
- [x] 增量缓存持久化（`ImportStateCache` / `DependencyGraph` 都可序列化）
- [x] 主线程事件处理（事件队列 + Tick 模式）
- [x] 删除时通知 Runtime 卸载（`AssetHandle.Unload`，幂等安全）
- [x] 重命名时移动 .meta + 清理旧缓存
- [x] FileSystemWatcher 错误自动恢复（条件定期扫描）
- [x] UI 自动刷新（`AssetReloaded` 事件 → ContentBrowser）
- [x] ForceFullScan 区分 Created / Changed 事件类型

---

## 二、发现的问题及修复状态

### P0 — 严重问题

- [x] **1. `FlushRuntimeReloads` 从不被调用** — 改为从 `Tick()` 每帧调用
- [x] **2. `ImportStateCache` 实例共享关系** — 构造函数注释文档化
- [x] **3. 事件从 Timer 线程触发** — 事件队列 + Tick 主线程处理模式

### P1 — 功能缺陷

- [ ] **4. `.meta` 文件变更不触发 reimport** — 决策：不监视，Inspector UI 直接触发
- [x] **5. 删除事件不清理 Runtime 引用** — `AssetHandle.Unload(guid)` 幂等安全
- [x] **6. 重命名处理产生孤立缓存** — `StateCache.Remove(oldPath)`
- [x] **7. 重命名处理不移动 `.meta` 文件** — `MetaFileManager.MoveMeta(oldPath, newPath)`
- [x] **8. `ForceFullScan` 触发错误事件类型** — `GetCachedHash` 区分 Created/Changed

### P2 — 健壮性 / 性能

- [x] **9. 缺少错误恢复机制** — 条件定期扫描（`_errorCount > 0` 时启动，恢复后停止）
- [x] **10. 批量导入合并** — 事件队列天然实现，Tick 统一处理
- [ ] **11. `ImportPipeline` 全局锁粒度过粗** — 未改动，单资产锁粒度足够
- [x] **12. 热重载事件通知 UI** — `EditorEventType.AssetReloaded` + ContentBrowser 订阅

---

## 三、已确认的决策

| 决策项 | 选择 | 理由 |
|--------|------|------|
| 事件调度方式 | **Tick 主线程轮询** | 延迟最多一帧（~16ms），实现简单可控 |
| .meta 监视 | **不监视** | Inspector UI 修改时 MetaFileManager 直接触发 reimport |
| 实施范围 | **Phase 1-4 全部** | 从 P0 修复到 UI 集成完整实施 |
| 批量合并 | **事件队列天然实现** | Tick 一次性处理所有积累事件，复用 ProcessEvent |
| 错误恢复 | **条件定期扫描** | 只在 `_errorCount > 0` 时启动 Timer，恢复后停止 |

---

## 四、实际修改的文件

| 文件 | 改动 |
|------|------|
| `Watch/HotReloadCoordinator.cs` | 全面重写：事件队列 + Tick + ProcessEvent + 脏传播 + EventEmit |
| `Watch/AssetWatcher.cs` | 错误恢复 Timer + 条件定期扫描 + ForceFullScan 事件类型修复 + **拆分重构** |
| `Watch/AssetWatcherFilter.cs` | **新增** — 文件/目录过滤规则（.meta、临时文件、Editor 管理资产） |
| `Watch/ImportStateCache.cs` | **新增** — SHA256 内容哈希缓存，从 AssetWatcher.cs 拆出 |
| `AssetsModuleImp.cs` | Tick 调用 coordinator + EventBus 注入 |
| `ContentBrowserService.cs` | 订阅 AssetReloaded 自动刷新 |
| `IEditorEventBus.cs` | 新增 `AssetReloaded` 枚举值 |

**文件拆分 + 目录重组:** 原始 `Database/AssetWatcher.cs`（552 行）→ `Database/Watch/` 目录下 3 文件：
- `Watch/AssetWatcher.cs`（~280 行）— FSWatcher 生命周期、防抖队列、增量扫描、错误恢复
- `Watch/AssetWatcherFilter.cs`（~50 行）— 过滤规则
- `Watch/ImportStateCache.cs`（~150 行）— 内容哈希缓存
- `Watch/HotReloadCoordinator.cs`（~260 行）— 热重载协调器

**新增类型:**
- `AssetChangeType` 枚举（Changed/Created/Deleted/Renamed）
- `AssetChangeEvent` record struct（事件数据）
- `AssetReloadedEventPayload` record struct（事件载荷）

---

## 五、未完成项（其他模块）

- [ ] **Inspector 通知**（`Neverness.Editor.Inspector`）— 已打开的资产 Inspector 在依赖变化时提示刷新
- [ ] **热重载状态栏**（Frontend 模块）— 显示正在热重载的资产数量和进度

事件基础设施（`AssetReloaded` + `AssetReloadedEventPayload`）已就绪，这些模块只需订阅 `EditorEventType.AssetReloaded` 即可。
