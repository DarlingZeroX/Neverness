namespace Neverness.Managed.Graph;

/// <summary>
/// 鍦栦腑鍠竴杓稿叆/杓稿嚭鍩犳弿杩般€?/// </summary>
public sealed class GraphPort
{
	/// <summary>鎵€灞瘈榛?Id銆?/summary>
	public string NodeId { get; }

	/// <summary>鍩犲悕绋便€?/summary>
	public string Name { get; }

	/// <summary>鏄惁鐐鸿几鍑哄煚銆?/summary>
	public bool IsOutput { get; }

	/// <summary>鍊煎瀷鍒ュ悕绋憋紙搴忓垪鍖栫敤锛夈€?/summary>
	public string ValueTypeName { get; }

	/// <summary>寤虹珛鍩犮€?/summary>
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
