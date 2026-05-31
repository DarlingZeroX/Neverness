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
                    // 1. 渲染场景（返回 Sprite 纹理 ID）
                    ulong spriteTextureId = api.ViewportRender.RenderSceneToTexture(
                        m_SceneHandle,
                        (uint)viewportSize.X,
                        (uint)viewportSize.Y);

                    var cursorPos = ImGui.GetCursorScreenPos();

                    // 2. 显示 Sprite 场景纹理
                    if (spriteTextureId != 0)
                    {
                        ImGui.Image(
                            new ImTextureRef(null, spriteTextureId),
                            viewportSize,
                            new Vector2(0, 1),  // UV0: 左下（OpenGL 纹理坐标）
                            new Vector2(1, 0)); // UV1: 右上
                    }

                    // 3. 叠加 RmlUI 纹理（透明叠加层）
                    if (api.ViewportRender.GetLastRmluiTexture != null)
                    {
                        ulong rmluiTextureId = api.ViewportRender.GetLastRmluiTexture();
                        if (rmluiTextureId != 0)
                        {
                            // 在同一位置叠加 RmlUI 纹理
                            var drawList = ImGui.GetWindowDrawList();
                            drawList.AddImage(
                                new ImTextureRef(null, rmluiTextureId),
                                cursorPos,
                                cursorPos + viewportSize,
                                new Vector2(0, 1),  // UV0: 左下
                                new Vector2(1, 0)); // UV1: 右上
                        }
                        else
                        {
                            Console.WriteLine("[EditorViewport] RmlUI textureId = 0");
                        }
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
