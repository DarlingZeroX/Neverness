using System.Reflection;
using System.Runtime.Loader;

namespace Neverness.Runtime.Scripting;

/// <summary>
/// 隔離式 <see cref="AssemblyLoadContext"/> 宿主：載入遊戲腳本程序集而不污染預設上下文。
/// </summary>
public sealed class ManagedAssemblyLoadContextHost : AssemblyLoadContext
{
	private readonly string _name;

	/// <summary>建立具名 ALC 宿主。</summary>
	public ManagedAssemblyLoadContextHost(string name, bool isCollectible = true)
		: base(name, isCollectible)
	{
		ArgumentException.ThrowIfNullOrWhiteSpace(name);
		_name = name;
	}

	/// <summary>宿主名稱。</summary>
	public string HostName => _name;

	/// <summary>自磁碟路徑載入程序集至本 ALC。</summary>
	public Assembly LoadFromPath(string assemblyPath)
	{
		ArgumentException.ThrowIfNullOrWhiteSpace(assemblyPath);
		return LoadFromAssemblyPath(Path.GetFullPath(assemblyPath));
	}

	/// <inheritdoc />
	protected override Assembly? Load(AssemblyName assemblyName) => null;
}
