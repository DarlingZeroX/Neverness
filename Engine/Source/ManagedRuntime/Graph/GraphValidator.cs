namespace VisionGal.Managed.Graph;

/// <summary>
/// 圖結構驗證：節點存在性、埠方向、重複連線等。
/// </summary>
public static class GraphValidator
{
	/// <summary>驗證結果。</summary>
	public sealed class ValidationResult
	{
		/// <summary>是否通過。</summary>
		public bool IsValid { get; init; }

		/// <summary>錯誤訊息清單。</summary>
		public List<string> Errors { get; init; } = [];
	}

	/// <summary>驗證圖結構。</summary>
	public static ValidationResult Validate(Graph graph)
	{
		ArgumentNullException.ThrowIfNull(graph);
		var errors = new List<string>();

		foreach (var edge in graph.Edges)
		{
			var from = graph.FindNode(edge.FromNodeId);
			var to = graph.FindNode(edge.ToNodeId);
			if (from == null)
			{
				errors.Add($"連線來源節點不存在：{edge.FromNodeId}");
			}

			if (to == null)
			{
				errors.Add($"連線目標節點不存在：{edge.ToNodeId}");
			}

			if (from != null && from.Outputs.All(p => p.Name != edge.FromPort))
			{
				errors.Add($"來源埠不存在：{edge.FromNodeId}.{edge.FromPort}");
			}

			if (to != null && to.Inputs.All(p => p.Name != edge.ToPort))
			{
				errors.Add($"目標埠不存在：{edge.ToNodeId}.{edge.ToPort}");
			}
		}

		return new ValidationResult
		{
			IsValid = errors.Count == 0,
			Errors = errors
		};
	}
}
