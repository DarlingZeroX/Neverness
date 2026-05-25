 NNSpriteRendererComponent — 工业级 2D 渲染组件设计

 Context

 Neverness Engine 的 NNRuntimeScene 已完成 ECS（entt）+ Reflection Registry + 序列化 + C
 API + C# 双端架构。
 现有组件：Transform、Relationship、Tag、Camera。

 Legacy 引擎（NNEngineLegacy）已有 SpriteRendererComponent，但它是 legacy 设计——包含
 Ref<Sprite>、Ref<Material> 等非 POD 成员，不可 memcpy，不可跨 ABI，不可反射。

 需要在新 ECS 中设计一个工业级的 NNSpriteRendererComponent：
 - 纯 POD 数据，零虚函数，零智能指针，零 std::string
 - Renderer 可直接消费的 Render Submission 数据
 - GPU Instancing 友好的数据布局
 - 完整集成：Reflection 注册、序列化、C# Interop、Inspector
 - 为未来 Animated Sprite、Tilemap、Multi-Material、GPU Driven 预留架构空间

 ---
 一、组件数据结构设计

 1.1 NNSpriteRendererComponent (POD, 56 bytes)

 新建文件:
 Engine/Source/Runtime/NNRuntimeScene/Include/Components/NNSpriteRendererComponent.h

 /**
  * @file NNSpriteRendererComponent.h
  * @brief 精灵渲染组件（POD，无虚表）：Render Data Component。
  *
  * 这不是"贴图+颜色"的简单组件，而是面向 Renderer 的渲染提交数据描述。
  * 设计参考：Unity SpriteRenderer、Godot CanvasItem、UE Paper2D SpriteComponent。
  *
  * 约束：
  * - trivially_copyable + standard_layout（可直接 memcpy）
  * - 无 std::string / shared_ptr / 虚函数
  * - 资源引用使用 uint64_t Asset Handle（非直接对象引用）
  * - 字段布局对 GPU Instancing 友好
  * - 所有字段可通过 NNComponentRegistry 反射自动编辑
  */

 #include <cstdint>
 #include "../../NNRuntimeSceneExport.h"

 namespace NN::Runtime::Scene
 {
     /**
      * @brief 精灵混合模式枚举。
      * 控制源像素与目标像素的混合方式。
      */
     enum class NNBlendMode : std::uint32_t
     {
         Alpha    = 0,   ///< 标准 Alpha 混合 (SrcAlpha, OneMinusSrcAlpha)
         Additive = 1,   ///< 加法混合 (SrcAlpha, One)
         Multiply = 2,   ///< 正片叠底
         Opaque   = 3,   ///< 不透明（关闭混合）
         Premultiplied = 4, ///< 预乘 Alpha
     };

     /**
      * @brief 精灵渲染标志位（可组合位掩码）。
      */
     enum class NNSpriteFlags : std::uint32_t
     {
         None          = 0,
         Visible       = 1u << 0,  ///< 是否可见（false 时不提交渲染）
         FlipX         = 1u << 1,  ///< 水平翻转
         FlipY         = 1u << 2,  ///< 垂直翻转
         CastShadow    = 1u << 3,  ///< 投射阴影（2D Light 预留）
         ReceiveShadow = 1u << 4,  ///< 接收阴影（预留）
         Instanced     = 1u << 5,  ///< 允许 GPU Instancing
         CustomShader  = 1u << 6,  ///< 使用自定义 Shader（MaterialHandle 有效）
         Flag7         = 1u << 7,  ///< 用户自定义
         Flag8         = 1u << 8,  ///< 用户自定义
         Flag9         = 1u << 9,  ///< 用户自定义
         Flag10        = 1u << 10, ///< 用户自定义
         Flag11        = 1u << 11, ///< 用户自定义
         Flag12        = 1u << 12, ///< 用户自定义
         Flag13        = 1u << 13, ///< 用户自定义
         Flag14        = 1u << 14, ///< 用户自定义
         Flag15        = 1u << 15, ///< 用户自定义
     };

     inline NNSpriteFlags operator|(NNSpriteFlags a, NNSpriteFlags b) noexcept
     {
         return static_cast<NNSpriteFlags>(
             static_cast<std::uint32_t>(a) | static_cast<std::uint32_t>(b));
     }
     inline NNSpriteFlags operator&(NNSpriteFlags a, NNSpriteFlags b) noexcept
     {
         return static_cast<NNSpriteFlags>(
             static_cast<std::uint32_t>(a) & static_cast<std::uint32_t>(b));
     }
     inline bool HasFlag(NNSpriteFlags flags, NNSpriteFlags test) noexcept
     {
         return (static_cast<std::uint32_t>(flags) & static_cast<std::uint32_t>(test)) !=
 0;
     }

     /**
      * @brief 精灵渲染组件：纯数据描述，行为由 SpriteRenderSystem 驱动。
      *
      * 内存布局（56 字节）：
      * ┌──────────────────────────────────────────────────────┐
      * │ TextureAsset    uint64_t   8B   纹理资源句柄          │
      * │ MaterialAsset   uint64_t   8B   材质资源句柄          │
      * │ Color           float[4]  16B  RGBA tint 颜色        │
      * │ UvRect          float[4]  16B  UV 区域 [u0,v0,u1,v1]│
      * │ Layer           uint32_t   4B  渲染层级              │
      * │ SortOrder       uint32_t   4B  层级内排序            │
      * │ BlendMode       uint32_t   4B  混合模式枚举          │
      * │ Flags           uint32_t   4B  标志位掩码            │
      * └──────────────────────────────────────────────────────┘
      * 对齐：8 字节，共 56 字节（14 × 4B）
      *
      * GPU Instancing：相邻实例 stride = 56B，无填充浪费。
      * SIMD 友好：Color 和 UvRect 各 16B，对齐到 16 字节边界。
      */
     struct NN_RUNTIME_SCENE_API NNSpriteRendererComponent
     {
         // ── 资源引用（Asset Handle，非直接对象引用）──
         // 值为 0 表示未设置 / 使用默认资源。
         // 序列化时作为 UInt64 持久化；Renderer 通过 Asset System 解析为 GPU 资源。
         std::uint64_t TextureAsset  = 0u;   ///< 纹理资源句柄（FNV-1a(virtualPath) 或 GUID
  low）
         std::uint64_t MaterialAsset = 0u;   ///< 材质资源句柄（0 = 使用引擎默认
 SpriteShader）

         // ── 颜色与 UV ──
         float Color[4] = {1.0f, 1.0f, 1.0f, 1.0f};  ///< Tint 颜色 RGBA
 [0,1]，与纹理颜色相乘
         float UvRect[4] = {0.0f, 0.0f, 1.0f, 1.0f};  ///< UV 区域 [u0, v0, u1, v1]，支持
 Atlas

         // ── 渲染排序与状态 ──
         std::uint32_t Layer     = 0u;   ///< 渲染层级（类似 Unity Sorting Layer 的 ID）
         std::uint32_t SortOrder = 0u;   ///< 同 Layer 内的排序（值大的后渲染，覆盖值小的）
         NNBlendMode   Blend     = NNBlendMode::Alpha; ///< 混合模式
         NNSpriteFlags Flags     = NNSpriteFlags::Visible; ///< 标志位（默认可见）

         NNSpriteRendererComponent() = default;
     };

     // ── 编译期验证 ──
     static_assert(std::is_trivially_copyable_v<NNSpriteRendererComponent>,
         "NNSpriteRendererComponent must be trivially copyable (memcpy-safe)");
     static_assert(std::is_standard_layout_v<NNSpriteRendererComponent>,
         "NNSpriteRendererComponent must have standard layout");
     static_assert(sizeof(NNSpriteRendererComponent) == 56,
         "NNSpriteRendererComponent should be 56 bytes (14 x uint32)");
 } // namespace NN::Runtime::Scene

 1.2 关键设计决策

 ┌─────────────────────────────────┬───────────────────────────────────────────────────┐
 │              决策               │                       理由                        │
 ├─────────────────────────────────┼───────────────────────────────────────────────────┤
 │ uint64_t Asset Handle 而非      │ POD 要求；ABI 稳定；跨 DLL/C# 安全；可            │
 │ std::string                     │ hash/vpath/GUID                                   │
 ├─────────────────────────────────┼───────────────────────────────────────────────────┤
 │ float Color[4] 而非自定义 Color │ 与 NNComponentFieldType::Float4                   │
 │  类型                           │ 兼容，序列化零成本                                │
 ├─────────────────────────────────┼───────────────────────────────────────────────────┤
 │ float UvRect[4] 而非自定义 Rect │ 同上；[u0,v0,u1,v1] 支持 Atlas 区域               │
 │  类型                           │                                                   │
 ├─────────────────────────────────┼───────────────────────────────────────────────────┤
 │ NNBlendMode 为 enum class       │ 与 NNComponentFieldType::UInt32 兼容              │
 │ uint32_t                        │                                                   │
 ├─────────────────────────────────┼───────────────────────────────────────────────────┤
 │ NNSpriteFlags 为 enum class     │ 单字段编码多布尔状态；扩展性好（16 位可用）       │
 │ uint32_t 位掩码                 │                                                   │
 ├─────────────────────────────────┼───────────────────────────────────────────────────┤
 │ MaterialHandle 默认 0 = 内置    │ 组件不依赖 Renderer 模块，零耦合                  │
 │ Shader                          │                                                   │
 ├─────────────────────────────────┼───────────────────────────────────────────────────┤
 │ 56 字节而非 64 字节             │ 避免 cache line 浪费；56 = 7 × 8B，GPU instancing │
 │                                 │  stride 合理                                      │
 └─────────────────────────────────┴───────────────────────────────────────────────────┘

 1.3 资源引用策略

 Editor 阶段:
   用户选择 "assets/sprites/player.png"
   → Editor 序列化为虚拟路径 hash: FNV-1a("assets/sprites/player.png")
   → 写入 NNSpriteRendererComponent.TextureAsset (uint64_t)
   → JSON 序列化时写入 "TextureAsset": "assets/sprites/player.png"（路径字符串）

 Runtime 阶段:
   AssetSystem 根据 TextureAsset hash 查找已加载的 GPU 纹理
   → 返回内部 NNTextureHandle（也是 uint64_t，但指向 GPU 资源）
   → SpriteRenderSystem 使用此 handle 提交绘制

 GUID 方案（可选）:
   若启用 .nnasset GUID 系统:
   → TextureAsset 存储 NNGuid.low (uint64_t)
   → AssetRegistry 根据 GUID 解析到 GPU 资源

 ---
 二、Dirty Flag 设计

 组件修改后需通知 Renderer、Bounds 等系统做增量更新。

 2.1 NNSpriteDirtyFlags

 /**
  * @brief 精灵渲染组件脏标记（增量更新用）。
  * SpriteRenderSystem 内部追踪上帧状态，比较本帧状态以确定需要更新的部分。
  */
 enum class NNSpriteDirtyFlags : std::uint32_t
 {
     None         = 0,
     Texture      = 1u << 0,   ///< 纹理变更 → 需重新绑定 SRV / 重建 batch key
     Material     = 1u << 1,   ///< 材质/Shader 变更 → 需重新绑定 Pipeline State
     Color        = 1u << 2,   ///< 颜色变更 → 常量缓冲区更新
     UvRect       = 1u << 3,   ///< UV 区域变更 → 顶点数据 / 常量缓冲区更新
     SortOrder    = 1u << 4,   ///< 排序变更 → 需重新排序 Render Queue
     Visibility   = 1u << 5,   ///< 可见性变更 → 加入/移出 Render Queue
     BlendMode    = 1u << 6,   ///< 混合模式变更 → 需切换 Pipeline State
     All          = 0xFFFFFFFF,
 };

 2.2 集成方式

 NNDirtyTracker 已有 MarkComponentDirty(entity, typeId) 粒度。
 Sprite 特定脏标记由 SpriteRenderSystem 内部管理：

 // SpriteRenderSystem 内部（不在组件内）：
 struct SpriteRenderState
 {
     std::uint64_t LastTexture = 0;
     std::uint64_t LastMaterial = 0;
     float         LastColor[4] = {};
     float         LastUvRect[4] = {};
     NNBlendMode   LastBlend = NNBlendMode::Alpha;
     NNSpriteFlags LastFlags = NNSpriteFlags::None;
     std::uint32_t LastSortOrder = 0;
 };

 // 每帧比较当前组件 vs LastState → 生成 dirty flags
 // dirty flags 影响：
 //   - Texture/Material 变更 → 重建 batch key
 //   - SortOrder 变更 → 重新排序
 //   - Visibility 变更 → 加入/移出 draw list

 ---
 三、组件注册（全链路）

 3.1 BuiltinComponentRegistration.cpp — 静态注册

 修改文件:
 Engine/Source/Runtime/NNRuntimeScene/Source/Reflection/BuiltinComponentRegistration.cpp

 #include "Components/NNSpriteRendererComponent.h"

 NN_REGISTER_COMPONENT(
     NN::Runtime::Scene::NNSpriteRendererComponent,
     "SpriteRenderer",
     NN_FIELD(NN::Runtime::Scene::NNSpriteRendererComponent, TextureAsset, UInt64),
     NN_FIELD(NN::Runtime::Scene::NNSpriteRendererComponent, MaterialAsset, UInt64),
     NN_FIELD(NN::Runtime::Scene::NNSpriteRendererComponent, Color, Float4),
     NN_FIELD(NN::Runtime::Scene::NNSpriteRendererComponent, UvRect, Float4),
     NN_FIELD(NN::Runtime::Scene::NNSpriteRendererComponent, Layer, UInt32),
     NN_FIELD(NN::Runtime::Scene::NNSpriteRendererComponent, SortOrder, UInt32),
     NN_FIELD(NN::Runtime::Scene::NNSpriteRendererComponent, Blend, UInt32),
     NN_FIELD(NN::Runtime::Scene::NNSpriteRendererComponent, Flags, UInt32));

 3.2 NNRuntimeScene — 绑定 Runtime 函数指针

 修改文件: Engine/Source/Runtime/NNRuntimeScene\Source\Scene\NNRuntimeScene.cpp

 在 RegisterBuiltinComponents() 中添加：
 BindComponentType<NNSpriteRendererComponent>("SpriteRenderer");

 修改文件: Engine/Source/Runtime/NNRuntimeScene/Include/Scene/NNRuntimeScene.h

 添加 include：
 #include "../Components/NNSpriteRendererComponent.h"

 3.3 CMakeLists.txt

 修改文件: Engine/Source/Runtime/NNRuntimeScene/CMakeLists.txt

 在 target_sources 中添加：
 Include/Components/NNSpriteRendererComponent.h

 ---
 四、ECS → RenderQueue 流程架构

 4.1 整体流程

 每帧：
   NNRuntimeScene::TickSystems(deltaTime)
     ├─ EarlyUpdate:  (预留)
     ├─ FixedUpdate:  (预留)
     ├─ Update:       NNCameraSystem, NNTransformSystem, NNHierarchySystem
     ├─ LateUpdate:   (预留)
     └─ Render:       NNSpriteRenderSystem (Phase 1: 构建 DrawList)
                           │
                           ▼
                      NNSpriteDrawList
                           │
                           ▼
                      Renderer 消费 DrawList → GPU 提交

 4.2 NNSpriteDrawItem — 渲染提交数据

 /**
  * @brief 精灵绘制项——SpriteRenderSystem 从 ECS 组件数据生成，
  * Renderer 直接消费的扁平化渲染描述。
  * 不属于 ECS 组件；每帧在栈上或帧分配器中生成。
  */
 struct NNSpriteDrawItem
 {
     std::uint64_t SortKey        = 0u;      ///< 排序键（Layer + SortOrder + Material +
 Blend）
     NNEntity      Entity         = NNEntityInvalid;
     Core::matrix  WorldMatrix{1.0f};        ///< 从 Transform.WorldMatrix 获取
     std::uint64_t TextureHandle = 0u;
     std::uint64_t MaterialHandle = 0u;
     float         Color[4]      = {1,1,1,1};
     float         UvRect[4]     = {0,0,1,1};
     NNBlendMode   Blend         = NNBlendMode::Alpha;
     NNSpriteFlags Flags         = NNSpriteFlags::None;
 };

 /**
  * @brief SortKey 编码规则（64-bit）：
  * ┌──────────┬──────────┬───────────────────┬──────────┐
  * │ Layer    │ SortOrder│ MaterialHash      │ BlendMode│
  * │ (16-bit) │ (16-bit) │ (24-bit truncated) │ (8-bit)  │
  * └──────────┴──────────┴───────────────────┴──────────┘
  *
  * SortKey 越大 → 越后渲染（后渲染的覆盖先渲染的）。
  * Alpha Blend 物体需要 back-to-front，由 Renderer 层处理。
  */
 inline std::uint64_t EncodeSpriteSortKey(
     std::uint32_t layer,
     std::uint32_t sortOrder,
     std::uint64_t materialHash,
     NNBlendMode blend)
 {
     return (static_cast<std::uint64_t>(layer) << 48)
          | (static_cast<std::uint64_t>(sortOrder) << 32)
          | ((materialHash & 0x00FFFFFF'FFFFFFFFu) >> 8)  // 取 24-bit
          | (static_cast<std::uint64_t>(blend) & 0xFF);
 }

 4.3 NNSpriteRenderSystem

 /**
  * @brief 精灵渲染 System：在 Render TickGroup 中遍历所有 SpriteRenderer 组件，
  * 生成排序后的 DrawList，供 Renderer 消费。
  */
 class NN_RUNTIME_SCENE_API NNSpriteRenderSystem final : public ISceneSystem
 {
 public:
     NNSceneTickGroup TickGroup() const noexcept override
     {
         return NNSceneTickGroup::Render;
     }

     void Tick(NNRuntimeScene& scene, float deltaTimeSeconds) noexcept override;

     /** @brief 获取上帧生成的 DrawList（Renderer 在 Render 之后读取）。 */
     [[nodiscard]] const std::vector<NNSpriteDrawItem>& GetDrawList() const noexcept
     {
         return m_DrawList;
     }

 private:
     std::vector<NNSpriteDrawItem> m_DrawList{};
 };

 实现 (NNSpriteRenderSystem.cpp)：

 void NNSpriteRenderSystem::Tick(NNRuntimeScene& scene, float /*deltaTime*/) noexcept
 {
     m_DrawList.clear();

     auto view = scene.GetRegistry().view<
         NNSpriteRendererComponent,
         NNTransformComponent>();

     for (auto enttEntity : view)
     {
         const auto& sprite = view.get<NNSpriteRendererComponent>(enttEntity);
         const auto& transform = view.get<NNTransformComponent>(enttEntity);

         // 不可见跳过
         if (!HasFlag(sprite.Flags, NNSpriteFlags::Visible))
             continue;

         NNSpriteDrawItem item{};
         item.Entity = scene.HandleFromEntt(enttEntity);
         item.WorldMatrix = transform.WorldMatrix;
         item.TextureHandle = sprite.TextureAsset;
         item.MaterialHandle = sprite.MaterialAsset;
         std::memcpy(item.Color, sprite.Color, sizeof(float) * 4);
         std::memcpy(item.UvRect, sprite.UvRect, sizeof(float) * 4);
         item.Blend = sprite.Blend;
         item.Flags = sprite.Flags;
         item.SortKey = EncodeSpriteSortKey(
             sprite.Layer, sprite.SortOrder,
             sprite.MaterialAsset, sprite.Blend);

         m_DrawList.push_back(item);
     }

     // 按 SortKey 升序排序（小的先渲染）
     std::sort(m_DrawList.begin(), m_DrawList.end(),
         [](const NNSpriteDrawItem& a, const NNSpriteDrawItem& b)
         {
             return a.SortKey < b.SortKey;
         });
 }

 4.4 与 Renderer 的接口

 NNSpriteRenderSystem 在 NNRuntimeScene 中注册。Renderer 模块（外部）通过以下方式消费数据：

 // Renderer 侧伪代码
 void OnRender(NNRuntimeScene& scene)
 {
     const auto& drawList = scene.GetSpriteRenderSystem().GetDrawList();

     for (const auto& item : drawList)
     {
         // 1. 根据 TextureHandle 从 AssetSystem 获取 GPU 纹理
         auto* texture = AssetSystem::ResolveTexture(item.TextureHandle);

         // 2. 根据 MaterialHandle 获取 Shader（0 = 内置 SpriteDefault）
         auto* shader = item.MaterialHandle == 0
             ? ShaderManager::GetBuiltIn("SpriteDefault")
             : AssetSystem::ResolveShader(item.MaterialHandle);

         // 3. 设置渲染状态
         VGFX::UseProgram(shader);
         VGFX::SetTexture(0, texture);
         VGFX::SetUniformMatrix4("u_Model", item.WorldMatrix);
         VGFX::SetUniformFloat4("u_Color", item.Color);
         VGFX::SetUniformFloat4("u_UvRect", item.UvRect);
         VGFX::SetBlendMode(static_cast<VGFX::BlendMode>(item.Blend));

         // 4. 绘制
         VGFX::RenderMesh(quadMesh);
     }
 }

 ---
 五、序列化设计

 5.1 Binary 序列化（Runtime）

 完全由 NNComponentRegistry 驱动，零硬编码。字段映射：

 ┌───────────────┬───────────┬─────────────┐
 │     字段      │ FieldType │ 二进制格式  │
 ├───────────────┼───────────┼─────────────┤
 │ TextureAsset  │ UInt64    │ 8 字节 LE   │
 ├───────────────┼───────────┼─────────────┤
 │ MaterialAsset │ UInt64    │ 8 字节 LE   │
 ├───────────────┼───────────┼─────────────┤
 │ Color         │ Float4    │ 16 字节 raw │
 ├───────────────┼───────────┼─────────────┤
 │ UvRect        │ Float4    │ 16 字节 raw │
 ├───────────────┼───────────┼─────────────┤
 │ Layer         │ UInt32    │ 4 字节 LE   │
 ├───────────────┼───────────┼─────────────┤
 │ SortOrder     │ UInt32    │ 4 字节 LE   │
 ├───────────────┼───────────┼─────────────┤
 │ Blend         │ UInt32    │ 4 字节 LE   │
 ├───────────────┼───────────┼─────────────┤
 │ Flags         │ UInt32    │ 4 字节 LE   │
 └───────────────┴───────────┴─────────────┘

 总计：60 字节（含前面组件头 4 字节 blobSize → 实际组件数据 56 字节）。

 5.2 JSON 序列化（Editor）

 {
   "SpriteRenderer": {
     "TextureAsset": "assets/sprites/player.png",
     "MaterialAsset": 0,
     "Color": [1.0, 1.0, 1.0, 1.0],
     "UvRect": [0.0, 0.0, 1.0, 1.0],
     "Layer": 0,
     "SortOrder": 0,
     "Blend": 0,
     "Flags": 1
   }
 }

 注意：当前 NNJsonSceneSerializer 中 UInt64 字段直接输出数字。
 若需在 JSON 中使用路径字符串而非数字，需要在 NNComponentFieldType 中新增 AssetHandle
 类型，
 或通过 Editor 层单独处理 Asset 引用。此为后续优化，当前 Phase 使用 UInt64 数值。

 5.3 版本兼容性

 当前版本 = 1（kFormatVersion）。未来新增字段时：
 - Binary：新增字段追加到结构体末尾；旧文件缺少的字段使用默认值
 - JSON：缺失字段使用默认值；未知字段忽略

 ---
 六、C# Interop 层

 6.1 NNSpriteRendererComponentData — C# 镜像结构体

 修改文件: Engine/Source/Managed/Runtime/Neverness.Runtime.Engine/NNNativeEngineApiTypes.cs

 /// <summary>
 /// 精灵渲染组件——与 Native NNSpriteRendererComponent 内存布局严格对齐（56 字节）。
 /// </summary>
 [StructLayout(LayoutKind.Sequential)]
 [ComponentId(/* fnv1a_64("SpriteRenderer") */, Name = "SpriteRenderer")]
 public struct NNSpriteRendererComponentData
 {
     public ulong TextureAsset;          // 8B offset 0
     public ulong MaterialAsset;         // 8B offset 8
     public float ColorR, ColorG, ColorB, ColorA;  // 16B offset 16
     public float UvU0, UvV0, UvU1, UvV1;          // 16B offset 32
     public uint Layer;                  // 4B offset 48
     public uint SortOrder;              // 4B offset 52
     public uint BlendMode;              // 4B offset 56
     public uint Flags;                  // 4B offset 60
 }

 TypeId 计算：fnv1a_64("SpriteRenderer") 需要与 C++ 端一致。
 可在 C++ 端计算后填入 C#，或使用 ComponentIdAttribute 的 FNV-1a 自动解析。

 6.2 NNSpriteBlendMode / NNSpriteFlags — C# 枚举

 修改文件: Engine/Source/Managed/Runtime/Neverness.Runtime.Engine/NNNativeEngineApiTypes.cs

 public enum NNBlendMode : uint
 {
     Alpha = 0,
     Additive = 1,
     Multiply = 2,
     Opaque = 3,
     Premultiplied = 4,
 }

 [Flags]
 public enum NNSpriteFlags : uint
 {
     None = 0,
     Visible = 1 << 0,
     FlipX = 1 << 1,
     FlipY = 1 << 2,
     CastShadow = 1 << 3,
     ReceiveShadow = 1 << 4,
     Instanced = 1 << 5,
     CustomShader = 1 << 6,
 }

 6.3 SpriteRendererInspector — Editor Inspector

 新建文件: Engine/Source/Managed/Editor/Neverness.Editor.Scene/Private/Inspector/SpriteRend
 ererInspector.cs

 using System.Numerics;
 using Hexa.NET.ImGui;
 using Neverness.Runtime.Engine;

 namespace Neverness.Editor.Scene.Private.Inspector;

 /// <summary>
 /// 精灵渲染组件 Inspector——绘制纹理、颜色、UV、混合模式、排序等字段。
 /// </summary>
 [InspectorOrder(50)]  // 排在 Transform(0) 和 Camera(100) 之间
 public sealed class SpriteRendererInspector
     : ComponentTypeInspector<NNSpriteRendererComponentData>
 {
     public override int Order => 50;

     protected override bool DrawFields(ref NNSpriteRendererComponentData data)
     {
         bool modified = false;

         // ── Texture Asset ──
         ImGui.Text("Texture");
         ImGui.SameLine(100f);
         // TODO: Asset Picker（后续 Phase 实现）
         ulong textureHash = data.TextureAsset;
         ImGui.Text($"0x{textureHash:X16}");

         // ── Material Asset ──
         ImGui.Text("Material");
         ImGui.SameLine(100f);
         ulong materialHash = data.MaterialAsset;
         ImGui.Text($"0x{materialHash:X16}");

         // ── Color (RGBA) ──
         var color = new Vector4(data.ColorR, data.ColorG, data.ColorB, data.ColorA);
         ImGui.Text("Color");
         ImGui.SameLine(100f);
         if (ImGui.ColorEdit4("##Color", ref color, ImGuiColorEditFlags.AlphaBar))
         {
             data.ColorR = color.X; data.ColorG = color.Y;
             data.ColorB = color.Z; data.ColorA = color.W;
             modified = true;
         }

         // ── UV Rect ──
         var uv = new Vector4(data.UvU0, data.UvV0, data.UvU1, data.UvV1);
         ImGui.Text("UV Rect");
         ImGui.SameLine(100f);
         ImGui.PushItemWidth(60f);
         modified |= ImGui.DragFloat("##u0", ref uv.X, 0.01f); ImGui.SameLine();
         modified |= ImGui.DragFloat("##v0", ref uv.Y, 0.01f); ImGui.SameLine();
         modified |= ImGui.DragFloat("##u1", ref uv.Z, 0.01f); ImGui.SameLine();
         modified |= ImGui.DragFloat("##v1", ref uv.W, 0.01f);
         ImGui.PopItemWidth();
         if (modified)
         {
             data.UvU0 = uv.X; data.UvV0 = uv.Y;
             data.UvU1 = uv.Z; data.UvV1 = uv.W;
         }

         // ── Layer ──
         int layer = (int)data.Layer;
         ImGui.Text("Layer");
         ImGui.SameLine(100f);
         if (ImGui.InputInt("##Layer", ref layer))
         {
             data.Layer = (uint)Math.Max(0, layer);
             modified = true;
         }

         // ── Sort Order ──
         int sortOrder = (int)data.SortOrder;
         ImGui.Text("Sort Order");
         ImGui.SameLine(100f);
         if (ImGui.InputInt("##SortOrder", ref sortOrder))
         {
             data.SortOrder = (uint)Math.Max(0, sortOrder);
             modified = true;
         }

         // ── Blend Mode ──
         int blend = (int)data.BlendMode;
         string[] blendNames = ["Alpha", "Additive", "Multiply", "Opaque",
 "Premultiplied"];
         ImGui.Text("Blend");
         ImGui.SameLine(100f);
         if (ImGui.Combo("##Blend", ref blend, blendNames, blendNames.Length))
         {
             data.BlendMode = (uint)blend;
             modified = true;
         }

         // ── Flags ──
         modified |= DrawSpriteFlags(ref data);

         return modified;
     }

     private static bool DrawSpriteFlags(ref NNSpriteRendererComponentData data)
     {
         bool modified = false;
         uint flags = data.Flags;

         bool visible = (flags & (uint)NNSpriteFlags.Visible) != 0;
         if (ImGui.Checkbox("Visible", ref visible))
         {
             flags = visible ? flags | (uint)NNSpriteFlags.Visible
                             : flags & ~(uint)NNSpriteFlags.Visible;
             modified = true;
         }

         bool flipX = (flags & (uint)NNSpriteFlags.FlipX) != 0;
         ImGui.SameLine();
         if (ImGui.Checkbox("Flip X", ref flipX))
         {
             flags = flipX ? flags | (uint)NNSpriteFlags.FlipX
                           : flags & ~(uint)NNSpriteFlags.FlipX;
             modified = true;
         }

         bool flipY = (flags & (uint)NNSpriteFlags.FlipY) != 0;
         ImGui.SameLine();
         if (ImGui.Checkbox("Flip Y", ref flipY))
         {
             flags = flipY ? flags | (uint)NNSpriteFlags.FlipY
                           : flags & ~(uint)NNSpriteFlags.FlipY;
             modified = true;
         }

         if (modified) data.Flags = flags;
         return modified;
     }
 }

 ---
 七、文件变更清单

 ┌────────────────────────────────────────────────┬─────┬──────────────────────────────┐
 │                      文件                      │ 操  │             说明             │
 │                                                │ 作  │                              │
 ├────────────────────────────────────────────────┼─────┼──────────────────────────────┤
 │ NNRuntimeScene/Include/Components/NNSpriteRend │ 新  │ POD 组件结构体 + 枚举 +      │
 │ ererComponent.h                                │ 建  │ static_assert                │
 ├────────────────────────────────────────────────┼─────┼──────────────────────────────┤
 │ NNRuntimeScene/Source/Reflection/BuiltinCompon │ 修  │ 新增 NN_REGISTER_COMPONENT(" │
 │ entRegistration.cpp                            │ 改  │ SpriteRenderer")             │
 ├────────────────────────────────────────────────┼─────┼──────────────────────────────┤
 │ NNRuntimeScene/Include/Scene/NNRuntimeScene.h  │ 修  │ 添加 #include                │
 │                                                │ 改  │ NNSpriteRendererComponent.h  │
 ├────────────────────────────────────────────────┼─────┼──────────────────────────────┤
 │ NNRuntimeScene/Source/Scene/NNRuntimeScene.cpp │ 修  │ BindComponentType("SpriteRen │
 │                                                │ 改  │ derer")                      │
 ├────────────────────────────────────────────────┼─────┼──────────────────────────────┤
 │                                                │ 修  │ 添加                         │
 │ NNRuntimeScene/CMakeLists.txt                  │ 改  │ NNSpriteRendererComponent.h  │
 │                                                │     │ 到 target_sources            │
 ├────────────────────────────────────────────────┼─────┼──────────────────────────────┤
 │                                                │ 修  │ 新增 NNSpriteRendererCompone │
 │ Managed/.../NNNativeEngineApiTypes.cs          │ 改  │ ntData + NNBlendMode +       │
 │                                                │     │ NNSpriteFlags                │
 ├────────────────────────────────────────────────┼─────┼──────────────────────────────┤
 │ Managed/.../Inspector/SpriteRendererInspector. │ 新  │ Inspector UI（ColorEdit4,    │
 │ cs                                             │ 建  │ Combo, Checkbox 等）         │
 └────────────────────────────────────────────────┴─────┴──────────────────────────────┘

 ---
 八、未来扩展预留

 当前架构为以下功能预留了空间：

 ┌───────────────────┬─────────────────────────────────────────────────────────────────┐
 │     未来功能      │                            预留设计                             │
 ├───────────────────┼─────────────────────────────────────────────────────────────────┤
 │ Animated Sprite   │ Flags::Flag7 可标记 animated；新增 NNAnimatorComponent 通过 ECS │
 │                   │  查询联动修改 SpriteRenderer 的 UvRect                          │
 ├───────────────────┼─────────────────────────────────────────────────────────────────┤
 │ 2D Skeletal       │ 独立 NNSkeletonComponent + NNSkinComponent；SpriteRenderer      │
 │ Animation         │ 不变，骨骼系统写入 Transform                                    │
 ├───────────────────┼─────────────────────────────────────────────────────────────────┤
 │ Tilemap           │ 新增 NNTilemapComponent（网格数据 + TileSet 引用）；共用        │
 │                   │ NNSpriteDrawItem 结构                                           │
 ├───────────────────┼─────────────────────────────────────────────────────────────────┤
 │ GPU Driven        │ Flags::Instanced 已预留；DrawItem 可转换为 indirect draw args   │
 │ Rendering         │                                                                 │
 ├───────────────────┼─────────────────────────────────────────────────────────────────┤
 │ Sprite Atlas      │ UvRect 字段直接支持 atlas 区域；Atlas 元数据由 Asset System     │
 │                   │ 管理                                                            │
 ├───────────────────┼─────────────────────────────────────────────────────────────────┤
 │ Multi-Material    │ 将 SpriteRenderer 拆分为 SpriteRendererComponent +              │
 │                   │ MaterialOverrideComponent                                       │
 ├───────────────────┼─────────────────────────────────────────────────────────────────┤
 │ Light2D           │ Flags::CastShadow + Flags::ReceiveShadow 预留；新增             │
 │                   │ NNLight2DComponent                                              │
 ├───────────────────┼─────────────────────────────────────────────────────────────────┤
 │ Shadow2D          │ 同上                                                            │
 ├───────────────────┼─────────────────────────────────────────────────────────────────┤
 │ Sorting Layers    │ Layer 字段 + 全局 Layer Table（Editor 侧管理 Layer 名称映射）   │
 └───────────────────┴─────────────────────────────────────────────────────────────────┘

 ---
 九、验证方案

 1. 编译验证：C++ cmake build，0 错误，0 警告
 2. POD 验证：static_assert(is_trivially_copyable_v<NNSpriteRendererComponent>) 通过
 3. ABI 验证：static_assert(sizeof == 56) 通过
 4. Reflection 注册验证：通过 SceneDebug.DumpTypeInfo() 确认 "SpriteRenderer"
 出现在类型快照中
 5. 序列化 Round-trip：
   - 创建实体 → AddComponent → 设置 TextureAsset, Color
   - Serialize → Deserialize → 验证字段值一致
 6. Inspector 验证：
   - C# Editor 打开含 SpriteRenderer 的实体 → Inspector 显示颜色选择器、UV 编辑、Blend 下拉
   - 修改颜色 → 验证 Native 端组件数据更新
 7. Render 验证（后续 Phase）：
   - 创建实体 + SpriteRenderer + Texture → 验证 NNSpriteRenderSystem 生成 DrawItem
   - DrawItem 的 SortKey、WorldMatrix、Color 等字段正确

 ---
 十、实施阶段划分

 Phase 1（本次实施）：组件定义 + 注册 + C# 镜像

 - 新建 NNSpriteRendererComponent.h（POD 结构体 + 枚举）
 - 修改 BuiltinComponentRegistration.cpp（静态注册）
 - 修改 NNRuntimeScene.cpp（BindComponentType）
 - 修改 CMakeLists.txt
 - 新增 C# 结构体 + 枚举
 - 新增 SpriteRendererInspector.cs

 Phase 2（后续）：SpriteRenderSystem

 - 新建 NNSpriteRenderSystem.h/.cpp
 - 新建 NNSpriteDrawItem.h
 - 注册到 NNRuntimeScene::RegisterDefaultSystems()

 Phase 3（后续）：Renderer 集成

 - Legacy Renderer 消费 NNSpriteDrawList
 - Asset Handle → GPU 纹理解析
 - Batch / Instancing 实现

 Phase 4（后续）：Editor Asset Picker

 - Texture/Material 字段使用 Asset Picker 而非 hex 数值
 - JSON 序列化支持路径字符串
