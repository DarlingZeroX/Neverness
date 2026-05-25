using System.Collections.Concurrent;
using System.Runtime.CompilerServices;
using System.Security.Cryptography;
using System.Text;
using System.Threading.Tasks;
using Neverness.Runtime.Assets;

namespace Neverness.Editor.Assets;

/// <summary>
/// 资产导入管线。
///
/// 编排完整的 8 阶段资产导入流程：
///   Phase 1: Meta 处理 — 读取或创建 .meta
///   Phase 2: 变化检测 — content hash 比对，跳过未变化资产
///   Phase 3: Import — 调用对应 importer 解析源文件
///   Phase 4: Compile — 将 ImportResult 序列化为 .nnasset 二进制格式
///   Phase 5: Database 更新 — 注册到 EditorAssetDatabase
///   Phase 6: Dependency 更新 — 更新依赖图
///   Phase 7: Thumbnail — 生成缩略图
///   Phase 8: 通知 — 触发事件
///
/// 设计原则：
///   - 支持增量导入（未变化文件跳过）
///   - 支持批量导入
///   - 支持进度回调
///   - 线程安全
///
/// @threadsafe 所有公共方法内部使用 s_lock 保护共享状态，
///   ImportDirectoryParallel 的 Phase 1-4 并行阶段无锁（各线程操作独立数据）。
///   ImportStateCache / ImporterRegistry / MetaFileManager 各自独立加锁。
/// </summary>
public static class ImportPipeline
{
    /* Library 目录常量 */
    private static NPath s_libraryRoot;
    private static NPath s_importedRoot => s_libraryRoot.Combine("Imported");
    private static NPath s_thumbnailRoot => s_libraryRoot.Combine("Thumbnails");

    private static readonly object s_lock = new();

    /* 导入状态缓存（复用 AssetWatcher 的 ImportStateCache） */
    private static readonly ImportStateCache s_stateCache = new();

    /* 依赖图 */
    private static readonly DependencyGraph s_dependencyGraph = new();

    /* ========== 事件 ========== */

    /// <summary>单个资产导入完成时触发。</summary>
    public static event Action<ImportProgressInfo>? OnProgress;

    /// <summary>批量导入完成时触发。</summary>
    public static event Action<ImportSummary>? OnBatchComplete;

    /* ========== 初始化 ========== */

    /// <summary>设置 Library 根目录。</summary>
    public static void Initialize(NPath libraryRoot)
    {
        lock (s_lock)
        {
            s_libraryRoot = libraryRoot;

            /* 尝试加载增量导入缓存 */
            var cachePath = GetStateCachePath();
            s_stateCache.Load(cachePath);

            /* 尝试加载依赖图缓存 */
            s_dependencyGraph.Load(GetDependencyCachePath().FullPath);

            /* 确保目录存在 */
            EnsureDirectory(s_importedRoot);
            EnsureDirectory(s_thumbnailRoot);
        }
    }

    /// <summary>是否已初始化。</summary>
    public static bool IsInitialized
    {
        get { lock (s_lock) return !s_libraryRoot.IsEmpty; }
    }

    /* ========== 单资产导入 ========== */

    /// <summary>导入单个 source asset（完整 8 阶段流程）。</summary>
    public static ImportResult Import(NPath sourceAssetPath)
    {
        lock (s_lock)
        {
            EnsureInitialized();
            ImporterRegistry.Discover();

            var virtualPath = ResolveVirtualPath(sourceAssetPath);
            var result = ImportInternal(sourceAssetPath, virtualPath);
            return result;
        }
    }

    /* ========== 批量导入 ========== */

