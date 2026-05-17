namespace VisionGal.Managed.Graph;

/// <summary>
/// 節點圖容器：節點、連線與基本查詢。
/// </summary>
public sealed class Graph
{
	/// <summary>圖名稱。</summary>
	public string Name { get; set; }

	/// <summary>節點集合。</summary>
	public List<GraphNode> Nodes { get; } = [];

	/// <summary>連線集合。</summary>
	public List<GraphEdge> Edges { get; } = [];

	/// <summary>建立空圖。</summary>
	public Graph(string name)
	{
		ArgumentException.ThrowIfNullOrWhiteSpace(name);
		Name = name;
	}

	/// <summary>新增節點；Id 重複時回傳 false。</summary>
	public bool AddNode(GraphNode node)
	{
		ArgumentNullException.ThrowIfNull(node);
		if (Nodes.Any(n => n.Id == node.Id))
		{
			return false;
		}

		Nodes.Add(node);
		return true;
	}

	/// <summary>新增連線。</summary>
	public void AddEdge(GraphEdge edge)
	{
		ArgumentNullException.ThrowIfNull(edge);
		Edges.Add(edge);
	}

	/// <summary>依 Id 查找節點。</summary>
	public GraphNode? FindNode(string id) => Nodes.FirstOrDefault(n => n.Id == id);
}
