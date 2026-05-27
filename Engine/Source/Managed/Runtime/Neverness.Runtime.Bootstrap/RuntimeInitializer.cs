// Neverness.Runtime.Bootstrap — 有序安装 Interop 与子系统（禁止 DllImport；仅函数表）。

using Neverness.Runtime.Interop;
using Neverness.Runtime.RuntimeLoop;

namespace Neverness.Runtime.Bootstrap;

/// <summary>
/// 托管 Runtime 初始化器：安装 Native API 表、注册默认子系统并调用 <see cref="RuntimeLoop.InitializeRegistered"/>。
/// </summary>
public static class RuntimeInitializer
{
	private static volatile bool s_initialized;

	/// <summary>Interop 与子系统是否已完成初始化。</summary>
	public static bool IsInitialized => s_initialized;

	/// <summary>供 <see cref="RuntimeMainLoop"/> 使用的运行时调度器实例。</summary>
	internal static RuntimeLoop.RuntimeLoop? Loop { get; private set; }

	/// <summary>
	/// 从 Native 表指针安装 Interop 并初始化已注册子系统。
	/// </summary>
	/// <param name="nativeApiTable"><c>NNNativeAPI*</c> 指针。</param>
	/// <returns>安装是否成功（至少 Core API 表可用）。</returns>
	public static bool Initialize(nint nativeApiTable)
	{
		NativeApiBootstrap.Install(nativeApiTable);
		if (!NativeApiBootstrap.IsInstalled)
		{
			s_initialized = false;
			return false;
		}

		EngineNativeApiBootstrap.InstallFromNativeApiTable(nativeApiTable);

		Loop ??= new RuntimeLoop.RuntimeLoop();
		RegisterDefaultSubsystems(Loop);
		Loop.InitializeRegistered();

		NativeApiBootstrap.LogInfoUtf8("Neverness.Managed.Bootstrap RuntimeInitializer OK"u8);
		s_initialized = true;
		return true;
	}

	/// <summary>
	/// 注册首包占位子系统；Gameplay / Scene 逻辑子系统在 Phase 6 由上层模块扩展。
	/// </summary>
	public static void RegisterDefaultSubsystems(RuntimeLoop.RuntimeLoop loop)
	{
		ArgumentNullException.ThrowIfNull(loop);
		// 首包无默认子系统；产品层通过 Bootstrap 扩展点注册。
	}

	/// <summary>
	/// 注册子系统到运行时调度器；若已初始化则立即调用 Initialize。
	/// 用于在 <see cref="Initialize"/> 完成后补充注册子系统（如 Editor 场景子系统）。
	/// </summary>
	public static void RegisterSubsystem(IManagedRuntimeSubsystem subsystem)
	{
		ArgumentNullException.ThrowIfNull(subsystem);
		Loop?.Register(subsystem);
		if (s_initialized)
		{
			subsystem.Initialize();
		}
	}

	/// <summary>逆序关闭子系统并清除初始化标记（进程退出或域卸载时调用）。</summary>
	public static void Shutdown()
	{
		if (Loop != null)
		{
			Loop.ShutdownRegistered();
			Loop = null;
		}

		s_initialized = false;
	}
}
