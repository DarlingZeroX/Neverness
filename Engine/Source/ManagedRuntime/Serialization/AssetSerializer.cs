using System.Text.Json;

namespace Neverness.Managed.Serialization;

/// <summary>
/// 資產描述（路徑、GUID、依賴）之 JSON 序列化。
/// </summary>
public static class AssetSerializer
{
	/// <summary>資產登記文件根 DTO。</summary>
	public sealed class AssetDocument
	{
		/// <summary>格式版本。</summary>
		public int FormatVersion { get; set; } = VersionTolerance.CurrentFormatVersion;

		/// <summary>虛擬路徑。</summary>
		public string VirtualPath { get; set; } = string.Empty;

		/// <summary>128-bit GUID 字串（32 hex）。</summary>
		public string Guid { get; set; } = string.Empty;

		/// <summary>依賴資產 GUID 清單。</summary>
		public List<string> Dependencies { get; set; } = [];
	}

	/// <summary>序列化資產文件。</summary>
	public static string Serialize(AssetDocument document)
	{
		ArgumentNullException.ThrowIfNull(document);
		document.FormatVersion = VersionTolerance.CurrentFormatVersion;
		return JsonSerializer.Serialize(document, VersionTolerance.CreateOptions());
	}

	/// <summary>反序列化資產文件。</summary>
	public static AssetDocument? Deserialize(string json) =>
		string.IsNullOrWhiteSpace(json)
			? null
			: JsonSerializer.Deserialize<AssetDocument>(json, VersionTolerance.CreateOptions());
}
