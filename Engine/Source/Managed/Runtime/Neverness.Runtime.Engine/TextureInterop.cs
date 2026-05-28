using System.Runtime.InteropServices;

namespace Neverness.Runtime.Engine;

/// <summary>
/// GPU Texture 资源互操作静态类。
/// 封装 <see cref="NNRenderAssetApi"/> 函数表，提供 Editor 和 Runtime 使用的 Texture 操作接口。
/// </summary>
/// <remarks>
/// 设计原则：
/// - 通过 <see cref="EngineNativeApiCache"/> 直接读取函数指针，无需单独 Initialize
/// - SpriteRenderer 只持有 Asset Handle，不持有 GPU Handle
/// - GPU 资源可能被驱逐、重建、跨 API 切换
/// - ImGui 的 ImTextureID 通过此层抽象，不直接暴露 OpenGL GLuint
/// </remarks>
public static unsafe class TextureInterop
{
	/// <summary>快速获取 RenderAsset API 子表；未安装时返回零结构。</summary>
	private static ref readonly NNRenderAssetApi Api =>
		ref EngineNativeApiCache.EngineApi.RenderAsset;

	/// <summary>API 是否可用（已安装且函数指针非空）。</summary>
	private static bool IsReady =>
		EngineNativeApiCache.IsInstalled &&
		Api.GetImGuiTextureHandle != null;

	/// <summary>
	/// 获取 Texture 的 ImGui Handle（后端无关）。
	/// </summary>
	/// <param name="textureKey">RenderAssetManager 返回的缓存 key</param>
	/// <returns>ImGui 兼容的 texture handle，0 = 无效</returns>
	public static ulong GetImGuiTextureHandle(ulong textureKey)
	{
		if (!IsReady || textureKey == 0)
			return 0;
		ulong handle = Api.GetImGuiTextureHandle(textureKey);
		//Console.WriteLine($"[TextureInterop] GetImGuiTextureHandle key={textureKey} → handle=0x{handle:X} ({handle})");
		return handle;
	}

	/// <summary>
	/// 从 RGBA8 像素数据创建 GPU Texture。
	/// </summary>
	/// <returns>缓存 key，0 = 失败</returns>
	public static ulong CreateTexture(
		uint width, uint height,
		ReadOnlySpan<byte> pixels,
		bool isSRGB = false)
	{
		if (!IsReady)
			return 0;

		fixed (byte* pPixels = pixels)
		{
			return Api.CreateTextureFromPixels(
				width, height,
				pPixels, (nuint)pixels.Length,
				isSRGB ? 1 : 0
			);
		}
	}

	/// <summary>
	/// 释放 GPU Texture 资源。
	/// </summary>
	public static void ReleaseTexture(ulong textureKey)
	{
		if (IsReady && textureKey != 0)
			Api.ReleaseTexture(textureKey);
	}

	/// <summary>
	/// 重载 Texture（Hot Reload 场景）。
	/// </summary>
	public static void ReloadTexture(
		ulong textureKey,
		uint width, uint height,
		ReadOnlySpan<byte> pixels,
		bool isSRGB = false)
	{
		if (!IsReady || textureKey == 0)
			return;

		fixed (byte* pPixels = pixels)
		{
			Api.ReloadTextureFromPixels(
				textureKey,
				width, height,
				pPixels, (nuint)pixels.Length,
				isSRGB ? 1 : 0
			);
		}
	}

	/// <summary>
	/// 获取 Texture 尺寸。
	/// </summary>
	public static (uint Width, uint Height) GetTextureSize(ulong textureKey)
	{
		if (!IsReady || textureKey == 0)
			return (0, 0);

		uint w = 0, h = 0;
		if (Api.GetTextureDesc(textureKey, &w, &h) != 0)
			return (w, h);
		return (0, 0);
	}

	/// <summary>
	/// Texture 是否已加载到 GPU。
	/// </summary>
	public static bool IsTextureResident(ulong textureKey)
	{
		if (!IsReady || textureKey == 0)
			return false;
		return Api.IsTextureResident(textureKey) != 0;
	}

	/// <summary>
	/// 获取已缓存的 Texture 数量。
	/// </summary>
	public static ulong GetCachedTextureCount()
	{
		if (!IsReady)
			return 0;
		return Api.GetCachedTextureCount();
	}

	/// <summary>
	/// 获取 GPU 纹理总内存使用量（字节）。
	/// </summary>
	public static ulong GetTotalGPUMemory()
	{
		if (!IsReady)
			return 0;
		return Api.GetTotalGPUMemory();
	}

	/// <summary>
	/// 从已加载的 .nnasset 资源句柄创建 GPU Texture。
	/// 读取 blob[0] 反序列化为纹理源资产，再上传 GPU。
	/// </summary>
	/// <param name="assetHandle">AssetManager 返回的资源句柄</param>
	/// <param name="guid">资产完整 128-bit GUID（内部取 Low 做缓存键）</param>
	/// <returns>缓存 key，0 = 失败</returns>
	public static ulong LoadTextureFromAsset(ulong assetHandle, NNGuid guid)
	{
		if (!IsReady || assetHandle == 0 || Api.LoadTextureFromAsset == null)
			return 0;
		return Api.LoadTextureFromAsset(assetHandle, guid.Low);
	}

	/// <summary>
	/// 从已解析的 blob 数据直接创建 GPU Texture（避免跨模块单例问题）。
	/// 通过 AssetManager API 获取 blob 数据指针，再传给 RenderAssetManager。
	/// </summary>
	/// <param name="assetHandle">AssetManager 返回的资源句柄</param>
	/// <param name="guid">资产完整 128-bit GUID</param>
	/// <returns>缓存 key，0 = 失败</returns>
	public static ulong LoadTextureFromBlob(NNAssetHandle assetHandle, NNGuid guid)
	{
		if (!IsReady || assetHandle.Value == 0 || Api.LoadTextureFromBlob == null)
			return 0;

		var amApi = EngineNativeApiCache.EngineApi.AssetManager;
		if (amApi.GetBlobCount == null || amApi.GetBlobData == null || amApi.GetBlobSize == null)
			return 0;

		uint blobCount = amApi.GetBlobCount(assetHandle);

		// 查找 TypeInfo (type=9) 和 DATA (type=0) blob
		// TypeInfo 是 24 字节（NNTextureTypeInfo），DATA 更大
		void* typeInfoData = null;
		ulong typeInfoSize = 0;
		void* pixelData = null;
		ulong pixelDataSize = 0;

		for (uint i = 0; i < blobCount; i++)
		{
			var data = amApi.GetBlobData(assetHandle, i);
			var size = amApi.GetBlobSize(assetHandle, i);
			if (size == 24 && typeInfoData == null)
			{
				typeInfoData = data;
				typeInfoSize = size;
			}
			else if (size > 24 && pixelData == null)
			{
				pixelData = data;
				pixelDataSize = size;
			}
		}

		if (typeInfoData == null || pixelData == null)
		{
			Console.WriteLine($"[TextureInterop] LoadTextureFromBlob: 找不到 TypeInfo 或 DATA blob (blobCount={blobCount})");
			return 0;
		}

		return Api.LoadTextureFromBlob(typeInfoData, typeInfoSize, pixelData, pixelDataSize, guid.Low);
	}
}
