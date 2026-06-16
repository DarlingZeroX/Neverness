using System.Numerics;
using Neverness.Runtime.Scene.Components;

namespace Neverness.Runtime.Scene.Systems;

/// <summary>
/// 摄像机系统——计算投影矩阵和视图矩阵。
/// 替代 C++ NNCameraSystem。
/// 使用 System.Numerics.Matrix4x4 替代 GLM。
/// </summary>
public sealed class CameraSystem : ISceneSystem
{
    private IScene? _scene;

    public string Name => "CameraSystem";
    public int Priority => 200;
    public SceneSystemTags Tags => SceneSystemTags.Render;
    public bool IsInitialized { get; private set; }

    public void Initialize(IScene scene)
    {
        _scene = scene;
        IsInitialized = true;
    }

    public void Update(float deltaTime)
    {
        if (_scene == null) return;

        // 查询所有有 CameraComponent 的实体
        _scene.Query<CameraComponent, TransformComponent>().ForEach(
            (ref CameraComponent camera, ref TransformComponent transform, IEntity entity) =>
        {
            // 计算投影矩阵
            if (camera.IsOrthographic)
            {
                var width = camera.OrthographicSize * camera.AspectRatio;
                var height = camera.OrthographicSize;
                camera.ProjectionMatrix = Matrix4x4.CreateOrthographic(
                    width, height, camera.NearPlane, camera.FarPlane);
            }
            else
            {
                camera.ProjectionMatrix = Matrix4x4.CreatePerspectiveFieldOfView(
                    camera.FieldOfView, camera.AspectRatio, camera.NearPlane, camera.FarPlane);
            }

            // 计算视图矩阵（从世界矩阵的逆矩阵）
            if (Matrix4x4.Invert(transform.WorldMatrix, out var inverseWorld))
            {
                camera.ViewMatrix = inverseWorld;
            }
            else
            {
                camera.ViewMatrix = Matrix4x4.Identity;
            }
        });
    }

    public void FixedUpdate(float fixedDeltaTime) { }

    public void Shutdown()
    {
        _scene = null;
        IsInitialized = false;
    }

    public void Dispose() { }
}
