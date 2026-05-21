using System.Text.Json;

namespace Neverness.Editor.Framework.Serialization;

/// <summary>
/// 资产描述（路径、GUID、依赖）之 JSON 序列化（Editor/工具链）。
/// </summary>
public static class AssetSerializer
{
	/// <summary>资产登记文件根 DTO。</summary>
	public sealed class AssetDocument
	{
		/// <summary>格式版本。</summary>
		public int FormatVersion { get; set; } = VersionTolerance.CurrentFormatVersion;

		/// <summary>虚拟路径。</summary>
		public string VirtualPath { get; set; } = string.Empty;

		/// <summary>128-bit GUID 字符串（32 hex）。</summary>
		public string Guid { get; set; } = string.Empty;

		/// <summary>依赖资产 GUID 清单。</summary>
		public List<string> Dependencies { get; set; } = [];
	}

	/// <summary>序列化资产文件。</summary>
	public static string Serialize(AssetDocument document)
	{
		ArgumentNullException.ThrowIfNull(document);
		document.FormatVersion = VersionTolerance.CurrentFormatVersion;
		return JsonSerializer.Serialize(document, VersionTolerance.CreateOptions());
	}

	/// <summary>反序列化资产文件。</summary>
	public static AssetDocument? Deserialize(string json) =>
		string.IsNullOrWhiteSpace(json)
			? null
			: JsonSerializer.Deserialize<AssetDocument>(json, VersionTolerance.CreateOptions());
}
