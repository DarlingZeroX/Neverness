#if VISIONGAL_ENABLE_ROSLYN
using Microsoft.CodeAnalysis;
using Microsoft.CodeAnalysis.CSharp;
#endif

namespace Neverness.Managed.Scripting;

/// <summary>
/// Roslyn C# 腳本編譯器；定義 <c>VISIONGAL_ENABLE_ROSLYN</c> 時使用 Microsoft.CodeAnalysis.CSharp。
/// 未定義時提供基於 <see cref="System.Reflection"/> 的占位實作（僅驗證語法非空）。
/// </summary>
public static class RoslynScriptCompiler
{
	/// <summary>編譯結果。</summary>
	public sealed class CompileResult
	{
		/// <summary>是否成功。</summary>
		public bool Success { get; init; }

		/// <summary>錯誤訊息。</summary>
		public IReadOnlyList<string> Errors { get; init; } = [];

#if VISIONGAL_ENABLE_ROSLYN
		/// <summary>編譯產物（成功時非 null）。</summary>
		public byte[]? AssemblyBytes { get; init; }
#endif
	}

	/// <summary>將 C# 原始碼編譯為程序集位元組或驗證占位。</summary>
	public static CompileResult Compile(string source, string assemblyName = "Neverness.DynamicScript")
	{
		ArgumentException.ThrowIfNullOrWhiteSpace(source);
		ArgumentException.ThrowIfNullOrWhiteSpace(assemblyName);

#if VISIONGAL_ENABLE_ROSLYN
		var syntaxTree = CSharpSyntaxTree.ParseText(source);
		var compilation = CSharpCompilation.Create(
			assemblyName,
			[syntaxTree],
			[MetadataReference.CreateFromFile(typeof(object).Assembly.Location)],
			new CSharpCompilationOptions(OutputKind.DynamicallyLinkedLibrary));

		using var ms = new MemoryStream();
		var emit = compilation.Emit(ms);
		if (!emit.Success)
		{
			return new CompileResult
			{
				Success = false,
				Errors = emit.Diagnostics
					.Where(d => d.Severity == DiagnosticSeverity.Error)
					.Select(d => d.ToString())
					.ToList()
			};
		}

		return new CompileResult
		{
			Success = true,
			AssemblyBytes = ms.ToArray()
		};
#else
		_ = assemblyName;
		return new CompileResult
		{
			Success = source.Contains("class", StringComparison.Ordinal),
			Errors = source.Contains("class", StringComparison.Ordinal)
				? []
				: ["Roslyn 未啟用：原始碼需包含 class 關鍵字（占位驗證）。"]
		};
#endif
	}
}
