using Neverness.Managed.Engine;

namespace Neverness.Managed.Object;

/// <summary>
/// 所有託管引擎物件之抽象基底；持有託管 <see cref="VGObjectId"/> 與 Native 橋接 <see cref="NNObjectHandle"/>。
/// 生命週期由 <see cref="LifetimeSystem"/> 與 <see cref="NativeHandleBridge"/> 協調。
/// </summary>
public abstract class VGObject : IDisposable
{
	private bool _disposed;

	/// <summary>託管註冊表鍵。</summary>
	public VGObjectId Id { get; }

	/// <summary>Native Object 子系統之不透明控制代碼。</summary>
	public NNObjectHandle Handle { get; }

	/// <summary>建立已綁定識別碼與 Native 控制代碼之託管物件。</summary>
	/// <param name="id">由 <see cref="ObjectRegistry"/> 分配之識別碼。</param>
	/// <param name="handle">由 <see cref="NativeHandleBridge"/> 建立之 Native 控制代碼。</param>
	protected VGObject(VGObjectId id, NNObjectHandle handle)
	{
		Id = id;
		Handle = handle;
	}

	/// <summary>物件之邏輯型別名稱（用於日誌與序列化）。</summary>
	public abstract string TypeName { get; }

	/// <summary>
	/// 釋放託管對 Native 控制代碼之唯一引用（<see cref="LifetimeSystem.Release"/>），並自 <see cref="ObjectRegistry"/> 移除。
	/// </summary>
	public void Dispose()
	{
		if (_disposed)
		{
			return;
		}

		_disposed = true;
		LifetimeSystem.Release(this);
		ObjectRegistry.Unregister(Id);
		GC.SuppressFinalize(this);
	}

	/// <summary>若已釋放則拋出 <see cref="ObjectDisposedException"/>。</summary>
	protected void ThrowIfDisposed()
	{
		ObjectDisposedException.ThrowIf(_disposed, this);
	}
}
