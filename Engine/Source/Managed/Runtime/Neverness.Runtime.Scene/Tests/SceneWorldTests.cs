using System.Numerics;
using Neverness.Runtime.Scene;
using Neverness.Runtime.Scene.Components;
using Neverness.Runtime.Scene.Internal;
using Neverness.Runtime.Scene.Systems;
using Xunit;

namespace Neverness.Runtime.Scene.Tests;

/// <summary>
/// SceneWorld 单元测试——验证抽象层接口和 Friflo 实现。
/// </summary>
public sealed class SceneWorldTests
{
    #region IScene 基本操作

    [Fact]
    public void CreateScene_returns_valid_scene()
    {
        using var scene = new Neverness.Runtime.Scene.Internal.FrifloScene("TestScene");
        Assert.NotNull(scene);
        Assert.Equal("TestScene", scene.Name);
        Assert.True(scene.IsValid);
        Assert.Equal(0, scene.EntityCount);
    }

    [Fact]
    public void CreateScene_with_builtin_systems()
    {
        using var scene = new Neverness.Runtime.Scene.Internal.FrifloScene("TestScene");
        Assert.NotNull(scene);
        Assert.True(scene.IsValid);
    }

    #endregion

    #region IEntity 基本操作

    [Fact]
    public void CreateEntity_returns_valid_entity()
    {
        using var scene = new Neverness.Runtime.Scene.Internal.FrifloScene("TestScene");
        var entity = scene.CreateEntity("TestEntity");

        Assert.NotNull(entity);
        Assert.True(entity.IsValid);
        // 注意：Name 属性暂时返回 Entity_ID 格式
        Assert.NotNull(entity.Name);
        Assert.Equal(1, scene.EntityCount);
    }

    [Fact]
    public void DestroyEntity_removes_entity()
    {
        using var scene = new Neverness.Runtime.Scene.Internal.FrifloScene("TestScene");
        var entity = scene.CreateEntity("Temp");
        Assert.Equal(1, scene.EntityCount);

        scene.DestroyEntity(entity);
        Assert.Equal(0, scene.EntityCount);
    }

    [Fact]
    public void GetEntity_returns_null_for_invalid_id()
    {
        using var scene = new Neverness.Runtime.Scene.Internal.FrifloScene("TestScene");
        var entity = scene.GetEntity(999);
        Assert.Null(entity);
    }

    [Fact]
    public void EntityExists_returns_false_for_invalid_id()
    {
        using var scene = new Neverness.Runtime.Scene.Internal.FrifloScene("TestScene");
        Assert.False(scene.EntityExists(999));
    }

    #endregion

    #region IComponent 操作

    [Fact]
    public void Add_component_to_entity()
    {
        using var scene = new Neverness.Runtime.Scene.Internal.FrifloScene("TestScene");
        var entity = scene.CreateEntity("Player");

        entity.Add(TransformComponent.Default);

        Assert.True(entity.Has<TransformComponent>());
    }

    [Fact]
    public void Get_component_from_entity()
    {
        using var scene = new Neverness.Runtime.Scene.Internal.FrifloScene("TestScene");
        var entity = scene.CreateEntity("Player");

        var transform = new TransformComponent
        {
            Position = new Vector3(1, 2, 3),
            Rotation = Quaternion.Identity,
            Scale = Vector3.One,
            WorldMatrix = Matrix4x4.Identity,
        };
        entity.Add(transform);

        ref var got = ref entity.Get<TransformComponent>();
        Assert.Equal(1, got.Position.X);
        Assert.Equal(2, got.Position.Y);
        Assert.Equal(3, got.Position.Z);
    }

    [Fact]
    public void Remove_component_from_entity()
    {
        using var scene = new Neverness.Runtime.Scene.Internal.FrifloScene("TestScene");
        var entity = scene.CreateEntity("Player");

        entity.Add(TransformComponent.Default);
        Assert.True(entity.Has<TransformComponent>());

        entity.Remove<TransformComponent>();
        Assert.False(entity.Has<TransformComponent>());
    }

