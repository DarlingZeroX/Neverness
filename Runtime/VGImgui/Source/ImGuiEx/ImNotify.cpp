#include "pch.h"
#include <vector>
#include "ImGuiEx/ImNotify.h"
#include <HCore/Include/System/HSystemTimer.h>

#include "HCore/Include/Core/HLocalization.h"
#include "ImGuiEx/IconFont/IconsFontAwesome5Pro.h"

#define NOTIFY_OPACITY					1.0f		// 0-1 Toast opacity
// Comment out if you don't want any separator between title and content
#define NOTIFY_USE_SEPARATOR
#define NOTIFY_NULL_OR_EMPTY(str)		(!str ||! strlen(str))
#define NOTIFY_PADDING_X				20.f		// Bottom-left X padding
#define NOTIFY_PADDING_Y				20.f		// Bottom-left Y padding
#define NOTIFY_PADDING_MESSAGE_Y		10.f		// Padding Y between each message
#define NOTIFY_FADE_IN_OUT_TIME			150			// Fade in and out duration

namespace ImGuiEx
{
	static std::vector<ImToast> notifications;

	void PushNotification(const ImToast& toast)
	{
		// Limit to 5 notifications
		if (notifications.size() > 5)
			return;

		notifications.push_back(toast);
	}

	void RemoveNotification(int index)
	{
		notifications.erase(notifications.begin() + index);
	}

