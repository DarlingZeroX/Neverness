using System.Reflection;
using VisionGal.Managed.Engine;

namespace VisionGal.Managed.Object;

/// <summary>
/// 協調託管 <see cref="VGObject"/> 與 Native <see cref="NativeHandleBridge"/> 之 retain/release 語義。
/// <para>
/// 契約：<c>createObject</c> 成功時 Native 側引用計數為 1，託管註冊表持有該唯一引用；
/// 僅在共享所有權場景才呼叫 <see cref="Retain"/> 遞增。Dispose 時 <see cref="Release"/> 遞減至 0 後銷毀控制代碼。
/// </para>
/// </summary>
public static class LifetimeSystem
{
	/// <summary>
	/// 建立並註冊託管物件：分配 Id、呼叫 Native Create（初始 ref=1）、寫入註冊表；不再額外 Retain。
	/// </summary>
	/// <typeparam name="T">
	/// 具備 <c>(VGObjectId, VGObjectHandle)</c> 或 <c>(VGObjectId, VGObjectHandle, string)</c> 建構子之 <see cref="VGObject"/> 衍生型別。
	/// 僅有第三參數為可選字串之建構子時，IL 仍為三參數簽名，<see cref="Activator.CreateInstance(Type, object[])"/> 無法匹配兩元素陣列，故改以反射建立。
	/// </typeparam>
	/// <param name="typeName">傳遞給 Native 的型別名稱；若使用三參數建構子亦作為第三參數傳入。</param>
	/// <returns>已註冊之託管物件實例。</returns>
	public static T CreateAndRegister<T>(string typeName) where T : VGObject
	{
		var id = ObjectRegistry.AllocateId();
		var handle = NativeHandleBridge.CreateObject(typeName);
		if (handle.Value == 0)
		{
			throw new InvalidOperationException($"Native createObject 失敗，型別：{typeName}");
		}

		var t = typeof(T);
		const BindingFlags ctorFlags = BindingFlags.Instance | BindingFlags.Public | BindingFlags.NonPublic;

		var ctor2 = t.GetConstructor(ctorFlags, null, [typeof(VGObjectId), typeof(VGObjectHandle)], null);
		T obj;
		if (ctor2 != null)
		{
			obj = (T)ctor2.Invoke([id, handle]);
		}
		else
		{
			var ctor3 = t.GetConstructor(ctorFlags, null, [typeof(VGObjectId), typeof(VGObjectHandle), typeof(string)], null);
			if (ctor3 == null)
			{
				throw new MissingMethodException(
					t.FullName,
					"Expected ctor(VGObjectId, VGObjectHandle) or ctor(VGObjectId, VGObjectHandle, string).");
			}

			obj = (T)ctor3.Invoke([id, handle, typeName]);
		}

		ObjectRegistry.Register(obj);
		return obj;
	}

	/// <summary>對已存在物件再次 Retain（共享所有權場景）。</summary>
	public static void Retain(VGObject obj)
	{
		ArgumentNullException.ThrowIfNull(obj);
		NativeHandleBridge.Retain(obj.Handle);
	}

	/// <summary>對應 Dispose 路徑：Release Native 控制代碼並可選銷毀。</summary>
	public static void Release(VGObject obj)
	{
		ArgumentNullException.ThrowIfNull(obj);
		NativeHandleBridge.Release(obj.Handle);
		if (!NativeHandleBridge.IsAlive(obj.Handle))
		{
			NativeHandleBridge.DestroyObject(obj.Handle);
		}
	}
}