    /// <summary>批量导入目录下所有资产。</summary>
    public static ImportSummary ImportDirectory(NPath directoryPath, bool recursive = true)
    {
        lock (s_lock)
        {
            EnsureInitialized();
            ImporterRegistry.Discover();

            var summary = new ImportSummary();

            if (!Directory.Exists(directoryPath.FullPath))
            {
                summary.Errors.Add($"目录不存在: {directoryPath}");
                return summary;
            }

            var searchOption = recursive ? SearchOption.AllDirectories : SearchOption.TopDirectoryOnly;
            var files = Directory.EnumerateFiles(directoryPath.FullPath, "*.*", searchOption);

            foreach (var file in files)
            {
                /* 跳过 .meta 文件 */
                if (file.EndsWith(".meta", StringComparison.OrdinalIgnoreCase))
                    continue;

                /* 跳过非支持格式 */
                var ext = Path.GetExtension(file).ToLowerInvariant();
                if (!ImporterRegistry.IsSupported(ext))
                    continue;

                var filePath = new NPath(file);
                var result = Import(filePath);
                summary.TotalFiles++;

                if (result.Success)
                    summary.ImportedFiles++;
                else
                {
                    summary.FailedFiles++;
                    summary.Errors.Add($"{file}: {result.ErrorMessage}");
                }

                OnProgress?.Invoke(new ImportProgressInfo
                {
                    CurrentFile = file,
                    TotalFiles = summary.TotalFiles,
                    ImportedFiles = summary.ImportedFiles,
                    FailedFiles = summary.FailedFiles
                });
            }

            /* 保存增量导入缓存 */
            s_stateCache.Save(GetStateCachePath());
            s_dependencyGraph.Save(GetDependencyCachePath().FullPath);

            OnBatchComplete?.Invoke(summary);
            return summary;
        }
    }

    /// <summary>
    /// 并行目录导入。Phase 1-4（Meta/ChangeDetection/Import/Compile）并行执行，
    /// Phase 5-8（Database/Dependency/Thumbnail/Notification）顺序执行。
    ///
    /// @threadsafe 全局锁仅保护共享状态更新，并行阶段无锁。
    /// </summary>
    public static ImportSummary ImportDirectoryParallel(NPath directoryPath, bool recursive = true)
    {
        EnsureInitialized();
        ImporterRegistry.Discover();

        /* 收集支持的文件列表 */
        if (!Directory.Exists(directoryPath.FullPath))
        {
            var err = new ImportSummary();
            err.Errors.Add($"目录不存在: {directoryPath}");
            return err;
        }

        var searchOption = recursive ? SearchOption.AllDirectories : SearchOption.TopDirectoryOnly;
        var files = Directory.EnumerateFiles(directoryPath.FullPath, "*.*", searchOption)
            .Where(f => !f.EndsWith(".meta", StringComparison.OrdinalIgnoreCase))
            .Where(f => ImporterRegistry.IsSupported(Path.GetExtension(f)))
            .ToList();

        /* Phase 1-4 并行执行 */
        var results = new ConcurrentBag<(string File, ImportResult Result)>();

        Parallel.ForEach(files, file =>
        {
            try
            {
                var result = ImportPhase1To4(new NPath(file));
                results.Add((file, result));
            }
            catch (Exception ex)
            {
                results.Add((file, ImportResult.Fail($"并行导入异常: {ex.Message}")));
            }
        });

        /* Phase 5-8 顺序执行（保护共享状态） */
        lock (s_lock)
        {
            var summary = new ImportSummary();

            foreach (var (file, result) in results)
            {
                summary.TotalFiles++;

                if (!result.Success)
                {
                    summary.FailedFiles++;
                    summary.Errors.Add($"{file}: {result.ErrorMessage}");
                    continue;
                }

                summary.ImportedFiles++;

                var filePath = new NPath(file);
                var virtualPath = ResolveVirtualPath(filePath);
                var assetGuid = result.AssetGuid;

                /* Phase 5: Database 更新 */
                EditorAssetDatabase.Register(virtualPath, assetGuid, result.TypeId);
                EditorAssetDatabase.SetSourcePath(virtualPath, filePath);

                var contentHash = ComputeContentHash(filePath);
                s_stateCache.MarkImported(filePath, contentHash);

                /* Phase 6: Dependency 更新 */
                s_dependencyGraph.SetDependencies(assetGuid, result.Dependencies);

                if (s_dependencyGraph.HasCycle(assetGuid))
                    Console.WriteLine($"[ImportPipeline] 检测到依赖环: {assetGuid.ToUuidString()}");

                var dirtyAssets = s_dependencyGraph.GetDirtyPropagation(assetGuid);
                foreach (var dirtyGuid in dirtyAssets)
                {
                    if (dirtyGuid != assetGuid)
                        EditorAssetDatabase.MarkDirty(dirtyGuid);
                }

                /* Phase 7: Thumbnail */
                if (result.ThumbnailData != null && result.ThumbnailData.Length > 0)
                {
                    var thumbDir = s_thumbnailRoot;
                    EnsureDirectory(thumbDir);
                    var thumbPath = thumbDir.Combine(assetGuid.ToHexString() + ".png");
                    File.WriteAllBytes(thumbPath.FullPath, result.ThumbnailData);
                }

                /* Phase 8: 通知 */
                EditorAssetDatabase.MarkDirty(assetGuid);

                OnProgress?.Invoke(new ImportProgressInfo
                {
                    CurrentFile = file,
                    TotalFiles = summary.TotalFiles,
                    ImportedFiles = summary.ImportedFiles,
                    FailedFiles = summary.FailedFiles
                });
            }

            s_stateCache.Save(GetStateCachePath());
            s_dependencyGraph.Save(GetDependencyCachePath().FullPath);

            OnBatchComplete?.Invoke(summary);
            return summary;
        }
    }

