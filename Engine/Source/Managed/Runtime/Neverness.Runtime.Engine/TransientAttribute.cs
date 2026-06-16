using System;

namespace Neverness.Runtime.Engine;

/// <summary>
/// 标记字段为瞬态，序列化时跳过。
/// 用于运行时计算字段（如 WorldMatrix、RuntimePlayerId）。
/// </summary>
[AttributeUsage(AttributeTargets.Field, Inherited = false, AllowMultiple = false)]
public sealed class TransientAttribute : Attribute
{
}
