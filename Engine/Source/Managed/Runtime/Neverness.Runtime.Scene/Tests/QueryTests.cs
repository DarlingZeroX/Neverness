using System.Numerics;
using Neverness.Runtime.Scene;
using Neverness.Runtime.Scene.Components;
using Neverness.Runtime.Scene.Internal;
using Neverness.Runtime.Scene.Query;
using Xunit;

namespace Neverness.Runtime.Scene.Tests;

/// <summary>
/// Query 系统单元测试。
/// </summary>
public sealed class QueryTests
{
    #region SceneQuery

    [Fact]
    public void SceneQuery_single_component_count()
    {
        using var scene = new FrifloScene("TestScene");

        // 创建实体
        scene.CreateEntity("Entity1").Add(new TransformComponent { Position = new Vector3(1, 0, 0) });
        scene.CreateEntity("Entity2").Add(new TransformComponent { Position = new Vector3(2, 0, 0) });
        scene.CreateEntity("Entity3"); // 没有 TransformComponent

        var query = scene.Query<TransformComponent>();
        Assert.Equal(2, query.Count);
    }

    [Fact]
    public void SceneQuery_single_component_forEach()
    {
        using var scene = new FrifloScene("TestScene");

        scene.CreateEntity("Entity1").Add(new TransformComponent { Position = new Vector3(1, 0, 0) });
        scene.CreateEntity("Entity2").Add(new TransformComponent { Position = new Vector3(2, 0, 0) });

        var positions = new List<Vector3>();
        scene.Query<TransformComponent>().ForEach((ref TransformComponent t, IEntity e) =>
        {
            positions.Add(t.Position);
        });

        Assert.Equal(2, positions.Count);
    }

    [Fact]
    public void SceneQuery_single_component_firstOrDefault()
    {
        using var scene = new FrifloScene("TestScene");

        scene.CreateEntity("Entity1").Add(new TransformComponent { Position = new Vector3(1, 0, 0) });

        var query = scene.Query<TransformComponent>();
        var entity = query.FirstOrDefault();

        Assert.NotNull(entity);
        Assert.True(entity.IsValid);
    }

    [Fact]
    public void SceneQuery_single_component_any()
    {
        using var scene = new FrifloScene("TestScene");

        Assert.False(scene.Query<TransformComponent>().Any());

        scene.CreateEntity("Entity1").Add(TransformComponent.Default);

        Assert.True(scene.Query<TransformComponent>().Any());
    }

    [Fact]
    public void SceneQuery_single_component_toList()
    {
        using var scene = new FrifloScene("TestScene");

        scene.CreateEntity("Entity1").Add(TransformComponent.Default);
        scene.CreateEntity("Entity2").Add(TransformComponent.Default);

        var list = new List<IEntity>();
        scene.Query<TransformComponent>().ForEach((ref TransformComponent t, IEntity e) =>
        {
            list.Add(e);
        });
        Assert.Equal(2, list.Count);
    }

    #endregion

    #region SceneQuery<T1, T2>

    [Fact]
    public void SceneQuery_two_components_count()
    {
        using var scene = new FrifloScene("TestScene");

        var entity1 = scene.CreateEntity("Entity1");
        entity1.Add(TransformComponent.Default);
        entity1.Add(CameraComponent.DefaultPerspective);

        var entity2 = scene.CreateEntity("Entity2");
        entity2.Add(TransformComponent.Default);
        // 没有 CameraComponent

        var query = scene.Query<TransformComponent, CameraComponent>();
        Assert.Equal(1, query.Count);
    }

    [Fact]
    public void SceneQuery_two_components_forEach()
    {
        using var scene = new FrifloScene("TestScene");

        var entity1 = scene.CreateEntity("Entity1");
        entity1.Add(TransformComponent.Default);
        entity1.Add(CameraComponent.DefaultPerspective);

        var entity2 = scene.CreateEntity("Entity2");
        entity2.Add(TransformComponent.Default);
        entity2.Add(CameraComponent.DefaultPerspective);

        int count = 0;
        scene.Query<TransformComponent, CameraComponent>().ForEach(
            (ref TransformComponent t, ref CameraComponent c, IEntity e) =>
        {
            count++;
        });

        Assert.Equal(2, count);
    }

    #endregion

    #region SceneQueryCache

    [Fact]
    public void SceneQueryCache_get_query()
    {
        using var scene = new FrifloScene("TestScene");
        var cache = new SceneQueryCache(scene.Store, scene);

        var query1 = cache.GetQuery<TransformComponent>();
        var query2 = cache.GetQuery<TransformComponent>();

        // 应该返回同一个查询对象
        Assert.Same(query1, query2);
    }

    [Fact]
    public void SceneQueryCache_get_different_queries()
    {
        using var scene = new FrifloScene("TestScene");
        var cache = new SceneQueryCache(scene.Store, scene);

        var query1 = cache.GetQuery<TransformComponent>();
        var query2 = cache.GetQuery<CameraComponent>();

        // 应该返回不同的查询对象
        Assert.NotSame(query1, query2);
    }

    [Fact]
    public void SceneQueryCache_clear()
    {
        using var scene = new FrifloScene("TestScene");
        var cache = new SceneQueryCache(scene.Store, scene);

        cache.GetQuery<TransformComponent>();
        Assert.Equal(1, cache.Count);

        cache.Clear();
        Assert.Equal(0, cache.Count);
    }

    #endregion

    #region SceneView

    [Fact]
    public void SceneView_single_component_refresh()
    {
        using var scene = new FrifloScene("TestScene");

        scene.CreateEntity("Entity1").Add(new TransformComponent { Position = new Vector3(1, 0, 0) });
        scene.CreateEntity("Entity2").Add(new TransformComponent { Position = new Vector3(2, 0, 0) });

        var view = scene.CreateView<TransformComponent>();
        view.Refresh();

        Assert.Equal(2, view.Count);
    }

    [Fact]
    public void SceneView_single_component_forEach()
    {
        using var scene = new FrifloScene("TestScene");

        scene.CreateEntity("Entity1").Add(new TransformComponent { Position = new Vector3(1, 0, 0) });
        scene.CreateEntity("Entity2").Add(new TransformComponent { Position = new Vector3(2, 0, 0) });

        var view = scene.CreateView<TransformComponent>();
        view.Refresh();

        var positions = new List<Vector3>();
        view.ForEach((entity, transform) =>
        {
            positions.Add(transform.Position);
        });

        Assert.Equal(2, positions.Count);
    }

    [Fact]
    public void SceneView_two_components_refresh()
    {
        using var scene = new FrifloScene("TestScene");

        var entity1 = scene.CreateEntity("Entity1");
        entity1.Add(TransformComponent.Default);
        entity1.Add(CameraComponent.DefaultPerspective);

        var view = scene.CreateView<TransformComponent, CameraComponent>();
        view.Refresh();

        Assert.Equal(1, view.Count);
    }

    #endregion
}
