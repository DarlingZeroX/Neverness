using System.Diagnostics;
using System.Runtime.InteropServices;
using Neverness.Editor.Scene.Private.Cache;
using Neverness.Runtime.Engine;
using Xunit;
using Xunit.Abstractions;

namespace Neverness.Editor.Scene.Tests;

/// <summary>
/// SceneHierarchyCache 压力测试——使用合成 Snapshot 数据验证 10k+ 实体的性能。
///
/// 测试场景：
///   - 10k 实体解析（ParseSnapshot）
///   - 各种展开/折叠状态下的可见列表重建
///   - 搜索过滤性能
///   - GC 分配检测
/// </summary>
public class SceneHierarchyCacheStressTests
{
    private readonly ITestOutputHelper _output;

    public SceneHierarchyCacheStressTests(ITestOutputHelper output)
    {
        _output = output;
    }

    // ── 工具方法 ──

    /// <summary>生成合成 Snapshot 数据（DFS 有序的二进制 buffer）。</summary>
    /// <param name="totalNodes">总节点数</param>
    /// <param name="treeFanout">每层子节点数（最后一层无子节点）</param>
    /// <param name="avgNameLen">名字平均长度</param>
    /// <returns>分配的 byte[]，包含完整的 Header + Nodes + NamePool</returns>
    private static byte[] BuildSyntheticSnapshot(int totalNodes, int treeFanout = 10, int avgNameLen = 12)
    {
        // 计算树结构
        // 第 0 层：totalNodes / treeFanout 个根节点
        // 第 1 层：根节点 × treeFanout
        // 第 2 层：上层 × treeFanout  ...
        // 直到 totalNodes 用完

        // 简化：创建一个平衡 N 叉树
        var entities = new List<(ulong handle, ulong parent, uint depth, uint childCount, string name)>();

        ulong handleCounter = 1;
        int remaining = totalNodes;

        // BFS 构建树
        var queue = new Queue<(ulong parent, uint depth)>();

        // 根节点
        int rootCount = Math.Max(1, totalNodes / (treeFanout * treeFanout));
        for (int r = 0; r < rootCount && remaining > 0; r++)
        {
            ulong h = handleCounter++;
            int childCount = Math.Min(treeFanout, remaining - 1);
            entities.Add((h, 0, 0, (uint)childCount, $"Root_{r}"));
            remaining--;
            queue.Enqueue((h, 1));
        }

        // BFS 展开
        while (queue.Count > 0 && remaining > 0)
        {
            var (parent, depth) = queue.Dequeue();

            // 找到 parent 在 entities 中的索引，更新 childCount
            int parentIdx = -1;
            for (int pi = 0; pi < entities.Count; pi++)
            {
                if (entities[pi].handle == parent)
                {
                    parentIdx = pi;
                    break;
                }
            }

            int actualChildCount = Math.Min(treeFanout, remaining);
            if (actualChildCount <= 0) continue;

            // 更新 parent 的 childCount
            entities[parentIdx] = (
                entities[parentIdx].handle,
                entities[parentIdx].parent,
                entities[parentIdx].depth,
                (uint)actualChildCount,
                entities[parentIdx].name
            );

            for (int c = 0; c < actualChildCount && remaining > 0; c++)
            {
                ulong h = handleCounter++;
                int grandChildren = remaining > treeFanout ? treeFanout : 0;
                entities.Add((h, parent, depth, (uint)grandChildren, $"Node_{h}"));
                remaining--;
                if (grandChildren > 0)
                    queue.Enqueue((h, depth + 1));
            }
        }

        // 如果还有剩余，挂在最后一个节点下
        if (remaining > 0)
        {
            var last = entities[^1];
            entities[^1] = (last.handle, last.parent, last.depth, last.childCount + (uint)remaining, last.name);
            while (remaining > 0)
            {
                ulong h = handleCounter++;
                entities.Add((h, last.handle, last.depth + 1, 0, $"Leaf_{h}"));
                remaining--;
            }
        }

        int nodeCount = entities.Count;

        // 计算 namePool 大小
        int namePoolBytes = 0;
        foreach (var e in entities)
            namePoolBytes += e.name.Length + 1; // UTF-8 每字节 + NUL

        // 分配 buffer
        int headerSize = 32; // sizeof(NNSceneSnapshotHeader)
        int nodeSize = 40;   // sizeof(NNSceneNodeSnapshot)
        int totalSize = headerSize + nodeCount * nodeSize + namePoolBytes;
        var buffer = new byte[totalSize];

        // 写 Header
        unsafe
        {
            fixed (byte* buf = buffer)
            {
                var header = (NNSceneSnapshotHeader*)buf;
                header->Magic = 0x56475343;
                header->LayoutVersion = 1;
                header->HierarchyVersion = 42;
                header->NodeCount = (uint)nodeCount;
                header->NamePoolBytes = (uint)namePoolBytes;
                header->RootCount = (uint)entities.Count(e => e.parent == 0);
                header->Pad = 0;

                // 写 Nodes + NamePool
                var nodes = (NNSceneNodeSnapshot*)(buf + headerSize);
                byte* namePool = buf + headerSize + nodeCount * nodeSize;

                int nameOffset = 0;
                for (int i = 0; i < nodeCount; i++)
                {
                    var e = entities[i];
                    nodes[i].Entity = e.handle;
                    nodes[i].Parent = e.parent;
                    nodes[i].Depth = e.depth;
                    nodes[i].ChildCount = e.childCount;
                    nodes[i].NameOffset = (uint)nameOffset;
                    nodes[i].NameLen = (uint)e.name.Length;
                    nodes[i].Flags = 0; // active by default

                    // 写名字到 namePool
                    var nameBytes = System.Text.Encoding.UTF8.GetBytes(e.name);
                    Marshal.Copy(nameBytes, 0, (nint)(namePool + nameOffset), nameBytes.Length);
                    namePool[nameOffset + nameBytes.Length] = 0; // NUL
                    nameOffset += nameBytes.Length + 1;
                }
            }
        }

        return buffer;
    }

