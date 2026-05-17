namespace VisionGal.Managed.Editor;

/// <summary>
/// 編輯器面板停靠佈局描述（區域名 → 面板 Id 清單）。
/// </summary>
public sealed class DockingLayout
{
	private readonly Dictionary<string, List<string>> _zones = new(StringComparer.OrdinalIgnoreCase);

	/// <summary>將面板 Id 放入指定停靠區。</summary>
	public void Place(string panelId, string zone)
	{
		ArgumentException.ThrowIfNullOrWhiteSpace(panelId);
		ArgumentException.ThrowIfNullOrWhiteSpace(zone);
		if (!_zones.TryGetValue(zone, out var list))
		{
			list = [];
			_zones[zone] = list;
		}

		if (!list.Contains(panelId))
		{
			list.Add(panelId);
		}
	}

	/// <summary>取得區域內面板 Id 清單。</summary>
	public IReadOnlyList<string> GetPanelsInZone(string zone) =>
		_zones.TryGetValue(zone, out var list) ? list : Array.Empty<string>();

	/// <summary>所有區域名稱。</summary>
	public IEnumerable<string> Zones => _zones.Keys;
}
