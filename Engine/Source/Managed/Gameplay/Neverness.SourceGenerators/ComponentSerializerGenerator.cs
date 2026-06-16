// ============================================================================
// ComponentSerializerGenerator.cs - 组件序列化 Source Generator
// ============================================================================
// 扫描实现 IComponent 的 struct，自动生成 JSON 序列化/反序列化代码。
// 跳过标记了 [Transient] 的字段。
// ============================================================================

using System.Collections.Generic;
using System.Collections.Immutable;
using System.Text;
using Microsoft.CodeAnalysis;
using Microsoft.CodeAnalysis.CSharp;
using Microsoft.CodeAnalysis.CSharp.Syntax;
using Microsoft.CodeAnalysis.Text;

namespace Neverness.SourceGenerators;

/// <summary>
/// 组件序列化 Source Generator。
/// </summary>
[Generator]
public class ComponentSerializerGenerator : IIncrementalGenerator
{
    // ========================================================================
    // 常量
    // ========================================================================

    /// <summary>IComponent 接口的完整名称。</summary>
    private const string IComponentFullName = "Neverness.Runtime.Scene.IComponent";

    /// <summary>Transient 特性的完整名称。</summary>
    private const string TransientAttributeFullName = "Neverness.Runtime.Engine.TransientAttribute";

    /// <summary>NNGuid 的完整名称。</summary>
    private const string NNGuidFullName = "Neverness.Runtime.Engine.NNGuid";

    // ========================================================================
    // IIncrementalGenerator
    // ========================================================================

    /// <inheritdoc/>
    public void Initialize(IncrementalGeneratorInitializationContext context)
    {
        // 1. 扫描所有 struct 声明
        var componentStructs = context.SyntaxProvider
            .CreateSyntaxProvider(
                predicate: static (node, _) => IsStructWithBaseList(node),
                transform: static (ctx, _) => GetComponentInfo(ctx))
            .Where(static info => info is not null)
            .Select(static (info, _) => info!);

        // 2. 合并
        var collected = componentStructs.Collect();

        // 3. 生成代码
        context.RegisterSourceOutput(collected, GenerateSerializer);
    }

    // ========================================================================
    // 语法过滤
    // ========================================================================

    /// <summary>检查是否是带有基类列表的 struct 声明。</summary>
    private static bool IsStructWithBaseList(SyntaxNode node)
    {
        return node is StructDeclarationSyntax structDecl
            && structDecl.BaseList is not null
            && structDecl.BaseList.Types.Count > 0;
    }

    // ========================================================================
    // 语义分析
    // ========================================================================

    /// <summary>获取组件结构体信息。</summary>
    private static ComponentInfo? GetComponentInfo(GeneratorSyntaxContext context)
    {
        var structDecl = (StructDeclarationSyntax)context.Node;
        var symbol = context.SemanticModel.GetDeclaredSymbol(structDecl) as INamedTypeSymbol;

        if (symbol is null)
            return null;

        // 检查是否实现了 IComponent
        if (!ImplementsIComponent(symbol))
            return null;

        // 枚举可序列化字段
        var fields = new List<FieldInfo>();
        foreach (var member in symbol.GetMembers())
        {
            if (member is not IFieldSymbol field)
                continue;

            // 跳过非公共字段
            if (field.DeclaredAccessibility != Accessibility.Public)
                continue;

            // 跳过 static / const
            if (field.IsStatic || field.IsConst)
                continue;

            // 跳过标记了 [Transient] 的字段
            if (HasTransientAttribute(field))
                continue;

            // 跳过 fixed 缓冲区字段
            if (field.IsFixedSizeBuffer)
                continue;

            // 跳过 Matrix4x4（计算字段，应标 [Transient]，但做兜底保护）
            if (field.Type.ToDisplayString() == "System.Numerics.Matrix4x4")
                continue;

            fields.Add(new FieldInfo
            {
                Name = field.Name,
                TypeName = field.Type.ToDisplayString(),
                IsEnum = field.Type.TypeKind == TypeKind.Enum,
                EnumUnderlyingType = field.Type.TypeKind == TypeKind.Enum
                    ? ((INamedTypeSymbol)field.Type).EnumUnderlyingType?.ToDisplayString()
                    : null,
            });
        }

        // 跳过 TagComponent 和 RelationshipComponent（名字和层级在场景顶层处理）
        if (symbol.Name == "TagComponent" || symbol.Name == "RelationshipComponent")
            return null;

        // 没有可序列化字段的组件也跳过
        if (fields.Count == 0)
            return null;

        return new ComponentInfo
        {
            Name = symbol.Name,
            FullName = symbol.ToDisplayString(SymbolDisplayFormat.FullyQualifiedFormat),
            Namespace = symbol.ContainingNamespace.IsGlobalNamespace
                ? null
                : symbol.ContainingNamespace.ToDisplayString(),
            Fields = fields,
        };
    }