    // ── 测试 ──

    [Theory]
    [InlineData(1000, 10)]
    [InlineData(10000, 10)]
    [InlineData(50000, 20)]
    public void ParseSnapshot_1k_to_50k_MeasuresPerformance(int nodeCount, int fanout)
    {
        var snapshot = BuildSyntheticSnapshot(nodeCount, fanout);
        var cache = new SceneHierarchyCache();

        // 预热
        unsafe
        {
            fixed (byte* buf = snapshot)
                cache.ParseSnapshot(buf, (uint)snapshot.Length);
        }

        // 测量
        var sw = Stopwatch.StartNew();
        long gcBefore = GC.GetTotalAllocatedBytes(true);

        unsafe
        {
            fixed (byte* buf = snapshot)
                cache.ParseSnapshot(buf, (uint)snapshot.Length);
        }

        sw.Stop();
        long gcAfter = GC.GetTotalAllocatedBytes(true);

        _output.WriteLine($"ParseSnapshot({nodeCount} nodes, fanout={fanout}):");
        _output.WriteLine($"  Time: {sw.Elapsed.TotalMicroseconds:F0} us");
        _output.WriteLine($"  Allocated: {(gcAfter - gcBefore) / 1024:F1} KB");
        _output.WriteLine($"  NodeCount: {cache.NodeCount}");

        Assert.Equal(nodeCount, cache.NodeCount);
        Assert.True(sw.Elapsed.TotalMilliseconds < 50, $"ParseSnapshot took {sw.Elapsed.TotalMilliseconds:F1}ms for {nodeCount} nodes");
    }