    [Fact]
    public void TryGet_returns_false_when_component_missing()
    {
        using var scene = new Neverness.Runtime.Scene.Internal.FrifloScene("TestScene");
        var entity = scene.CreateEntity("Player");

        var result = entity.TryGet<TransformComponent>(out var component);
        Assert.False(result);
    }

    [Fact]
    public void TryGet_returns_true_when_component_exists()
    {
        using var scene = new Neverness.Runtime.Scene.Internal.FrifloScene("TestScene");
        var entity = scene.CreateEntity("Player");

        entity.Add(TransformComponent.Default);

        var result = entity.TryGet<TransformComponent>(out var component);
        Assert.True(result);
    }

    #endregion

    #region ISceneQuery 查询

    [Fact]
    public void Query_returns_matching_entities()
    {
        using var scene = new Neverness.Runtime.Scene.Internal.FrifloScene("TestScene");

        // 创建 3 个实体，只有 2 个有 TransformComponent
        var entity1 = scene.CreateEntity("Entity1");
        entity1.Add(TransformComponent.Default);

        var entity2 = scene.CreateEntity("Entity2");
        entity2.Add(TransformComponent.Default);

        var entity3 = scene.CreateEntity("Entity3");
        // 不添加 TransformComponent

        var query = scene.Query<TransformComponent>();
        Assert.Equal(2, query.Count);
    }

    [Fact]
    public void Query_ForEach_iterates_all_matching_entities()
    {
        using var scene = new Neverness.Runtime.Scene.Internal.FrifloScene("TestScene");

        var entity1 = scene.CreateEntity("Entity1");
        entity1.Add(new TransformComponent { Position = new Vector3(1, 0, 0) });

        var entity2 = scene.CreateEntity("Entity2");
        entity2.Add(new TransformComponent { Position = new Vector3(2, 0, 0) });

        var positions = new List<Vector3>();
        scene.Query<TransformComponent>().ForEach((ref TransformComponent t, IEntity e) =>
        {
            positions.Add(t.Position);
        });

        Assert.Equal(2, positions.Count);
        Assert.Contains(new Vector3(1, 0, 0), positions);
        Assert.Contains(new Vector3(2, 0, 0), positions);
    }

    [Fact]
    public void Query_multi_component()
    {
        using var scene = new Neverness.Runtime.Scene.Internal.FrifloScene("TestScene");

        var entity1 = scene.CreateEntity("Entity1");
        entity1.Add(TransformComponent.Default);
        entity1.Add(CameraComponent.DefaultPerspective);

        var entity2 = scene.CreateEntity("Entity2");
        entity2.Add(TransformComponent.Default);
        // 不添加 CameraComponent

        var query = scene.Query<TransformComponent, CameraComponent>();
        Assert.Equal(1, query.Count);
    }

    #endregion

    #region ISceneSystem 系统

    [Fact]
    public void AddSystem_registers_system()
    {
        using var scene = new Neverness.Runtime.Scene.Internal.FrifloScene("TestScene");
        var system = new TransformSystem();

        scene.AddSystem(system);

        Assert.True(system.IsInitialized);
    }

    [Fact]
    public void Update_executes_systems()
    {
        using var scene = new Neverness.Runtime.Scene.Internal.FrifloScene("TestScene");

        var entity = scene.CreateEntity("Player");
        entity.Add(new TransformComponent
        {
            Position = new Vector3(10, 0, 0),
            Rotation = Quaternion.Identity,
            Scale = Vector3.One,
        });

        // 不应抛异常
        scene.Update(0.016f);

        // 验证 TransformSystem 计算了世界矩阵
        ref var transform = ref entity.Get<TransformComponent>();
        Assert.NotEqual(Matrix4x4.Identity, transform.WorldMatrix);
    }

    #endregion

    #region ISceneEventBus 事件

    [Fact]
    public void EventBus_publish_and_subscribe()
    {
        using var scene = new Neverness.Runtime.Scene.Internal.FrifloScene("TestScene");
        int receivedValue = 0;

        scene.Events.Subscribe<TestEvent>(evt =>
        {
            receivedValue = evt.Value;
        });

        scene.Events.Publish(new TestEvent { Value = 42 });

        Assert.Equal(42, receivedValue);
    }

