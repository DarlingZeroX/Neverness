using System.Numerics;
using Neverness.Runtime.Scene;
using Neverness.Runtime.Scene.Components;
using Neverness.Runtime.Scene.Internal;
using Xunit;

namespace Neverness.Runtime.Scene.Tests;

/// <summary>
/// EntityFactory 单元测试。
/// </summary>
public sealed class EntityFactoryTests
{
    [Fact]
    public void CreateCamera_creates_entity_with_components()
    {
        using var scene = new FrifloScene("TestScene");

        var entity = EntityFactory.CreateCamera(scene, "MainCamera");

        Assert.NotNull(entity);
        Assert.True(entity.IsValid);
        Assert.True(entity.Has<TransformComponent>());
        Assert.True(entity.Has<CameraComponent>());
    }

    [Fact]
    public void CreateCamera_sets_position()
    {
        using var scene = new FrifloScene("TestScene");

        var entity = EntityFactory.CreateCamera(scene, position: new Vector3(10, 20, 30));

        ref var transform = ref entity.Get<TransformComponent>();
        Assert.Equal(10, transform.Position.X);
        Assert.Equal(20, transform.Position.Y);
        Assert.Equal(30, transform.Position.Z);
    }

    [Fact]
    public void CreateOrthographicCamera_creates_entity()
    {
        using var scene = new FrifloScene("TestScene");

        var entity = EntityFactory.CreateOrthographicCamera(scene);

        Assert.NotNull(entity);
        ref var camera = ref entity.Get<CameraComponent>();
        Assert.True(camera.IsOrthographic);
    }

    [Fact]
    public void CreateSprite_creates_entity_with_components()
    {
        using var scene = new FrifloScene("TestScene");

        var entity = EntityFactory.CreateSprite(scene, "MySprite");

        Assert.NotNull(entity);
        Assert.True(entity.Has<TransformComponent>());
        Assert.True(entity.Has<SpriteRendererComponent>());
        ref var sprite = ref entity.Get<SpriteRendererComponent>();
        Assert.Equal(1f, sprite.ColorR);
        Assert.Equal(1f, sprite.ColorG);
        Assert.Equal(1f, sprite.ColorB);
        Assert.Equal(1f, sprite.ColorA);
    }

    [Fact]
    public void CreateAudioSource_creates_entity_with_components()
    {
        using var scene = new FrifloScene("TestScene");

        var entity = EntityFactory.CreateAudioSource(scene, "MyAudio");

        Assert.NotNull(entity);
        Assert.True(entity.Has<TransformComponent>());
        Assert.True(entity.Has<AudioSourceComponent>());
        ref var audio = ref entity.Get<AudioSourceComponent>();
        Assert.Equal(1f, audio.Volume);
        Assert.Equal(1f, audio.Pitch);
    }

    [Fact]
    public void CreateVideoPlayer_creates_entity_with_components()
    {
        using var scene = new FrifloScene("TestScene");

        var entity = EntityFactory.CreateVideoPlayer(scene, "MyVideo");

        Assert.NotNull(entity);
        Assert.True(entity.Has<TransformComponent>());
        Assert.True(entity.Has<VideoPlayerComponent>());
        ref var video = ref entity.Get<VideoPlayerComponent>();
        Assert.Equal(1f, video.Volume);
    }

    [Fact]
    public void CreateRmlUIDocument_creates_entity_with_components()
    {
        using var scene = new FrifloScene("TestScene");

        var entity = EntityFactory.CreateRmlUIDocument(scene, "MyUI");

        Assert.NotNull(entity);
        Assert.True(entity.Has<TransformComponent>());
        Assert.True(entity.Has<RmlUIDocumentComponent>());
        ref var rml = ref entity.Get<RmlUIDocumentComponent>();
        Assert.Equal(RmlUIViewTarget.Both, rml.ViewTarget);
    }

    [Fact]
    public void CreateEmpty_creates_entity_with_transform()
    {
        using var scene = new FrifloScene("TestScene");

        var entity = EntityFactory.CreateEmpty(scene, "Empty");

        Assert.NotNull(entity);
        Assert.True(entity.Has<TransformComponent>());
    }

    [Fact]
    public void CreateScriptEntity_creates_entity_with_script()
    {
        using var scene = new FrifloScene("TestScene");

        var entity = EntityFactory.CreateScriptEntity(scene, 12345, "ScriptEntity");

        Assert.NotNull(entity);
        Assert.True(entity.Has<TransformComponent>());
        Assert.True(entity.Has<ScriptComponent>());
        ref var script = ref entity.Get<ScriptComponent>();
        Assert.Equal(12345UL, script.ScriptTypeId);
    }

    [Fact]
    public void CreateParentChild_creates_hierarchy()
    {
        using var scene = new FrifloScene("TestScene");

        var (parent, child) = EntityFactory.CreateParentChild(scene);

        Assert.NotNull(parent);
        Assert.NotNull(child);
        Assert.Equal(2, scene.EntityCount);

        var actualParent = scene.GetParent(child);
        Assert.NotNull(actualParent);
        Assert.Equal(parent.Id, actualParent.Id);
    }

    [Fact]
    public void CreateBatch_creates_multiple_entities()
    {
        using var scene = new FrifloScene("TestScene");

        var entities = EntityFactory.CreateBatch(scene, 5, "Entity");

        Assert.Equal(5, entities.Count);
        Assert.Equal(5, scene.EntityCount);
    }

    [Fact]
    public void CreateBatch_sets_positions()
    {
        using var scene = new FrifloScene("TestScene");

        var entities = EntityFactory.CreateBatch(scene, 3, spacing: new Vector3(2, 0, 0));

        ref var t0 = ref entities[0].Get<TransformComponent>();
        ref var t1 = ref entities[1].Get<TransformComponent>();
        ref var t2 = ref entities[2].Get<TransformComponent>();

        Assert.Equal(0, t0.Position.X);
        Assert.Equal(2, t1.Position.X);
        Assert.Equal(4, t2.Position.X);
    }
}