    [Theory]
    [InlineData(1000)]
    [InlineData(10000)]
    public void RebuildVisibleList_AllCollapsed_MeasuresPerformance(int nodeCount)
    {
        var snapshot = BuildSyntheticSnapshot(nodeCount, 10);
        var cache = new SceneHierarchyCache();

        unsafe
        {
            fixed (byte* buf = snapshot)
                cache.ParseSnapshot(buf, (uint)snapshot.Length);
        }

        // 全部折叠（默认状态）
        cache.CollapseAll();

        // 预热
        cache.RebuildVisibleList();

        // 测量
        var sw = Stopwatch.StartNew();
        cache.RebuildVisibleList();
        sw.Stop();

        int visibleCount = cache.VisibleList.Count;
        _output.WriteLine($"RebuildVisibleList(all collapsed, {nodeCount} nodes):");
        _output.WriteLine($"  Time: {sw.Elapsed.TotalMicroseconds:F0} us");
        _output.WriteLine($"  Visible: {visibleCount}");

        Assert.True(sw.Elapsed.TotalMilliseconds < 10, $"RebuildVisibleList took {sw.Elapsed.TotalMilliseconds:F1}ms");
    }

    [Theory]
    [InlineData(1000)]
    [InlineData(10000)]
    public void RebuildVisibleList_AllExpanded_MeasuresPerformance(int nodeCount)
    {
        var snapshot = BuildSyntheticSnapshot(nodeCount, 10);
        var cache = new SceneHierarchyCache();

        unsafe
        {
            fixed (byte* buf = snapshot)
                cache.ParseSnapshot(buf, (uint)snapshot.Length);
        }

        // 全部展开
        cache.ExpandAll();

        // 预热
        cache.RebuildVisibleList();

        // 测量
        var sw = Stopwatch.StartNew();
        cache.RebuildVisibleList();
        sw.Stop();

        int visibleCount = cache.VisibleList.Count;
        _output.WriteLine($"RebuildVisibleList(all expanded, {nodeCount} nodes):");
        _output.WriteLine($"  Time: {sw.Elapsed.TotalMicroseconds:F0} us");
        _output.WriteLine($"  Visible: {visibleCount}");

        Assert.Equal(nodeCount, visibleCount);
        Assert.True(sw.Elapsed.TotalMilliseconds < 20, $"RebuildVisibleList took {sw.Elapsed.TotalMilliseconds:F1}ms");
    }

    [Theory]
    [InlineData(10000)]
    public void RebuildVisibleList_PartialExpand_MeasuresPerformance(int nodeCount)
    {
        var snapshot = BuildSyntheticSnapshot(nodeCount, 10);
        var cache = new SceneHierarchyCache();

        unsafe
        {
            fixed (byte* buf = snapshot)
                cache.ParseSnapshot(buf, (uint)snapshot.Length);
        }

        // 只展开第一层根节点（通常 ~10 个）
        var allNodes = cache.AllNodes;
        for (int i = 0; i < allNodes.Length; i++)
        {
            if (allNodes[i].Depth == 0 && allNodes[i].ChildCount > 0)
                cache.SetExpanded(allNodes[i].Entity, true);
        }

        // 预热
        cache.RebuildVisibleList();

        // 测量
        var sw = Stopwatch.StartNew();
        cache.RebuildVisibleList();
        sw.Stop();

        int visibleCount = cache.VisibleList.Count;
        _output.WriteLine($"RebuildVisibleList(partial expand 1st level, {nodeCount} nodes):");
        _output.WriteLine($"  Time: {sw.Elapsed.TotalMicroseconds:F0} us");
        _output.WriteLine($"  Visible: {visibleCount}");

        Assert.True(sw.Elapsed.TotalMilliseconds < 10, $"RebuildVisibleList took {sw.Elapsed.TotalMilliseconds:F1}ms");
    }

    [Fact]
    public void RebuildVisibleList_SearchFilter_MeasuresPerformance()
    {
        int nodeCount = 10000;
        var snapshot = BuildSyntheticSnapshot(nodeCount, 10);
        var cache = new SceneHierarchyCache();

        unsafe
        {
            fixed (byte* buf = snapshot)
                cache.ParseSnapshot(buf, (uint)snapshot.Length);
        }

        cache.SetSearch("Root_");

        // 预热
        cache.RebuildVisibleList();

        // 测量
        var sw = Stopwatch.StartNew();
        cache.RebuildVisibleList();
        sw.Stop();

        int visibleCount = cache.VisibleList.Count;
        _output.WriteLine($"RebuildVisibleList(search='Root_', {nodeCount} nodes):");
        _output.WriteLine($"  Time: {sw.Elapsed.TotalMicroseconds:F0} us");
        _output.WriteLine($"  Visible: {visibleCount}");

        Assert.True(sw.Elapsed.TotalMilliseconds < 20, $"Search took {sw.Elapsed.TotalMilliseconds:F1}ms");
    }

