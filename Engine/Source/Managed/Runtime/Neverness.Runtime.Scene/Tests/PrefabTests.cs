using System.Numerics;
using Neverness.Runtime.Scene;
using Neverness.Runtime.Scene.Components;
using Neverness.Runtime.Scene.Internal;
using Neverness.Runtime.Scene.Prefab;
using Xunit;

namespace Neverness.Runtime.Scene.Tests;

/// <summary>
/// Prefab 系统单元测试。
/// </summary>
public sealed class PrefabTests
{
    #region PrefabAsset

    [Fact]
    public void PrefabAsset_create_with_name()
    {
        var prefab = new PrefabAsset("TestPrefab");
        Assert.Equal("TestPrefab", prefab.Name);
        Assert.NotEqual(Guid.Empty, prefab.Guid);
    }

    [Fact]
    public void PrefabAsset_add_component_data()
    {
        var prefab = new PrefabAsset("TestPrefab");
        prefab.Components.Add(new PrefabComponentData
        {
            TypeName = "TransformComponent",
            Data = "{}"
        });

        Assert.Single(prefab.Components);
    }

    [Fact]
    public void PrefabAsset_add_child_data()
    {
        var prefab = new PrefabAsset("TestPrefab");
        prefab.Children.Add(new PrefabChildData
        {
            Name = "Child",
            Position = new Vector3(1, 0, 0)
        });

        Assert.Single(prefab.Children);
    }

    #endregion

    #region PrefabInstantiator

    [Fact]
    public void PrefabInstantiator_instantiate_at_origin()
    {
        using var scene = new FrifloScene("TestScene");
        var instantiator = new PrefabInstantiator(scene);

        var prefab = new PrefabAsset("TestPrefab");
        var entity = instantiator.Instantiate(prefab);

        Assert.NotNull(entity);
        Assert.True(entity.IsValid);
        Assert.Equal(1, scene.EntityCount);
    }

    [Fact]
    public void PrefabInstantiator_instantiate_at_position()
    {
        using var scene = new FrifloScene("TestScene");
        var instantiator = new PrefabInstantiator(scene);

        var prefab = new PrefabAsset("TestPrefab");
        var entity = instantiator.InstantiateAt(prefab, new Vector3(10, 20, 30));

        Assert.NotNull(entity);
        ref var transform = ref entity.Get<TransformComponent>();
        Assert.Equal(10, transform.Position.X);
        Assert.Equal(20, transform.Position.Y);
        Assert.Equal(30, transform.Position.Z);
    }

    [Fact]
    public void PrefabInstantiator_instantiate_with_children()
    {
        using var scene = new FrifloScene("TestScene");
        var instantiator = new PrefabInstantiator(scene);

        var prefab = new PrefabAsset("TestPrefab");
        prefab.Children.Add(new PrefabChildData
        {
            Name = "Child1",
            Position = new Vector3(1, 0, 0)
        });
        prefab.Children.Add(new PrefabChildData
        {
            Name = "Child2",
            Position = new Vector3(2, 0, 0)
        });

        var root = instantiator.Instantiate(prefab);

        Assert.Equal(3, scene.EntityCount); // root + 2 children
    }

    [Fact]
    public void PrefabInstantiator_instantiate_as_child()
    {
        using var scene = new FrifloScene("TestScene");
        var instantiator = new PrefabInstantiator(scene);

        var parent = scene.CreateEntity("Parent");
        var prefab = new PrefabAsset("TestPrefab");
        var child = instantiator.InstantiateAsChild(prefab, parent, new Vector3(5, 0, 0));

        Assert.Equal(2, scene.EntityCount);
        var actualParent = scene.GetParent(child);
        Assert.NotNull(actualParent);
        Assert.Equal(parent.Id, actualParent.Id);
    }

    #endregion

    #region PrefabSerializer

    [Fact]
    public void PrefabSerializer_serialize_and_deserialize()
    {
        var prefab = new PrefabAsset("TestPrefab");
        prefab.Components.Add(new PrefabComponentData
        {
            TypeName = "TransformComponent",
            Data = "{\"position\":[1,2,3]}"
        });
        prefab.Children.Add(new PrefabChildData
        {
            Name = "Child",
            Position = new Vector3(1, 0, 0)
        });

        var json = PrefabSerializer.Serialize(prefab);
        Assert.NotNull(json);
        Assert.Contains("TestPrefab", json);

        var deserialized = PrefabSerializer.Deserialize(json);
        Assert.NotNull(deserialized);
        Assert.Equal("TestPrefab", deserialized.Name);
        Assert.Single(deserialized.Components);
        Assert.Single(deserialized.Children);
    }

    [Fact]
    public void PrefabSerializer_save_and_load_file()
    {
        var prefab = new PrefabAsset("TestPrefab");
        var tempFile = Path.GetTempFileName();

        try
        {
            PrefabSerializer.SaveToFile(prefab, tempFile);
            var loaded = PrefabSerializer.LoadFromFile(tempFile);

            Assert.NotNull(loaded);
            Assert.Equal("TestPrefab", loaded.Name);
        }
        finally
        {
            File.Delete(tempFile);
        }
    }

    #endregion

    #region PrefabManager

    [Fact]
    public void PrefabManager_register_and_get()
    {
        var manager = new PrefabManager();
        var prefab = new PrefabAsset("TestPrefab");

        manager.Register(prefab);

        Assert.Equal(1, manager.Count);
        Assert.True(manager.Exists("TestPrefab"));
        Assert.Same(prefab, manager.GetByName("TestPrefab"));
        Assert.Same(prefab, manager.GetByGuid(prefab.Guid));
    }

    [Fact]
    public void PrefabManager_unregister()
    {
        var manager = new PrefabManager();
        var prefab = new PrefabAsset("TestPrefab");
        manager.Register(prefab);

        var result = manager.Unregister("TestPrefab");

        Assert.True(result);
        Assert.Equal(0, manager.Count);
        Assert.False(manager.Exists("TestPrefab"));
    }

    [Fact]
    public void PrefabManager_get_all_names()
    {
        var manager = new PrefabManager();
        manager.Register(new PrefabAsset("Prefab1"));
        manager.Register(new PrefabAsset("Prefab2"));
        manager.Register(new PrefabAsset("Prefab3"));

        var names = manager.GetAllNames();
        Assert.Equal(3, names.Count);
        Assert.Contains("Prefab1", names);
        Assert.Contains("Prefab2", names);
        Assert.Contains("Prefab3", names);
    }

    #endregion
}
