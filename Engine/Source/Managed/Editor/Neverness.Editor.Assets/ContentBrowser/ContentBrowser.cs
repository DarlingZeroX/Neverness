using Neverness.Editor.Assets;
using Neverness.Editor.Framework.Public;
using Neverness.Editor.ProjectSystem.Public;
using Neverness.Runtime.Assets;
using Neverness.Runtime.Engine;
using Neverness.Runtime.VFS.Public;
using System.Collections.Concurrent;
using System.Runtime.InteropServices.JavaScript;

namespace Neverness.Editor.Assets.Private.Core;

/// <summary>
/// Content Browser
/// </summary>
public sealed class ContentBrowser
{
    ////////////////////////////////////////////////////////////////
    /// Singleton
    ////////////////////////////////////////////////////////////////

    public static ContentBrowser? Instance { get; private set; }

    public static void Create(string path)
    {
        if (Instance != null)
            return;

        Instance = new ContentBrowser(path);
    }

    ////////////////////////////////////////////////////////////////
    /// Constructor
    ////////////////////////////////////////////////////////////////

    public ContentBrowser(string path)
    {
        _assetDirectory = path;

        OpenDirectory(_assetDirectory);

        RefreshDirectoryTreeRoot();
    }

    ////////////////////////////////////////////////////////////////
    /// Private Fields
    ////////////////////////////////////////////////////////////////

    private readonly object _refreshLock = new();

    private readonly string _assetDirectory;

    private string _prevDirectory = string.Empty;

    private string _currentDirectory = string.Empty;

    private ContentDirectory _directoryTreeRootNode = new ();

    private ContentDirectory _currentDirectoryNode = new();

    private bool _isRefreshed;

    ////////////////////////////////////////////////////////////////
    /// Events
    ////////////////////////////////////////////////////////////////

    /// <summary>
    /// 内容发生变化时触发（create / delete / rename）。
    /// 仅由 mutating 操作触发，RefreshDirectory（Query）不触发。
    /// </summary>
    public event Action? ContentChanged;

    /// <summary>
    /// 通知外部调用方触发 ContentChanged（供外部 mutate 操作使用）。
    /// </summary>
    public void NotifyContentChanged()
    {
        ContentChanged?.Invoke();
    }

    ////////////////////////////////////////////////////////////////
    /// Public API
    ////////////////////////////////////////////////////////////////

    /// <summary>
    /// Refresh current directory（纯 Query，不触发 ContentChanged）
    /// </summary>
    public void RefreshDirectory()
    {
        // TODO:
        // VFSHost.RebuildNativeFilesystem(...)
        // Runtime VFS rebuild integration
        VFS.RebuildNativeFileSystemFiles(_currentDirectory);

        _isRefreshed = true;

        OpenDirectory(_currentDirectory);
    }

    /// <summary>
    /// Open directory
    /// </summary>
    public void OpenDirectory(string path)
    {
        if (!Directory.Exists(path))
        {
            // TODO:
            // Logger
            return;
        }

        _prevDirectory = _currentDirectory;

        _currentDirectory = path;

        _currentDirectoryNode = new ContentDirectory
        {
            Name = System.IO.Path.GetFileName(path),
            SystemPath = new NPath(path),
        };

        foreach (var entry in Directory.EnumerateFileSystemEntries(path))
        {
            if (Directory.Exists(entry))
            {
                var dir = new ContentDirectory
                {
                    Name = System.IO.Path.GetFileName(entry),
                    SystemPath = new NPath(entry),
                    AssetPath = GetResourcePathVFS(entry),
                    Icon = EditorIcons.GetTexture(EditorIcons.Folder),

                    // TODO:
                    // ImGui tree flags abstraction
                    UIFlags = 0,
                };

                _currentDirectoryNode.Directories.Add(dir);
            }
            else
            {
                var entryNPath = new NPath(entry);
                var meta = MetaFileManager.ReadMeta(entryNPath);

                if(entryNPath.Extension == ".meta")
                    continue;

                var file = new ContentFile
                {
                    Name = System.IO.Path.GetFileNameWithoutExtension(entry),
                    SystemPath = entryNPath,
                    Extension = System.IO.Path.GetExtension(entry),
                    AssetPath = GetResourcePathVFS(entry),
                    AssetType = meta?.Importer
                        ?? MetaFileManager.InferImporterName(
                            System.IO.Path.GetExtension(entry)),
                    Icon = EditorIcons.GetTexture(EditorIcons.File),
                    UIFlags = 0,
                };

                EditorAssetDatabase.TryGetGuid(file.AssetPath, out file.AssetGuid);

                _currentDirectoryNode.Files.Add(file);
            }
        }
    }

