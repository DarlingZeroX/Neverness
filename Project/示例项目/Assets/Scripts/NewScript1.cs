// ============================================================================
// NewScript1.cs
// ============================================================================
// Auto-generated C# script using Neverness Gameplay Framework.
// ============================================================================

using Neverness.Gameplay;
using Neverness.Runtime.Scene.Components;
using Vector3 = System.Numerics.Vector3;

namespace Game.Scripts
{
    /// <summary>
    /// NewScript1 脚本。
    /// </summary>
    [AutoRegisterScript]
    public class GameManager : EntityBehaviour
    {
        // ========================================================================
        // 公共字段（可在 Inspector 中编辑）
        // ========================================================================

        public Entity AxisX = null;
        public Entity AxisY = null;
        public Entity AxisZ = null;
        public Entity _Camera;
        private bool _IsInitialize = false;

        private static GameManager __Instance = null;

        /// <summary>移动速度。</summary>
        public float Speed = 5.0f;

        private ChineseCheckersGameInstance _GameInstance;
        // ========================================================================
        // 生命周期回调
        // ========================================================================
        public static GameManager GetInstance()
        {
            return __Instance;
        }

        private void Initialize()
        {
            if (_IsInitialize)
                return;

            if (AxisX == null || AxisY == null || AxisZ == null)
                return;

            _IsInitialize = true;
            Debug.Log("Initialize");
            //Instantiate(AxisX, new Vector3(500, 500, 0));

            _GameInstance = new ChineseCheckersGameInstance(AxisX, AxisY, AxisZ, _Camera);
        }

        /// <summary>
        /// 组件创建时调用（Awake）。
        /// ⚠️ 禁止在此访问其他 Entity。
        /// </summary>
        public override void OnCreate()
        {
            base.OnCreate();
            Debug.Log("GameManager: OnCreate");
        }

        /// <summary>
        /// 首次 Update 前调用（Start）。
        /// 可以安全访问其他 Entity。
        /// </summary>
        public override void OnStart()
        {
            __Instance = this;
            base.OnStart();
            Debug.Log("GameManager: OnStart");
        }

        /// <summary>
        /// 每帧调用。
        /// </summary>
        /// <param name="deltaTime">帧间隔时间（秒）。</param>
        public override void OnUpdate(float deltaTime)
        {
            base.OnUpdate(deltaTime);
            Initialize();
            // TODO: 在此添加游戏逻辑

            if (_IsInitialize == false)
                return;

            _GameInstance.CameraEntity = _Camera;
            _GameInstance.OnUpdate(deltaTime);
            //Debug.Log(AxisX.ToString() + AxisY.ToString() + AxisZ.ToString());
        }

        /// <summary>
        /// 固定时间步调用（适用于物理模拟）。
        /// </summary>
        /// <param name="fixedDeltaTime">固定时间步（秒）。</param>
        public override void OnFixedUpdate(float fixedDeltaTime)
        {
            base.OnFixedUpdate(fixedDeltaTime);

            // TODO: 在此添加物理相关逻辑
        }

        /// <summary>
        /// 每帧末尾调用（适用于相机跟随等后处理）。
        /// </summary>
        /// <param name="deltaTime">帧间隔时间（秒）。</param>
        public override void OnLateUpdate(float deltaTime)
        {
            base.OnLateUpdate(deltaTime);

            // TODO: 在此后处理逻辑
        }

        /// <summary>
        /// 组件销毁时调用。
        /// </summary>
        public override void OnDestroy()
        {
            Debug.Log("NewScript1: OnDestroy");
            base.OnDestroy();
        }
    }
}
