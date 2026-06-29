using System;
using System.Collections.Generic;
using System.Numerics;
using Neverness.Runtime.Scene.Components;
using Vector2 = System.Numerics.Vector2;
using Vector3 = System.Numerics.Vector3;
using Vector4 = System.Numerics.Vector4;
using Matrix4x4 = System.Numerics.Matrix4x4;
using Quaternion = System.Numerics.Quaternion;
using Neverness.Gameplay;

/// <summary>
/// 棋子类型。
/// </summary>
public enum PieceType
{
    /// <summary>空位。</summary>
    None,
    /// <summary>蓝方。</summary>
    Blue,
    /// <summary>红方。</summary>
    Red
}

/// <summary>
/// 中国跳棋棋盘。
/// 使用立方体六角坐标 (q, r, s)，满足 q + r + s = 0。
/// 棋盘为六角星形（六芒星），共 121 个位置：
///   - 中心六边形 61 格（max(|q|,|r|,|s|) ≤ 4）
///   - 6 个三角形尖角各 10 格（1+2+3+4）
/// 同时管理棋子数据：放置、移动、查询。
/// </summary>
public class ChineseCheckersBoard
{
    // ====================================================================
    // 棋盘位置数据
    // ====================================================================

    /// <summary>一个棋盘位置的六角坐标。</summary>
    public struct HexCoord
    {
        public int Q;
        public int R;
        public int S;

        public HexCoord(int q, int r, int s) { Q = q; R = r; S = s; }

        /// <summary>到原点的六角距离 = max(|q|, |r|, |s|)。</summary>
        public int Distance => System.Math.Max(System.Math.Abs(Q),
                               System.Math.Max(System.Math.Abs(R), System.Math.Abs(S)));

        public override bool Equals(object obj) =>
            obj is HexCoord other && Q == other.Q && R == other.R && S == other.S;

        public override int GetHashCode() => Q * 1000003 + R * 1000033 + S;

        public static bool operator ==(HexCoord a, HexCoord b) => a.Q == b.Q && a.R == b.R && a.S == b.S;
        public static bool operator !=(HexCoord a, HexCoord b) => !(a == b);

        public override string ToString() => $"({Q}, {R}, {S})";
    }

    // ====================================================================
    // 棋盘结构
    // ====================================================================

    /// <summary>全部 121 个棋盘位置（按生成顺序）。</summary>
    public IReadOnlyList<HexCoord> Positions => positions;

    /// <summary>棋盘位置总数。</summary>
    public int Count => positions.Count;

    // ====================================================================
    // 六角坐标系定义（屏幕空间）
    // ====================================================================

    /// <summary>棋盘中心点屏幕坐标。</summary>
    public Vector2 Origin { get; private set; }

    /// <summary>+q 轴单位步进向量（hex (1,0) 对应的屏幕像素偏移）。</summary>
    public Vector2 StepQ { get; private set; }

    /// <summary>+r 轴单位步进向量（hex (0,1) 对应的屏幕像素偏移）。</summary>
    public Vector2 StepR { get; private set; }

    // ====================================================================
    // 内部数据
    // ====================================================================

    /// <summary>所有棋盘位置（有序列表）。</summary>
    private List<HexCoord> positions;

    /// <summary>位置索引字典（HexCoord → 在 positions 中的下标），用于快速查找。</summary>
    private Dictionary<HexCoord, int> positionIndex;

    /// <summary>棋子数据（与 positions 等长，pieceData[i] 表示 positions[i] 上的棋子类型）。</summary>
    private PieceType[] pieceData;

    // ====================================================================
    // 构造
    // ====================================================================

