// SceneDebug — Editor 场景运行时诊断工具。
// 基于 EditorSceneNativeBridge 的 Reflection API，输出结构化诊断信息。
// 仅供开发调试，不参与正式 Editor UI 逻辑。
/*
 *SceneDebug 公共方法

   ┌────────────────────────────────────────┬────────────────────────────────────────────────┐
   │                  方法                  │                      用途                      │
   ├────────────────────────────────────────┼────────────────────────────────────────────────┤
   │ CheckApiAvailability()                 │ 检查 Native API 函数指针是否已安装             │
   ├────────────────────────────────────────┼────────────────────────────────────────────────┤
   │ DumpReflectionVersion(sceneHandle)     │ 输出 Reflection 版本号                         │
   ├────────────────────────────────────────┼────────────────────────────────────────────────┤
   │ DumpTypeInfo(sceneHandle)              │ 拉取类型信息快照，输出所有注册组件类型及其字段 │
   ├────────────────────────────────────────┼────────────────────────────────────────────────┤
   │ DumpEntityComponents(sceneHandle,      │ 输出指定实体的组件列表                         │
   │ entity)                                │                                                │
   ├────────────────────────────────────────┼────────────────────────────────────────────────┤
   │ DumpEntityRawData(sceneHandle, entity, │ Hex dump + 字段级格式化输出组件原始数据        │
   │  typeId)                               │                                                │
   ├────────────────────────────────────────┼────────────────────────────────────────────────┤
   │ DumpHierarchy(sceneHandle)             │ 输出场景层级树（DFS 顺序）                     │
   ├────────────────────────────────────────┼────────────────────────────────────────────────┤
   │ FullDiagnostic(sceneHandle)            │ 完整诊断（API 检查 + 类型注册 + 层级）         │
   ├────────────────────────────────────────┼────────────────────────────────────────────────┤
   │ HexDump(data, length)                  │ 通用 hex dump 工具                             │
   └────────────────────────────────────────┴────────────────────────────────────────────────┘
 *
 */
using System.Text;
using Neverness.Runtime.Engine;
using Neverness.Runtime.Interop;

namespace Neverness.Editor.Scene.Private.Debug;

/// <summary>
/// 场景运行时诊断工具——通过 <see cref="EditorSceneNativeBridge"/> 访问 Native Reflection API，
/// 输出层级、组件、字段、原始数据等结构化诊断信息。
///
/// 所有方法为静态，输出到 Console。用于：
/// - Inspector UI 开发调试
/// - Reflection API 集成验证
/// - 组件数据问题排查
/// - ABI 接线验证
/// </summary>
public static class SceneDebug
{
    // ── 常量 ──

    /// <summary>字段类型名称映射（与 NNComponentFieldType 枚举对齐）。</summary>
    private static readonly string[] FieldTypeNames =
    [
        "Float",       // 0
        "Float3",      // 1
        "Float4",      // 2
        "Float4x4",    // 3
        "Quaternion",  // 4
        "UInt32",      // 5
        "UInt64",      // 6
        "Entity",      // 7
        "CharArray",   // 8
        "Guid",        // 9
        "Bool",        // 10
    ];

    // ── API 可用性 ──

