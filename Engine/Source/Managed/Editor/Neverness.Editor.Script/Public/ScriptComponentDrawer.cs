// ============================================================================
// ScriptComponentDrawer.cs - 脚本组件绘制器
// ============================================================================
// 在 Inspector 中绘制 Entity 上挂载的 ScriptComponent。
// ============================================================================

using Neverness.Gameplay;

namespace Neverness.Editor.Script.Public;

/// <summary>
/// 脚本组件绘制器：在 Inspector 中绘制 ScriptComponent。
/// </summary>
public sealed class ScriptComponentDrawer
{
    // ========================================================================
    // 公共方法
    // ========================================================================

    /// <summary>
    /// 绘制 Entity 的脚本组件列表。
    /// </summary>
    /// <param name="entity">Entity 实例。</param>
    public void Draw(Entity entity)
    {
        ArgumentNullException.ThrowIfNull(entity);

        var behaviours = entity.GetBehaviours();

        foreach (var behaviour in behaviours)
        {
            DrawBehaviour(behaviour);
        }

        // "添加脚本" 按钮
        DrawAddScriptButton(entity);
    }

    // ========================================================================
    // 内部方法
    // ========================================================================

    /// <summary>绘制单个 Behaviour。</summary>
    private void DrawBehaviour(EntityBehaviour behaviour)
    {
        var typeName = behaviour.GetType().Name;
        var enabled = behaviour.Enabled;

        // 折叠标题
        // ImGui.BeginGroup();
        // ImGui.Checkbox($"##enabled_{behaviour.GetHashCode()}", ref enabled);
        // ImGui.SameLine();
        // ImGui.Text(typeName);

        behaviour.Enabled = enabled;

        // 绘制字段
        var fields = ScriptInspector.GetFieldsForBehaviour(behaviour);
        foreach (var field in fields)
        {
            if (field.IsHidden)
                continue;

            // 绘制 Header
            if (!string.IsNullOrEmpty(field.Header))
            {
                // ImGui.Separator();
                // ImGui.Text(field.Header);
            }

            DrawField(behaviour, field);
        }

        // 移除按钮
        // if (ImGui.Button($"Remove##{behaviour.GetHashCode()}"))
        // {
        //     ScriptInspector.RemoveScriptComponent(behaviour.Entity, behaviour);
        // }

        // ImGui.EndGroup();
        // ImGui.Separator();
    }

    /// <summary>绘制单个字段。</summary>
    private void DrawField(EntityBehaviour behaviour, ScriptInspector.FieldInfo field)
    {
        if (!field.IsEditable)
            return;

        // 根据字段类型绘制不同的控件
        switch (field.Value)
        {
            case bool boolVal:
                // ImGui.Checkbox(field.Name, ref boolVal);
                ScriptInspector.SetFieldValue(behaviour, field.Name, boolVal);
                break;

            case int intVal:
                // ImGui.DragInt(field.Name, ref intVal);
                ScriptInspector.SetFieldValue(behaviour, field.Name, intVal);
                break;

            case float floatVal:
                // ImGui.DragFloat(field.Name, ref floatVal, 0.1f);
                ScriptInspector.SetFieldValue(behaviour, field.Name, floatVal);
                break;

            case string strVal:
                // ImGui.InputText(field.Name, ref strVal, 256);
                ScriptInspector.SetFieldValue(behaviour, field.Name, strVal);
                break;

            case Vector2 vec2Val:
                // ImGui.DragFloat2(field.Name, ref vec2Val);
                ScriptInspector.SetFieldValue(behaviour, field.Name, vec2Val);
                break;

            case Vector3 vec3Val:
                // ImGui.DragFloat3(field.Name, ref vec3Val);
                ScriptInspector.SetFieldValue(behaviour, field.Name, vec3Val);
                break;

            case Vector4 vec4Val:
                // ImGui.DragFloat4(field.Name, ref vec4Val);
                ScriptInspector.SetFieldValue(behaviour, field.Name, vec4Val);
                break;

            default:
                // 不支持的类型显示为文本
                // ImGui.Text($"{field.Name}: {field.Value}");
                break;
        }
    }

    /// <summary>绘制"添加脚本"按钮。</summary>
    private void DrawAddScriptButton(Entity entity)
    {
        // ImGui.Button("+ Add Script");
        // 点击后弹出脚本类型选择器

        // 获取所有注册的脚本类型
        var registry = GameplayContext.Current?.ScriptRegistry;
        if (registry is null)
            return;

        var allScripts = registry.GetAllScripts();
        // 绘制脚本类型列表供选择
    }
}
