using Hexa.NET.ImGui;
using System.Numerics;

namespace Neverness.Editor.ImGuiEx;

public struct StyleColor : IDisposable
{
    public StyleColor(ImGuiCol idx, Vector4 col) => ImGui.PushStyleColor(idx, col);
    public void Dispose() => ImGui.PopStyleColor();
}

public struct StyleVar : IDisposable
{
    public StyleVar(ImGuiStyleVar idx, Vector2 col) => ImGui.PushStyleVar(idx, col);
    public void Dispose() => ImGui.PopStyleVar();
}