    /// <summary>
    /// 创建棋盘并生成全部 121 个位置，初始化棋子数据为空。
    /// </summary>
    /// <param name="origin">棋盘中心屏幕坐标。</param>
    /// <param name="axisX">+q 轴方向参考点（hex (1,0) 对应的屏幕位置）。</param>
    /// <param name="axisY">+r 轴方向参考点（hex (0,1) 对应的屏幕位置）。</param>
    public ChineseCheckersBoard(Vector2 origin, Vector2 axisX, Vector2 axisY)
    {
        Origin = origin;
        StepQ = new Vector2(axisX.X - origin.X, axisX.Y - origin.Y);
        StepR = new Vector2(axisY.X - origin.X, axisY.Y - origin.Y);

        positions = GeneratePositions();
        pieceData = new PieceType[positions.Count];

        // 构建位置索引字典
        positionIndex = new Dictionary<HexCoord, int>(positions.Count);
        for (int i = 0; i < positions.Count; i++)
            positionIndex[positions[i]] = i;
    }

    // ====================================================================
    // 坐标转换
    // ====================================================================

    /// <summary>
    /// 将六角坐标转换为屏幕坐标（Vector2）。
    /// </summary>
    public Vector2 HexToScreen(HexCoord hex)
    {
        return new Vector2(
            Origin.X + hex.Q * StepQ.X + hex.R * StepR.X,
            Origin.Y + hex.Q * StepQ.Y + hex.R * StepR.Y);
    }

    /// <summary>
    /// 将六角坐标转换为世界坐标（Vector3，Z=0）。
    /// </summary>
    public Vector3 HexToWorld(HexCoord hex)
    {
        var screen = HexToScreen(hex);
        return new Vector3(screen.X, screen.Y, 0f);
    }

    // ====================================================================
    // 位置查询
    // ====================================================================

    /// <summary>
    /// 判断该坐标是否是合法的棋盘位置。
    /// </summary>
    public bool IsValidPosition(HexCoord hex) => positionIndex.ContainsKey(hex);

    public bool IsBlueStart(HexCoord hex) => hex.R >= 5;

    public bool IsRedStart(HexCoord hex) => hex.R <= -5;

    // ====================================================================
    // 坐标转换：屏幕 → 世界
    // ====================================================================

    /// <summary>
    /// 通过相机将鼠标屏幕坐标转换为世界坐标（Z=0 平面）。
    /// </summary>
    /// <param name="camera">相机实体（需有 CameraComponent + TransformComponent）。</param>
    /// <param name="screenX">鼠标屏幕 X。</param>
    /// <param name="screenY">鼠标屏幕 Y。</param>
    /// <param name="viewportWidth">视口宽度（像素）。</param>
    /// <param name="viewportHeight">视口高度（像素）。</param>
    /// <returns>世界坐标（Z=0 平面）。</returns>
    public static Vector3 ScreenToWorld(Neverness.Gameplay.Entity camera,
                                         float screenX, float screenY,
                                         float viewportWidth, float viewportHeight)
    {
        ref var cam = ref camera.GetComponent<CameraComponent>();
        ref var camTransform = ref camera.GetComponent<TransformComponent>();

        // 屏幕 → NDC（-1 ~ +1）
        float ndcX = (screenX / viewportWidth) * 2f - 1f;
        float ndcY = 1f - (screenY / viewportHeight) * 2f;

        // 逆投影：NDC → 世界
        var viewProj = cam.ViewMatrix * cam.ProjectionMatrix;
        if (!Matrix4x4.Invert(viewProj, out var invViewProj))
            return camTransform.Position;

        // 近/远平面裁剪坐标
        var nearPoint = Vector4.Transform(new Vector4(ndcX, ndcY, 0f, 1f), invViewProj);
        var farPoint  = Vector4.Transform(new Vector4(ndcX, ndcY, 1f, 1f), invViewProj);

        // 透视除法
        var nearWorld = new Vector3(nearPoint.X / nearPoint.W, nearPoint.Y / nearPoint.W, nearPoint.Z / nearPoint.W);
        var farWorld  = new Vector3(farPoint.X / farPoint.W,  farPoint.Y / farPoint.W,  farPoint.Z / farPoint.W);

        // 射线与 Z=0 平面求交
        var rayDir = farWorld - nearWorld;
        if (MathF.Abs(rayDir.Z) < 1e-6f)
            return nearWorld;

        float t = -nearWorld.Z / rayDir.Z;
        return nearWorld + rayDir * t;
    }