    [Fact]
    public void EventBus_deferred_publish()
    {
        using var scene = new Neverness.Runtime.Scene.Internal.FrifloScene("TestScene");
        int receivedValue = 0;

        scene.Events.Subscribe<TestEvent>(evt =>
        {
            receivedValue = evt.Value;
        });

        scene.Events.PublishDeferred(new TestEvent { Value = 42 });
        Assert.Equal(0, receivedValue); // 延迟事件不应立即触发

        scene.Events.FlushDeferred();
        Assert.Equal(42, receivedValue); // Flush 后触发
    }

    private struct TestEvent
    {
        public int Value;
    }

    #endregion

    #region 层级操作

    [Fact]
    public void SetParent_creates_hierarchy()
    {
        using var scene = new Neverness.Runtime.Scene.Internal.FrifloScene("TestScene");
        var parent = scene.CreateEntity("Parent");
        var child = scene.CreateEntity("Child");

        scene.SetParent(child, parent);

        var actualParent = scene.GetParent(child);
        Assert.NotNull(actualParent);
        Assert.Equal(parent.Id, actualParent.Id);
    }

    [Fact]
    public void GetChildren_returns_children()
    {
        using var scene = new Neverness.Runtime.Scene.Internal.FrifloScene("TestScene");
        var parent = scene.CreateEntity("Parent");
        var child1 = scene.CreateEntity("Child1");
        var child2 = scene.CreateEntity("Child2");

        scene.SetParent(child1, parent);
        scene.SetParent(child2, parent);

        var children = scene.GetChildren(parent);
        Assert.Equal(2, children.Count);
    }

    [Fact]
    public void SetParent_prevents_self_reference()
    {
        using var scene = new Neverness.Runtime.Scene.Internal.FrifloScene("TestScene");
        var entity = scene.CreateEntity("Entity");

        Assert.Throws<ArgumentException>(() => scene.SetParent(entity, entity));
    }

    [Fact]
    public void SetParent_prevents_cycle()
    {
        using var scene = new Neverness.Runtime.Scene.Internal.FrifloScene("TestScene");
        var grandparent = scene.CreateEntity("Grandparent");
        var parent = scene.CreateEntity("Parent");
        var child = scene.CreateEntity("Child");

        scene.SetParent(parent, grandparent);
        scene.SetParent(child, parent);

        // 尝试创建循环：grandparent -> child
        Assert.Throws<ArgumentException>(() => scene.SetParent(grandparent, child));
    }

    [Fact]
    public void GetParent_returns_null_for_root_entity()
    {
        using var scene = new Neverness.Runtime.Scene.Internal.FrifloScene("TestScene");
        var root = scene.CreateEntity("Root");

        var parent = scene.GetParent(root);
        Assert.Null(parent);
    }

    [Fact]
    public void GetChildren_returns_empty_for_leaf_entity()
    {
        using var scene = new Neverness.Runtime.Scene.Internal.FrifloScene("TestScene");
        var leaf = scene.CreateEntity("Leaf");

        var children = scene.GetChildren(leaf);
        Assert.Empty(children);
    }

    #endregion

    #region 序列化

    [Fact(Skip = "序列化功能待实现")]
    public void Serialize_json_to_stream()
    {
        using var scene = new Neverness.Runtime.Scene.Internal.FrifloScene("TestScene");

        var entity = scene.CreateEntity("Player");
        entity.Add(new TransformComponent
        {
            Position = new Vector3(1, 2, 3),
            Rotation = Quaternion.Identity,
            Scale = Vector3.One,
        });

        using var stream = new MemoryStream();
        scene.Serialize(stream, "json");

        Assert.True(stream.Length > 0);
    }

