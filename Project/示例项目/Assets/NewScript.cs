// ============================================================================
// NewScript.cs
// ============================================================================
// Auto-generated C# script using Neverness Gameplay Framework.
// ============================================================================

using Neverness.Gameplay;

namespace Game.Scripts
{
    /// <summary>
    /// NewScript 脚本。
    /// </summary>
    [AutoRegisterScript]
    public class NewScript : EntityBehaviour
    {
        // ========================================================================
        // 公共字段（可在 Inspector 中编辑）
        // ========================================================================

        /// <summary>移动速度。</summary>
        public float Speed = 5.0f;

        // ========================================================================
        // 生命周期回调
        // ========================================================================

        /// <summary>
        /// 组件创建时调用（Awake）。
        /// ⚠️ 禁止在此访问其他 Entity。
        /// </summary>
        public override void OnCreate()
        {
            base.OnCreate();
            Debug.Log("NewScript: OnCreate");
        }

        /// <summary>
        /// 首次 Update 前调用（Start）。
        /// 可以安全访问其他 Entity。
        /// </summary>
        public override void OnStart()
        {
            base.OnStart();
            Debug.Log("NewScript: OnStart");
        }

        /// <summary>
        /// 每帧调用。
        /// </summary>
        /// <param name="deltaTime">帧间隔时间（秒）。</param>
        public override void OnUpdate(float deltaTime)
        {
            base.OnUpdate(deltaTime);
            Debug.Log("NewScript: OnUpdate1");
            //TransformComponent? transform = GetComponent<TransformComponent>();
            //Debug.Log("Has TransformComponent:" + HasComponent<TransformComponent>());
            //Debug.Log("HasValue TransformComponent:" + transform.HasValue);
            //if (transform.HasValue)
            //{
            //    Debug.Log("Current Position: " + transform.Value.Position);
            //    TransformComponent com = transform.Value;
            //    com.Position.X = 10;
            //    SetComponent(com);
            //}
            // TODO: 在此添加游戏逻辑
        }

        /// <summary>
        /// 固定时间步调用（适用于物理模拟）。
        /// </summary>
        /// <param name="fixedDeltaTime">固定时间步（秒）。</param>
        public override void OnFixedUpdate(float fixedDeltaTime)
        {
            base.OnFixedUpdate(fixedDeltaTime);
            Debug.Log("NewScript: OnFixedUpdate");
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
            Debug.Log("NewScript: OnDestroy");
            base.OnDestroy();
        }
    }
}
