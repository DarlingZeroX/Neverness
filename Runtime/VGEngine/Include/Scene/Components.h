/*
* This source file is part of VisionGal, the Visual Novel Engine
*
* For the latest information, see https://darlingzerox.github.io/VisionGalDoc/
* GitHub page: https://github.com/DarlingZeroX/VisionGal
*
* Copyright (c) 2025-present 梦旅缘心
*
* See the LICENSE file in the project root for details.
*/

#pragma once
#include "../Core/Core.h"
#include "../Render/Sprite.h"
#include "../Resource/Audio.h"
#include "../Resource/Video.h"
#include "../Resource/UIDocument.h"
#include "../Render/Material.h"
#include "../Render/Transition.h"
#include "../Interface/SceneInterface.h"
#include "../Interface/GameInterface.h"
#include <VGEngine/Include/EngineConfig.h>
#include <HCore/Include/Core/HSerialization.h>

namespace VisionGal {

	struct VG_ENGINE_API TransformComponent : public IComponent
	{
		TransformComponent();
		TransformComponent(const TransformComponent&) = default;
		TransformComponent& operator=(const TransformComponent&) = default;
		TransformComponent(TransformComponent&&) noexcept = default;
		TransformComponent& operator=(TransformComponent&&) noexcept = default;
		~TransformComponent() override = default;
		 
		// 通过 IComponent 继承
		std::string GetComponentType() const override;
		 
		void Update();

		template <class Archive>
		void serialize(Archive& ar) {
			serializeIComponent(ar);
			ar(cereal::make_nvp("location", location),
				cereal::make_nvp("rotation", rotation),
				cereal::make_nvp("scale", scale),
				cereal::make_nvp("visible", visible));
		}
	public:
		float3 location;
		quaternion rotation;
		float3 scale;
		matrix4x4 model;
		bool is_dirty;
		bool visible;
	};

	struct VG_ENGINE_API SpriteRendererComponent: public IComponent
	{
		SpriteRendererComponent();
		SpriteRendererComponent(const SpriteRendererComponent&) = default;
		SpriteRendererComponent& operator=(const SpriteRendererComponent&) = default;
		SpriteRendererComponent(SpriteRendererComponent&&) noexcept = default;
		SpriteRendererComponent& operator=(SpriteRendererComponent&&) noexcept = default;
		~SpriteRendererComponent() override = default;

		// 通过 IComponent 继承
		std::string GetComponentType() const override;

		struct DeserializeData
		{
			bool HasSprite = false;
			float2 m_SpriteSize;
			float2 m_SpritePosition;
			String m_SpriteTexturePath;
		};

		DeserializeData __DeserializeData;

		template <class Archive>
		void save(Archive& archive) const
		{
			saveIComponent(archive);
			if (sprite != nullptr)
			{
				archive(cereal::make_nvp("HasSprite", true));
				archive(cereal::make_nvp("m_SpriteSize", sprite->GetSize()));
				archive(cereal::make_nvp("m_SpritePosition", sprite->GetPosition()));
				if (sprite->GetTexture2D())
				{
					archive(cereal::make_nvp("m_SpriteTexturePath", sprite->GetTexture2D()->GetResourcePath()));
				}
				else
				{
					archive(cereal::make_nvp("m_SpriteTexturePath", Core::GetDefaultSpriteTexturePath()));
				}
			}
			else
			{
				archive(cereal::make_nvp("HasSprite", false));
			}

			archive(cereal::make_nvp("Color",color));
			archive(cereal::make_nvp("Flip", flip));
		}

		template<class Archive>
		void load(Archive& archive) {
			loadIComponent(archive);
			archive(__DeserializeData.HasSprite);

			if (__DeserializeData.HasSprite) {
				archive(__DeserializeData.m_SpriteSize);
				archive(__DeserializeData.m_SpritePosition);
				archive(__DeserializeData.m_SpriteTexturePath);
			}
			else {
				sprite = nullptr;
			}

			archive(color);
			archive(flip);
		}

		uint pipelineIndex;
		Ref<Sprite> sprite;
		float4 color;
		int2 flip;
		Ref<Material> material;
	};

	struct VG_ENGINE_API FullScreenRendererComponent : public IComponent
	{
		FullScreenRendererComponent();
		FullScreenRendererComponent(const FullScreenRendererComponent&) = default;
		FullScreenRendererComponent& operator=(const FullScreenRendererComponent&) = default;
		FullScreenRendererComponent(FullScreenRendererComponent&&) noexcept = default;
		FullScreenRendererComponent& operator=(FullScreenRendererComponent&&) noexcept = default;
		~FullScreenRendererComponent() override = default;

		// 通过 IComponent 继承
		std::string GetComponentType() const override;
	public:
		Ref<FullScreenQuad> image;
		VGFX::IShaderProgram* shader;
	};

