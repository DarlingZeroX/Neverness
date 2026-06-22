# Component API 简化计划

## 结论

```text
✅ 修改 NewScript.cs 模板 —— 展示 ref 正确用法
✅ 修改 EntityBehaviour 文档注释 —— 引导用户用 ref
✅ 保留现有 API 不变

❌ 不增加 ModifyComponent<T>()
❌ 不增加 ActionRef<T>
❌ 不增加 ComponentRef<T>
❌ 不增加 GetRef<T>()
```

## 为什么不增加新 API

`ref T GetComponent<T>()` 已经是 ECS 最优写法：

| 框架 | 写法 |
|------|------|
| Arch | `ref var t = ref world.Get<Transform>(entity);` |
| Flecs | `auto& t = entity.get_mut<Transform>();` |
| EnTT | `auto& t = registry.get<Transform>();` |
| Unity ECS | `ref var t = ref SystemAPI.GetComponentRW<LocalTransform>(entity);` |
| **Neverness** | `ref var t = ref GetComponent<TransformComponent>();` |

全部是：**拿 ref → 直接改**。不需要 `Modify()` / `With()` / `Apply()` 等包装层。

新增任何包装（`ModifyComponent`、`ComponentRef`、`GetRef`）都是降级——多了 Delegate/Lambda/额外类型，没有获得新能力。

## 两次查表是假问题

```csharp
if (HasComponent<T>())        // 查表 1
    ref var c = ref GetComponent<T>();  // 查表 2
```

实际场景中：
- ECS 组件查询是 O(1) 哈希查找，和脚本逻辑相比可忽略
- 用户不会每帧检查——核心组件（Transform）在 OnCreate 验证一次，之后直接用

## 改动清单

### 1. 更新 `NewScript.cs` 脚本模板

**当前（错误示范）**：
```csharp
TransformComponent? transform = GetComponent<TransformComponent>();
if (transform.HasValue)
{
    TransformComponent com = transform.Value;  // 值拷贝
    com.Position.X = 10;
    SetComponent(com);                         // 手动回写
}
```

**改为（正确示范）**：
```csharp
// 直接通过属性访问（EntityBehaviour.Transform 是 ref 返回）
Transform.Position.X += Speed * deltaTime;

// 或者显式获取其他组件的 ref
ref var camera = ref GetComponent<CameraComponent>();
camera.FieldOfView = 60f;
```

### 2. 更新 `EntityBehaviour.cs` 文档注释

在 `GetComponent<T>()` 的 `<remarks>` 中明确说明：

```csharp
/// <summary>
/// 获取组件引用（直接修改生效，无需 SetComponent）。
/// </summary>
/// <remarks>
/// ⚠️ 返回的是 ECS 组件的 ref 引用，修改直接反映到 ECS。
/// 组件不存在时抛异常——对核心组件可在 OnCreate 中验证，
/// 对可选组件请先使用 HasComponent 检查。
///
/// 正确用法：
/// <code>
/// ref var transform = ref GetComponent&lt;TransformComponent&gt;();
/// transform.Position.X += Speed * deltaTime;
/// </code>
///
/// 错误用法（值拷贝，修改不会生效）：
/// <code>
/// TransformComponent transform = GetComponent&lt;TransformComponent&gt;();  // ❌ 值拷贝
/// transform.Position.X = 10;  // 修改的是拷贝，不影响 ECS
/// </code>
/// </remarks>
```

### 3. 更新 `Entity.cs` 文档注释

同步更新 Entity 门面层的 `GetComponent<T>()` 文档。

### 4. 检查并清理模板生成器

确认 `ScriptBehaviourFactoryGenerator.cs` 等 Source Generator 不会生成 `TransformComponent?` 风格的代码。

## 涉及文件

| 文件 | 改动 |
|------|------|
| `Project/示例项目/Assets/NewScript.cs` | 重写 OnUpdate，展示 ref 用法 |
| `Neverness.Gameplay/Entity/EntityBehaviour.cs` | 更新 GetComponent/SetComponent 文档注释 |
| `Neverness.Gameplay/Entity/Entity.cs` | 更新 GetComponent 文档注释 |
| `Neverness.Gameplay/Entity/EntityExtensions.cs` | 更新 GetOrAddComponent 文档注释 |
| 脚本模板文件（如有） | 确认模板不生成 Nullable 风格代码 |