    // ====================================================================
    // 鼠标拾取
    // ====================================================================

    /// <summary>
    /// 获取离指定世界坐标最近的棋盘位置。
    /// </summary>
    /// <param name="worldPos">世界坐标。</param>
    /// <param name="nearest">最近的棋盘位置。</param>
    /// <param name="maxDistance">最大搜索距离（超出则不命中）。负值表示不限。</param>
    /// <returns>是否找到有效位置。</returns>
    public bool GetNearestHex(Vector3 worldPos, out HexCoord nearest, float maxDistance = -1f)
    {
        nearest = default;
        float bestDistSq = float.MaxValue;

        for (int i = 0; i < positions.Count; i++)
        {
            var hex = positions[i];
            var hexWorld = HexToWorld(hex);

            float dx = hexWorld.X - worldPos.X;
            float dy = hexWorld.Y - worldPos.Y;
            float distSq = dx * dx + dy * dy;

            if (distSq < bestDistSq)
            {
                bestDistSq = distSq;
                nearest = hex;
            }
        }

        if (maxDistance > 0f && bestDistSq > maxDistance * maxDistance)
            return false;

        return bestDistSq < float.MaxValue;
    }

    /// <summary>
    /// 从鼠标屏幕坐标获取最近的棋盘位置（自动使用 Window.Size）。
    /// </summary>
    /// <param name="camera">相机实体。</param>
    /// <param name="screenX">鼠标屏幕 X。</param>
    /// <param name="screenY">鼠标屏幕 Y。</param>
    /// <param name="nearest">最近的棋盘位置。</param>
    /// <param name="maxDistance">最大搜索距离（世界坐标）。负值表示不限。</param>
    /// <returns>是否找到有效位置。</returns>
    public bool GetNearestHexFromScreen(Neverness.Gameplay.Entity camera,
                                         float screenX, float screenY,
                                         out HexCoord nearest, float maxDistance = -1f)
    {
        var (w, h) = Window.Size;
        var worldPos = ScreenToWorld(camera, screenX, screenY, w, h);
        return GetNearestHex(worldPos, out nearest, maxDistance);
    }

    // ====================================================================
    // 棋子数据操作
    // ====================================================================

    /// <summary>
    /// 获取指定位置的棋子类型。
    /// </summary>
    public PieceType GetPiece(HexCoord hex)
    {
        if (positionIndex.TryGetValue(hex, out int idx))
            return pieceData[idx];
        return PieceType.None;
    }

    /// <summary>
    /// 在指定位置放置棋子。如果该位置已有棋子则覆盖。
    /// </summary>
    public void SetPiece(HexCoord hex, PieceType type)
    {
        if (positionIndex.TryGetValue(hex, out int idx))
            pieceData[idx] = type;
    }

    /// <summary>
    /// 移除指定位置的棋子。
    /// </summary>
    public void RemovePiece(HexCoord hex)
    {
        SetPiece(hex, PieceType.None);
    }

    /// <summary>
    /// 将棋子从一个位置移动到另一个位置。
    /// 目标位置必须为空，源位置必须有棋子。
    /// 成功返回 true，失败返回 false。
    /// </summary>
    public bool MovePiece(HexCoord from, HexCoord to)
    {
        PieceType srcPiece = GetPiece(from);
        PieceType dstPiece = GetPiece(to);

        // 源位置无棋子，或目标位置已有棋子
        if (srcPiece == PieceType.None || dstPiece != PieceType.None)
            return false;

        SetPiece(to, srcPiece);
        RemovePiece(from);
        return true;
    }

    /// <summary>
    /// 判断指定位置是否为空。
    /// </summary>
    public bool IsEmpty(HexCoord hex) => GetPiece(hex) == PieceType.None;

    /// <summary>
    /// 获取指定类型棋子的所有位置。
    /// </summary>
    public List<HexCoord> GetPieces(PieceType type)
    {
        var result = new List<HexCoord>();
        for (int i = 0; i < positions.Count; i++)
        {
            if (pieceData[i] == type)
                result.Add(positions[i]);
        }
        return result;
    }

