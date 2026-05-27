using Hexa.NET.ImGui;
using Neverness.Editor.Framework.Interface;
using Neverness.Editor.Framework.Public;
using Neverness.Runtime.Interop;
using System;
using System.Numerics;

namespace Neverness.Editor.Scene.Private.Panel;

/// <summary>
/// Editor 视口面板：调用 Native 渲染场景，使用 ImGui.Image 显示结果。
/// </summary>
public class EditorViewport : IEditorPanel
{
    private bool m_IsOpen = true;
    private ulong m_SceneHandle = 0;

    public EditorViewport()
    {
    }

    /// 设置要渲染的场景句柄
    public void SetScene(ulong sceneHandle)
    {
        m_SceneHandle = sceneHandle;
    }

    public void OnUpdate(float delta)
    {
    }

    public void OnFixedUpdate()
    {
        throw new NotImplementedException();
    }

    public bool IsAsync()
    {
        throw new NotImplementedException();
    }

    public unsafe void OnGUI()
    {
        if (!m_IsOpen)
            return;

        // 无边距，最大化视口区域
        ImGui.PushStyleVar(ImGuiStyleVar.WindowPadding, Vector2.Zero);

        if (ImGui.Begin(GetWindowFullName(),
            ImGuiWindowFlags.NoScrollbar | ImGuiWindowFlags.NoCollapse))
        {
            // 获取 Viewport 可用区域大小
            var viewportSize = ImGui.GetContentRegionAvail();

            if (viewportSize.X > 1 && viewportSize.Y > 1 && m_SceneHandle != 0)
            {
                // 调用 Native 渲染
                ref readonly var api = ref EngineNativeApiBootstrap.EngineApi;
                if (api.ViewportRender.RenderSceneToTexture != null)
                {
                    ulong textureId = api.ViewportRender.RenderSceneToTexture(
                        m_SceneHandle,
                        (uint)viewportSize.X,
                        (uint)viewportSize.Y);

                    // 用 ImGui.Image 显示
                    if (textureId != 0)
                    {
                        // FlipY：OpenGL 纹理坐标原点在左下角，ImGui Image 原点在左上角
                        ImGui.Image(
                            new ImTextureRef(null, textureId),
                            //(nint)textureId,
                            viewportSize,
                            new Vector2(0, 1),  // UV0: 左下
                            new Vector2(1, 0)); // UV1: 右上
                    }
                }
            }
        }

        ImGui.End();
        ImGui.PopStyleVar();
    }

    public string GetWindowFullName()
    {
        return FontAwesome5Pro.Window + " " + GetWindowName();
    }

    public string GetWindowName() => "Main Viewport";

    public void OpenWindow(bool open) => m_IsOpen = open;

    public bool IsWindowOpened() => m_IsOpen;
}
