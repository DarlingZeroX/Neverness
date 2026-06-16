using System.Text;
using Neverness.Runtime.Assets;

namespace Neverness.Editor.Assets;

/// <summary>
/// 资产依赖图。
///
/// 维护资产之间的有向依赖关系（A → B 表示 A 依赖 B）。
///
/// 核心能力：
///   - 前向依赖查询（A 依赖哪些资产）
///   - 反向依赖查询（哪些资产依赖 A）
///   - 递归依赖/反向依赖遍历
///   - DFS + 三色标记环检测
///   - 脏传播（某资产变化时，返回所有需要重新处理的资产）
///   - 二进制序列化（Library/Cache/Dependency.cache）
///
/// 设计：
///   - 前向图 + 反向图双向维护
///   - Thread-safe
///   - 与 C++ NNDependencyTable API 对齐
/// </summary>
public sealed class DependencyGraph
{
    private static readonly object s_lock = new();

    /* 前向依赖：asset → [dep1, dep2, ...] */
    private readonly Dictionary<string, HashSet<string>> s_forward
        = new(StringComparer.OrdinalIgnoreCase);

    /* 反向依赖：dep → [asset1, asset2, ...] */
    private readonly Dictionary<string, HashSet<string>> s_reverse
        = new(StringComparer.OrdinalIgnoreCase);

    /* ========== 核心 API ========== */

    /// <summary>设置资产的完整依赖列表（替换原有）。</summary>
    public void SetDependencies(GUID asset, IReadOnlyList<GUID> dependencies)
    {
        lock (s_lock)
        {
            var ak = asset.ToHexString();

            /* 移除旧的反向边 */
            if (s_forward.TryGetValue(ak, out var oldDeps))
            {
                foreach (var oldDep in oldDeps)
                {
                    if (s_reverse.TryGetValue(oldDep, out var revSet))
                        revSet.Remove(ak);
                }
            }

            /* 设置新的前向边 */
            var depSet = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
            foreach (var dep in dependencies)
            {
                if (!dep.IsZero)
                    depSet.Add(dep.ToHexString());
            }

            if (depSet.Count > 0)
                s_forward[ak] = depSet;
            else
                s_forward.Remove(ak);

            /* 添加新的反向边 */
            foreach (var dep in depSet)
            {
                if (!s_reverse.TryGetValue(dep, out var revSet))
                {
                    revSet = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
                    s_reverse[dep] = revSet;
                }
                revSet.Add(ak);
            }
        }
    }

    /// <summary>添加单个依赖（不重复）。</summary>
    public void AddDependency(GUID asset, GUID dependency)
    {
        if (dependency.IsZero) return;

        lock (s_lock)
        {
            var ak = asset.ToHexString();
            var dk = dependency.ToHexString();

            if (!s_forward.TryGetValue(ak, out var depSet))
            {
                depSet = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
                s_forward[ak] = depSet;
            }

            if (!depSet.Add(dk))
                return; /* 已存在 */

            /* 添加反向边 */
            if (!s_reverse.TryGetValue(dk, out var revSet))
            {
                revSet = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
                s_reverse[dk] = revSet;
            }
            revSet.Add(ak);
        }
    }

    /// <summary>移除单个依赖。</summary>
    public void RemoveDependency(GUID asset, GUID dependency)
    {
        if (dependency.IsZero) return;

        lock (s_lock)
        {
            var ak = asset.ToHexString();
            var dk = dependency.ToHexString();

            if (s_forward.TryGetValue(ak, out var depSet))
                depSet.Remove(dk);

            if (s_reverse.TryGetValue(dk, out var revSet))
                revSet.Remove(ak);
        }
    }

    /// <summary>清除某资产的所有依赖。</summary>
    public void ClearDependencies(GUID asset)
    {
        lock (s_lock)
        {
            var ak = asset.ToHexString();
            if (s_forward.TryGetValue(ak, out var depSet))
            {
                foreach (var dk in depSet)
                {
                    if (s_reverse.TryGetValue(dk, out var revSet))
                        revSet.Remove(ak);
                }
                s_forward.Remove(ak);
            }
        }
    }

    /* ========== 查询 ========== */

    /// <summary>查询直接依赖（A 依赖哪些资产）。</summary>
    public IReadOnlyCollection<GUID> GetDirectDependencies(GUID asset)
    {
        lock (s_lock)
        {
            var ak = asset.ToHexString();
            if (!s_forward.TryGetValue(ak, out var depSet))
                return Array.Empty<GUID>();
            return depSet.Select(g => GUID.Parse(g)).ToList();
        }
    }

    /// <summary>查询所有递归依赖（深度优先遍历）。</summary>
    public IReadOnlyCollection<GUID> GetAllDependencies(GUID asset)
    {
        lock (s_lock)
        {
            var ak = asset.ToHexString();
            var result = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
            CollectForwardRecursive(ak, result);
            result.Remove(ak); /* 不包含自身 */
            return result.Select(g => GUID.Parse(g)).ToList();
        }
    }