	struct VG_ENGINE_API ScriptComponent: public IComponent
	{
		ScriptComponent();
		ScriptComponent(const ScriptComponent&) = default;
		ScriptComponent& operator=(const ScriptComponent&) = default;
		ScriptComponent(ScriptComponent&&) noexcept = default;
		ScriptComponent& operator=(ScriptComponent&&) noexcept = default;
		~ScriptComponent() override = default;

		struct SerializeScriptData
		{
			String Type;
			String Path;
			std::unordered_map<String, IScriptVariable>  Variables;

			// Cereal 序列化函数
			template <class Archive>
			void save(Archive& archive) const {
				archive(CEREAL_NVP(Type), CEREAL_NVP(Path));
				archive(cereal::make_nvp("VariableSize", Variables.size()));
				if (!Variables.empty())
					archive(cereal::make_nvp("Variables", Variables));
			}

			template <class Archive>
			void load(Archive& archive) {
				archive(Type, Path);
				int variableSize = 0;
				archive(variableSize);
				if (variableSize > 0)
					archive(Variables);
			}
		};

		struct DeserializeData
		{
			std::vector<SerializeScriptData> m_ScriptsData;
		};

		DeserializeData __DeserializeData;

		template <class Archive>
		void save(Archive& archive) const
		{
			saveIComponent(archive);

			std::vector<SerializeScriptData> scriptsData;
			for (auto& script: scripts)
			{
				if (script == nullptr)
					continue;

				scriptsData.push_back({ 
					script->GetScriptType(),
					script->GetResourcePath(),
					script->GetVariables()
				});
			}

			archive(cereal::make_nvp("ScriptsData",scriptsData));
		}

		template<class Archive>
		void load(Archive& archive) {
			loadIComponent(archive);

			archive(__DeserializeData.m_ScriptsData);
		}

		// 通过 IComponent 继承
		std::string GetComponentType() const override;
	public:
		std::vector<Ref<IScript>>  scripts;
	};

	struct VG_ENGINE_API CameraComponent: public IComponent
	{
		CameraComponent();
		CameraComponent(const CameraComponent&) = default;
		CameraComponent& operator=(const CameraComponent&) = default;
		CameraComponent(CameraComponent&&) noexcept = default;
		CameraComponent& operator=(CameraComponent&&) noexcept = default;
		~CameraComponent() override = default;

		// 通过 IComponent 继承
		std::string GetComponentType() const override;

		struct DeserializeData
		{
			bool HasCamera = false;
			std::string m_CameraType;
		};

		DeserializeData __DeserializeData;

		template <class Archive>
		void save(Archive& ar) const
		{
			saveIComponent(ar);

			if (camera != nullptr)
			{
				ar(cereal::make_nvp("HasCamera", true));
				ar(cereal::make_nvp("m_CameraType", camera->GetCameraType()));
			}
			else
			{
				ar(cereal::make_nvp("HasCamera", false));
			}
		}

		template<class Archive>
		void load(Archive& archive) {
			loadIComponent(archive);
			archive(__DeserializeData.HasCamera);

			if (__DeserializeData.HasCamera) {
				archive(__DeserializeData.m_CameraType);
			}
			else {
				camera = nullptr;
			}
		}

		Ref<ICamera> camera;
	};

	 struct VG_ENGINE_API VideoPlayerComponent: public IComponent
	 {
		 VideoPlayerComponent();
		 VideoPlayerComponent(const VideoPlayerComponent&) = default;
		 VideoPlayerComponent& operator=(const VideoPlayerComponent&) = default;
		 VideoPlayerComponent(VideoPlayerComponent&&) noexcept = default;
		 VideoPlayerComponent& operator=(VideoPlayerComponent&&) noexcept = default;
		 ~VideoPlayerComponent() override = default;

		 // 通过 IComponent 继承
		 std::string GetComponentType() const override;

		 struct DeserializeData
		 {
			 bool HasVideo = false;
			 float2 m_VideoSize;
			 String m_VideoPath;
		 };

		 DeserializeData __DeserializeData;

		 template <class Archive>
		 void save(Archive& archive) const
		 {
			 saveIComponent(archive);
			 if (videoClip != nullptr)
			 {
				 archive(cereal::make_nvp("HasVideo", true));
				 archive(cereal::make_nvp("m_VideoSize", videoClip->GetSize()));
				 archive(cereal::make_nvp("m_VideoPath", videoClip->GetResourcePath()));
			 }
			 else
			 {
				 archive(cereal::make_nvp("HasSprite", false));
			 }

			 archive(cereal::make_nvp("playOnAwake", playOnAwake));
			 archive(cereal::make_nvp("waitForFirstFrame", waitForFirstFrame));
			 archive(cereal::make_nvp("loop", loop));
		 }

		 template<class Archive>
		 void load(Archive& archive) {
			 loadIComponent(archive);
			 archive(__DeserializeData.HasVideo);

			 if (__DeserializeData.HasVideo) {
				 archive(__DeserializeData.m_VideoSize);
				 archive(__DeserializeData.m_VideoPath);
			 }
			 else {
				 videoClip = nullptr;
			 }

			 archive(playOnAwake);
			 archive(waitForFirstFrame);
			 archive(loop);
		 }

