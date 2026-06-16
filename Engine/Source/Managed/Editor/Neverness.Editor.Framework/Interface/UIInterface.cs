using Neverness.Editor.Framework.Private;
using Neverness.Editor.Framework.Private.Menu;
using Neverness.Editor.Framework.Public;
using System;
using System.Collections.Generic;

namespace Neverness.Editor.Framework.Interface
{
    // =============================================
    //  UI 基础接口
    // =============================================

    public interface IPanel
    {
        void OnUpdate(float delta);
        void OnFixedUpdate();
        bool IsAsync();
        void OnGUI();
    }

    public interface IEditorPanel : IPanel
    {
        string GetWindowFullName();
        string GetWindowName();
        void OpenWindow(bool open);
        bool IsWindowOpened();
    }

    // =============================================
    //  基础设施接口（从具体类提取，供 Core 消费）
    // =============================================

    /// <summary>面板管理器接口。</summary>
    public interface IPanelManager
    {
        bool AddPanel(IEditorPanel panel);
        bool AddPanelWithID(string id, IEditorPanel panel);
        bool RemovePanel(IEditorPanel panel);
        bool RemovePanel(string id);
        IEditorPanel? GetPanelWithID(string id);
        bool HasPanel(string id);
        bool OpenPanel(string id);
        bool ClosePanel(string id);
        void Clear();
        void OnGUI();

        /// <summary>向主窗口添加子面板（Feature 模块注册入口）。</summary>
        bool AddChildPanel(string id, IEditorPanel panel);
    }

    /// <summary>主窗口宿主接口——Shell 模块实现，供 PanelManager 委托子面板注册。</summary>
    public interface IMainWindowHost
    {
        /// <summary>向主窗口添加带 ID 的子面板。</summary>
        void AddPanelWithID(string id, IEditorPanel panel);

        /// <summary>按 ID 获取子面板。</summary>
        IEditorPanel? GetPanelWithID(string id);
    }

    /// <summary>命令注册表接口（Legacy IEditorCommand 体系）。</summary>
    public interface ICommandRegistry
    {
        void Register(IEditorCommand command);
        bool TryExecute(string commandId);
        bool Contains(string commandId);
    }

    /// <summary>菜单注册表接口。</summary>
    public interface IMenuRegistry
    {
        void Register(EditorMenuItem item);
        void RegisterAll(ReadOnlySpan<EditorMenuItem> items);
        void RegisterDynamic(string menuPath, Action<DynamicMenuBuilder> builder);
        void RegisterCommand(EditorCommand command);
        bool ExecuteCommand(string commandId);
        EditorCommand? FindCommand(string commandId);
        void RegisterContributor(IMenuContributor contributor);
        void UnregisterByPrefix(string pathPrefix);
    }

    /// <summary>上下文菜单注册表接口。</summary>
    public interface IContextMenuRegistry
    {
        void RegisterContributor(IContextMenuContributor contributor);
        void RegisterItem(string contextId, EditorMenuItem item);
        void RegisterItems(string contextId, ReadOnlySpan<EditorMenuItem> items);
        void RegisterCallback(string contextId, Action<ContextMenuBuilder> builder);
        void Clear(string contextId);
    }
}
