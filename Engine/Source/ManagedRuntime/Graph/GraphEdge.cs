namespace VisionGal.Managed.Graph;

/// <summary>
/// 節點間有向連線（輸出埠 → 輸入埠）。
/// </summary>
public sealed class GraphEdge
{
	/// <summary>來源節點 Id。</summary>
	public string FromNodeId { get; }

	/// <summary>來源埠名。</summary>
	public string FromPort { get; }

	/// <summary>目標節點 Id。</summary>
	public string ToNodeId { get; }

	/// <summary>目標埠名。</summary>
	public string ToPort { get; }

	/// <summary>建立連線。</summary>
	public GraphEdge(string fromNodeId, string fromPort, string toNodeId, string toPort)
	{
		ArgumentException.ThrowIfNullOrWhiteSpace(fromNodeId);
		ArgumentException.ThrowIfNullOrWhiteSpace(fromPort);
		ArgumentException.ThrowIfNullOrWhiteSpace(toNodeId);
		ArgumentException.ThrowIfNullOrWhiteSpace(toPort);
		FromNodeId = fromNodeId;
		FromPort = fromPort;
		ToNodeId = toNodeId;
		ToPort = toPort;
	}
}
