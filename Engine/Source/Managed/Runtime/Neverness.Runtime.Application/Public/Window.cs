// Neverness.Runtime.Application — 面向对象窗口封装；避免业务层直接操作 WindowHost。
// 提供：生命周期、属性式 API、IDisposable、Future Editor 多窗口友好。

using Neverness.Runtime.Application.Private;
using Neverness.Runtime.Engine;
using System;

namespace Neverness.Runtime.Application.Public;

/// <summary>
/// Runtime 窗口对象封装。
/// </summary>
public sealed class Window : IDisposable
{
    private bool _disposed;

    /// <summary>
    /// Native Runtime Window Handle。
    /// </summary>
    public NNWindowHandle Handle { get; }

    /// <summary>
    /// 是否为有效窗口。
    /// </summary>
    public bool IsValid => !_disposed && Handle.IsValid;

    /// <summary>
    /// 获取平台原生窗口句柄（HWND / NSWindow / X11）。
    /// </summary>
    public nint NativeHandle => WindowHost.GetNativeHandle(Handle);

    /// <summary>
    /// 窗口标题。
    /// </summary>
    public string Title
    {
        set
        {
            ThrowIfDisposed();
            WindowHost.SetTitle(Handle, value);
        }
    }

    /// <summary>
    /// 窗口宽度。
    /// </summary>
    public int Width
    {
        get
        {
            ThrowIfDisposed();
            return WindowHost.GetSize(Handle).Width;
        }
        set
        {
            ThrowIfDisposed();

            var (_, height) = WindowHost.GetSize(Handle);
            WindowHost.SetSize(Handle, value, height);
        }
    }

    /// <summary>
    /// 窗口高度。
    /// </summary>
    public int Height
    {
        get
        {
            ThrowIfDisposed();
            return WindowHost.GetSize(Handle).Height;
        }
        set
        {
            ThrowIfDisposed();

            var (width, _) = WindowHost.GetSize(Handle);
            WindowHost.SetSize(Handle, width, value);
        }
    }

    /// <summary>
    /// 窗口尺寸。
    /// </summary>
    public (int Width, int Height) Size
    {
        get
        {
            ThrowIfDisposed();
            return WindowHost.GetSize(Handle);
        }
        set
        {
            ThrowIfDisposed();
            WindowHost.SetSize(Handle, value.Width, value.Height);
        }
    }

    /// <summary>
    /// 窗口位置。
    /// </summary>
    public (int X, int Y) Position
    {
        get
        {
            ThrowIfDisposed();
            return WindowHost.GetPosition(Handle);
        }
        set
        {
            ThrowIfDisposed();
            WindowHost.SetPosition(Handle, value.X, value.Y);
        }
    }

    internal Window(NNWindowHandle handle)
    {
        Handle = handle;
    }

    /// <summary>
    /// 创建 Runtime Window。
    /// </summary>
    public static Window? Create(
        string title,
        int width,
        int height,
        bool resizable = true,
        bool maximized = false,
        bool hidden = false)
    {
        var handle = WindowHost.Create(
            title,
            width,
            height,
            resizable,
            maximized,
            hidden);

        if (!handle.IsValid)
        {
            return null;
        }

        return new Window(handle);
    }

    /// <summary>
    /// 显示窗口。
    /// </summary>
    public void Show()
    {
        ThrowIfDisposed();
        WindowHost.Show(Handle);
    }

    /// <summary>
    /// 隐藏窗口。
    /// </summary>
    public void Hide()
    {
        ThrowIfDisposed();
        WindowHost.Hide(Handle);
    }

    /// <summary>
    /// 最大化窗口。
    /// </summary>
    public void Maximize()
    {
        ThrowIfDisposed();
        WindowHost.Maximize(Handle);
    }

    /// <summary>
    /// 最小化窗口。
    /// </summary>
    public void Minimize()
    {
        ThrowIfDisposed();
        WindowHost.Minimize(Handle);
    }

    /// <summary>
    /// 恢复窗口。
    /// </summary>
    public void Restore()
    {
        ThrowIfDisposed();
        WindowHost.Restore(Handle);
    }

    /// <summary>
    /// 设置窗口是否可调整大小。
    /// </summary>
    public void SetResizable(bool value)
    {
        ThrowIfDisposed();
        WindowHost.SetResizable(Handle, value);
    }

    /// <summary>
    /// 销毁窗口。
    /// </summary>
    public void Destroy()
    {
        if (_disposed)
        {
            return;
        }

        WindowHost.Destroy(Handle);

        _disposed = true;

        GC.SuppressFinalize(this);
    }

    /// <summary>
    /// IDisposable。
    /// </summary>
    public void Dispose()
    {
        Destroy();
    }

    ~Window()
    {
        Destroy();
    }

    private void ThrowIfDisposed()
    {
        ObjectDisposedException.ThrowIf(_disposed, this);
    }
}
