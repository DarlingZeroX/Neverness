using System.Collections.Generic;
using System.Numerics;
using Neverness.Gameplay;
using Neverness.Runtime.Application;
using Neverness.Runtime.Scene.Components;
using Vector2 = System.Numerics.Vector2;
using Vector3 = System.Numerics.Vector3;
using Quaternion = System.Numerics.Quaternion;

/// <summary>
/// 中国跳棋游戏实例。
/// 负责实体创建和游戏流程控制，棋盘/棋子/拾取由 ChineseCheckersBoard 管理。
/// </summary>
public class ChineseCheckersGameInstance
{
    // ====================================================================
    // 模板实体（由外部赋值）
    // ====================================================================

    /// <summary>蓝方棋子模板。</summary>
    public Entity BluePieceEntity = null;

    /// <summary>红方棋子模板。</summary>
    public Entity RedPieceEntity = null;

    /// <summary>落子提示模板。</summary>
    public Entity PlaceEntity = null;

    /// <summary>相机实体（用于鼠标拾取）。</summary>
    public Entity CameraEntity = null;

    // ====================================================================
    // 运行时状态
    // ====================================================================

    /// <summary>棋盘（含棋子数据、拾取功能）。</summary>
    public ChineseCheckersBoard Board { get; private set; }

    /// <summary>位置 → 实体映射。</summary>
    private Dictionary<ChineseCheckersBoard.HexCoord, Entity> entityMap
        = new Dictionary<ChineseCheckersBoard.HexCoord, Entity>();

    // ====================================================================
    // 构造
    // ====================================================================

    public ChineseCheckersGameInstance(Entity blue, Entity red, Entity place, Entity camera = null)
    {
        BluePieceEntity = blue;
        RedPieceEntity = red;
        PlaceEntity = place;
        CameraEntity = camera;

        Initialize();
    }

    // ====================================================================
    // 公共方法
    // ====================================================================

    /// <summary>
    /// 初始化：创建棋盘、设置布局、生成实体。
    /// </summary>
    public void Initialize()
    {
        Board = new ChineseCheckersBoard(
            origin: new Vector2(10, -5),
            axisX:  new Vector2(96, -5),
            axisY:  new Vector2(52, 70));

        Board.SetupTwoPlayer();

        Debug.Log($"棋盘初始化：共 {Board.Count} 个位置，" +
                  $"蓝方 {Board.GetBluePieces().Count} 子，" +
                  $"红方 {Board.GetRedPieces().Count} 子");

        SyncEntities();
    }

    /// <summary>
    /// 清理所有已创建的棋盘实体。
    /// </summary>
    public void ClearBoard()
    {
        foreach (var kv in entityMap)
            kv.Value.Destroy();
        entityMap.Clear();
    }

    /// <summary>
    /// 从鼠标位置获取最近的棋盘位置（自动使用 Window.Size）。
    /// </summary>
    public bool GetNearestHexFromMouse(out ChineseCheckersBoard.HexCoord nearest, float maxDistance = -1f)
    {
        nearest = default;
        if (CameraEntity == null) return false;

        var (mx, my) = Input.MousePosition;
        return Board.GetNearestHexFromScreen(CameraEntity, mx, my, out nearest, maxDistance);
    }

    // ====================================================================
    // 实体同步
    // ====================================================================

    private void SyncEntities()
    {
        ClearBoard();

        for (int i = 0; i < Board.Count; i++)
        {
            var hex = Board.Positions[i];
            var piece = Board.GetPiece(hex);
            var worldPos = Board.HexToWorld(hex);

            Entity template;
            switch (piece)
            {
                case PieceType.Blue: template = BluePieceEntity; break;
                case PieceType.Red:  template = RedPieceEntity;  break;
                default:             template = null;     break;
            }

            if(template == null)
                continue;

            var entity = Instantiate(template, worldPos);
            entityMap[hex] = entity;
        }
    }

    public void OnUpdate(float deltaTime)
    {
        if (Input.GetMouseButtonDown(MouseButton.Left))
        {
            ChineseCheckersBoard.HexCoord coord = new ChineseCheckersBoard.HexCoord();
            if (GetNearestHexFromMouse(out coord ,50))
            {
                Debug.Log(coord);
            }
        }
    }

    // ====================================================================
    // 内部方法
    // ====================================================================

    Entity Instantiate(Entity source, Vector3 position, Quaternion? rotation = null)
    {
        var clone = source.Clone();
        ref var transform = ref clone.GetComponent<TransformComponent>();
        transform.Position = position;
        transform.Rotation = rotation ?? Quaternion.Identity;
        return clone;
    }
}
