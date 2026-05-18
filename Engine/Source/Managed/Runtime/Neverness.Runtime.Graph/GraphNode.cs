namespace Neverness.Managed.Graph;

/// <summary>
/// 圖節點：具唯一 Id、型別名與平面座標。
/// </summary>
public sealed class GraphNode
{
	/// <summary>節點唯一 Id。</summary>
	public string Id { get; }

	/// <summary>節點邏輯型別名。</summary>
	public string TypeName { get; set; }

	/// <summary>編輯器 X 座標。</summary>
	public float X { get; set; }

	/// <summary>編輯器 Y 座標。</summary>
	public float Y { get; set; }

	/// <summary>輸入埠。</summary>
	public List<GraphPort> Inputs { get; } = [];

	/// <summary>輸出埠。</summary>
	public List<GraphPort> Outputs { get; } = [];

	/// <summary>建立節點。</summary>
	public GraphNode(string id, string typeName, float x = 0, float y = 0)
	{
		ArgumentException.ThrowIfNullOrWhiteSpace(id);
		ArgumentException.ThrowIfNullOrWhiteSpace(typeName);
		Id = id;
		TypeName = typeName;
		X = x;
		Y = y;
	}
}
