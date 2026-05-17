namespace VisionGal.Managed.Graph;

/// <summary>
/// 圖中單一輸入/輸出埠描述。
/// </summary>
public sealed class GraphPort
{
	/// <summary>所屬節點 Id。</summary>
	public string NodeId { get; }

	/// <summary>埠名稱。</summary>
	public string Name { get; }

	/// <summary>是否為輸出埠。</summary>
	public bool IsOutput { get; }

	/// <summary>值型別名稱（序列化用）。</summary>
	public string ValueTypeName { get; }

	/// <summary>建立埠。</summary>
	public GraphPort(string nodeId, string name, bool isOutput, string valueTypeName = "object")
	{
		ArgumentException.ThrowIfNullOrWhiteSpace(nodeId);
		ArgumentException.ThrowIfNullOrWhiteSpace(name);
		NodeId = nodeId;
		Name = name;
		IsOutput = isOutput;
		ValueTypeName = valueTypeName;
	}
}
