using System.Numerics;

namespace Neverness.Runtime.Scene.Prefab;

/// <summary>
/// 预制体资产——可复用的实体模板。
/// 包含一组组件定义，可以实例化为多个实体。
/// </summary>
public sealed class PrefabAsset
{
    /// <summary>预制体名称。</summary>
    public string Name { get; set; }

    /// <summary>预制体 GUID。</summary>
    public Guid Guid { get; set; }

    /// <summary>组件列表（序列化用）。</summary>
    public List<PrefabComponentData> Components { get; set; } = new();

    /// <summary>子预制体列表。</summary>
    public List<PrefabChildData> Children { get; set; } = new();

    public PrefabAsset(string name)
    {
        Name = name;
        Guid = Guid.NewGuid();
    }
}

/// <summary>
/// 预制体组件数据——用于序列化。
/// </summary>
public sealed class PrefabComponentData
{
    /// <summary>组件类型名称。</summary>
    public string TypeName { get; set; } = "";

    /// <summary>组件数据（JSON 格式）。</summary>
    public string Data { get; set; } = "";
}

/// <summary>
/// 预制体子节点数据——用于序列化。
/// </summary>
public sealed class PrefabChildData
{
    /// <summary>子节点名称。</summary>
    public string Name { get; set; } = "";

    /// <summary>相对位置。</summary>
    public Vector3 Position { get; set; }

    /// <summary>相对旋转。</summary>
    public Quaternion Rotation { get; set; } = Quaternion.Identity;

    /// <summary>相对缩放。</summary>
    public Vector3 Scale { get; set; } = Vector3.One;

    /// <summary>组件列表。</summary>
    public List<PrefabComponentData> Components { get; set; } = new();

    /// <summary>子节点列表（递归）。</summary>
    public List<PrefabChildData> Children { get; set; } = new();
}
