namespace NevernessEditor;

/// <summary>
/// 从命令行解析的首包启动参数（空应用窗口）。
/// </summary>
internal sealed record EditorLaunchOptions
{
	/// <summary>窗口标题。</summary>
	public string WindowTitle { get; init; } = "Neverness";

	/// <summary>客户区宽度（像素）。</summary>
	public int Width { get; init; } = 1280;

	/// <summary>客户区高度（像素）。</summary>
	public int Height { get; init; } = 720;

	/// <summary>
	/// 解析命令行；支持 <c>--title</c>、<c>--width</c>、<c>--height</c>。
	/// </summary>
	public static EditorLaunchOptions Parse(string[] args)
	{
		var options = new EditorLaunchOptions();
		for (var i = 0; i < args.Length; i++)
		{
			switch (args[i])
			{
				case "--title" when i + 1 < args.Length:
					options = options with { WindowTitle = args[++i] };
					break;
				case "--width" when i + 1 < args.Length && int.TryParse(args[++i], out var w) && w > 0:
					options = options with { Width = w };
					break;
				case "--height" when i + 1 < args.Length && int.TryParse(args[++i], out var h) && h > 0:
					options = options with { Height = h };
					break;
			}
		}

		return options;
	}
}