		 uint pipelineIndex;
		 Ref<VideoClip> videoClip;
		 bool playOnAwake = true;
		 bool waitForFirstFrame = true;
		 bool loop = false;
		 Ref<VideoPlayer> videoPlayer;

		 bool Play();
		 void Stop();
		 void Update();

		 // 循环播放
		 void SetLoop(bool enable);
		 // 是否正在播放
		 bool IsPlaying() const;
		 // 是否循环播放
		 bool IsLooping() const;
	 };

	 struct VG_ENGINE_API AudioSourceComponent : public IComponent
	 {
		 AudioSourceComponent();
		 AudioSourceComponent(const AudioSourceComponent&) = default;
		 AudioSourceComponent& operator=(const AudioSourceComponent&) = default;
		 AudioSourceComponent(AudioSourceComponent&&) noexcept = default;
		 AudioSourceComponent& operator=(AudioSourceComponent&&) noexcept = default;
		 ~AudioSourceComponent() override = default;

		 // 通过 IComponent 继承
		 std::string GetComponentType() const override;

		 struct DeserializeData
		 {
			 bool HasAudio = false;
			 String m_AudioPath;
		 };

		 DeserializeData __DeserializeData;

		 template <class Archive>
		 void save(Archive& archive) const
		 {
			 saveIComponent(archive);
			 if (audioClip != nullptr)
			 {
				 archive(cereal::make_nvp("HasAudio", true));
				 archive(cereal::make_nvp("m_AudioPath", audioClip->GetResourcePath()));
			 }
			 else
			 {
				 archive(cereal::make_nvp("HasAudio", false));
			 }

			 archive(cereal::make_nvp("playOnAwake", playOnAwake));
			 archive(cereal::make_nvp("loop", loop));
			 archive(cereal::make_nvp("volume", volume));
		 }

		 template<class Archive>
		 void load(Archive& archive) {
			 loadIComponent(archive);
			 archive(__DeserializeData.HasAudio);

			 if (__DeserializeData.HasAudio) {
				 archive(__DeserializeData.m_AudioPath);
			 }
			 else {
				 audioClip = nullptr;
			 }

			 archive(playOnAwake);
			 archive(loop);
			 archive(volume);
		 }

		 Ref<AudioClip> audioClip;
		 bool playOnAwake = true;
		 bool loop = false;
		 float volume = 1.0f;
		 Ref<AudioPlayer> audioPlayer;

		// 播放
		 bool Play();
		 // 停止播放
		 void Stop();
		// 更新
		 void Update();
		 // 设置音量
		 void SetVolume();
		 // 设置音量
		 void SetVolume(float volume);
		 // 获取音量
		 float GetVolume() const;
		 // 循环播放
		 void SetLoop(bool enable);
		 // 是否正在播放音频
		 bool IsPlayingAudio() const;
		 // 是否循环播放
		 bool IsLooping() const;
	 };

	struct VG_ENGINE_API RmlUIDocumentComponent: public IComponent
	{
		RmlUIDocumentComponent();
		RmlUIDocumentComponent(const RmlUIDocumentComponent&) = default;
		RmlUIDocumentComponent& operator=(const RmlUIDocumentComponent&) = default;
		RmlUIDocumentComponent(RmlUIDocumentComponent&&) noexcept = default;
		RmlUIDocumentComponent& operator=(RmlUIDocumentComponent&&) noexcept = default;
		~RmlUIDocumentComponent() override = default;

		// 通过 IComponent 继承
		std::string GetComponentType() const override;

		struct DeserializeData
		{
			bool HasDocument = false;
			String m_DocumentPath;
		};

		DeserializeData __DeserializeData;

		template <class Archive>
		void save(Archive& archive) const
		{
			saveIComponent(archive);
			if (document != nullptr)
			{
				archive(cereal::make_nvp("HasDocument", true));
				archive(cereal::make_nvp("m_DocumentPath", document->GetResourcePath()));
			}
			else
			{
				archive(cereal::make_nvp("HasDocument", false));
			}
		}

		template<class Archive>
		void load(Archive& archive) {
			loadIComponent(archive);
			archive(__DeserializeData.HasDocument);

			if (__DeserializeData.HasDocument) {
				archive(__DeserializeData.m_DocumentPath);
			}
			else {
				document = nullptr;
			}
		}
	public:
		Ref<RmlUIDocument> document = nullptr;
	};

	class AnimationScriptComponent : public IComponent
	{
	public:
		AnimationScriptComponent() = default;
		AnimationScriptComponent(const AnimationScriptComponent&) = default;
		AnimationScriptComponent& operator=(const AnimationScriptComponent&) = default;
		AnimationScriptComponent(AnimationScriptComponent&&) noexcept = default;
		AnimationScriptComponent& operator=(AnimationScriptComponent&&) noexcept = default;
		~AnimationScriptComponent() override = default;

		// 通过 IComponent 继承
		std::string GetComponentType() const override;

	public:
		std::vector<Ref<IAnimationScript>> scripts;
	};
}