    /// <summary>
    /// Delete file or directory
    /// </summary>
    public void DeleteDirectoryItem(ContentItem item)
    {
        try
        {
            if (item is ContentDirectory)
            {
                if (Directory.Exists(item.SystemPath.FullPath))
                {
                    Directory.Delete(item.SystemPath.FullPath, true);
                }

                RefreshDirectory();
                RefreshDirectoryTreeRoot();

                ContentChanged?.Invoke();
                return;
            }

            if (File.Exists(item.SystemPath.FullPath))
            {
                File.Delete(item.SystemPath.FullPath);
            }

            if (File.Exists(item.SystemPath.FullPath + ".meta"))
            {
                File.Delete(item.SystemPath.FullPath + ".meta");
            }

            // TODO:
            // AssetDatabase.RemoveAsset(item.NVirtualPath)

            RefreshDirectory();

            ContentChanged?.Invoke();
        }
        catch (Exception ex)
        {
            // TODO:
            // Logger
            Console.WriteLine(ex);
        }
    }

    /// <summary>
    /// Rename file or directory
    /// </summary>
    public void RenameDirectoryItem(ContentItem item, string name)
    {
        try
        {
            if (item is ContentDirectory)
            {
                var parent = Directory.GetParent(item.SystemPath.FullPath);

                if (parent == null)
                    return;

                var newPath = System.IO.Path.Combine(parent.FullName, name);

                Directory.Move(item.SystemPath.FullPath, newPath);
            }
            else
            {
                var parent = Directory.GetParent(item.SystemPath.FullPath);

                if (parent == null)
                    return;

                var newPath = System.IO.Path.Combine(
                    parent.FullName,
                    name + item.Extension);

                File.Move(item.SystemPath.FullPath, newPath);

                // TODO:
                // Move .meta file
                // AssetDatabase rename
            }

            RefreshDirectory();
            RefreshDirectoryTreeRoot();

            ContentChanged?.Invoke();
        }
        catch (Exception ex)
        {
            // TODO:
            // Logger
            Console.WriteLine(ex);
        }
    }

    /// <summary>
    /// Recursive delete
    /// </summary>
    public void DeleteDirectoryItemRecursion(string path)
    {
        try
        {
            if (Directory.Exists(path))
            {
                Directory.Delete(path, true);
            }
        }
        catch (Exception ex)
        {
            // TODO:
            // Logger
            Console.WriteLine(ex);
        }
    }

    /// <summary>
    /// Show in system explorer
    /// </summary>
    public void ShowInExplorer(string path)
    {
        try
        {
            if (OperatingSystem.IsWindows())
            {
                // 规范化路径：去尾部分隔符、统一反斜杠
                var normalized = path.TrimEnd('/', '\\').Replace('/', '\\');

                if (File.Exists(normalized))
                {
                    // 文件：打开所在目录并选中该文件
                    System.Diagnostics.Process.Start("explorer.exe", $"/select,\"{normalized}\"");
                }
                else
                {
                    // 目录（或不存在的路径）：直接打开
                    System.Diagnostics.Process.Start("explorer.exe", $"\"{normalized}\"");
                }
            }
            // TODO: Linux/macOS support
        }
        catch (Exception ex)
        {
            Console.WriteLine(ex);
        }
    }

