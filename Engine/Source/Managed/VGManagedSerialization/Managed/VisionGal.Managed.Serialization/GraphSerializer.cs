using System.Text.Json;

namespace VisionGal.Managed.Serialization;

/// <summary>
/// 節點圖（節點、連線、埠）之 JSON 序列化。
/// </summary>
public static class GraphSerializer
{
	/// <summary>圖文件根 DTO。</summary>
	public sealed class GraphDocument
	{
		/// <summary>格式版本。</summary>
		public int FormatVersion { get; set; } = VersionTolerance.CurrentFormatVersion;

		/// <summary>圖名稱。</summary>
		public string Name { get; set; } = string.Empty;

		/// <summary>節點清單。</summary>
		public List<GraphNodeEntry> Nodes { get; set; } = [];

		/// <summary>連線清單。</summary>
		public List<GraphEdgeEntry> Edges { get; set; } = [];
	}

	/// <summary>節點條目。</summary>
	public sealed class GraphNodeEntry
	{
		/// <summary>節點唯一識別字串。</summary>
		public string Id { get; set; } = string.Empty;

		/// <summary>節點型別全名或 Authoring 型別 id。</summary>
		public string TypeName { get; set; } = string.Empty;

		/// <summary>編輯器畫布 X 座標。</summary>
		public float X { get; set; }

		/// <summary>編輯器畫布 Y 座標。</summary>
		public float Y { get; set; }

		/// <summary>可序列化屬性鍵值（JSON 元素）。</summary>
		public Dictionary<string, JsonElement> Properties { get; set; } = new();
	}

	/// <summary>連線條目。</summary>
	public sealed class GraphEdgeEntry
	{
		/// <summary>來源節點 Id。</summary>
		public string FromNodeId { get; set; } = string.Empty;

		/// <summary>來源埠名稱。</summary>
		public string FromPort { get; set; } = string.Empty;

		/// <summary>目標節點 Id。</summary>
		public string ToNodeId { get; set; } = string.Empty;

		/// <summary>目標埠名稱。</summary>
		public string ToPort { get; set; } = string.Empty;
	}

	/// <summary>序列化圖文件。</summary>
	public static string Serialize(GraphDocument document)
	{
		ArgumentNullException.ThrowIfNull(document);
		document.FormatVersion = VersionTolerance.CurrentFormatVersion;
		return JsonSerializer.Serialize(document, VersionTolerance.CreateOptions());
	}

	/// <summary>反序列化圖文件。</summary>
	public static GraphDocument? Deserialize(string json) =>
		string.IsNullOrWhiteSpace(json)
			? null
			: JsonSerializer.Deserialize<GraphDocument>(json, VersionTolerance.CreateOptions());
}
