using System.Runtime.InteropServices;
using Neverness.Rendering.Diligent;
using Neverness.Runtime.Assets;
using Neverness.Runtime.Assets.Formats;
using Neverness.Runtime.Engine;

namespace Neverness.Rendering.RenderAssets;

/// <summary>
/// 纹理类型信息（与 C++ NNTextureTypeInfo 对齐）。
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public struct TextureTypeInfo
{
    public uint Width;
    public uint Height;
    public uint Format;
    public uint MipCount;
    public uint Flags;
}

/// <summary>
/// Render Asset Manager。
/// 负责 CPU Asset → GPU Resource 的生命周期管理。
/// </summary>
public sealed class RenderAssetManager
{
    private static RenderAssetManager? _instance;
    private static readonly object _instanceLock = new();

    private readonly object _mutex = new();
    private readonly Dictionary<ulong, CacheEntry> _entryCache = new();
    private readonly Dictionary<ulong, ulong> _guidToCacheKeyMap = new(); // GUID.Low → cache key
    private IRenderResourceFactory? _factory;
    private ulong _currentFrame;
    private ulong _nextKey = 1;
    private bool _initialized;

    /// <summary>
    /// 获取单例实例（首次访问自动调用 InitializeFromNative 完成初始化）。
    /// </summary>
    public static RenderAssetManager Instance
    {
        get
        {
            if (_instance == null)
            {
                lock (_instanceLock)
                {
                    if (_instance == null)
                    {
                        _instance = new RenderAssetManager();
                        _instance.InitializeFromNative();
                    }
                }
            }
            return _instance;
        }
    }

    private RenderAssetManager() { }

    /// <summary>
    /// 初始化（需要传入渲染资源工厂）。
    /// </summary>
    public bool Initialize(IRenderResourceFactory factory)
    {
        lock (_mutex)
        {
            if (_initialized)
                return true;

            if (factory == null)
            {
                Console.Error.WriteLine("[RenderAssetManager] Initialize: factory 为空");
                return false;
            }

            _factory = factory;
            _entryCache.Clear();
            _guidToCacheKeyMap.Clear();
            _nextKey = 1;
            _currentFrame = 0;
            _initialized = true;
            return true;
        }
    }

    /// <summary>
    /// 从 NNDiligentAPI 初始化。
    /// 通过 GraphicsDevice.InitializePrimary() 统一获取主窗口设备，并创建渲染资源工厂。
    /// </summary>
    public bool InitializeFromNative()
    {
        if (!GraphicsDevice.IsInitialized)
        {
            var device = GraphicsDevice.InitializePrimary();
            if (device == null)
            {
                Console.Error.WriteLine("[RenderAssetManager] InitializeFromNative: GraphicsDevice 初始化失败");
                return false;
            }
        }

        var factory = new DiligentRenderResourceFactory(GraphicsDevice.Instance);
        return Initialize(factory);
    }

    /// <summary>
    /// 关闭并释放所有资源。
    /// </summary>
    public void Shutdown()
    {
        lock (_mutex)
        {
            // 释放所有缓存条目
            foreach (var entry in _entryCache.Values)
            {
                entry.Resource.Dispose();
            }
            _entryCache.Clear();
            _guidToCacheKeyMap.Clear();
            _initialized = false;
            _factory = null;
            _nextKey = 1;
        }
    }

    /// <summary>
    /// 从 Source Asset 直接创建 GPU Texture。
    /// </summary>
    /// <returns>缓存 key，0 = 失败</returns>
    public ulong CreateTextureFromSource(TextureSourceAsset source)
    {
        if (!_initialized || source == null || !source.IsValid)
            return 0;

        lock (_mutex)
        {
            var res = UploadTextureInternal(source);
            if (res == null)
                return 0;
            return _nextKey - 1;
        }
    }

    /// <summary>
    /// 从原始像素数据创建 GPU Texture。
    /// </summary>
    /// <returns>缓存 key，0 = 失败</returns>
    public ulong CreateTextureFromPixels(uint width, uint height, byte[] pixels, bool isSRGB = false)
    {
        if (!_initialized || pixels == null || pixels.Length == 0)
            return 0;

        var source = new TextureSourceAsset();
        source.SetFromDecodedImage(width, height, TextureFormat.RGBA8_UNorm,
                                   pixels, isSRGB, true);

        lock (_mutex)
        {
            var res = UploadTextureInternal(source);
            if (res == null)
                return 0;
            return _nextKey - 1;
        }
    }