	void RenderNotifications()
	{
		const auto vp_size = ImGui::GetMainViewport()->Size;
		const auto vp_pos = ImGui::GetMainViewport()->Pos;

		float height = 0.f;

		for (auto i = 0; i < notifications.size(); i++)
		{
			auto* current_toast = &notifications[i];

			// Remove toast if expired
			if (current_toast->GetPhase() == ImGuiExToastPhase::Expired)
			{
				RemoveNotification(i);
				continue;
			}

			// Get icon, title and other data
			const auto icon = current_toast->GetIcon();
			const auto title = current_toast->GetTitle();
			const auto content = current_toast->GetContent();
			const auto default_title = current_toast->GetDefaultTitle();
			const auto opacity = current_toast->GetFadePercent(); // Get opacity based of the current phase

			// Window rendering
			auto text_color = current_toast->GetColor();
			text_color.w = opacity;

			// Generate new unique name for this toast
			char window_name[50];
			sprintf_s(window_name, "##TOAST%d", i);

			auto notify_pos = vp_pos + vp_size;
			notify_pos.x = notify_pos.x - NOTIFY_PADDING_X;
			notify_pos.y = notify_pos.y - NOTIFY_PADDING_Y - height;

			notify_pos.x = std::max(notify_pos.x, ImGui::GetMainViewport()->Pos.x);
			notify_pos.y = std::max(notify_pos.y, ImGui::GetMainViewport()->Pos.y + (ImGui::GetMainViewport()->Size.y / 2));

			//PushStyleColor(ImGuiCol_Text, text_color);
			ImGui::SetNextWindowBgAlpha(opacity);
			//ImGui::SetNextWindowPos(ImVec2(vp_size.x - NOTIFY_PADDING_X, vp_size.y - NOTIFY_PADDING_Y - height), ImGuiCond_Always, ImVec2(1.0f, 1.0f));
			ImGui::SetNextWindowPos(ImVec2(notify_pos.x, notify_pos.y), ImGuiCond_Always, ImVec2(1.0f, 1.0f));

			ImGuiWindowFlags NOTIFY_TOAST_FLAGS =
				ImGuiWindowFlags_AlwaysAutoResize |
				ImGuiWindowFlags_NoDecoration |
				ImGuiWindowFlags_NoInputs |
				ImGuiWindowFlags_NoNav |
				ImGuiWindowFlags_NoDocking;

			if (ImGui::Begin(window_name, NULL, NOTIFY_TOAST_FLAGS))
			{
				// Here we render the toast content
				{
					ImGui::PushTextWrapPos(vp_size.x / 3.f); // We want to support multi-line text, this will wrap the text after 1/3 of the screen width

					bool was_title_rendered = false;

					// If an icon is set
					if (!NOTIFY_NULL_OR_EMPTY(icon))
					{
						//Text(icon); // Render icon text
						ImGui::TextColored(text_color, icon);
						was_title_rendered = true;
					}

					// If a title is set
					if (!NOTIFY_NULL_OR_EMPTY(title.c_str()))
					{
						// If a title and an icon is set, we want to render on same line
						if (!NOTIFY_NULL_OR_EMPTY(icon))
							ImGui::SameLine();

						ImGui::Text(title.c_str()); // Render title text
						was_title_rendered = true;
					}
					else if (!NOTIFY_NULL_OR_EMPTY(default_title.c_str()))
					{
						if (!NOTIFY_NULL_OR_EMPTY(icon))
							ImGui::SameLine();

						ImGui::Text(default_title.c_str()); // Render default title text (ImGuiToastType_Success -> "Success", etc...)
						was_title_rendered = true;
					}

					// In case ANYTHING was rendered in the top, we want to add a small padding so the text (or icon) looks centered vertically
					if (was_title_rendered && !NOTIFY_NULL_OR_EMPTY(content.c_str()))
					{
						ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5.f); // Must be a better way to do this!!!!
					}

					// If a content is set
					if (!NOTIFY_NULL_OR_EMPTY(content.c_str()))
					{
						if (was_title_rendered)
						{
#ifdef NOTIFY_USE_SEPARATOR
							ImGui::Separator();
#endif
						}

						ImGui::Text(content.c_str()); // Render content text
					}

					ImGui::PopTextWrapPos();
				}

				// Save height for next toasts
				height += ImGui::GetWindowHeight() + NOTIFY_PADDING_MESSAGE_Y;

			}

			// End
			ImGui::End();
		}
	}

	void RenderDefaultStyleNotifications()
	{
		// Notify
		// Render toasts on top of everything, at the end of your code!
		// You should push style vars here
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 5.f);
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(43.f / 255.f, 43.f / 255.f, 43.f / 255.f, 100.f / 255.f));
		ImGuiEx::RenderNotifications();
		ImGui::PopStyleVar(1); // Don't forget to Pop()
		ImGui::PopStyleColor(1);
	}

	const ImVec4& ImToast::GetColor()
	{
		switch (mType)
		{
		case ImGuiExToastType::None:
			return { 255, 255, 255, 255 }; // White
		case ImGuiExToastType::Success:
			return { 0, 255, 0, 255 }; // Green
		case ImGuiExToastType::Warning:
			return { 255, 255, 0, 255 }; // Yellow
		case ImGuiExToastType::Error:
			return { 255, 0, 0, 255 }; // Error
		case ImGuiExToastType::Info:
			return { 0, 157, 255, 255 }; // Blue
		}
	}

	const char* ImToast::GetIcon()
	{
		switch (mType)
		{
		case ImGuiExToastType::None:
			return NULL;
		case ImGuiExToastType::Success:
			return ICON_FA_CHECK_CIRCLE;
		case ImGuiExToastType::Warning:
			return ICON_FA_EXCLAMATION_TRIANGLE;
		case ImGuiExToastType::Error:
			return ICON_FA_TIMES_CIRCLE;
		case ImGuiExToastType::Info:
			return ICON_FA_INFO_CIRCLE;
		}

		return nullptr;
	}

	std::string ImToast::GetDefaultTitle()
	{
		if(!mTitle.empty())
			return mTitle;

		switch (mType)
		{
		case ImGuiExToastType::None:
			return nullptr;
		case ImGuiExToastType::Success:
			return Horizon::GetTranslateText("Success");
		case ImGuiExToastType::Warning:
			return Horizon::GetTranslateText("Warning");
		case ImGuiExToastType::Error:
			return Horizon::GetTranslateText("Error");
		case ImGuiExToastType::Info:
			return Horizon::GetTranslateText("Info") ;
		}

		return "";
	}

	auto ImToast::GetElapsedTime()
	{
		return Horizon::HSystemTimer::GetTicks64() - mCreationTime;
	}

	const ImGuiExToastPhase& ImToast::GetPhase()
	{
		const auto elapsed = GetElapsedTime();

		if (elapsed > NOTIFY_FADE_IN_OUT_TIME + mDismissTime + NOTIFY_FADE_IN_OUT_TIME)
		{
			return ImGuiExToastPhase::Expired;
		}
		else if (elapsed > NOTIFY_FADE_IN_OUT_TIME + mDismissTime)
		{
			return ImGuiExToastPhase::FadeOut;
		}
		else if (elapsed > NOTIFY_FADE_IN_OUT_TIME)
		{
			return ImGuiExToastPhase::Wait;
		}
		else
		{
			return ImGuiExToastPhase::FadeIn;
		}
	}

	float ImToast::GetFadePercent()
	{
		const auto phase = GetPhase();
		const auto elapsed = GetElapsedTime();

		if (phase == ImGuiExToastPhase::FadeIn)
		{
			return ((float)elapsed / (float)NOTIFY_FADE_IN_OUT_TIME) * NOTIFY_OPACITY;
		}
		else if (phase == ImGuiExToastPhase::FadeOut)
		{
			return (1.f - (((float)elapsed - (float)NOTIFY_FADE_IN_OUT_TIME - (float)mDismissTime) / (float)NOTIFY_FADE_IN_OUT_TIME)) * NOTIFY_OPACITY;
		}

		return 1.f * NOTIFY_OPACITY;
	}

	ImToast::ImToast(ImGuiExToastType type, int dismiss_time)
		:
		mType(type),
		mDismissTime(dismiss_time),
		mCreationTime(Horizon::HSystemTimer::GetTicks64())
	{
		IM_ASSERT(type < ImGuiExToastType::COUNT);

	}

	ImToast::ImToast(ImGuiExToastType type, const char* title, const char* content)
		:
		ImToast(type)
	{
		mTitle = title;
		mContent = Horizon::GetTranslateText(content);
	}

	ImToast::ImToast(ImGuiExToastType type, const char* text)
		:
		ImToast(type)
	{
		mContent = Horizon::GetTranslateText(text);
	}

	ImToast::ImToast(ImGuiExToastType type, const std::string& text)
		:
		ImToast(type)
	{
		mContent = Horizon::GetTranslateText(text);
	}

	ImToast::ImToast(ImGuiExToastType type, int dismiss_time, const char* text)
		:
		ImToast(type, dismiss_time)
	{
		mContent = Horizon::GetTranslateText(text);
	}
}



