using System.Collections.Concurrent;

namespace Neverness.Editor.Framework.Private.Core;

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
        _projectDirectory = path;

        OpenDirectory(_projectDirectory);

        RefreshDirectoryTreeRoot();
    }

    ////////////////////////////////////////////////////////////////
    /// Private Fields
    ////////////////////////////////////////////////////////////////

    private readonly object _refreshLock = new();

    private readonly string _projectDirectory;

    private string _prevDirectory = string.Empty;

    private string _currentDirectory = string.Empty;

    private ContentDirectory _directoryTreeRootNode = new ();

    private ContentDirectory _currentDirectoryNode = new();

    private bool _isRefreshed;

    ////////////////////////////////////////////////////////////////
    /// Public API
    ////////////////////////////////////////////////////////////////

    /// <summary>
    /// Refresh current directory
    /// </summary>
    public void RefreshDirectory()
    {
        // TODO:
        // VFSHost.RebuildNativeFilesystem(...)
        // Runtime VFS rebuild integration

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
            AbsolutePath = path,
        };

        foreach (var entry in Directory.EnumerateFileSystemEntries(path))
        {
            if (Directory.Exists(entry))
            {
                var dir = new ContentDirectory
                {
                    Name = System.IO.Path.GetFileName(entry),
                    AbsolutePath = entry,
                    Path = GetResourcePathVFS(entry),

                    // TODO:
                    // ImGui tree flags abstraction
                    UIFlags = 0,
                };

                _currentDirectoryNode.Directories.Add(dir);
            }
            else
            {
                var file = new ContentFile
                {
                    Name = System.IO.Path.GetFileNameWithoutExtension(entry),

                    AbsolutePath = entry,

                    Extension = System.IO.Path.GetExtension(entry),

                    Path = GetResourcePathVFS(entry),

                    UIFlags = 0,
                };

                // TODO:
                // AssetDatabase.GetMetadata(...)
                // AssetType resolve
                // MetaData loading

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
                if (Directory.Exists(item.AbsolutePath))
                {
                    Directory.Delete(item.AbsolutePath, true);
                }

                RefreshDirectory();
                RefreshDirectoryTreeRoot();

                return;
            }

            if (File.Exists(item.AbsolutePath))
            {
                File.Delete(item.AbsolutePath);
            }

            // TODO:
            // AssetDatabase.RemoveAsset(item.Path)

            RefreshDirectory();
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
                var parent = Directory.GetParent(item.AbsolutePath);

                if (parent == null)
                    return;

                var newPath = System.IO.Path.Combine(parent.FullName, name);

                Directory.Move(item.AbsolutePath, newPath);
            }
            else
            {
                var parent = Directory.GetParent(item.AbsolutePath);

                if (parent == null)
                    return;

                var newPath = System.IO.Path.Combine(
                    parent.FullName,
                    name + item.Extension);

                File.Move(item.AbsolutePath, newPath);

                // TODO:
                // Move .meta file
                // AssetDatabase rename
            }

            RefreshDirectory();
            RefreshDirectoryTreeRoot();
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
            // Windows
            if (OperatingSystem.IsWindows())
            {
                System.Diagnostics.Process.Start(
                    "explorer.exe",
                    $"\"{path}\"");
            }

            // TODO:
            // Linux/macOS support
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
            AbsolutePath = _projectDirectory,
            UIFlags = 0,
        };

        RefreshDirectoryTree(
            _directoryTreeRootNode,
            _projectDirectory);
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
            if (ExistChildDirectory(node, dir))
                continue;

            var child = new ContentDirectory
            {
                Name = System.IO.Path.GetFileName(dir),

                AbsolutePath = dir,

                Path = GetResourcePathVFS(dir),

                UIFlags = 0,
            };

            node.Directories.Add(child);
        }

        foreach (var subChild in node.Directories)
        {
            RefreshDirectoryTree(
                subChild,
                subChild.AbsolutePath);
        }
    }

    public bool ExistChildDirectory(
        ContentDirectory node,
        string path)
    {
        foreach (var child in node.Directories)
        {
            if (child.AbsolutePath == path)
            {
                return true;
            }
        }

        return false;
    }

    ////////////////////////////////////////////////////////////////
    /// Getter
    ////////////////////////////////////////////////////////////////

    public string GetProjectDirectory()
    {
        return _projectDirectory;
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
    private static string GetResourcePathVFS(string absolutePath)
    {
        // TODO:
        // Migrate from RuntimeCore
        // ProjectPaths + VFSHost

        return absolutePath;
    }
}