    /// <summary>
    /// 从已加载的 .nnasset 资源句柄创建 GPU Texture。
    /// </summary>
    /// <param name="assetHandle">AssetManager 返回的资源句柄</param>
    /// <param name="guidLow">资产 GUID.Low（可选，用于建立 GUID→cacheKey 索引）</param>
    /// <returns>缓存 key，0 = 失败</returns>
    public ulong LoadTextureFromAsset(ulong assetHandle, ulong guidLow = 0)
    {
        if (!_initialized || assetHandle == 0)
        {
            Console.Error.WriteLine($"[RenderAssetManager] LoadTextureFromAsset: 未初始化或 handle=0");
            return 0;
        }

        // 如果有 guidLow，先检查是否已缓存
        if (guidLow != 0)
        {
            lock (_mutex)
            {
                if (_guidToCacheKeyMap.TryGetValue(guidLow, out var cachedKey) &&
                    _entryCache.ContainsKey(cachedKey))
                {
                    Console.WriteLine($"[RenderAssetManager] LoadTextureFromAsset: GUID.Low={guidLow} 已缓存 key={cachedKey}");
                    return cachedKey;
                }
            }
        }

        // TODO: 通过 AssetManager 获取 blob 数据
        // 这里需要调用 C++ 端的 AssetManager API
        Console.Error.WriteLine("[RenderAssetManager] LoadTextureFromAsset: 暂未实现跨语言资产加载");
        return 0;
    }

    /// <summary>
    /// 从已解析的 blob 数据直接创建 GPU Texture。
    /// </summary>
    public ulong LoadTextureFromBlob(IntPtr typeInfoData, ulong typeInfoSize,
                                     IntPtr pixelData, ulong pixelDataSize,
                                     ulong guidLow = 0)
    {
        if (!_initialized)
        {
            Console.Error.WriteLine("[RenderAssetManager] LoadTextureFromBlob: 未初始化");
            return 0;
        }

        if (typeInfoData == IntPtr.Zero || typeInfoSize < (ulong)Marshal.SizeOf<TextureTypeInfo>())
        {
            Console.Error.WriteLine("[RenderAssetManager] LoadTextureFromBlob: TypeInfo 数据无效");
            return 0;
        }

        if (pixelData == IntPtr.Zero || pixelDataSize == 0)
        {
            Console.Error.WriteLine("[RenderAssetManager] LoadTextureFromBlob: 像素数据无效");
            return 0;
        }

        // 如果有 guidLow，先检查是否已缓存
        if (guidLow != 0)
        {
            lock (_mutex)
            {
                if (_guidToCacheKeyMap.TryGetValue(guidLow, out var cachedKey) &&
                    _entryCache.ContainsKey(cachedKey))
                {
                    Console.WriteLine($"[RenderAssetManager] LoadTextureFromBlob: GUID.Low={guidLow} 已缓存 key={cachedKey}");
                    return cachedKey;
                }
            }
        }

        var texInfo = Marshal.PtrToStructure<TextureTypeInfo>(typeInfoData);

        // 合理性校验
        if (texInfo.Width == 0 || texInfo.Height == 0 || texInfo.Width > 16384 || texInfo.Height > 16384)
        {
            Console.Error.WriteLine($"[RenderAssetManager] LoadTextureFromBlob: TypeInfo 宽高无效 {texInfo.Width}x{texInfo.Height}");
            return 0;
        }
        if (texInfo.Format > 4)
        {
            Console.Error.WriteLine($"[RenderAssetManager] LoadTextureFromBlob: TypeInfo format={texInfo.Format} 无效");
            return 0;
        }

        var format = (TextureFormat)texInfo.Format;
        bool isSRGB = (texInfo.Flags & 1) != 0;
        bool hasAlpha = (format == TextureFormat.RGBA8_UNorm);

        // 复制像素数据
        var pixels = new byte[pixelDataSize];
        Marshal.Copy(pixelData, pixels, 0, (int)pixelDataSize);

        var source = new TextureSourceAsset();
        source.SetFromDecodedImage(texInfo.Width, texInfo.Height, format,
                                   pixels, isSRGB, hasAlpha);

        lock (_mutex)
        {
            var res = UploadTextureInternal(source);
            if (res == null)
            {
                Console.Error.WriteLine($"[RenderAssetManager] LoadTextureFromBlob: GPU 上传失败 {texInfo.Width}x{texInfo.Height}");
                return 0;
            }

            var key = _nextKey - 1;
            Console.WriteLine($"[RenderAssetManager] LoadTextureFromBlob: 成功 key={key} {texInfo.Width}x{texInfo.Height}");

            if (guidLow != 0)
                _guidToCacheKeyMap[guidLow] = key;

            return key;
        }
    }