    /// <summary>检查 Native Editor Scene API 是否可用，并输出状态。</summary>
    public static unsafe void CheckApiAvailability()
    {
        Console.WriteLine("═══════════════════════════════════════════");
        Console.WriteLine("  Native Editor Scene API 可用性检查");
        Console.WriteLine("═══════════════════════════════════════════");

        bool installed = EngineNativeApiBootstrap.IsInstalled;
        Console.WriteLine($"  EngineApi 已安装:       {installed}");

        if (!installed)
        {
            Console.WriteLine("  ✗ EngineApi 未安装，所有 Native API 不可用");
            Console.WriteLine();
            return;
        }

        var editorScene = EngineNativeApiBootstrap.EngineApi.EditorScene;
        Console.WriteLine($"  EditorScene LayoutVersion: {editorScene.LayoutVersion}");
        Console.WriteLine();

        // 检查每个函数指针
        Console.WriteLine("  函数指针状态:");
        PrintFnStatus("getHierarchyVersion",     editorScene.GetHierarchyVersion);
        PrintFnStatus("getSnapshotSize",          editorScene.GetSnapshotSize);
        PrintFnStatus("getHierarchySnapshot",     editorScene.GetHierarchySnapshot);
        PrintFnStatus("getTransformVersion",      editorScene.GetTransformVersion);
        PrintFnStatus("getTransformSnapshot",     editorScene.GetTransformSnapshot);
        PrintFnStatus("getIncrementalSnapshot",   editorScene.GetIncrementalSnapshot);
        PrintFnStatus("getReflectionVersion",     editorScene.GetReflectionVersion);
        PrintFnStatus("getTypeInfoSnapshotSize",  editorScene.GetTypeInfoSnapshotSize);
        PrintFnStatus("getTypeInfoSnapshot",      editorScene.GetTypeInfoSnapshot);
        PrintFnStatus("getEntityComponentCount",  editorScene.GetEntityComponentCount);
        PrintFnStatus("getEntityComponents",      editorScene.GetEntityComponents);
        PrintFnStatus("getComponentFieldInfos",   editorScene.GetComponentFieldInfos);
        PrintFnStatus("getComponentRawData",      editorScene.GetComponentRawData);

        Console.WriteLine();
    }

    private static unsafe void PrintFnStatus(string name, void* fn)
    {
        string status = fn != null ? "✓" : "✗ null";
        Console.WriteLine($"    {name,-30} {status}");
    }

    // ── Reflection 版本 ──

    /// <summary>输出当前 Reflection 版本号。</summary>
    public static void DumpReflectionVersion(ulong sceneHandle)
    {
        Console.WriteLine($"[Reflection] version = {EditorSceneNativeBridge.GetReflectionVersion(sceneHandle)}");
    }

    // ── 类型信息快照 ──

    /// <summary>拉取并输出所有注册的组件类型信息（Type Registry）。</summary>
    public static unsafe void DumpTypeInfo(ulong sceneHandle)
    {
        Console.WriteLine("═══════════════════════════════════════════");
        Console.WriteLine("  组件类型注册表 (Type Info Snapshot)");
        Console.WriteLine("═══════════════════════════════════════════");

        if (!EditorSceneNativeBridge.IsAvailable)
        {
            Console.WriteLine("  ✗ EditorSceneNativeBridge 不可用");
            Console.WriteLine();
            return;
        }

        // 查询所需大小
        uint needed = EditorSceneNativeBridge.GetTypeInfoSnapshotSize(sceneHandle);
        Console.WriteLine($"  快照大小: {needed} bytes");
        if (needed == 0)
        {
            Console.WriteLine("  （无注册的组件类型）");
            Console.WriteLine();
            return;
        }

        // 拉取快照
        byte[] buffer = new byte[needed];
        uint written;
        fixed (byte* p = buffer)
        {
            written = EditorSceneNativeBridge.GetTypeInfoSnapshot(sceneHandle, p, needed);
        }

        if (written == 0)
        {
            Console.WriteLine("  ✗ getTypeInfoSnapshot 返回 0");
            Console.WriteLine();
            return;
        }

        // 解析快照
        fixed (byte* buf = buffer)
        {
            ParseAndDumpTypeInfo(buf, written);
        }

        Console.WriteLine();
    }