    [Fact]
    public void Snapshot_Preserves_Node_Data()
    {
        var snapshot = BuildSyntheticSnapshot(100, 5);
        var cache = new SceneHierarchyCache();

        unsafe
        {
            fixed (byte* buf = snapshot)
                cache.ParseSnapshot(buf, (uint)snapshot.Length);
        }

        Assert.Equal(100, cache.NodeCount);

        var allNodes = cache.AllNodes;

        // 验证至少有一个根节点
        int rootCount = allNodes.Count(n => n.Parent == 0);
        Assert.True(rootCount > 0, "Should have at least one root");

        // 验证所有名字不为空
        foreach (var node in allNodes)
            Assert.NotEmpty(node.Name);

        // 验证 FlatIndex 一致性
        for (int i = 0; i < allNodes.Length; i++)
            Assert.Equal(i, allNodes[i].FlatIndex);

        // 验证 Entity handle 唯一
        var entitySet = new HashSet<ulong>();
        foreach (var node in allNodes)
            Assert.True(entitySet.Add(node.Entity), $"Duplicate entity handle: {node.Entity}");
    }

    [Fact]
    public void ExpandAll_CollapseAll_RoundTrip()
    {
        var snapshot = BuildSyntheticSnapshot(500, 10);
        var cache = new SceneHierarchyCache();

        unsafe
        {
            fixed (byte* buf = snapshot)
                cache.ParseSnapshot(buf, (uint)snapshot.Length);
        }

        // 全部展开 → 可见列表 = 所有节点
        cache.ExpandAll();
        var allExpanded = cache.VisibleList;
        Assert.Equal(cache.NodeCount, allExpanded.Count);

        // 全部折叠 → 可见列表 = 仅根节点
        cache.CollapseAll();
        var allCollapsed = cache.VisibleList;
        int rootCount = cache.AllNodes.Count(n => n.Parent == 0);
        Assert.Equal(rootCount, allCollapsed.Count);
    }

    [Fact]
    public void FullPipeline_10k_Simulates_EditorFrame()
    {
        int nodeCount = 10000;
        var snapshot = BuildSyntheticSnapshot(nodeCount, 10);
        var cache = new SceneHierarchyCache();

        // 模拟：首次加载
        var swFull = Stopwatch.StartNew();
        unsafe
        {
            fixed (byte* buf = snapshot)
                cache.ParseSnapshot(buf, (uint)snapshot.Length);
        }
        cache.ExpandAll();
        cache.RebuildVisibleList();
        swFull.Stop();

        _output.WriteLine($"Full pipeline (10k nodes):");
        _output.WriteLine($"  First load: {swFull.Elapsed.TotalMicroseconds:F0} us");

        // 模拟：无变化帧（版本检查）
        var swIdle = Stopwatch.StartNew();
        // 版本相同 → 跳过
        swIdle.Stop();
        _output.WriteLine($"  Idle frame: {swIdle.Elapsed.TotalMicroseconds:F0} us (no-op)");

        // 模拟：展开状态变化触发可见列表重建
        var firstNode = cache.AllNodes[0];
        cache.ToggleExpanded(firstNode.Entity);

        var swRebuild = Stopwatch.StartNew();
        cache.RebuildVisibleList();
        swRebuild.Stop();

        _output.WriteLine($"  Rebuild visible (partial): {swRebuild.Elapsed.TotalMicroseconds:F0} us");

        Assert.True(swFull.Elapsed.TotalMilliseconds < 100, "Full pipeline should be under 100ms");
        Assert.True(swRebuild.Elapsed.TotalMilliseconds < 20, "Visible list rebuild should be under 20ms");
    }
}
