#include "EditorComponents/DetailBrowser/ComponentDrawer.h"
#include "EditorCore/Localization.h"
#include <VGImgui/IncludeImGuiEx.h>
#include <VGImgui/Include/ImGuiEx/ImGuiVector.h>
#include <VGEngine/Include/Galgame/GalGameEngine.h>
#include <VGEngine/Include/Galgame/Components.h>

#include "VGEngine/Include/UI/UISystem.h"
#include "VGEngine/Include/Galgame/GameEngineCore.h"
#include "VGEngine/Include/Interface/Loader.h"
#include "VGEngine/Include/Lua/LuaScript.h"

namespace VisionGal::Editor
{
	void TransformComponentDrawer::OnGUI(IEntity* entity)
	{
		auto* transform = entity->GetComponent<TransformComponent>();

		if (transform == nullptr)
			return;

		if (ImGui::CollapsingHeader(EditorText{ "Transform" }.c_str(), ImGuiTreeNodeFlags_DefaultOpen) == false)
			return;

		const ImVec2 buttonSize = { 70, ImGui::GetFrameHeight() };

		if (ImGui::BeginTable("TransformTable", 2, ImGuiTableFlags_SizingFixedFit))
		{

			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text(EditorText{ "Location" }.c_str());
				ImGui::TableSetColumnIndex(1);

				ImGuiEx::DrawVec3Control("Location", transform->location, 0.f, 1.0f);
			}

			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text(EditorText{ "Rotation" }.c_str());
				ImGui::TableSetColumnIndex(1);

				float3 rotation = glm::degrees(glm::eulerAngles(transform->rotation));
				float3 copy = rotation;
				ImGuiEx::DrawVec3Control("Rotation", rotation, 0.f, 1.0f, -360.f, 360.f);

				transform->rotation = quaternion( glm::radians(rotation) );
			}

			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text(EditorText{ "Scale" }.c_str());
				ImGui::TableSetColumnIndex(1);