    /// <summary>
    /// Create new directory
    /// </summary>
    public void CreateNewDirectory(string path)
    {
        try
        {
            var newFolder = System.IO.Path.Combine(path, "New Folder");

            int index = 1;

            while (Directory.Exists(newFolder))
            {
                newFolder = System.IO.Path.Combine(
                    path,
                    $"New Folder {index++}");
            }

            Directory.CreateDirectory(newFolder);

            RefreshDirectory();
            RefreshDirectoryTreeRoot();

            ContentChanged?.Invoke();
        }
        catch (Exception ex)
        {
            Console.WriteLine(ex);
        }
    }

    /// <summary>
    /// Copy path to clipboard
    /// </summary>
    public void CopyPath(string path)
    {
        // TODO:
        // Clipboard abstraction layer
        // SDL / Win32 / Avalonia / GLFW
    }

    /// <summary>
    /// Select all items
    /// </summary>
    public void SelectAll(bool select)
    {
        foreach (var item in _currentDirectoryNode.Directories)
        {
            item.Selected = select;
        }

        foreach (var item in _currentDirectoryNode.Files)
        {
            item.Selected = select;
        }
    }

    ////////////////////////////////////////////////////////////////
    /// Tree
    ////////////////////////////////////////////////////////////////

    public void RefreshDirectoryTreeRoot()
    {
        _isRefreshed = true;

        _directoryTreeRootNode = new ContentDirectory
        {
            Name = "Content",
            SystemPath = new NPath(_assetDirectory),
            UIFlags = 0,
        };

        RefreshDirectoryTree(
            _directoryTreeRootNode,
            _assetDirectory);
    }

    public void RefreshDirectoryTree(
        ContentDirectory node,
        string path)
    {
        if (!Directory.Exists(path))
        {
            return;
        }

        _isRefreshed = true;

        foreach (var dir in Directory.EnumerateDirectories(path))
        {
            if (ExistChildDirectory(node, new NPath(dir)))
                continue;

            var child = new ContentDirectory
            {
                Name = System.IO.Path.GetFileName(dir),

                SystemPath = new NPath(dir),

                AssetPath = GetResourcePathVFS(dir),

                UIFlags = 0,
            };

            node.Directories.Add(child);
        }

        foreach (var subChild in node.Directories)
        {
            RefreshDirectoryTree(
                subChild,
                subChild.SystemPath.FullPath);
        }
    }

    public bool ExistChildDirectory(
        ContentDirectory node,
        NPath path)
    {
        foreach (var child in node.Directories)
        {
            if (child.SystemPath == path)
            {
                return true;
            }
        }

        return false;
    }

    ////////////////////////////////////////////////////////////////
    /// Getter
    ////////////////////////////////////////////////////////////////

    public string GetAssetDirectory()
    {
        return _assetDirectory;
    }

    public string GetPrevDirectory()
    {
        return _prevDirectory;
    }

    public string GetCurrentBrowserDirectory()
    {
        return _currentDirectory;
    }

    public ContentDirectory GetCurrentDirectoryNode()
    {
        return _currentDirectoryNode;
    }

    public ContentDirectory GetDirectoryTreeRootNode()
    {
        return _directoryTreeRootNode;
    }

    public bool IsRefreshed()
    {
        return _isRefreshed;
    }

    public void ClearRefreshedFlag()
    {
        _isRefreshed = false;
    }

    ////////////////////////////////////////////////////////////////
    /// Helpers
    ////////////////////////////////////////////////////////////////

    /// <summary>
    /// absolute path -> asset:// path
    /// </summary>
    private static NVirtualPath GetResourcePathVFS(string absolutePath)
    {
        // TODO:
        // Migrate from RuntimeCore
        // ProjectPaths + VFSHost
        return (NVirtualPath)ProjectPaths.GetResourcePath(new NPath(absolutePath));

        //return absolutePath;
    }
}