    /// <summary>检查是否实现了 IComponent。</summary>
    private static bool ImplementsIComponent(INamedTypeSymbol symbol)
    {
        foreach (var iface in symbol.AllInterfaces)
        {
            if (iface.ToDisplayString() == IComponentFullName)
                return true;
        }
        return false;
    }

    /// <summary>检查是否标记了 [Transient]。</summary>
    private static bool HasTransientAttribute(IFieldSymbol field)
    {
        foreach (var attr in field.GetAttributes())
        {
            if (attr.AttributeClass?.ToDisplayString() == TransientAttributeFullName)
                return true;
        }
        return false;
    }

    // ========================================================================
    // 代码生成
    // ========================================================================

    /// <summary>生成序列化器代码。</summary>
    private static void GenerateSerializer(
        SourceProductionContext context,
        ImmutableArray<ComponentInfo> components)
    {
        if (components.Length == 0)
            return;

        var source = GenerateSource(components);
        context.AddSource("ComponentSerializer_AutoGenerated.g.cs", SourceText.From(source, Encoding.UTF8));
    }

    /// <summary>生成源代码。</summary>
    private static string GenerateSource(ImmutableArray<ComponentInfo> components)
    {
        var sb = new StringBuilder();

        // 文件头
        sb.AppendLine("// <auto-generated/>");
        sb.AppendLine("// 由 Neverness.SourceGenerators 自动生成");
        sb.AppendLine("// 请勿手动修改此文件");
        sb.AppendLine();

        // using
        sb.AppendLine("using System;");
        sb.AppendLine("using System.Numerics;");
        sb.AppendLine("using System.Text.Json;");
        sb.AppendLine("using Friflo.Engine.ECS;");
        sb.AppendLine("using Neverness.Runtime.Engine;");
        sb.AppendLine("using Neverness.Runtime.Scene.Components;");
        sb.AppendLine();

        // 命名空间和类
        sb.AppendLine("namespace Neverness.Runtime.Scene.Internal;");
        sb.AppendLine();
        sb.AppendLine("/// <summary>");
        sb.AppendLine("/// 组件序列化器（自动生成）。");
        sb.AppendLine("/// </summary>");
        sb.AppendLine("public static class ComponentSerializer");
        sb.AppendLine("{");

        // ── TryAddComponent 方法 ──
        GenerateTryAddComponent(sb, components);
        sb.AppendLine();

        // ── 每个组件的 Write 方法 ──
        foreach (var comp in components)
        {
            GenerateWriteMethod(sb, comp);
            sb.AppendLine();
        }

        // ── 每个组件的 Read 方法 ──
        foreach (var comp in components)
        {
            GenerateReadMethod(sb, comp);
            sb.AppendLine();
        }

        // ── 基础类型辅助方法 ──
        GenerateHelperMethods(sb);

        sb.AppendLine("}");

        return sb.ToString();
    }

    /// <summary>生成 TryAddComponent 分发方法。</summary>
    private static void GenerateTryAddComponent(StringBuilder sb, ImmutableArray<ComponentInfo> components)
    {
        sb.AppendLine("    /// <summary>");
        sb.AppendLine("    /// 根据类型名反序列化组件并添加到实体。");
        sb.AppendLine("    /// </summary>");
        sb.AppendLine("    /// <returns>是否成功识别并添加了组件。</returns>");
        sb.AppendLine("    public static bool TryAddComponent(Entity entity, string typeName, JsonElement elem)");
        sb.AppendLine("    {");
        sb.AppendLine("        switch (typeName)");
        sb.AppendLine("        {");

        foreach (var comp in components)
        {
            sb.AppendLine($"            case \"{comp.Name}\":");
            sb.AppendLine($"                entity.AddComponent(Read{comp.Name}(elem));");
            sb.AppendLine($"                return true;");
        }

        sb.AppendLine("            default:");
        sb.AppendLine("                return false;");
        sb.AppendLine("        }");
        sb.AppendLine("    }");
    }

    /// <summary>生成组件的 Write 方法。</summary>
    private static void GenerateWriteMethod(StringBuilder sb, ComponentInfo comp)
    {
        sb.AppendLine($"    /// <summary>序列化 {comp.Name}。</summary>");
        sb.AppendLine($"    public static void Write{comp.Name}(ref {comp.FullName} c, Utf8JsonWriter w)");
        sb.AppendLine("    {");
        sb.AppendLine($"        w.WritePropertyName(\"{comp.Name}\");");
        sb.AppendLine("        w.WriteStartObject();");

        foreach (var field in comp.Fields)
        {
            var writeCall = GetWriteCall(field, "c." + field.Name);
            sb.AppendLine($"        {writeCall}");
        }

        sb.AppendLine("        w.WriteEndObject();");
        sb.AppendLine("    }");
    }