    /// <summary>
    /// 通过 key 获取 GPU Texture Resource。
    /// </summary>
    public TextureResource? GetTextureResource(ulong key)
    {
        lock (_mutex)
        {
            if (!_entryCache.TryGetValue(key, out var entry))
                return null;

            entry.Resource.LastUsedFrame = _currentFrame;
            return entry.Resource;
        }
    }

    /// <summary>
    /// 释放 GPU Texture 资源。
    /// </summary>
    public void ReleaseTexture(ulong key)
    {
        lock (_mutex)
        {
            if (_entryCache.TryGetValue(key, out var entry))
            {
                entry.Resource.Dispose();
                _entryCache.Remove(key);
            }
        }
    }

    /// <summary>
    /// 重载 Texture（Hot Reload 场景）。
    /// </summary>
    public void ReloadTexture(ulong key, TextureSourceAsset source)
    {
        if (source == null || !source.IsValid)
            return;

        lock (_mutex)
        {
            if (!_entryCache.ContainsKey(key))
                return;

            // 上传新纹理
            var newRes = UploadTextureInternal(source);
            if (newRes == null)
                return;

            // 更新缓存条目
            var newKey = _nextKey - 1;
            if (_entryCache.TryGetValue(newKey, out var newEntry))
            {
                // 将新条目移到旧 key 下
                _entryCache[key] = newEntry;
                _entryCache.Remove(newKey);
            }
        }
    }

    /// <summary>
    /// 获取 ImGui Texture Handle。
    /// </summary>
    public ulong GetImGuiTextureHandle(ulong key)
    {
        var res = GetTextureResource(key);
        if (res == null)
            return 0;
        return res.GetImGuiHandle();
    }

    /// <summary>
    /// 驱逐最近最少使用的 Texture。
    /// </summary>
    public void EvictLRU(ulong targetMemoryBytes)
    {
        lock (_mutex)
        {
            // 简单实现：遍历找到最旧的条目并删除
            ulong totalMem = 0;
            foreach (var entry in _entryCache.Values)
                totalMem += entry.GPUMemory;

            while (totalMem > targetMemoryBytes && _entryCache.Count > 1)
            {
                ulong oldestKey = 0;
                ulong oldestFrame = ulong.MaxValue;

                foreach (var (k, e) in _entryCache)
                {
                    if (e.Resource != null && e.Resource.LastUsedFrame < oldestFrame)
                    {
                        oldestFrame = e.Resource.LastUsedFrame;
                        oldestKey = k;
                    }
                }

                if (oldestKey == 0)
                    break;

                if (_entryCache.TryGetValue(oldestKey, out var entry))
                {
                    totalMem -= entry.GPUMemory;
                    entry.Resource.Dispose();
                    _entryCache.Remove(oldestKey);
                }
            }
        }
    }

    /// <summary>
    /// 更新帧号（用于 LRU / Residency）。
    /// </summary>
    public void SetCurrentFrame(ulong frame)
    {
        _currentFrame = frame;
    }

    /// <summary>
    /// 获取缓存的纹理数量。
    /// </summary>
    public ulong GetCachedTextureCount()
    {
        lock (_mutex)
        {
            return (ulong)_entryCache.Count;
        }
    }

    /// <summary>
    /// 获取估算的 GPU 内存使用量。
    /// </summary>
    public ulong GetEstimatedGPUMemory()
    {
        lock (_mutex)
        {
            ulong total = 0;
            foreach (var entry in _entryCache.Values)
                total += entry.GPUMemory;
            return total;
        }
    }

    /// <summary>
    /// 通过 GUID.Low 查询已缓存的 cache key。
    /// </summary>
    public ulong GetCacheKeyByGuidLow(ulong guidLow)
    {
        if (guidLow == 0)
            return 0;

        lock (_mutex)
        {
            return _guidToCacheKeyMap.TryGetValue(guidLow, out var key) ? key : 0;
        }
    }

    /// <summary>
    /// 通过 cache key 获取纹理 Handle。
    /// </summary>
    public ulong GetGLTextureId(ulong cacheKey)
    {
        if (cacheKey == 0)
            return 0;

        lock (_mutex)
        {
            if (!_entryCache.TryGetValue(cacheKey, out var entry))
                return 0;
            return entry.Resource?.GetImGuiHandle() ?? 0;
        }
    }