    /// <summary>
    /// 获取蓝方所有棋子位置。
    /// </summary>
    public List<HexCoord> GetBluePieces() => GetPieces(PieceType.Blue);

    /// <summary>
    /// 获取红方所有棋子位置。
    /// </summary>
    public List<HexCoord> GetRedPieces() => GetPieces(PieceType.Red);

    /// <summary>
    /// 获取所有空位。
    /// </summary>
    public List<HexCoord> GetEmptyPositions() => GetPieces(PieceType.None);

    /// <summary>
    /// 清空所有棋子数据（重置为空棋盘）。
    /// </summary>
    public void ClearPieces()
    {
        for (int i = 0; i < pieceData.Length; i++)
            pieceData[i] = PieceType.None;
    }

    // ====================================================================
    // 初始化布局
    // ====================================================================

    /// <summary>
    /// 按两人对战模式初始化棋子布局：
    /// 蓝方占据 +s 尖角（s ≥ 5），红方占据 -s 尖角（s ≤ -5）。
    /// </summary>
    public void SetupTwoPlayer()
    {
        ClearPieces();

        for (int i = 0; i < positions.Count; i++)
        {
            var hex = positions[i];
            if (IsBlueStart(hex))
                pieceData[i] = PieceType.Blue;
            else if (IsRedStart(hex))
                pieceData[i] = PieceType.Red;
        }
    }

    // ====================================================================
    // 棋盘生成（静态工厂）
    // ====================================================================

    /// <summary>
    /// 生成中国跳棋六角星棋盘的全部 121 个位置。
    ///
    /// 棋盘结构：
    ///   ① 中心六边形（61 格）：max(|q|, |r|, |s|) ≤ 4
    ///   ② +q 尖角（10 格）：q = 5..8
    ///   ③ -q 尖角（10 格）：q = -8..-5
    ///   ④ +r 尖角（10 格）：r = 5..8
    ///   ⑤ -r 尖角（10 格）：r = -8..-5
    ///   ⑥ +s 尖角（10 格）：s = 5..8（蓝方起始区）
    ///   ⑦ -s 尖角（10 格）：s = -8..-5（红方起始区）
    /// </summary>
    static List<HexCoord> GeneratePositions()
    {
        var result = new List<HexCoord>(121);

        // ① 中心六边形：所有坐标绝对值 ≤ 4
        for (int q = -4; q <= 4; q++)
        {
            for (int r = -4; r <= 4; r++)
            {
                int s = -q - r;
                if (s >= -4 && s <= 4)
                    result.Add(new HexCoord(q, r, s));
            }
        }

        // ② +q 尖角（棋盘右侧）
        for (int q = 5; q <= 8; q++)
            for (int r = -4; r <= 4 - q; r++)
                result.Add(new HexCoord(q, r, -q - r));

        // ③ -q 尖角（棋盘左侧）
        for (int q = -8; q <= -5; q++)
            for (int r = -q - 4; r <= 4; r++)
                result.Add(new HexCoord(q, r, -q - r));

        // ④ +r 尖角（右下方向）
        for (int r = 5; r <= 8; r++)
            for (int q = -4; q <= 4 - r; q++)
                result.Add(new HexCoord(q, r, -q - r));

        // ⑤ -r 尖角（左上方向）
        for (int r = -8; r <= -5; r++)
            for (int q = -4 - r; q <= 4; q++)
                result.Add(new HexCoord(q, r, -q - r));

        // ⑥ +s 尖角（左下方向，蓝方起始区）
        for (int s = 5; s <= 8; s++)
            for (int q = -4; q <= 4 - s; q++)
                result.Add(new HexCoord(q, -s - q, s));

        // ⑦ -s 尖角（右上方向，红方起始区）
        for (int s = -8; s <= -5; s++)
            for (int q = -s - 4; q <= 4; q++)
                result.Add(new HexCoord(q, -s - q, s));

        return result;
    }
}