    private static unsafe void ParseAndDumpTypeInfo(byte* buf, uint size)
    {
        if (size < (uint)sizeof(NNSceneSnapshotHeader))
            return;

        var header = *(NNSceneSnapshotHeader*)buf;
        if (header.Magic != 0x56475343)
        {
            Console.WriteLine($"  ✗ 魔数不匹配: 0x{header.Magic:X8} (期望 0x56475343)");
            return;
        }

        int typeCount = (int)header.NodeCount;
        Console.WriteLine($"  LayoutVersion:    {header.LayoutVersion}");
        Console.WriteLine($"  ReflectionVersion:{header.HierarchyVersion}");
        Console.WriteLine($"  组件类型数:       {typeCount}");
        Console.WriteLine($"  NamePool 大小:    {header.NamePoolBytes} bytes");
        Console.WriteLine();

        var compInfos = (NNEditorComponentInfo*)(buf + sizeof(NNSceneSnapshotHeader));
        var fieldInfos = (NNEditorFieldInfo*)(compInfos + typeCount);

        // 计算 FieldInfo 总数
        uint totalFields = 0;
        for (int i = 0; i < typeCount; i++)
            totalFields += compInfos[i].FieldCount;

        var namePool = (byte*)(fieldInfos + totalFields);

        // 遍历每个组件类型
        uint fieldOffset = 0;
        for (int i = 0; i < typeCount; i++)
        {
            ref readonly var ci = ref compInfos[i];
            string compName = ReadName(namePool, ci.NameOffset, ci.NameLen);

            Console.WriteLine($"  ┌─ [{i}] {compName}");
            Console.WriteLine($"  │  TypeId:     0x{ci.TypeId:X16}");
            Console.WriteLine($"  │  FieldCount: {ci.FieldCount}");
            Console.WriteLine($"  │  Flags:      0x{ci.Flags:X8}");

            for (uint f = 0; f < ci.FieldCount; f++)
            {
                ref readonly var fi = ref fieldInfos[fieldOffset + f];
                string fieldName = ReadName(namePool, fi.NameOffset, fi.NameLen);
                string typeName = fi.FieldType < (uint)FieldTypeNames.Length
                    ? FieldTypeNames[fi.FieldType]
                    : $"Unknown({fi.FieldType})";

                string connector = (f == ci.FieldCount - 1) ? "└─" : "├─";
                Console.WriteLine($"  │  {connector} {fieldName,-20} {typeName,-12} offset={fi.DataOffset,4}  size={fi.DataSize,4}");
            }

            fieldOffset += ci.FieldCount;
            Console.WriteLine($"  └─");
            Console.WriteLine();
        }
    }

    // ── Entity 组件列表 ──

    /// <summary>输出指定实体拥有的所有组件（含组件名称）。</summary>
    public static unsafe void DumpEntityComponents(ulong sceneHandle, ulong entity)
    {
        Console.WriteLine($"═══════════════════════════════════════════");
        Console.WriteLine($"  Entity 0x{entity:X16} — 组件列表");
        Console.WriteLine("═══════════════════════════════════════════");

        if (!EditorSceneNativeBridge.IsAvailable)
        {
            Console.WriteLine("  ✗ EditorSceneNativeBridge 不可用");
            Console.WriteLine();
            return;
        }

        // 先拉取类型信息快照，构建 typeId → 名称 映射
        var nameMap = BuildTypeNameMap(sceneHandle);

        uint count = EditorSceneNativeBridge.GetEntityComponentCount(sceneHandle, entity);
        Console.WriteLine($"  组件数量: {count}");

        if (count == 0)
        {
            Console.WriteLine("  （实体无组件或已销毁）");
            Console.WriteLine();
            return;
        }

        Span<NNEditorComponentInfo> components = stackalloc NNEditorComponentInfo[(int)count];
        uint written = EditorSceneNativeBridge.GetEntityComponents(sceneHandle, entity, components);

        for (int i = 0; i < written; i++)
        {
            ref readonly var c = ref components[i];
            string compName = nameMap.TryGetValue(c.TypeId, out var n) ? n : $"?0x{c.TypeId:X16}";
            Console.WriteLine($"  [{i}] {compName,-24} TypeId=0x{c.TypeId:X16}  Fields={c.FieldCount}  Flags=0x{c.Flags:X8}");

            //{
                //ref readonly var c = ref components[i];
            Console.WriteLine($"  [{i}] TypeId = 0x{c.TypeId:X16}  Fields = {c.FieldCount}  Flags = 0x{c.Flags:X8}");
            //}
        }

        Console.WriteLine();
    }

    // ── Entity 组件原始数据 ──