    /// <summary>生成组件的 Read 方法。</summary>
    private static void GenerateReadMethod(StringBuilder sb, ComponentInfo comp)
    {
        sb.AppendLine($"    /// <summary>反序列化 {comp.Name}。</summary>");
        sb.AppendLine($"    public static {comp.FullName} Read{comp.Name}(JsonElement elem)");
        sb.AppendLine("    {");

        // 使用 default 初始化（struct 值类型安全）
        sb.AppendLine($"        var c = default({comp.FullName});");

        foreach (var field in comp.Fields)
        {
            var readCall = GetReadCall(field, "c." + field.Name);
            sb.AppendLine($"        if (elem.TryGetProperty(\"{CamelCase(field.Name)}\", out var {CamelCase(field.Name)}Elem))");
            sb.AppendLine($"            {readCall}");
        }

        sb.AppendLine("        return c;");
        sb.AppendLine("    }");
    }

    /// <summary>获取字段的 Write 调用代码。</summary>
    private static string GetWriteCall(FieldInfo field, string accessor)
    {
        if (field.IsEnum)
        {
            return $"w.WriteString(\"{CamelCase(field.Name)}\", {accessor}.ToString());";
        }

        switch (field.TypeName)
        {
            case "float":
                return $"w.WriteNumber(\"{CamelCase(field.Name)}\", {accessor});";
            case "double":
                return $"w.WriteNumber(\"{CamelCase(field.Name)}\", {accessor});";
            case "int":
                return $"w.WriteNumber(\"{CamelCase(field.Name)}\", {accessor});";
            case "uint":
                return $"w.WriteNumber(\"{CamelCase(field.Name)}\", {accessor});";
            case "long":
                return $"w.WriteNumber(\"{CamelCase(field.Name)}\", {accessor});";
            case "ulong":
                return $"w.WriteNumber(\"{CamelCase(field.Name)}\", {accessor});";
            case "bool":
                return $"w.WriteBoolean(\"{CamelCase(field.Name)}\", {accessor});";
            case "System.Numerics.Vector3":
                return $"WriteVector3(w, \"{CamelCase(field.Name)}\", {accessor});";
            case "System.Numerics.Quaternion":
                return $"WriteQuaternion(w, \"{CamelCase(field.Name)}\", {accessor});";
            case "Neverness.Runtime.Engine.NNGuid":
                return $"WriteNNGuid(w, \"{CamelCase(field.Name)}\", {accessor});";
            default:
                return $"// 跳过未知类型: {field.TypeName}";
        }
    }

    /// <summary>获取字段的 Read 调用代码。</summary>
    private static string GetReadCall(FieldInfo field, string accessor)
    {
        var elemName = CamelCase(field.Name) + "Elem";

        if (field.IsEnum)
        {
            var enumType = field.FullName ?? field.TypeName;
            return $"if (Enum.TryParse<{enumType}>({elemName}.GetString(), out var {CamelCase(field.Name)}Val)) {accessor} = {CamelCase(field.Name)}Val;";
        }

        switch (field.TypeName)
        {
            case "float":
                return $"{accessor} = {elemName}.GetSingle();";
            case "double":
                return $"{accessor} = {elemName}.GetDouble();";
            case "int":
                return $"{accessor} = {elemName}.GetInt32();";
            case "uint":
                return $"{accessor} = {elemName}.GetUInt32();";
            case "long":
                return $"{accessor} = {elemName}.GetInt64();";
            case "ulong":
                return $"{accessor} = {elemName}.GetUInt64();";
            case "bool":
                return $"{accessor} = {elemName}.GetBoolean();";
            case "System.Numerics.Vector3":
                return $"{accessor} = ReadVector3({elemName});";
            case "System.Numerics.Quaternion":
                return $"{accessor} = ReadQuaternion({elemName});";
            case "Neverness.Runtime.Engine.NNGuid":
                return $"{accessor} = ReadNNGuid({elemName});";
            default:
                return $"// 跳过未知类型: {field.TypeName}";
        }
    }

