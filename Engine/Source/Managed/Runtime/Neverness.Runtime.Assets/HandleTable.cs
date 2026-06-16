using System.Diagnostics;

namespace Neverness.Runtime.Assets;

/// <summary>
/// 集中式 Handle 分配表（索引 + generation 防 ABA）。
///
/// Handle 為 ulong 編碼：
///   - 低 32 位：slot 索引 + 1（0 表示無效 Handle）
///   - 高 32 位：generation（每次 slot 重用時自增）
///
/// Slot 被釋放後進入 free list；重新分配時 generation++，
/// 使舊 Handle 自動失效（generation 不匹配 → Resolve 回傳 null）。
///
/// 與 C++ NNHandleTable 對應。
/// </summary>
public sealed class HandleTable
{
    /// <summary>最大索引值（32 位）。</summary>
    private const uint InvalidIndex = 0xFFFFFFFFu;

    /* ======================== Slot 結構 ======================== */

    /// <summary>
    /// 內部 slot，持有資產資料引用和元數據。
    /// </summary>
    private sealed class Slot
    {
        /// <summary>資產資料引用（AssetEntry 等）。</summary>
        public object? Data;

        /// <summary>資產型別 ID。</summary>
        public ulong TypeId;

        /// <summary>引用計數（原子操作）。</summary>
        public int RefCount;

        /// <summary>generation counter，每次重用時自增。</summary>
        public uint Generation;

        /// <summary>是否存活。</summary>
        public bool Alive;
    }

    /* ======================== 內部狀態 ======================== */

    private readonly object _lock = new();
    private readonly List<Slot> _slots = new();
    private readonly Stack<int> _freeList = new();
    private int _allocatedCount;

    /* ======================== Handle 編解碼 ======================== */

    /// <summary>從 Handle 解碼 slot 索引。</summary>
    private static int HandleToIndex(ulong h) => (int)(h & 0xFFFFFFFFul) - 1;

    /// <summary>從 Handle 解碼 generation。</summary>
    private static uint HandleToGen(ulong h) => (uint)(h >> 32);

    /// <summary>從 slot 索引和 generation 編碼為 Handle。</summary>
    private static ulong MakeHandle(int idx, uint gen) =>
        ((ulong)gen << 32) | (uint)(idx + 1);

    /* ======================== 公共 API ======================== */

    /// <summary>
    /// 分配一個 slot，回傳編碼後的 Handle。
    /// 初始引用計數為 1。
    /// </summary>
    /// <param name="data">資產資料引用。</param>
    /// <param name="typeId">資產型別 ID。</param>
    /// <returns>編碼後的 Handle（非零）。</returns>
    public ulong Allocate(object? data, ulong typeId)
    {
        lock (_lock)
        {
            int index;
            if (_freeList.Count > 0)
            {
                index = _freeList.Pop();
            }
            else
            {
                index = _slots.Count;
                _slots.Add(new Slot());
            }

            var slot = _slots[index];
            slot.Data = data;
            slot.TypeId = typeId;
            slot.RefCount = 1;
            slot.Alive = true;
            // generation 在 slot 建構時為 0，每次重用時自增

            _allocatedCount++;
            return MakeHandle(index, slot.Generation);
        }
    }

    /// <summary>
    /// 釋放 slot（進入 free list）。
    /// generation 自增使舊 Handle 失效。
    /// </summary>
    /// <param name="handle">要釋放的 Handle。</param>
    public void Free(ulong handle)
    {
        if (handle == 0) return;

        lock (_lock)
        {
            var idx = HandleToIndex(handle);
            if (idx < 0 || idx >= _slots.Count)
                return;

            var slot = _slots[idx];
            if (!slot.Alive)
                return;
            if (HandleToGen(handle) != slot.Generation)
                return;

            slot.Data = null;
            slot.Alive = false;
            slot.RefCount = 0;
            slot.Generation++; // 使舊 Handle 失效

            _freeList.Push(idx);
            _allocatedCount--;
        }
    }

    /// <summary>
    /// 解析 Handle → 資料引用（generation 不匹配或已釋放回傳 null）。
    /// </summary>
    /// <param name="handle">要解析的 Handle。</param>
    /// <returns>資產資料引用；無效則回傳 null。</returns>
    public object? Resolve(ulong handle)
    {
        if (handle == 0)
            return null;

        lock (_lock)
        {
            var idx = HandleToIndex(handle);
            if (idx < 0 || idx >= _slots.Count)
                return null;

            var slot = _slots[idx];
            if (!slot.Alive)
                return null;
            if (HandleToGen(handle) != slot.Generation)
                return null;

            return slot.Data;
        }
    }

    /// <summary>
    /// 取得 slot 之型別 ID。無效 Handle 回傳 0。
    /// </summary>
    public ulong GetTypeId(ulong handle)
    {
        lock (_lock)
        {
            var idx = HandleToIndex(handle);
            if (idx < 0 || idx >= _slots.Count)
                return 0;

            var slot = _slots[idx];
            if (!slot.Alive || HandleToGen(handle) != slot.Generation)
                return 0;

            return slot.TypeId;
        }
    }

    /// <summary>
    /// 增加引用計數。
    /// </summary>
    public void AddRef(ulong handle)
    {
        if (handle == 0) return;

        lock (_lock)
        {
            var idx = HandleToIndex(handle);
            if (idx >= 0 && idx < _slots.Count)
            {
                var slot = _slots[idx];
                if (slot.Alive && HandleToGen(handle) == slot.Generation)
                {
                    Interlocked.Increment(ref slot.RefCount);
                }
            }
        }
    }

    /// <summary>
    /// 減少引用計數。
    /// 回傳 true 表示計數降至 0（但不自動 Free）。
    /// </summary>
    public bool Release(ulong handle)
    {
        if (handle == 0) return false;

        lock (_lock)
        {
            var idx = HandleToIndex(handle);
            if (idx < 0 || idx >= _slots.Count)
                return false;

            var slot = _slots[idx];
            if (!slot.Alive || HandleToGen(handle) != slot.Generation)
                return false;

            var prev = Interlocked.Decrement(ref slot.RefCount);
            Debug.Assert(prev >= 0, "RefCount 不應降至負數");
            return prev == 0; // 回傳 true 表示計數降至 0
        }
    }

    /// <summary>
    /// 取得當前引用計數。無效 Handle 回傳 0。
    /// </summary>
    public uint GetRefCount(ulong handle)
    {
        lock (_lock)
        {
            var idx = HandleToIndex(handle);
            if (idx < 0 || idx >= _slots.Count)
                return 0;

            var slot = _slots[idx];
            if (!slot.Alive || HandleToGen(handle) != slot.Generation)
                return 0;

            return (uint)Volatile.Read(ref slot.RefCount);
        }
    }

    /// <summary>
    /// 取得已分配 slot 數量。
    /// </summary>
    public int AllocatedCount
    {
        get
        {
            lock (_lock)
            {
                return _allocatedCount;
            }
        }
    }

    /// <summary>
    /// Handle 是否有效（可解析）。
    /// </summary>
    public bool IsAlive(ulong handle)
    {
        if (handle == 0) return false;

        lock (_lock)
        {
            var idx = HandleToIndex(handle);
            if (idx < 0 || idx >= _slots.Count)
                return false;

            var slot = _slots[idx];
            return slot.Alive && HandleToGen(handle) == slot.Generation;
        }
    }
}