    /// <summary>查询直接反向依赖（谁引用了 asset）。</summary>
    public IReadOnlyCollection<GUID> GetDirectReverseDependencies(GUID asset)
    {
        lock (s_lock)
        {
            var ak = asset.ToHexString();
            if (!s_reverse.TryGetValue(ak, out var revSet))
                return Array.Empty<GUID>();
            return revSet.Select(g => GUID.Parse(g)).ToList();
        }
    }

    /// <summary>查询所有递归反向依赖。</summary>
    public IReadOnlyCollection<GUID> GetAllReverseDependencies(GUID asset)
    {
        lock (s_lock)
        {
            var ak = asset.ToHexString();
            var result = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
            CollectReverseRecursive(ak, result);
            result.Remove(ak);
            return result.Select(g => GUID.Parse(g)).ToList();
        }
    }

    /* ========== 环检测 ========== */

    /// <summary>检测整个图是否存在环。</summary>
    public bool HasCycle()
    {
        lock (s_lock)
        {
            var visited = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
            var inStack = new HashSet<string>(StringComparer.OrdinalIgnoreCase);

            foreach (var node in s_forward.Keys)
            {
                if (!visited.Contains(node))
                {
                    if (DfsHasCycle(node, visited, inStack))
                        return true;
                }
            }
            return false;
        }
    }

    /// <summary>检测从指定资产出发是否存在环。</summary>
    public bool HasCycle(GUID asset)
    {
        lock (s_lock)
        {
            var ak = asset.ToHexString();
            var visited = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
            var inStack = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
            return DfsHasCycle(ak, visited, inStack);
        }
    }

    /// <summary>找到图中所有环（返回环中的节点列表）。</summary>
    public List<List<GUID>> FindCycles()
    {
        lock (s_lock)
        {
            var cycles = new List<List<GUID>>();
            var visited = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
            var inStack = new List<string>();
            var inStackSet = new HashSet<string>(StringComparer.OrdinalIgnoreCase);

            foreach (var node in s_forward.Keys)
            {
                if (!visited.Contains(node))
                    FindAllCycles(node, visited, inStack, inStackSet, cycles);
            }

            return cycles;
        }
    }

    /* ========== 脏传播 ========== */

    /// <summary>
    /// 当某资产变化时，返回所有需要重新处理的资产。
    /// 即所有递归反向依赖（谁直接或间接引用了此资产）。
    /// </summary>
    public IReadOnlyCollection<GUID> GetDirtyPropagation(GUID changedAsset)
    {
        return GetAllReverseDependencies(changedAsset);
    }

    /* ========== 统计 ========== */

    /// <summary>有依赖记录的资产数量。</summary>
    public int AssetCount
    {
        get { lock (s_lock) return s_forward.Count; }
    }

    /// <summary>依赖边总数。</summary>
    public int EdgeCount
    {
        get
        {
            lock (s_lock)
            {
                int count = 0;
                foreach (var deps in s_forward.Values)
                    count += deps.Count;
                return count;
            }
        }
    }

    /// <summary>图中节点总数（前向 + 反向中出现的所有节点）。</summary>
    public int NodeCount
    {
        get
        {
            lock (s_lock)
            {
                var all = new HashSet<string>(s_forward.Keys, StringComparer.OrdinalIgnoreCase);
                all.UnionWith(s_reverse.Keys);
                return all.Count;
            }
        }
    }

    /* ========== 清理 ========== */

    /// <summary>清除所有数据（测试用）。</summary>
    public void Clear()
    {
        lock (s_lock)
        {
            s_forward.Clear();
            s_reverse.Clear();
        }
    }

    /* ========== 序列化 ========== */

