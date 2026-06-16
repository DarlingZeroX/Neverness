using System.Numerics;
using Neverness.Runtime.Scene;
using Neverness.Runtime.Scene.Components;
using Neverness.Runtime.Scene.Internal;
using Neverness.Runtime.Scene.Systems;
using Xunit;

namespace Neverness.Runtime.Scene.Tests;

/// <summary>
/// 调试测试——用于诊断系统执行问题。
/// </summary>
public sealed class DebugTests
{
    [Fact]
    public void Debug_query_count()
    {
        using var scene = new FrifloScene("DebugScene");

        // 创建实体
        var entity1 = scene.CreateEntity("Entity1");
        entity1.Add(new TransformComponent { Position = new Vector3(1, 0, 0) });

        var entity2 = scene.CreateEntity("Entity2");
        entity2.Add(new TransformComponent { Position = new Vector3(2, 0, 0) });

        // 查询
        var query = scene.Query<TransformComponent>();
        var count = query.Count;

        // 输出调试信息
        Console.WriteLine($"Query count: {count}");
        Console.WriteLine($"Scene entity count: {scene.EntityCount}");

        Assert.Equal(2, count);
    }

    [Fact]
    public void Debug_query_forEach()
    {
        using var scene = new FrifloScene("DebugScene");

        // 创建实体
        var entity1 = scene.CreateEntity("Entity1");
        entity1.Add(new TransformComponent { Position = new Vector3(1, 0, 0) });

        var entity2 = scene.CreateEntity("Entity2");
        entity2.Add(new TransformComponent { Position = new Vector3(2, 0, 0) });

        // 查询并遍历
        var positions = new List<Vector3>();
        scene.Query<TransformComponent>().ForEach((ref TransformComponent t, IEntity e) =>
        {
            positions.Add(t.Position);
            Console.WriteLine($"Entity {e.Name}: Position = {t.Position}");
        });

        Assert.Equal(2, positions.Count);
    }

    [Fact]
    public void Debug_transform_system()
    {
        using var scene = new FrifloScene("DebugScene");

        // 添加系统
        var transformSystem = new TransformSystem();
        scene.AddSystem(transformSystem);

        // 创建实体
        var entity = scene.CreateEntity("Entity1");
        entity.Add(new TransformComponent
        {
            Position = new Vector3(10, 0, 0),
            Rotation = Quaternion.Identity,
            Scale = Vector3.One,
        });

        // 检查系统是否初始化
        Console.WriteLine($"TransformSystem initialized: {transformSystem.IsInitialized}");

        // 手动调用系统更新
        transformSystem.Update(0.016f);

        // 检查结果
        ref var transform = ref entity.Get<TransformComponent>();
        Console.WriteLine($"WorldMatrix: {transform.WorldMatrix}");
        Console.WriteLine($"Position: {transform.Position}");

        // 验证世界矩阵被计算
        Assert.NotEqual(Matrix4x4.Identity, transform.WorldMatrix);
    }

    [Fact]
    public void Debug_scene_update()
    {
        using var scene = new FrifloScene("DebugScene");

        // 添加系统
        var transformSystem = new TransformSystem();
        scene.AddSystem(transformSystem);

        // 创建实体
        var entity = scene.CreateEntity("Entity1");
        entity.Add(new TransformComponent
        {
            Position = new Vector3(10, 0, 0),
            Rotation = Quaternion.Identity,
            Scale = Vector3.One,
        });

        // 检查系统状态
        Console.WriteLine($"TransformSystem initialized: {transformSystem.IsInitialized}");

        // 更新场景
        scene.Update(0.016f);

        // 检查结果
        ref var transform = ref entity.Get<TransformComponent>();
        Console.WriteLine($"After scene.Update - WorldMatrix: {transform.WorldMatrix}");
        Console.WriteLine($"Position: {transform.Position}");

        // 验证世界矩阵被计算
        Assert.NotEqual(Matrix4x4.Identity, transform.WorldMatrix);
    }

    [Fact]
    public void Debug_multiple_entities()
    {
        using var scene = new FrifloScene("DebugScene");

        // 添加系统
        scene.AddSystem(new TransformSystem());

        // 创建多个实体
        for (int i = 0; i < 10; i++)
        {
            var entity = scene.CreateEntity($"Entity_{i}");
            entity.Add(new TransformComponent
            {
                Position = new Vector3(i, 0, 0),
                Rotation = Quaternion.Identity,
                Scale = Vector3.One,
            });
        }

        // 更新场景
        scene.Update(0.016f);

        // 检查所有实体
        for (int i = 0; i < 10; i++)
        {
            var entity = scene.GetEntity(i);
            if (entity != null)
            {
                ref var transform = ref entity.Get<TransformComponent>();
                Console.WriteLine($"Entity {i}: Position = {transform.Position}, WorldMatrix = {transform.WorldMatrix}");
            }
        }

        // 验证
        Assert.Equal(10, scene.EntityCount);
    }

    [Fact]
    public void Debug_cycle_detection()
    {
        using var scene = new FrifloScene("DebugScene");

        // 创建实体
        var grandparent = scene.CreateEntity("Grandparent");
        var parent = scene.CreateEntity("Parent");
        var child = scene.CreateEntity("Child");

        Console.WriteLine($"Grandparent ID: {grandparent.Id}");
        Console.WriteLine($"Parent ID: {parent.Id}");
        Console.WriteLine($"Child ID: {child.Id}");

        // 设置父子关系
        scene.SetParent(parent, grandparent);
        scene.SetParent(child, parent);

        // 检查层级关系
        var parentOfParent = scene.GetParent(parent);
        var parentOfChild = scene.GetParent(child);

        Console.WriteLine($"Parent of Parent: {parentOfParent?.Id}");
        Console.WriteLine($"Parent of Child: {parentOfChild?.Id}");

        // 尝试创建循环
        try
        {
            scene.SetParent(grandparent, child);
            Console.WriteLine("ERROR: 循环检测失败，没有抛出异常");
        }
        catch (ArgumentException ex)
        {
            Console.WriteLine($"循环检测成功: {ex.Message}");
        }

        // 验证
        Assert.Throws<ArgumentException>(() => scene.SetParent(grandparent, child));
    }

    [Fact]
    public void Debug_hierarchy_transform_propagation()
    {
        using var scene = new FrifloScene("DebugScene");

        // 添加系统
        scene.AddSystem(new TransformSystem());

        // 创建父实体
        var root = scene.CreateEntity("Root");
        root.Add(new TransformComponent
        {
            Position = new Vector3(10, 0, 0),
            Rotation = Quaternion.Identity,
            Scale = Vector3.One,
        });

        // 创建子实体
        var child = scene.CreateEntity("Child");
        child.Add(TransformComponent.Default);

        // 设置父子关系
        scene.SetParent(child, root);

        // 检查层级关系
        var parentOfChild = scene.GetParent(child);
        Console.WriteLine($"Parent of Child: {parentOfChild?.Id}");

        // 更新场景
        scene.Update(0.016f);

        // 检查结果
        ref var childTransform = ref child.Get<TransformComponent>();
        Console.WriteLine($"Child Position: {childTransform.Position}");
        Console.WriteLine($"Child WorldMatrix: {childTransform.WorldMatrix}");
        Console.WriteLine($"Child WorldMatrix Translation: {childTransform.WorldMatrix.Translation}");

        // 验证
        Assert.Equal(10, childTransform.WorldMatrix.Translation.X);
    }
}