    /// <summary>以 hex dump 形式输出指定实体的指定组件原始数据。</summary>
    public static unsafe void DumpEntityRawData(ulong sceneHandle, ulong entity, ulong componentTypeId)
    {
        Console.WriteLine($"═══════════════════════════════════════════");
        Console.WriteLine($"  Entity 0x{entity:X16} — Component 0x{componentTypeId:X16} Raw Data");
        Console.WriteLine("═══════════════════════════════════════════");

        if (!EditorSceneNativeBridge.IsAvailable)
        {
            Console.WriteLine("  ✗ EditorSceneNativeBridge 不可用");
            Console.WriteLine();
            return;
        }

        // 先查询字段信息获取组件大小
        Span<NNEditorFieldInfo> fields = stackalloc NNEditorFieldInfo[64];
        uint fieldCount = EditorSceneNativeBridge.GetComponentFieldInfos(sceneHandle, componentTypeId, fields);

        uint totalSize = 0;
        if (fieldCount > 0)
        {
            // 组件大小 = 最后一个字段的 offset + size
            // 注意：字段不一定按 offset 排序，取最大值
            for (int i = 0; i < fieldCount; i++)
            {
                uint end = fields[i].DataOffset + fields[i].DataSize;
                if (end > totalSize) totalSize = end;
            }
        }

        Console.WriteLine($"  字段数:     {fieldCount}");
        Console.WriteLine($"  组件大小:   {totalSize} bytes");

        if (totalSize == 0)
        {
            Console.WriteLine("  （无法确定组件大小）");
            Console.WriteLine();
            return;
        }

        // 拉取原始数据
        byte[] data = new byte[totalSize];
        uint read;
        fixed (byte* p = data)
        {
            read = EditorSceneNativeBridge.GetComponentRawData(sceneHandle, entity, componentTypeId, p, totalSize);
        }

        if (read == 0)
        {
            Console.WriteLine("  ✗ getComponentRawData 返回 0（实体无此组件）");
            Console.WriteLine();
            return;
        }

        // Hex dump
        Console.WriteLine();
        HexDump(data, (int)read);

        // 字段级输出
        if (fieldCount > 0)
        {
            Console.WriteLine();
            Console.WriteLine("  字段数据:");
            for (int i = 0; i < fieldCount; i++)
            {
                ref readonly var fi = ref fields[i];
                string typeName = fi.FieldType < (uint)FieldTypeNames.Length
                    ? FieldTypeNames[fi.FieldType]
                    : $"Unknown({fi.FieldType})";

                string value = FormatFieldData(data, fi.DataOffset, fi.DataSize, fi.FieldType);
                Console.WriteLine($"    offset={fi.DataOffset,4}  size={fi.DataSize,4}  {typeName,-12}  = {value}");
            }
        }

        Console.WriteLine();
    }

    // ── 层级结构 ──

    /// <summary>输出场景层级结构（从 SceneHierarchyCache 读取）。</summary>
    public static void DumpHierarchy(ulong sceneHandle)
    {
        Console.WriteLine("═══════════════════════════════════════════");
        Console.WriteLine("  场景层级结构 (Hierarchy)");
        Console.WriteLine("═══════════════════════════════════════════");

        if (!EditorSceneNativeBridge.IsAvailable)
        {
            Console.WriteLine("  ✗ EditorSceneNativeBridge 不可用");
            Console.WriteLine();
            return;
        }

        ulong version = EditorSceneNativeBridge.GetHierarchyVersion(sceneHandle);
        Console.WriteLine($"  HierarchyVersion: {version}");

        // 通过 SceneModuleImp 获取缓存（如果可用）
        var cache = SceneModuleImp.HierarchyCache;
        if (cache == null || cache.NodeCount == 0)
        {
            Console.WriteLine("  （层级缓存未初始化或为空）");
            Console.WriteLine();
            return;
        }

        Console.WriteLine($"  节点数: {cache.NodeCount}");
        Console.WriteLine();

        var nodes = cache.AllNodes;
        for (int i = 0; i < nodes.Length; i++)
        {
            var node = nodes[i];
            string indent = new string(' ', (int)node.Depth * 2);
            string flags = FormatNodeFlags(node.Flags);
            Console.WriteLine($"  {indent}[{i,4}] {node.EntityId} \"{node.Name}\" (children={node.ChildCount}) {flags}");
        }

        Console.WriteLine();
    }

