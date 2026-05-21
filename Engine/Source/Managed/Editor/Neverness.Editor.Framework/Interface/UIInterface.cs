using System;

namespace Neverness.Editor.Framework.Interface
{
    // C# 接口默认就是 public 的，不需要显式声明析构函数
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

    public interface ISidebarComponent : IPanel
    {
        void Toggle();
        void OnSideBarUI();
    }
}