    /// <summary>保存到磁碟（Library/Cache/Dependency.cache）。</summary>
    public void Save(string cachePath)
    {
        lock (s_lock)
        {
            try
            {
                var dir = Path.GetDirectoryName(cachePath);
                if (!string.IsNullOrEmpty(dir) && !Directory.Exists(dir))
                    Directory.CreateDirectory(dir);

                using var stream = File.Create(cachePath);
                using var w = new BinaryWriter(stream, Encoding.UTF8, leaveOpen: true);

                /* Header */
                w.Write(0x4E4E4447u); /* 'NNDG' = Neverness Dependency Graph */
                w.Write(1u);          /* version */

                /* 前向边 */
                int forwardEdgeCount = 0;
                foreach (var deps in s_forward.Values)
                    forwardEdgeCount += deps.Count;
                w.Write(forwardEdgeCount);

                foreach (var (ak, deps) in s_forward)
                {
                    foreach (var dk in deps)
                    {
                        w.Write(ak);
                        w.Write(dk);
                    }
                }

                /* 反向边（冗余存储，加速加载） */
                int reverseEdgeCount = 0;
                foreach (var revs in s_reverse.Values)
                    reverseEdgeCount += revs.Count;
                w.Write(reverseEdgeCount);

                foreach (var (dk, revs) in s_reverse)
                {
                    foreach (var ak in revs)
                    {
                        w.Write(dk);
                        w.Write(ak);
                    }
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine($"[DependencyGraph] 序列化失败: {cachePath} → {ex.Message}");
            }
        }
    }

    /// <summary>从磁碟加载。</summary>
    public void Load(string cachePath)
    {
        lock (s_lock)
        {
            if (!File.Exists(cachePath))
                return;

            try
            {
                using var stream = File.OpenRead(cachePath);
                using var r = new BinaryReader(stream, Encoding.UTF8, leaveOpen: true);

                var magic = r.ReadUInt32();
                var version = r.ReadUInt32();
                if (magic != 0x4E4E4447u || version != 1u)
                    return;

                s_forward.Clear();
                s_reverse.Clear();

                /* 前向边 */
                var forwardCount = r.ReadInt32();
                for (int i = 0; i < forwardCount; i++)
                {
                    var ak = r.ReadString();
                    var dk = r.ReadString();

                    if (!s_forward.TryGetValue(ak, out var depSet))
                    {
                        depSet = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
                        s_forward[ak] = depSet;
                    }
                    depSet.Add(dk);
                }

                /* 反向边 */
                var reverseCount = r.ReadInt32();
                for (int i = 0; i < reverseCount; i++)
                {
                    var dk = r.ReadString();
                    var ak = r.ReadString();

                    if (!s_reverse.TryGetValue(dk, out var revSet))
                    {
                        revSet = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
                        s_reverse[dk] = revSet;
                    }
                    revSet.Add(ak);
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine($"[DependencyGraph] 反序列化失败: {cachePath} → {ex.Message}");
                s_forward.Clear();
                s_reverse.Clear();
            }
        }
    }

    /* ========== 调试 ========== */

    /// <summary>打印依赖图（调试用）。</summary>
    public string Dump()
    {
        lock (s_lock)
        {
            var sb = new StringBuilder();
            sb.AppendLine($"DependencyGraph: {s_forward.Count} assets, {EdgeCount} edges");

            foreach (var (ak, deps) in s_forward.OrderBy(p => p.Key))
            {
                sb.AppendLine($"  {ak[..8]}.. → [{string.Join(", ", deps.Select(d => d[..8] + ".."))}]");
            }

            return sb.ToString();
        }
    }

    /* ========== 内部实现 ========== */

    private void CollectForwardRecursive(string node, HashSet<string> result)
    {
        if (!result.Add(node))
            return;

        if (s_forward.TryGetValue(node, out var deps))
        {
            foreach (var dep in deps)
                CollectForwardRecursive(dep, result);
        }
    }

    private void CollectReverseRecursive(string node, HashSet<string> result)
    {
        if (!result.Add(node))
            return;

        if (s_reverse.TryGetValue(node, out var revs))
        {
            foreach (var rev in revs)
                CollectReverseRecursive(rev, result);
        }
    }

    /// <summary>DFS + 三色标记环检测。</summary>
    private bool DfsHasCycle(string node, HashSet<string> visited, HashSet<string> inStack)
    {
        visited.Add(node);
        inStack.Add(node);

        if (s_forward.TryGetValue(node, out var deps))
        {
            foreach (var dep in deps)
            {
                if (!visited.Contains(dep))
                {
                    if (DfsHasCycle(dep, visited, inStack))
                        return true;
                }
                else if (inStack.Contains(dep))
                {
                    /* 回边 → 发现环 */
                    return true;
                }
            }
        }

        inStack.Remove(node);
        return false;
    }

    /// <summary>DFS 查找所有环（收集环路径）。</summary>
    private void FindAllCycles(
        string node,
        HashSet<string> visited,
        List<string> inStack,
        HashSet<string> inStackSet,
        List<List<GUID>> cycles)
    {
        visited.Add(node);
        inStack.Add(node);
        inStackSet.Add(node);

        if (s_forward.TryGetValue(node, out var deps))
        {
            foreach (var dep in deps)
            {
                if (!visited.Contains(dep))
                {
                    FindAllCycles(dep, visited, inStack, inStackSet, cycles);
                }
                else if (inStackSet.Contains(dep))
                {
                    /* 找到环的起点 */
                    var cycleStart = inStack.IndexOf(dep);
                    var cycle = new List<GUID>();
                    for (int i = cycleStart; i < inStack.Count; i++)
                        cycle.Add(GUID.Parse(inStack[i]));
                    cycles.Add(cycle);
                }
            }
        }

        inStack.RemoveAt(inStack.Count - 1);
        inStackSet.Remove(node);
    }
}