    /// <summary>执行 Phase 1-4（Meta/ChangeDetection/Import/Compile），可并行调用。</summary>
    private static ImportResult ImportPhase1To4(NPath sourceAssetPath)
    {
        /* Phase 1: Meta 处理 */
        var importerName = MetaFileManager.InferImporterName(sourceAssetPath.Extension);
        var meta = MetaFileManager.GetOrCreateMeta(sourceAssetPath, importerName);

        if (meta == null)
            return ImportResult.Fail("无法创建或读取 .meta 文件");

        var assetGuid = meta.Guid;

        /* Phase 2: 变化检测 */
        var outputDir = GetImportedDir(assetGuid);
        var outputPath = GetImportedPath(assetGuid);

        if (!s_stateCache.HasChanged(sourceAssetPath) && File.Exists(outputPath.FullPath))
            return ImportResult.Ok(assetGuid, 0);

        /* Phase 3: Import */
        var importer = ImporterRegistry.GetImporter(meta.Importer)
            ?? ImporterRegistry.GetImporter("DefaultImporter")
            ?? new DefaultImporter();

        var virtualPath = ResolveVirtualPath(sourceAssetPath);
        var context = new AssetImportContext
        {
            SourceAssetPath = sourceAssetPath,
            MetaFilePath = MetaFileManager.GetMetaPath(sourceAssetPath),
            AssetGuid = assetGuid,
            VirtualPath = virtualPath,
            OutputPath = outputPath,
            LibraryRoot = s_libraryRoot,
            ImportSettings = meta.ImportSettings
        };

        var importResult = importer.Import(context);

        if (!importResult.Success)
            return importResult;

        importResult.AssetGuid = assetGuid;
        importResult.Dependencies.AddRange(context.Dependencies.Dependencies);

        /* Phase 4: Compile（原子写入） */
        EnsureDirectory(outputDir);
        CompileToNnassetAtomic(importResult, outputPath);

        return importResult;
    }

    /// <summary>原子写入 .nnasset（先写 .tmp 再 rename）。</summary>
    private static void CompileToNnassetAtomic(ImportResult result, NPath outputPath)
    {
        var tmpPath = new NPath(outputPath.FullPath + ".tmp");
        CompileToNnasset(result, tmpPath);

        if (File.Exists(outputPath.FullPath))
            File.Delete(outputPath.FullPath);
        File.Move(tmpPath.FullPath, outputPath.FullPath);
    }

    /// <summary>仅导入已变化的资产（增量导入）。</summary>
    public static ImportSummary ImportChanged(NPath directoryPath, IEnumerable<NPath> changedFiles)
    {
        lock (s_lock)
        {
            EnsureInitialized();
            ImporterRegistry.Discover();

            var summary = new ImportSummary();

            foreach (var file in changedFiles)
            {
                var ext = file.Extension.ToLowerInvariant();
                if (!ImporterRegistry.IsSupported(ext))
                    continue;

                /* 增量检测 */
                if (!s_stateCache.HasChanged(file))
                {
                    summary.SkippedFiles++;
                    continue;
                }

                var result = Import(file);
                summary.TotalFiles++;

                if (result.Success)
                    summary.ImportedFiles++;
                else
                {
                    summary.FailedFiles++;
                    summary.Errors.Add($"{file}: {result.ErrorMessage}");
                }
            }

            s_stateCache.Save(GetStateCachePath());
            s_dependencyGraph.Save(GetDependencyCachePath().FullPath);
            return summary;
        }
    }