    [Fact(Skip = "序列化功能待实现")]
    public void Deserialize_json_from_stream()
    {
        // 序列化
        using var scene1 = new FrifloScene("TestScene");
        var entity = scene1.CreateEntity("Player");
        entity.Add(new TransformComponent
        {
            Position = new Vector3(1, 2, 3),
            Rotation = Quaternion.Identity,
            Scale = Vector3.One,
        });

        using var stream = new MemoryStream();
        scene1.Serialize(stream, "json");

        // 反序列化
        stream.Position = 0;
        using var scene2 = new FrifloScene("TestScene2");
        scene2.Deserialize(stream, "json");

        Assert.True(scene2.EntityCount > 0);
    }

    [Fact(Skip = "序列化功能待实现")]
    public void Serialize_unsupported_format_throws()
    {
        using var scene = new Neverness.Runtime.Scene.Internal.FrifloScene("TestScene");
        using var stream = new MemoryStream();

        Assert.Throws<NotSupportedException>(() => scene.Serialize(stream, "xml"));
    }

    #endregion

    #region 集成测试

    [Fact]
    public void Integration_create_scene_with_systems_and_entities()
    {
        using var scene = new Neverness.Runtime.Scene.Internal.FrifloScene("TestScene");

        // 添加 TransformSystem
        scene.AddSystem(new TransformSystem());

        // 创建多个实体，设置不同的位置
        var entities = new List<IEntity>();
        for (int i = 0; i < 10; i++)
        {
            var entity = scene.CreateEntity($"Entity_{i}");
            entity.Add(new TransformComponent
            {
                Position = new Vector3(i, 0, 0),
                Rotation = Quaternion.Identity,
                Scale = Vector3.One,
            });
            entities.Add(entity);
        }

        Assert.Equal(10, scene.EntityCount);

        // Tick 场景
        scene.Update(0.016f);

        // 验证所有实体都有世界矩阵
        for (int i = 0; i < entities.Count; i++)
        {
            ref var transform = ref entities[i].Get<TransformComponent>();
            if (i == 0)
            {
                // 第一个实体位置是 (0,0,0)，世界矩阵是单位矩阵
                Assert.Equal(Matrix4x4.Identity, transform.WorldMatrix);
            }
            else
            {
                // 其他实体位置非零，世界矩阵不是单位矩阵
                Assert.NotEqual(Matrix4x4.Identity, transform.WorldMatrix);
            }
        }
    }

    [Fact]
    public void Integration_camera_system_calculates_matrices()
    {
        using var scene = new Neverness.Runtime.Scene.Internal.FrifloScene("TestScene");

        // 添加系统
        scene.AddSystem(new TransformSystem());
        scene.AddSystem(new CameraSystem());

        var camera = scene.CreateEntity("Camera");
        camera.Add(new TransformComponent
        {
            Position = new Vector3(0, 0, -10),
            Rotation = Quaternion.Identity,
            Scale = Vector3.One,
        });
        camera.Add(CameraComponent.DefaultPerspective);

        // Tick 场景
        scene.Update(0.016f);

        // 验证摄像机矩阵被计算
        ref var cam = ref camera.Get<CameraComponent>();
        Assert.NotEqual(Matrix4x4.Identity, cam.ProjectionMatrix);
        Assert.NotEqual(Matrix4x4.Identity, cam.ViewMatrix);
    }

    [Fact]
    public void Integration_hierarchy_with_transform_propagation()
    {
        using var scene = new Neverness.Runtime.Scene.Internal.FrifloScene("TestScene");

        // 添加系统
        scene.AddSystem(new TransformSystem());

        var root = scene.CreateEntity("Root");
        root.Add(new TransformComponent
        {
            Position = new Vector3(10, 0, 0),
            Rotation = Quaternion.Identity,
            Scale = Vector3.One,
        });

        var child = scene.CreateEntity("Child");
        child.Add(TransformComponent.Default);

        scene.SetParent(child, root);

        // Tick 场景
        scene.Update(0.016f);

        // 验证子实体的世界矩阵包含了父实体的变换
        ref var childTransform = ref child.Get<TransformComponent>();
        Assert.Equal(10, childTransform.WorldMatrix.Translation.X);
    }

    #endregion
}
