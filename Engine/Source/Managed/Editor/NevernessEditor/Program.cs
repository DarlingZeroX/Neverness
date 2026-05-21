// NervernessEditor — 首包：经 NevernessRuntime-Managed.dll 提供 Native API 表，显示空 SDL 应用窗口。

namespace NevernessEditor;

internal static class Program
{
	/// <summary>
	/// 编辑器入口：解析命令行并进入 <see cref="EditorApplicationRunner"/> 主循环。
	/// </summary>
	/// <param name="args">可选 <c>--title</c>、<c>--width</c>、<c>--height</c>。</param>
	/// <returns>进程退出码。</returns>
	public static int Main(string[] args)
	{
		var options = EditorLaunchOptions.Parse(args);
		return EditorApplicationRunner.Run(options);
	}
}
