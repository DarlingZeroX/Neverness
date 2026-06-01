// ============================================================================
// ScriptRegistryGenerator.cs - 脚本注册表 Source Generator
// ============================================================================
// 扫描标记了 [AutoRegisterScript] 的类，生成 ScriptRegistry 静态注册代码。
// ============================================================================

using System.Collections.Immutable;
using System.Text;
using Microsoft.CodeAnalysis;
using Microsoft.CodeAnalysis.CSharp.Syntax;
using Microsoft.CodeAnalysis.Text;

namespace Neverness.SourceGenerators;

/// <summary>
/// 脚本注册表 Source Generator。
/// </summary>
[Generator]
public class ScriptRegistryGenerator : IIncrementalGenerator
{
    // ========================================================================
    // 常量
    // ========================================================================

    /// <summary>AutoRegisterScript 特性的完整名称。</summary>
    private const string AutoRegisterScriptAttributeFullName = "Neverness.Gameplay.AutoRegisterScriptAttribute";

    /// <summary>EntityBehaviour 基类的完整名称。</summary>
    private const string EntityBehaviourFullName = "Neverness.Gameplay.EntityBehaviour";

    // ========================================================================
    // IIncrementalGenerator
    // ========================================================================

    /// <inheritdoc/>
    public void Initialize(IncrementalGeneratorInitializationContext context)
    {
        // 1. 扫描标记了 [AutoRegisterScript] 的类
        var scriptClasses = context.SyntaxProvider
            .CreateSyntaxProvider(
                predicate: static (node, _) => IsClassWithAttribute(node),
                transform: static (ctx, _) => GetScriptClassInfo(ctx))
            .Where(static info => info is not null)
            .Select(static (info, _) => info!);

        // 2. 合并所有脚本类
        var collectedScripts = scriptClasses.Collect();

        // 3. 生成代码
        context.RegisterSourceOutput(collectedScripts, GenerateScriptRegistry);
    }

    // ========================================================================
    // 语法过滤
    // ========================================================================

    /// <summary>检查是否是带有属性的类声明。</summary>
    private static bool IsClassWithAttribute(SyntaxNode node)
    {
        return node is ClassDeclarationSyntax classDecl
            && classDecl.AttributeLists.Count > 0;
    }

    // ========================================================================
    // 语义分析
    // ========================================================================

    /// <summary>获取脚本类信息。</summary>
    private static ScriptClassInfo? GetScriptClassInfo(GeneratorSyntaxContext context)
    {
        var classDecl = (ClassDeclarationSyntax)context.Node;
        var symbol = context.SemanticModel.GetDeclaredSymbol(classDecl) as INamedTypeSymbol;

        if (symbol is null)
        {
            return null;
        }

        // 检查是否标记了 [AutoRegisterScript]
        if (!HasAutoRegisterScriptAttribute(symbol))
        {
            return null;
        }

        // 检查是否继承了 EntityBehaviour
        if (!InheritsFromEntityBehaviour(symbol))
        {
            return null;
        }

        // 检查是否是抽象类
        if (symbol.IsAbstract)
        {
            return null;
        }

        return new ScriptClassInfo
        {
            Name = symbol.Name,
            FullName = symbol.ToDisplayString(SymbolDisplayFormat.FullyQualifiedFormat),
            Namespace = symbol.ContainingNamespace.IsGlobalNamespace
                ? null
                : symbol.ContainingNamespace.ToDisplayString()
        };
    }

    /// <summary>检查是否标记了 [AutoRegisterScript]。</summary>
    private static bool HasAutoRegisterScriptAttribute(INamedTypeSymbol symbol)
    {
        foreach (var attr in symbol.GetAttributes())
        {
            if (attr.AttributeClass?.ToDisplayString() == AutoRegisterScriptAttributeFullName)
            {
                return true;
            }
        }
        return false;
    }

    /// <summary>检查是否继承了 EntityBehaviour。</summary>
    private static bool InheritsFromEntityBehaviour(INamedTypeSymbol symbol)
    {
        var current = symbol.BaseType;
        while (current is not null)
        {
            if (current.ToDisplayString() == EntityBehaviourFullName)
            {
                return true;
            }
            current = current.BaseType;
        }
        return false;
    }

    // ========================================================================
    // 代码生成
    // ========================================================================

    /// <summary>生成脚本注册表代码。</summary>
    private static void GenerateScriptRegistry(
        SourceProductionContext context,
        ImmutableArray<ScriptClassInfo> scripts)
    {
        if (scripts.Length == 0)
        {
            return;
        }

        var source = GenerateSource(scripts);
        context.AddSource("ScriptRegistry_AutoGenerated.g.cs", SourceText.From(source, Encoding.UTF8));
    }

    /// <summary>生成源代码。</summary>
    private static string GenerateSource(ImmutableArray<ScriptClassInfo> scripts)
    {
        var sb = new StringBuilder();

        // 文件头
        sb.AppendLine("// <auto-generated/>");
        sb.AppendLine("// 由 Neverness.SourceGenerators 自动生成");
        sb.AppendLine("// 请勿手动修改此文件");
        sb.AppendLine();

        // using
        sb.AppendLine("using System;");
        sb.AppendLine("using System.Collections.Generic;");
        sb.AppendLine("using Neverness.Gameplay;");
        sb.AppendLine();

        // 类定义
        sb.AppendLine("namespace Neverness.Gameplay");
        sb.AppendLine("{");
        sb.AppendLine("    /// <summary>");
        sb.AppendLine("    /// 脚本注册表自动生成代码。");
        sb.AppendLine("    /// </summary>");
        sb.AppendLine("    public static class ScriptRegistry_AutoGenerated");
        sb.AppendLine("    {");

        // RegisterAll 方法
        sb.AppendLine("        /// <summary>注册所有标记了 [AutoRegisterScript] 的脚本类型。</summary>");
        sb.AppendLine("        public static void RegisterAll(ScriptRegistry registry)");
        sb.AppendLine("        {");

        foreach (var script in scripts)
        {
            sb.AppendLine($"            registry.Register<{script.FullName}>();");
        }

        sb.AppendLine("        }");
        sb.AppendLine();

        // GetAllScriptTypes 方法
        sb.AppendLine("        /// <summary>获取所有脚本类型。</summary>");
        sb.AppendLine("        public static IReadOnlyList<Type> GetAllScriptTypes()");
        sb.AppendLine("        {");
        sb.AppendLine("            return new Type[]");
        sb.AppendLine("            {");

        foreach (var script in scripts)
        {
            sb.AppendLine($"                typeof({script.FullName}),");
        }

        sb.AppendLine("            };");
        sb.AppendLine("        }");

        sb.AppendLine("    }");
        sb.AppendLine("}");

        return sb.ToString();
    }

    // ========================================================================
    // 内部类型
    // ========================================================================

    /// <summary>脚本类信息。</summary>
    private sealed class ScriptClassInfo
    {
        /// <summary>类名。</summary>
        public string Name { get; set; } = string.Empty;

        /// <summary>完整类型名（含命名空间）。</summary>
        public string FullName { get; set; } = string.Empty;

        /// <summary>命名空间。</summary>
        public string? Namespace { get; set; }
    }
}
