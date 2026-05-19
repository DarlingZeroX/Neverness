using System.Reflection;
using Neverness.Managed.Engine;
using Neverness.Managed.Interop;

namespace Neverness.Managed.Object;

/// <summary>
/// 鍗旇瑷楃 <see cref="VGObject"/> 鑸?Native <see cref="NativeHandleBridge"/> 涔?retain/release 瑾炵京銆?/// <para>
/// 濂戠磩锛?c>createObject</c> 鎴愬姛鏅?Native 鍋村紩鐢ㄨ▓鏁哥偤 1锛岃绠¤ɑ鍐婅〃鎸佹湁瑭插敮涓€寮曠敤锛?/// 鍍呭湪鍏变韩鎵€鏈夋瑠鍫存櫙鎵嶅懠鍙?<see cref="Retain"/> 閬炲銆侱ispose 鏅?<see cref="Release"/> 閬炴笡鑷?0 寰岄姺姣€鎺у埗浠ｇ⒓銆?/// </para>
/// </summary>
public static class LifetimeSystem
{
	/// <summary>
	/// 寤虹珛涓﹁ɑ鍐婅绠＄墿浠讹細鍒嗛厤 Id銆佸懠鍙?Native Create锛堝垵濮?ref=1锛夈€佸鍏ヨɑ鍐婅〃锛涗笉鍐嶉澶?Retain銆?	/// </summary>
	/// <typeparam name="T">
	/// 鍏峰倷 <c>(VGObjectId, NNObjectHandle)</c> 鎴?<c>(VGObjectId, NNObjectHandle, string)</c> 寤烘瀛愪箣 <see cref="VGObject"/> 琛嶇敓鍨嬪垾銆?	/// 鍍呮湁绗笁鍙冩暩鐐哄彲閬稿瓧涓蹭箣寤烘瀛愭檪锛孖L 浠嶇偤涓夊弮鏁哥敖鍚嶏紝<see cref="Activator.CreateInstance(Type, object[])"/> 鐒℃硶鍖归厤鍏╁厓绱犻櫍鍒楋紝鏁呮敼浠ュ弽灏勫缓绔嬨€?	/// </typeparam>
	/// <param name="typeName">鍌抽仦绲?Native 鐨勫瀷鍒ュ悕绋憋紱鑻ヤ娇鐢ㄤ笁鍙冩暩寤烘瀛愪害浣滅偤绗笁鍙冩暩鍌冲叆銆?/param>
	/// <returns>宸茶ɑ鍐婁箣瑷楃鐗╀欢瀵︿緥銆?/returns>
	public static T CreateAndRegister<T>(string typeName) where T : VGObject
	{
		var id = ObjectRegistry.AllocateId();
		var handle = NativeHandleBridge.CreateObject(typeName);
		if (handle.Value == 0)
		{
			throw new InvalidOperationException($"Native createObject 澶辨晽锛屽瀷鍒ワ細{typeName}");
		}

		var t = typeof(T);
		const BindingFlags ctorFlags = BindingFlags.Instance | BindingFlags.Public | BindingFlags.NonPublic;

		var ctor2 = t.GetConstructor(ctorFlags, null, [typeof(VGObjectId), typeof(NNObjectHandle)], null);
		T obj;
		if (ctor2 != null)
		{
			obj = (T)ctor2.Invoke([id, handle]);
		}
		else
		{
			var ctor3 = t.GetConstructor(ctorFlags, null, [typeof(VGObjectId), typeof(NNObjectHandle), typeof(string)], null);
			if (ctor3 == null)
			{
				throw new MissingMethodException(
					t.FullName,
					"Expected ctor(VGObjectId, NNObjectHandle) or ctor(VGObjectId, NNObjectHandle, string).");
			}

			obj = (T)ctor3.Invoke([id, handle, typeName]);
		}

		ObjectRegistry.Register(obj);
		return obj;
	}

	/// <summary>灏嶅凡瀛樺湪鐗╀欢鍐嶆 Retain锛堝叡浜墍鏈夋瑠鍫存櫙锛夈€?/summary>
	public static void Retain(VGObject obj)
	{
		ArgumentNullException.ThrowIfNull(obj);
		NativeHandleBridge.Retain(obj.Handle);
	}

	/// <summary>灏嶆噳 Dispose 璺緫锛歊elease Native 鎺у埗浠ｇ⒓涓﹀彲閬搁姺姣€銆?/summary>
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