    // ── 完整诊断 ──

    /// <summary>输出完整场景诊断信息（API 检查 + 类型注册 + 层级 + Reflection 版本）。</summary>
    public static void FullDiagnostic(ulong sceneHandle)
    {
        Console.WriteLine();
        Console.WriteLine("╔═══════════════════════════════════════════╗");
        Console.WriteLine("║   Neverness Scene — Full Diagnostic       ║");
        Console.WriteLine("╚═══════════════════════════════════════════╝");
        Console.WriteLine();

        CheckApiAvailability();
        DumpReflectionVersion(sceneHandle);
        Console.WriteLine();
        DumpTypeInfo(sceneHandle);
        DumpHierarchy(sceneHandle);
    }

    // ── 工具方法 ──

    /// <summary>Hex dump 输出（16 字节/行，带 ASCII 可视化）。</summary>
    public static void HexDump(byte[] data, int length, int bytesPerLine = 16)
    {
        for (int offset = 0; offset < length; offset += bytesPerLine)
        {
            var sb = new StringBuilder(80);
            sb.Append($"  {offset:X4}: ");

            // Hex
            for (int i = 0; i < bytesPerLine; i++)
            {
                if (offset + i < length)
                    sb.Append($"{data[offset + i]:X2} ");
                else
                    sb.Append("   ");

                if (i == 7)
                    sb.Append(' ');
            }

            sb.Append(" |");

            // ASCII
            for (int i = 0; i < bytesPerLine && offset + i < length; i++)
            {
                byte b = data[offset + i];
                sb.Append(b >= 0x20 && b < 0x7F ? (char)b : '.');
            }

            sb.Append('|');
            Console.WriteLine(sb.ToString());
        }
    }

    /// <summary>从 namePool 读取 UTF-8 字符串。</summary>
    private static unsafe string ReadName(byte* namePool, uint offset, uint len)
    {
        if (len == 0) return "";
        return Encoding.UTF8.GetString(namePool + offset, (int)len);
    }

    /// <summary>
    /// 拉取类型信息快照，构建 typeId → 组件名称 映射。
    /// 多个诊断方法共用，避免重复拉取。
    /// </summary>
    private static unsafe Dictionary<ulong, string> BuildTypeNameMap(ulong sceneHandle)
    {
        var map = new Dictionary<ulong, string>();

        uint needed = EditorSceneNativeBridge.GetTypeInfoSnapshotSize(sceneHandle);
        if (needed == 0) return map;

        byte[] buffer = new byte[needed];
        uint written;
        fixed (byte* p = buffer)
        {
            written = EditorSceneNativeBridge.GetTypeInfoSnapshot(sceneHandle, p, needed);
        }
        if (written == 0) return map;

        fixed (byte* buf = buffer)
        {
            if (written < (uint)sizeof(NNSceneSnapshotHeader)) return map;

            var header = *(NNSceneSnapshotHeader*)buf;
            if (header.Magic != 0x56475343) return map;

            int typeCount = (int)header.NodeCount;
            var compInfos = (NNEditorComponentInfo*)(buf + sizeof(NNSceneSnapshotHeader));
            var fieldInfos = (NNEditorFieldInfo*)(compInfos + typeCount);

            uint totalFields = 0;
            for (int i = 0; i < typeCount; i++)
                totalFields += compInfos[i].FieldCount;

            var namePool = (byte*)(fieldInfos + totalFields);

            for (int i = 0; i < typeCount; i++)
            {
                ref readonly var ci = ref compInfos[i];
                string name = ReadName(namePool, ci.NameOffset, ci.NameLen);
                map[ci.TypeId] = name;
            }
        }

        return map;
    }

