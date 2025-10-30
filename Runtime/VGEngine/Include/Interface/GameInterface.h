#pragma once
#include "AppInterface.h"
#include "../Core/Core.h"
#include <HCore/Include/Scene/HEntityInterface.h>

namespace VisionGal
{
	struct IViewport;

	struct IScriptVariable
	{
		int NativeType;
		String VariableType;
		String VariableName;
		bool Changed = false;

		EntityID ValueEntityID;
		VGPath ValuePath;
		String ValueString;
		bool ValueBoolean;
		double ValueDouble;

		template <class Archive>
		void serialize(Archive& archive) {
			archive(cereal::make_nvp("NativeType", NativeType));
			archive(cereal::make_nvp("VariableType", VariableType));
			archive(cereal::make_nvp("VariableName", VariableName));
			archive(cereal::make_nvp("Changed", Changed));
			archive(cereal::make_nvp("ValueEntityID", ValueEntityID));
			archive(cereal::make_nvp("ValuePath", ValuePath));
			archive(cereal::make_nvp("ValueString", ValueString));
			archive(cereal::make_nvp("ValueBoolean", ValueBoolean));
			archive(cereal::make_nvp("ValueDouble", ValueDouble));
		}
	};

	struct IScript: public VGEngineResource
	{
		virtual ~IScript() = default;

		virtual void Awake(GameActor* actor) {};
		virtual void Start(GameActor* actor) {};
		virtual void Update(GameActor* actor, float deltaTime) {};
		virtual void FixedUpdate(GameActor* actor) {};

		virtual String GetScriptType() = 0;
		virtual std::unordered_map<String, IScriptVariable>& GetVariables() = 0;
	};

	struct ICamera
	{
		virtual ~ICamera();

		virtual const matrix4x4& GetMatrix() = 0;
		virtual void OnViewportSizeChange(int width, int height) = 0;
		virtual std::string GetCameraType() = 0;

		virtual void AttachViewport(IViewport* viewport);
	private:
		std::vector<IViewport*> m_Viewports;
	};

	struct IViewport
	{
		virtual ~IViewport() = default;

		virtual void AttachCamera(ICamera* camera) = 0;
		virtual bool RemoveCamera(ICamera* camera) = 0;
	};

	struct IOrthoCamera: public ICamera
	{
		~IOrthoCamera() override = default;
		std::string GetCameraType() override { return "Ortho"; }

		virtual float GetLeft() = 0;
		virtual float GetRight() = 0;
		virtual float GetTop() = 0;
		virtual float GetBottom() = 0;
		virtual float GetWidth() = 0;
		virtual float GetHeight() = 0;
	};

	struct IAnimationScript
	{
		virtual ~IAnimationScript() = default;

		virtual void Start() {}
		virtual bool IsFinished() { return true; }
		virtual void OnUpdate(Horizon::HEntityInterface* actor) {};
		virtual void OnFixUpdate(Horizon::HEntityInterface* actor) {};
	};
}