    /// <summary>生成基础类型辅助方法。</summary>
    private static void GenerateHelperMethods(StringBuilder sb)
    {
        // WriteVector3
        sb.AppendLine("    private static void WriteVector3(Utf8JsonWriter w, string name, Vector3 v)");
        sb.AppendLine("    {");
        sb.AppendLine("        w.WritePropertyName(name);");
        sb.AppendLine("        w.WriteStartArray();");
        sb.AppendLine("        w.WriteNumberValue(v.X);");
        sb.AppendLine("        w.WriteNumberValue(v.Y);");
        sb.AppendLine("        w.WriteNumberValue(v.Z);");
        sb.AppendLine("        w.WriteEndArray();");
        sb.AppendLine("    }");
        sb.AppendLine();

        // ReadVector3
        sb.AppendLine("    private static Vector3 ReadVector3(JsonElement elem)");
        sb.AppendLine("    {");
        sb.AppendLine("        var enumerator = elem.EnumerateArray().GetEnumerator();");
        sb.AppendLine("        enumerator.MoveNext(); float x = enumerator.Current.GetSingle();");
        sb.AppendLine("        enumerator.MoveNext(); float y = enumerator.Current.GetSingle();");
        sb.AppendLine("        enumerator.MoveNext(); float z = enumerator.Current.GetSingle();");
        sb.AppendLine("        return new Vector3(x, y, z);");
        sb.AppendLine("    }");
        sb.AppendLine();

        // WriteQuaternion
        sb.AppendLine("    private static void WriteQuaternion(Utf8JsonWriter w, string name, Quaternion q)");
        sb.AppendLine("    {");
        sb.AppendLine("        w.WritePropertyName(name);");
        sb.AppendLine("        w.WriteStartArray();");
        sb.AppendLine("        w.WriteNumberValue(q.X);");
        sb.AppendLine("        w.WriteNumberValue(q.Y);");
        sb.AppendLine("        w.WriteNumberValue(q.Z);");
        sb.AppendLine("        w.WriteNumberValue(q.W);");
        sb.AppendLine("        w.WriteEndArray();");
        sb.AppendLine("    }");
        sb.AppendLine();

        // ReadQuaternion
        sb.AppendLine("    private static Quaternion ReadQuaternion(JsonElement elem)");
        sb.AppendLine("    {");
        sb.AppendLine("        var enumerator = elem.EnumerateArray().GetEnumerator();");
        sb.AppendLine("        enumerator.MoveNext(); float qx = enumerator.Current.GetSingle();");
        sb.AppendLine("        enumerator.MoveNext(); float qy = enumerator.Current.GetSingle();");
        sb.AppendLine("        enumerator.MoveNext(); float qz = enumerator.Current.GetSingle();");
        sb.AppendLine("        enumerator.MoveNext(); float qw = enumerator.Current.GetSingle();");
        sb.AppendLine("        return new Quaternion(qx, qy, qz, qw);");
        sb.AppendLine("    }");
        sb.AppendLine();

        // WriteNNGuid
        sb.AppendLine("    private static void WriteNNGuid(Utf8JsonWriter w, string name, NNGuid guid)");
        sb.AppendLine("    {");
        sb.AppendLine("        w.WritePropertyName(name);");
        sb.AppendLine("        w.WriteStringValue($\"{guid.High:x16}{guid.Low:x16}\");");
        sb.AppendLine("    }");
        sb.AppendLine();

        // ReadNNGuid
        sb.AppendLine("    private static NNGuid ReadNNGuid(JsonElement elem)");
        sb.AppendLine("    {");
        sb.AppendLine("        var hex = elem.GetString() ?? \"\";");
        sb.AppendLine("        if (hex.Length == 32)");
        sb.AppendLine("        {");
        sb.AppendLine("            return new NNGuid");
        sb.AppendLine("            {");
        sb.AppendLine("                High = Convert.ToUInt64(hex[..16], 16),");
        sb.AppendLine("                Low = Convert.ToUInt64(hex[16..], 16),");
        sb.AppendLine("            };");
        sb.AppendLine("        }");
        sb.AppendLine("        return default;");
        sb.AppendLine("    }");
    }

    // ========================================================================
    // 工具方法
    // ========================================================================

    /// <summary>PascalCase 转 camelCase。</summary>
    private static string CamelCase(string name)
    {
        if (string.IsNullOrEmpty(name))
            return name;
        return char.ToLowerInvariant(name[0]) + name[1..];
    }

    // ========================================================================
    // 内部类型
    // ========================================================================

    /// <summary>组件结构体信息。</summary>
    private sealed class ComponentInfo
    {
        public string Name { get; set; } = string.Empty;
        public string FullName { get; set; } = string.Empty;
        public string? Namespace { get; set; }
        public List<FieldInfo> Fields { get; set; } = new();
    }

    /// <summary>字段信息。</summary>
    private sealed class FieldInfo
    {
        public string Name { get; set; } = string.Empty;
        public string TypeName { get; set; } = string.Empty;
        public string? FullName { get; set; }
        public bool IsEnum { get; set; }
        public string? EnumUnderlyingType { get; set; }
    }
}