    /// <summary>格式化节点标志位为可读字符串。</summary>
    private static string FormatNodeFlags(uint flags)
    {
        if (flags == 0) return "";
        var sb = new StringBuilder();
        sb.Append('[');
        bool first = true;
        void AddFlag(string name)
        {
            if (!first) sb.Append('|');
            sb.Append(name);
            first = false;
        }
        if ((flags & 0x01) != 0) AddFlag("Active");
        if ((flags & 0x02) != 0) AddFlag("Prefab");
        if ((flags & 0x04) != 0) AddFlag("Dirty");
        if ((flags & 0x08) != 0) AddFlag("Selected");
        sb.Append(']');
        return sb.ToString();
    }

    /// <summary>格式化字段原始数据为可读字符串（根据 fieldType）。</summary>
    private static unsafe string FormatFieldData(byte[] data, uint offset, uint size, uint fieldType)
    {
        if (offset + size > data.Length)
            return "<out of range>";

        fixed (byte* p = data)
        {
            byte* fieldPtr = p + offset;

            switch (fieldType)
            {
                case 0: // Float
                    if (size >= 4)
                        return $"{BitConverter.ToSingle(data, (int)offset):F4}";
                    break;

                case 1: // Float3
                    if (size >= 12)
                    {
                        float* f = (float*)fieldPtr;
                        return $"({f[0]:F2}, {f[1]:F2}, {f[2]:F2})";
                    }
                    break;

                case 2: // Float4
                    if (size >= 16)
                    {
                        float* f = (float*)fieldPtr;
                        return $"({f[0]:F2}, {f[1]:F2}, {f[2]:F2}, {f[3]:F2})";
                    }
                    break;

                case 4: // Quaternion
                    if (size >= 16)
                    {
                        float* f = (float*)fieldPtr;
                        return $"Q({f[0]:F3}, {f[1]:F3}, {f[2]:F3}, {f[3]:F3})";
                    }
                    break;

                case 3: // Float4x4
                    if (size >= 64)
                    {
                        float* m = (float*)fieldPtr;
                        return $"[({m[0]:F1},{m[1]:F1},{m[2]:F1},{m[3]:F1}) " +
                               $"({m[4]:F1},{m[5]:F1},{m[6]:F1},{m[7]:F1}) " +
                               $"({m[8]:F1},{m[9]:F1},{m[10]:F1},{m[11]:F1}) " +
                               $"({m[12]:F1},{m[13]:F1},{m[14]:F1},{m[15]:F1})]";
                    }
                    break;

                case 5: // UInt32
                    if (size >= 4)
                        return $"{BitConverter.ToUInt32(data, (int)offset)}";
                    break;

                case 6: // UInt64
                    if (size >= 8)
                        return $"{BitConverter.ToUInt64(data, (int)offset)}";
                    break;

                case 7: // Entity
                    if (size >= 8)
                    {
                        ulong handle = BitConverter.ToUInt64(data, (int)offset);
                        return handle == 0 ? "null" : $"0x{handle:X16}";
                    }
                    break;

                case 8: // CharArray
                    {
                        int len = 0;
                        while (len < size && fieldPtr[len] != 0) len++;
                        return $"\"{Encoding.UTF8.GetString(fieldPtr, len)}\"";
                    }

                case 9: // Guid
                    if (size >= 16)
                    {
                        ulong high = BitConverter.ToUInt64(data, (int)offset);
                        ulong low = BitConverter.ToUInt64(data, (int)offset + 8);
                        return $"{high:X16}:{low:X16}";
                    }
                    break;

                case 10: // Bool
                    if (size >= 1)
                        return data[offset] != 0 ? "true" : "false";
                    break;
            }
        }

        // 回退：hex 短格式
        int showLen = Math.Min((int)size, 8);
        var hex = new StringBuilder(showLen * 3);
        for (int i = 0; i < showLen; i++)
            hex.Append($"{data[offset + i]:X2} ");
        if (size > 8) hex.Append("...");
        return hex.ToString().TrimEnd();
    }
}