    /// <summary>强制重新导入（忽略增量缓存）。</summary>
    public static ImportResult Reimport(NPath sourceAssetPath)
    {
        lock (s_lock)
        {
            s_stateCache.Remove(sourceAssetPath);
            return Import(sourceAssetPath);
        }
    }

    /* ========== 缓存访问 ========== */

    /// <summary>获取增量导入状态缓存（供 AssetWatcher 集成）。</summary>
    public static ImportStateCache StateCache => s_stateCache;

    /// <summary>获取依赖图。</summary>
    public static DependencyGraph DependencyGraph => s_dependencyGraph;

    /* ========== 内部实现：8 阶段流程 ========== */

    private static ImportResult ImportInternal(NPath sourceAssetPath, NVirtualPath virtualPath)
    {
        try
        {
            /* ========== Phase 1: Meta 处理 ========== */
            var importerName = MetaFileManager.InferImporterName(sourceAssetPath.Extension);
            var meta = MetaFileManager.GetOrCreateMeta(sourceAssetPath, importerName);

            if (meta == null)
                return ImportResult.Fail("无法创建或读取 .meta 文件");

            var assetGuid = meta.Guid;

            /* ========== Phase 2: 变化检测 ========== */
            var outputDir = GetImportedDir(assetGuid);
            var outputPath = GetImportedPath(assetGuid);

            if (!s_stateCache.HasChanged(sourceAssetPath) && File.Exists(outputPath.FullPath))
            {
                /* 未变化，跳过导入 */
                return ImportResult.Ok(assetGuid, 0);
            }

            /* ========== Phase 3: Import ========== */
            var importer = ImporterRegistry.GetImporter(meta.Importer);
            if (importer == null)
            {
                /* 降级到 DefaultImporter */
                importer = ImporterRegistry.GetImporter("DefaultImporter")
                    ?? new DefaultImporter();
            }

            var context = new AssetImportContext
            {
                SourceAssetPath = sourceAssetPath,
                MetaFilePath = MetaFileManager.GetMetaPath(sourceAssetPath),
                AssetGuid = assetGuid,
                VirtualPath = virtualPath,
                OutputPath = outputPath,
                LibraryRoot = s_libraryRoot,
                ImportSettings = meta.ImportSettings
            };

            var importResult = importer.Import(context);

            if (!importResult.Success)
                return importResult;

            importResult.AssetGuid = assetGuid;

            /* 合并依赖 */
            importResult.Dependencies.AddRange(context.Dependencies.Dependencies);

            /* ========== Phase 4: Compile ========== */
            EnsureDirectory(outputDir);
            CompileToNnasset(importResult, outputPath);

            /* ========== Phase 5: Database 更新 ========== */
            EditorAssetDatabase.Register(virtualPath, assetGuid, importResult.TypeId);
            EditorAssetDatabase.SetSourcePath(virtualPath, sourceAssetPath);

            /* 更新增量导入缓存 */
            var contentHash = ComputeContentHash(sourceAssetPath);
            s_stateCache.MarkImported(sourceAssetPath, contentHash);

            /* ========== Phase 6: Dependency 更新 ========== */
            s_dependencyGraph.SetDependencies(assetGuid, importResult.Dependencies);

            /* 环检测警告 */
            if (s_dependencyGraph.HasCycle(assetGuid))
            {
                Console.WriteLine($"[ImportPipeline] 检测到依赖环: {assetGuid.ToUuidString()}");
            }

            /* 脏传播：标记所有反向依赖为脏 */
            var dirtyAssets = s_dependencyGraph.GetDirtyPropagation(assetGuid);
            foreach (var dirtyGuid in dirtyAssets)
            {
                if (dirtyGuid != assetGuid)
                    EditorAssetDatabase.MarkDirty(dirtyGuid);
            }

            /* ========== Phase 7: Thumbnail ========== */
            if (importResult.ThumbnailData != null && importResult.ThumbnailData.Length > 0)
            {
                var thumbDir = s_thumbnailRoot;
                EnsureDirectory(thumbDir);
                var thumbPath = thumbDir.Combine(assetGuid.ToHexString() + ".png");
                File.WriteAllBytes(thumbPath.FullPath, importResult.ThumbnailData);
            }

            /* ========== Phase 8: 通知 ========== */
            EditorAssetDatabase.MarkDirty(assetGuid);

            return importResult;
        }
        catch (Exception ex)
        {
            return ImportResult.Fail($"导入管线异常: {ex.Message}");
        }
    }