				ImGuiEx::DrawVec3Control("Scale", transform->scale, 1.f, 0.0025f);
			}

			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text(EditorText{ "Visible" }.c_str());
				ImGui::TableSetColumnIndex(1);
				ImGui::Checkbox("##ActorVisible", &transform->visible);
			}

			ImGui::EndTable();
		}

		transform->is_dirty = true;

		ImGui::Separator();
	}

	const String TransformComponentDrawer::GetBindType() const
	{
		return TransformComponent{}.GetComponentType();
	}

	void CameraComponentDrawer::OnGUI(IEntity* entity)
	{
		if (ImGui::CollapsingHeader(EditorText{ "Camera" }.c_str(), ImGuiTreeNodeFlags_DefaultOpen) == false)
			return;

		auto* com = entity->GetComponent<CameraComponent>();

		if (com == nullptr)
			return;

		if (com->camera == nullptr)
			return;

		auto* icamera = com->camera.get();

		if (icamera->GetCameraType() == "Letterbox2D")
		{
			auto* camera = dynamic_cast<IOrthoCamera*>(icamera);

			float left = camera->GetLeft();
			float right = camera->GetRight();
			float top = camera->GetTop();
			float bottom = camera->GetBottom();
			float width = right * 2.0f;
			float height = top * 2.0f;

			ImGui::Text("left: "); ImGui::SameLine(); ImGui::Text(std::to_string(left).c_str());
			ImGui::Text("right: "); ImGui::SameLine(); ImGui::Text(std::to_string(right).c_str());
			ImGui::Text("top: "); ImGui::SameLine(); ImGui::Text(std::to_string(top).c_str());
			ImGui::Text("bottom: "); ImGui::SameLine(); ImGui::Text(std::to_string(bottom).c_str());
			ImGui::Text("width: "); ImGui::SameLine(); ImGui::Text(std::to_string(width).c_str());
			ImGui::Text("height: "); ImGui::SameLine(); ImGui::Text(std::to_string(height).c_str());
		}

	}

	const String CameraComponentDrawer::GetBindType() const
	{
		return CameraComponent{}.GetComponentType();
	}

	void SpriteRendererComponentDrawer::OnGUI(IEntity* entity)
	{
		if (ImGui::CollapsingHeader(EditorText{ "Sprite Renderer" }.c_str(), ImGuiTreeNodeFlags_DefaultOpen) == false)
			return;

		auto* com = entity->GetComponent<SpriteRendererComponent>();

		if (com == nullptr)
			return;

		std::string spritePath;

		if (com->sprite != nullptr && com->sprite->GetTexture2D() != nullptr)
		{
			spritePath = com->sprite->GetTexture2D()->GetResourcePath();
		}
		static ImGuiTableFlags flags = ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_Hideable | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV;

		if (ImGui::BeginTable("SpriteRendererComponentDrawerTable", 2, flags))
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text(EditorText{ "Sprite" }.c_str());
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
			ImGuiEx::InputText("##Sprite", spritePath, ImGuiInputTextFlags_ReadOnly);
			SpriteBeginDropTarget(com);

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text(EditorText{ "Color" }.c_str());
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
			ImGui::ColorEdit4("##Color", &com->color.x,ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_Float);

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text(EditorText{ "Flip" }.c_str());
			ImGui::TableSetColumnIndex(1);
			bool flipX = com->flip.x;
			bool flipY = com->flip.y;
			ImGui::Checkbox("X", &flipX);
			ImGui::SameLine();
			ImGui::Checkbox("Y", &flipY);
			com->flip.x = flipX;
			com->flip.y = flipY;

			ImGui::EndTable();
		}

		if (com->sprite->GetITexture() != nullptr)
		{
			auto* tex = com->sprite->GetITexture();
			ImGuiEx::ImageGL(tex->GetShaderResourceView(), 100, 100);
		}
	}

	const String SpriteRendererComponentDrawer::GetBindType() const
	{
		return SpriteRendererComponent{}.GetComponentType();
	}

	void SpriteRendererComponentDrawer::SpriteBeginDropTarget(SpriteRendererComponent* com)
	{
		if (ImGui::BeginDragDropTarget())
		{
			if (const auto* payload = ImGui::AcceptDragDropPayload("PLACE_CONTENT_BROWSER_ITEM"))
			{
				std::string data = static_cast<char*>(payload->Data);
				//std::cout << data << std::endl;
				auto text2d = LoadObject<Texture2D>(data);
				if (text2d)
				{
					com->sprite = Sprite::Create(text2d, text2d->Size());
				}

				ImGuiEx::PushNotification({ ImGuiExToastType::Info, "Drag Sprite" });
			}
			ImGui::EndDragDropTarget();
		}
	}

	void ScriptComponentDrawer::OnGUI(IEntity* entity)
	{
		if (ImGui::CollapsingHeader(EditorText{ "Script" }.c_str(), ImGuiTreeNodeFlags_DefaultOpen) == false)
			return;

		auto* com = entity->GetComponent<ScriptComponent>();

		if (com == nullptr)
			return;

		for (auto& script: com->scripts)
		{
			if (script->GetScriptType() != "LuaScript")
				continue;

			auto* luaScript = dynamic_cast<LuaScript*>(script.get());

			ImGui::Text("");
			ImGui::SameLine(20);
			if (ImGui::CollapsingHeader(script->GetResourcePath().c_str(), ImGuiTreeNodeFlags_DefaultOpen) == false)
				continue;

			static ImGuiTableFlags flags = ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV;

			if (ImGui::BeginTable("ScriptVariableTable", 3, flags))
			{
				ImGui::TableSetupColumn(EditorText{ "Name" }.c_str());
				ImGui::TableSetupColumn(EditorText{ "Type" }.c_str());
				ImGui::TableSetupColumn(EditorText{ "Value" }.c_str());

				ImGui::TableHeadersRow();

				for (auto& var: luaScript->GetVariables())
				{
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text(var.second.VariableName.c_str());
					ImGui::TableSetColumnIndex(1);
					ImGui::Text(var.second.VariableType.c_str());
					ImGui::TableSetColumnIndex(2);

					auto& v = var.second;
					String vName = "##" + v.VariableName;
					if (v.VariableType == "Number")
					{
						if (ImGui::InputDouble(vName.c_str(), &v.ValueDouble));
						{
							v.Changed = true;
						}
					}
					else if (v.VariableType == "String")
					{
						if (ImGuiEx::InputText(vName.c_str(), v.ValueString))
						{
							v.Changed = true;
						}
					}
					else if (v.VariableType == "Boolean")
					{
						if (ImGui::Checkbox(vName.c_str(), &v.ValueBoolean))
						{
							v.Changed = true;
						}
					}
					else if (v.VariableType == "GameActor")
					{
						String entityID = std::to_string(v.ValueEntityID);
						if (ImGuiEx::InputText(vName.c_str(), entityID, ImGuiInputTextFlags_ReadOnly))
						{
							v.Changed = true;
						}
					}

					ScriptBeginDropTarget(script.get(), v);
				}

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Reload");
				ImGui::TableSetColumnIndex(1);
				if (ImGui::Button(ICON_FA_REDO "##ReloadLuaScript"))
				{
					script = luaScript->Reload();
				}

				ImGui::EndTable();
			}
			//ImGui::Text(script->GetResourcePath().c_str());
		}
	}

	const String ScriptComponentDrawer::GetBindType() const
	{
		return ScriptComponent{}.GetComponentType();
	}

	void ScriptComponentDrawer::ScriptBeginDropTarget(IScript* script, IScriptVariable& var)
	{
		if (ImGui::BeginDragDropTarget())
		{
			if (const auto* payload = ImGui::AcceptDragDropPayload("PLACE_GAME_ACTOR"))
			{
				GameActor* actor = static_cast<GameActor*>(payload->Data);
				var.VariableType = "GameActor";
				var.ValueEntityID = actor->GetEntityID();

				ImGuiEx::PushNotification({ ImGuiExToastType::Info, "Drop Content Item" });
			}
			ImGui::EndDragDropTarget();
		}
	}

	void RmlUIDocumentComponentDrawer::OnGUI(IEntity* entity)
	{
		if (ImGui::CollapsingHeader(EditorText{ "UI Document" }.c_str(), ImGuiTreeNodeFlags_DefaultOpen) == false)
			return;

		auto* com = entity->GetComponent<RmlUIDocumentComponent>();

		if (com == nullptr)
			return;

		std::string documentPath;

		if (com->document != nullptr)
		{
			documentPath = com->document->GetResourcePath();
		}

		static ImGuiTableFlags flags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Hideable | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV;

		if (ImGui::BeginTable("VideoPlayerComponentDrawerTable", 2, flags))
		{
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text(EditorText{ "Document" }.c_str());
				ImGui::TableSetColumnIndex(1);
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 20);
				ImGuiEx::InputText("##UIDocument", documentPath, ImGuiInputTextFlags_ReadOnly);
				DocumentBeginDropTarget(com);
				ImGui::SameLine();
				if (ImGui::Button(ICON_FA_TIMES "##RemoveVideoClip"))
				{
					//UISystem::Get()->
					com->document->Close();
					com->document = nullptr;
				}
			}

			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text(EditorText{ "Reload" }.c_str());
				ImGui::TableSetColumnIndex(1);
				if (ImGui::Button(ICON_FA_REDO "##ReloadUIDocument"))
				{
					H_ASSERT_NOT_NULL(com->document)
					UISystem::Get()->ReloadUIDocument(com->document);
				}}


			ImGui::EndTable();
		}
	}

	const String RmlUIDocumentComponentDrawer::GetBindType() const
	{
		return RmlUIDocumentComponent{}.GetComponentType();
	}

	void RmlUIDocumentComponentDrawer::DocumentBeginDropTarget(RmlUIDocumentComponent* com)
	{
		if (ImGui::BeginDragDropTarget())
		{
			if (const auto* payload = ImGui::AcceptDragDropPayload("PLACE_CONTENT_BROWSER_ITEM"))
			{
				std::string data = static_cast<char*>(payload->Data);
				//std::cout << data << std::endl;
				//auto video = LoadObject<VideoClip>(data);
				//if (video)
				//{
				//	com->videoClip = video;
				//	//com->Play();
				//}
				com->document = UISystem::Get()->LoadUIDocument(data);
				UISystem::Get()->ShowUIDocument(com->document.get());

				ImGuiEx::PushNotification({ ImGuiExToastType::Info, "Drop Video" });
			}
			ImGui::EndDragDropTarget();
		}
	}

	void AudioSourceComponentDrawer::OnGUI(IEntity* entity)
	{
		if (ImGui::CollapsingHeader(EditorText{ "Audio Source" }.c_str(), ImGuiTreeNodeFlags_DefaultOpen) == false)
			return;

		auto* com = entity->GetComponent<AudioSourceComponent>();

		if (com == nullptr)
			return;

		std::string audioPath;

		if (com->audioClip != nullptr)
		{
			audioPath = com->audioClip->GetResourcePath();
		}

		static ImGuiTableFlags flags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Hideable | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV;

		if (ImGui::BeginTable("AudioSourceComponentDrawerTable", 2, flags))
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text(EditorText{ "Audio Clip" }.c_str());
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 20);
			ImGuiEx::InputText("##AudioSource", audioPath, ImGuiInputTextFlags_ReadOnly);
			AudioSourceBeginDropTarget(com);
			ImGui::SameLine();
			if (ImGui::Button(ICON_FA_TIMES "##RemoveAudioSource"))
			{
				com->audioClip = nullptr;
			}

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text(EditorText{ "Play On Awake" }.c_str());
			ImGui::TableSetColumnIndex(1);
			ImGui::Checkbox("##Play On Awake", &com->playOnAwake);

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text(EditorText{ "Loop" }.c_str());
			ImGui::TableSetColumnIndex(1);
			ImGui::Checkbox("##Loop", &com->loop);

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text(EditorText{ "Volume" }.c_str());
			ImGui::TableSetColumnIndex(1);
			float volume = com->GetVolume();
			ImGui::SliderFloat("##Volume", &volume, 0 ,1.0);
			com->SetVolume(volume);

			ImGui::EndTable();
		}
	}

	const String AudioSourceComponentDrawer::GetBindType() const
	{
		return AudioSourceComponent{}.GetComponentType();
	}

	void AudioSourceComponentDrawer::AudioSourceBeginDropTarget(AudioSourceComponent* com)
	{
		if (ImGui::BeginDragDropTarget())
		{
			if (const auto* payload = ImGui::AcceptDragDropPayload("PLACE_CONTENT_BROWSER_ITEM"))
			{
				std::string data = static_cast<char*>(payload->Data);
				//std::cout << data << std::endl;
				auto audioClip = LoadObject<AudioClip>(data);
				if (audioClip)
				{
					com->audioClip = audioClip;
					//com->Play();
				}

				ImGuiEx::PushNotification({ ImGuiExToastType::Info, "Drop Video" });
			}
			ImGui::EndDragDropTarget();
		}
	}

	void VideoPlayerComponentDrawer::OnGUI(IEntity* entity)
	{
		if (ImGui::CollapsingHeader(EditorText{ "Video Player" }.c_str(), ImGuiTreeNodeFlags_DefaultOpen) == false)
			return;

		auto* com = entity->GetComponent<VideoPlayerComponent>();

		if (com == nullptr)
			return;

		std::string videoPath;

		if (com->videoClip != nullptr)
		{
			videoPath = com->videoClip->GetResourcePath();
		}
		static ImGuiTableFlags flags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Hideable | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV;

		if (ImGui::BeginTable("VideoPlayerComponentDrawerTable", 2, flags))
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text(EditorText{ "Video Clip" }.c_str());
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 20);
			ImGuiEx::InputText("##VideoClip", videoPath, ImGuiInputTextFlags_ReadOnly);
			VideoPlayerBeginDropTarget(com);
			ImGui::SameLine();
			if (ImGui::Button(ICON_FA_TIMES "##RemoveVideoClip"))
			{
				com->videoClip = nullptr;
			}

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text(EditorText{ "Play On Awake" }.c_str());
			ImGui::TableSetColumnIndex(1);
			ImGui::Checkbox("##Play On Awake", &com->playOnAwake);

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text(EditorText{ "Wait For First Frame" }.c_str());
			ImGui::TableSetColumnIndex(1);
			ImGui::Checkbox("##Wait For First Frame", &com->waitForFirstFrame);

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text(EditorText{ "Loop" }.c_str());
			ImGui::TableSetColumnIndex(1);
			ImGui::Checkbox("##Loop", &com->loop);

			ImGui::EndTable();
		}

		if (com->videoPlayer == nullptr)
			return;

		if (com->videoPlayer->GetSprite() == nullptr)
			return;

		if (com->videoPlayer->GetSprite()->GetITexture() != nullptr)
		{
			auto* tex = com->videoPlayer->GetSprite()->GetITexture();
			ImGuiEx::ImageGL(tex->GetShaderResourceView(), 100, 100);
		}
	}

	const String VideoPlayerComponentDrawer::GetBindType() const
	{
		return VideoPlayerComponent{}.GetComponentType();
		//return "VideoPlayer";
	}

	void VideoPlayerComponentDrawer::VideoPlayerBeginDropTarget(VideoPlayerComponent* com)
	{
		if (ImGui::BeginDragDropTarget())
		{
			if (const auto* payload = ImGui::AcceptDragDropPayload("PLACE_CONTENT_BROWSER_ITEM"))
			{
				std::string data = static_cast<char*>(payload->Data);
				//std::cout << data << std::endl;
				auto video = LoadObject<VideoClip>(data);
				if (video)
				{
					com->videoClip = video;
					//com->Play();
				}

				ImGuiEx::PushNotification({ ImGuiExToastType::Info, "Drop Video" });
			}
			ImGui::EndDragDropTarget();
		}
	}

	void GalGameEngineComponentDrawer::OnGUI(IEntity* entity)
	{
		//if (ImGui::CollapsingHeader("Plot Script", ImGuiTreeNodeFlags_DefaultOpen) == false)
		//	return;
		//
		//auto* com = entity->GetComponent<GalGame::GalGameEngineComponent>();
		//
		//if (com == nullptr)
		//	return;
		//
		//ImGui::Text("Path: %s", com->path.c_str());
		//
		//if (ImGui::Button(ICON_FA_REDO "##ReloadPlot"))
		//{
		//	GalGame::GalGameEngine::Get()->ReloadPlotScript();
		//}

		if (ImGui::CollapsingHeader(EditorText{ "GalGame Engine" }.c_str(), ImGuiTreeNodeFlags_DefaultOpen) == false)
			return;

		auto* com = entity->GetComponent<GalGame::GalGameEngineComponent>();

		if (com == nullptr)
			return;

		std::string scriptPath;

		if (com->script != nullptr)
		{
			scriptPath = com->script->GetResourcePath();
		}

		static ImGuiTableFlags flags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Hideable | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV;

		if (ImGui::BeginTable("GalGameEngineComponentDrawerTable", 2, flags))
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text(EditorText{ "Story Script" }.c_str());
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 20);
			ImGuiEx::InputText("##GalGameStoryScript", scriptPath, ImGuiInputTextFlags_ReadOnly);
			ScriptBeginDropTarget(com);
			ImGui::SameLine();
			if (ImGui::Button(ICON_FA_TIMES "##RemoveAudioSource"))
			{
				com->script = nullptr;
			}

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text(EditorText{ "Reload Story Script" }.c_str());
			ImGui::TableSetColumnIndex(1);
			if (ImGui::Button(ICON_FA_REDO "##Reload Story Script"))
			{
				dynamic_cast<GalGame::GalGameEngine*>(GalGame::GameEngineCore::GetCurrentEngine())->ReloadStoryScript();
			}

			ImGui::EndTable();
		}
	}

	const String GalGameEngineComponentDrawer::GetBindType() const
	{
		//return GalGame::StoryScriptComponent{}.GetComponentType();
		return GalGame::GalGameEngineComponent{}.GetComponentType();
	}

	void GalGameEngineComponentDrawer::ScriptBeginDropTarget(GalGame::GalGameEngineComponent* com)
	{
		if (ImGui::BeginDragDropTarget())
		{
			if (const auto* payload = ImGui::AcceptDragDropPayload("PLACE_CONTENT_BROWSER_ITEM"))
			{
				std::string path = static_cast<char*>(payload->Data);

				com->script = GalGame::LuaStoryScript::LoadFromFile(path);

				ImGuiEx::PushNotification({ ImGuiExToastType::Info, "Drop Video" });
			}
			ImGui::EndDragDropTarget();
		}
	}
}