    /// <summary>
    /// 确保纹理已加载到 GPU，返回 TextureHandle（ITextureView* 编码为 ulong）。
    /// 内部完成全链路：缓存查询 → AssetManager.LoadAssetSync → Blob 解析 → GPU 上传。
    /// 返回 0 表示失败（空 GUID、加载失败、上传失败）。
    /// </summary>
    /// <param name="textureGuid">纹理资产 GUID（来自 SpriteRendererComponent.TextureAsset）。</param>
    /// <returns>GPU TextureHandle，0 = 失败。</returns>
    public ulong EnsureTextureLoaded(NNGuid textureGuid)
    {
        // 1. 空 GUID 检查
        if (textureGuid.High == 0 && textureGuid.Low == 0)
            return 0;

        // 2. 查询缓存（GUID.Low → cacheKey → Handle）
        var cacheKey = GetCacheKeyByGuidLow(textureGuid.Low);
        if (cacheKey != 0)
            return GetGLTextureId(cacheKey);

        // 3. Cache miss → 从 AssetManager 同步加载 .nnasset
        var guid = GUID.FromNative(textureGuid);
        var handle = AssetManager.Instance.LoadAssetSync(guid);
        if (handle == 0)
            return 0;

        // 4. 解析 Blob（TypeInfo = 9, Data = 0）
        var typeInfo = AssetManager.Instance.GetBlobByType(handle, BlobType.TypeInfo);
        var pixelData = AssetManager.Instance.GetBlobByType(handle, BlobType.Data);
        if (typeInfo.IsEmpty || pixelData.IsEmpty)
            return 0;

        // 5. 上传 GPU 并注册 GUID → cacheKey 映射
        unsafe
        {
            fixed (byte* typeInfoPtr = typeInfo)
            fixed (byte* pixelPtr = pixelData)
            {
                cacheKey = LoadTextureFromBlob(
                    (IntPtr)typeInfoPtr, (ulong)typeInfo.Length,
                    (IntPtr)pixelPtr, (ulong)pixelData.Length,
                    textureGuid.Low);
            }
        }

        if (cacheKey == 0)
            return 0;

        // 6. 返回 GPU Handle
        return GetGLTextureId(cacheKey);
    }

    /// <summary>
    /// 热更新已存在纹理的像素数据。
    /// </summary>
    public bool UpdateTexturePixels(ulong cacheKey, byte[] pixels)
    {
        if (!_initialized || pixels == null || pixels.Length == 0 || cacheKey == 0)
            return false;

        if (_factory == null)
            return false;

        lock (_mutex)
        {
            if (!_entryCache.TryGetValue(cacheKey, out var entry))
                return false;

            if (entry.Resource == null)
                return false;

            // 通过工厂更新纹理像素
            bool result = _factory.UpdateTexturePixels(entry.Resource, pixels);

            if (result)
                entry.Resource.LastUsedFrame = _currentFrame;

            return result;
        }
    }

    /// <summary>
    /// 是否已初始化。
    /// </summary>
    public bool IsInitialized => _initialized;

    // ===== 内部实现 =====

    private TextureResource? UploadTextureInternal(TextureSourceAsset source)
    {
        if (_factory == null)
        {
            Console.Error.WriteLine("[UploadTextureInternal] _factory 为空");
            return null;
        }

        Console.WriteLine($"[UploadTextureInternal] 开始 {source.Width}x{source.Height} format={source.Format} mips={source.MipCount}");

        var baseMip = source.GetMip(0);
        Console.WriteLine($"[UploadTextureInternal] baseMip pixels={baseMip.Pixels.Length}");

        // 通过工厂创建 GPU 纹理
        Console.WriteLine("[UploadTextureInternal] 调用工厂 CreateTexture...");
        var resource = _factory.CreateTexture(
            source.Width, source.Height,
            source.Format,
            baseMip.Pixels,
            source.IsSRGB);

        if (resource == null)
        {
            Console.Error.WriteLine("[UploadTextureInternal] 工厂 CreateTexture 返回 null");
            return null;
        }

        Console.WriteLine("[UploadTextureInternal] 工厂 CreateTexture 成功");

        // 补充元数据
        resource.Desc.MipCount = source.MipCount;
        resource.Desc.Format = source.Format;
        resource.Desc.IsSRGB = source.IsSRGB;
        resource.Residency = TextureResidency.Resident;
        resource.LastUsedFrame = _currentFrame;

        // 估算 GPU 内存
        ulong gpuMem = (ulong)source.Width * source.Height * 4;

        // 分配 key
        ulong key = _nextKey++;

        // 存入缓存
        var entry = new CacheEntry
        {
            Resource = resource,
            GPUMemory = gpuMem
        };

        _entryCache[key] = entry;
        return resource;
    }

    /// <summary>
    /// 缓存条目。
    /// </summary>
    private sealed class CacheEntry
    {
        public required TextureResource Resource { get; init; }
        public required ulong GPUMemory { get; init; }
    }
}