    /* ========== .nnasset 编译器（C# 版） ========== */

    /// <summary>
    /// 将 ImportResult 序列化为 .nnasset 二进制格式。
    ///
    /// .nnasset 布局（与 C++ NNAssetFormat.h 一致）：
    ///   [Header: 96 bytes]
    ///   [Dependency GUIDs: dependencyCount × 16 bytes]
    ///   [Padding to 64-byte alignment]
    ///   [Blob Descriptors: blobCount × 32 bytes]
    ///   [Padding to 64-byte alignment]
    ///   [Payload: 连续 blob 数据，每个 blob 64-byte 对齐]
    /// </summary>
    private static void CompileToNnasset(ImportResult result, NPath outputPath)
    {
        using var ms = new MemoryStream();
        using var w = new BinaryWriter(ms, Encoding.UTF8, leaveOpen: true);

        var depCount = result.Dependencies.Count;
        var blobCount = result.Blobs.Count;

        /* 计算各段偏移 */
        var headerSize = 96;
        var depSectionSize = depCount * 16;
        var blobTableSize = blobCount * 32;

        var depOffset = (long)headerSize;
        var blobTableOffset = Align64(depOffset + depSectionSize);
        var payloadOffset = Align64(blobTableOffset + blobTableSize);

        /* 计算 payload 总大小 */
        long payloadSize = 0;
        foreach (var blob in result.Blobs)
            payloadSize += Align64(blob.Payload.Length);

        /* ====== Header (96 bytes) ====== */
        w.Write(0x4E4E4153u);          /* magic: "NNAS" */
        w.Write(1u);                   /* version */
        w.Write(result.AssetGuid.High); /* assetGuid.high */
        w.Write(result.AssetGuid.Low);  /* assetGuid.low */
        w.Write(result.TypeId);         /* typeId */
        w.Write(depCount);             /* dependencyCount */
        w.Write(blobCount);            /* blobCount */
        w.Write(depOffset);            /* dependencyOffset */
        w.Write(blobTableOffset);      /* blobTableOffset */
        w.Write(payloadOffset);        /* payloadOffset */
        w.Write(payloadSize);          /* payloadSize */

        /* flags */
        uint flags = 0;
        if (result.TypeInfo != null)
            flags |= 0x8; /* HAS_TYPE_INFO */
        w.Write(flags);

        /* 填充到 64 字节 */
        PadTo(w, headerSize);

        /* ====== Dependency GUIDs ====== */
        foreach (var depGuid in result.Dependencies)
        {
            w.Write(depGuid.High);
            w.Write(depGuid.Low);
        }

        /* 对齐到 64 字节 */
        PadTo(w, blobTableOffset);

        /* ====== Blob Descriptors ====== */
        long blobPayloadOffset = payloadOffset;
        foreach (var blob in result.Blobs)
        {
            var blobData = blob.Payload;
            var blobSize = blobData.Length;
            var compressedSize = blob.CompressedData != null ? blob.CompressedData.Length : 0;

            w.Write(blobPayloadOffset);   /* offset */
            w.Write(blobSize);            /* size */
            w.Write(compressedSize);      /* compressedSize */
            w.Write(blob.BlobType);       /* blobType */
            w.Write(blob.Flags);          /* flags */

            /* 填充 descriptor 到 32 字节 */
            PadTo(w, (int)(ms.Position + 12)); /* 32 - 20 = 12 bytes padding */
            for (int p = 0; p < 12; p++) w.Write((byte)0);

            blobPayloadOffset += Align64(blobSize);
        }

        /* 对齐到 64 字节 */
        PadTo(w, payloadOffset);

        /* ====== Payload ====== */
        foreach (var blob in result.Blobs)
        {
            var blobData = blob.Payload;
            w.Write(blobData);
            /* 64-byte 对齐 */
            var aligned = Align64(blobData.Length);
            var pad = aligned - blobData.Length;
            for (int p = 0; p < pad; p++) w.Write((byte)0);
        }

        /* 写入文件 */
        var dir = Path.GetDirectoryName(outputPath.FullPath);
        if (!string.IsNullOrEmpty(dir) && !Directory.Exists(dir))
            Directory.CreateDirectory(dir);

        File.WriteAllBytes(outputPath.FullPath, ms.ToArray());
    }

