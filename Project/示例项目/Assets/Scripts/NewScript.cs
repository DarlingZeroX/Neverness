// ============================================================================
// NewScript.cs
// ============================================================================
// Auto-generated C# script using Neverness Gameplay Framework.
// ============================================================================

using Neverness.Gameplay;
using Neverness.Runtime.Scene.Components;

namespace Game.Scripts
{
    /// <summary>
    /// NewScript 脚本。
    /// </summary>
    [AutoRegisterScript]
    public class NewScript1 : EntityBehaviour
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
            Debug.Log("Camera: OnCreate");
        }

        /// <summary>
        /// 首次 Update 前调用（Start）。
        /// 可以安全访问其他 Entity。
        /// </summary>
        public override void OnStart()
        {
            base.OnStart();
            Debug.Log("Camera: OnStart");
        }

        /// <summary>
        /// 每帧调用。
        /// </summary>
        /// <param name="deltaTime">帧间隔时间（秒）。</param>
        public override void OnUpdate(float deltaTime)
        {
            base.OnUpdate(deltaTime);
            ref var camera = ref GetComponent<CameraComponent>();
            var (width, height) = Window.Size;

            //Debug.Log(actualWidth.ToString() + actualHeight.ToString());

            float designWidth = 2732;
            float designHeight = 1534;
            float actualWidth = width;
            float actualHeight = height;

            // 计算设计宽高比与实际宽高比
            float designAspect = designWidth / designHeight;
            float actualAspect = actualWidth / actualHeight;

            bool screen_small = (designWidth >= actualWidth) && (designHeight >= actualHeight);

            if (actualAspect > designAspect)
            {
                // 实际屏幕更宽，固定垂直范围，缩放水平范围
                float scale = designAspect / actualAspect;

                // 显示的屏幕小于逻辑尺寸要换为倒数
                if (screen_small)
                    scale = 1.0f / scale;

                designWidth *= scale;
            }
            else
            {
                // 实际屏幕更高，固定水平范围，缩放垂直范围
                float scale = actualAspect / designAspect;

                // 显示的屏幕小于逻辑尺寸要换为倒数
                if (screen_small)
                    scale = 1.0f / scale;

                designHeight *= scale;
            }

            camera.OrthographicSize = designHeight;
            camera.AspectRatio = designWidth / designHeight;

            GameManager.GetInstance()._Camera = Entity;
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
            Debug.Log("NewScript: OnDestroy");
            base.OnDestroy();
        }
    }
}