    /* ========== 工具方法 ========== */

    /// <summary>将源文件路径解析为虚拟路径。</summary>
    private static NVirtualPath ResolveVirtualPath(NPath sourceAssetPath)
    {
        /* NPath → 相对于 Library 的虚拟路径 */
        return new NVirtualPath($"/assets/{sourceAssetPath.FileName}");
    }

    private static long Align64(long offset)
    {
        return (offset + 63) & ~63L;
    }

    private static int Align64(int size)
    {
        return (size + 63) & ~63;
    }

    private static void PadTo(BinaryWriter w, long targetPosition)
    {
        while (w.BaseStream.Position < targetPosition)
            w.Write((byte)0);
    }

    private static NPath GetImportedDir(GUID guid)
    {
        var hex = guid.ToHexString();
        var prefix = hex[..2];
        return s_importedRoot.Combine(prefix);
    }

    private static NPath GetImportedPath(GUID guid)
    {
        return GetImportedDir(guid).Combine(guid.ToHexString() + ".nnasset");
    }

    private static NPath GetStateCachePath()
    {
        return s_libraryRoot.Combine("Cache/ImportState.cache");
    }

    private static NPath GetDependencyCachePath()
    {
        return s_libraryRoot.Combine("Cache/Dependency.cache");
    }

    internal static byte[] ComputeContentHash(NPath filePath)
    {
        try
        {
            using var stream = File.OpenRead(filePath.FullPath);
            return SHA256.HashData(stream);
        }
        catch
        {
            return Array.Empty<byte>();
        }
    }

    private static void EnsureDirectory(NPath dir)
    {
        if (!dir.IsEmpty && !Directory.Exists(dir.FullPath))
            Directory.CreateDirectory(dir.FullPath);
    }

    private static void EnsureInitialized()
    {
        if (s_libraryRoot.IsEmpty)
            throw new InvalidOperationException("ImportPipeline 未初始化。请先调用 Initialize(libraryRoot)。");
    }
}

/* ========== 数据模型 ========== */

/// <summary>导入进度信息。</summary>
public sealed class ImportProgressInfo
{
    public string CurrentFile { get; init; } = string.Empty;
    public int TotalFiles { get; init; }
    public int ImportedFiles { get; init; }
    public int FailedFiles { get; init; }
    public float Progress => TotalFiles > 0 ? (float)(ImportedFiles + FailedFiles) / TotalFiles : 0f;
}

/// <summary>批量导入结果摘要。</summary>
public sealed class ImportSummary
{
    public int TotalFiles { get; set; }
    public int ImportedFiles { get; set; }
    public int FailedFiles { get; set; }
    public int SkippedFiles { get; set; }
    public List<string> Errors { get; } = new();

    public bool HasErrors => Errors.Count > 0;

    public override string ToString()
    {
        return $"导入完成: {ImportedFiles}/{TotalFiles} 成功, {FailedFiles} 失败, {SkippedFiles} 跳过";
    }
}

/// <summary>
/// 默认导入器 — 当没有匹配的 importer 时使用。
/// 将文件作为原始二进制数据导入。
/// </summary>
public class DefaultImporter : IAssetImporter
{
    public string[] SupportedExtensions => Array.Empty<string>();
    public string DisplayName => "Default Importer";

    public ImportResult Import(AssetImportContext context)
    {
        var result = ImportResult.Ok(context.AssetGuid, 0);
        result.Blobs.Add(new ImportedBlob
        {
            BlobType = AssetTypeId.BlobType.Data,
            Data = context.ReadAllBytes()
        });
        return result;
    }
}
