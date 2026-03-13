// imgui-notify by patrickcjk
// https://github.com/patrickcjk/imgui-notify
#pragma once
#ifndef IMGUI_NOTIFY
#define IMGUI_NOTIFY

#pragma once
#include <string>
#include "../imgui/imgui.h"

enum class ImGuiExToastType
{
	None,
	Success,
	Warning,
	Error,
	Info,
	COUNT
};

enum class ImGuiExToastPhase
{
	FadeIn,
	Wait,
	FadeOut,
	Expired,
	COUNT
};

enum class ImGuiExToastLocation
{
	TopLeft,
	TopCenter,
	TopRight,
	BottomLeft,
	BottomCenter,
	BottomRight,
	Center,
	COUNT
};

namespace ImGuiEx
{
	class IMGUI_API ImToast
	{
		static constexpr unsigned NOTIFY_DEFAULT_DISMISS = 3000;		// Auto dismiss after X ms (default, applied only of no data provided in constructors)
	private:
		ImGuiExToastType		mType = ImGuiExToastType::None;
		std::string				mTitle;
		std::string				mContent;

		int						mDismissTime = NOTIFY_DEFAULT_DISMISS;
		uint64_t				mCreationTime = 0;
	
	public:

		auto SetTitle(const char* title) -> void { mTitle = title; }

		auto SetContent(const char* text) -> void { mContent = text; }

		auto SetType(const ImGuiExToastType& type) -> void { IM_ASSERT(type < ImGuiExToastType::COUNT); mType = type; };

	public:
		// Getters

		std::string GetTitle() { return mTitle; };

		std::string GetDefaultTitle();

		const ImGuiExToastType& GetType() { return mType; };

		const ImVec4& GetColor();

		const char* GetIcon();

		std::string GetContent() { return mContent; };

		auto GetElapsedTime();

		const ImGuiExToastPhase& GetPhase();

		float GetFadePercent();

	public:
		// Constructors

		ImToast(ImGuiExToastType type, int dismiss_time = NOTIFY_DEFAULT_DISMISS);

		ImToast(ImGuiExToastType type, const char* title, const char* content);

		ImToast(ImGuiExToastType type, const char* text);

		ImToast(ImGuiExToastType type, const std::string& text);

		ImToast(ImGuiExToastType type, int dismiss_time, const char* text);
	};

	/// <summary>
	/// Push a new toast in the list
	/// </summary>
	IMGUI_API void PushNotification(const ImToast& toast);

	/// <summary>
	/// Remove a toast from the list by its index
	/// </summary>
	/// <param name="index">index of the toast to remove</param>
	IMGUI_API void RemoveNotification(int index);

	/// <summary>
	/// Render toasts, call at the end of your rendering!
	/// </summary>
	IMGUI_API void RenderNotifications();

	IMGUI_API void RenderDefaultStyleNotifications();

	/// <summary>
	/// Adds font-awesome font, must be called ONCE on initialization
	/// <param name="FontDataOwnedByAtlas">Fonts are loaded from read-only memory, should be set to false!</param>
	/// </summary>
	//NOTIFY_INLINE void MergeIconsWithLatestFont(float font_size, bool FontDataOwnedByAtlas = false)
	//{
	//	static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
	//
	//	ImFontConfig icons_config;
	//	icons_config.MergeMode = true;
	//	icons_config.PixelSnapH = true;
	//	icons_config.FontDataOwnedByAtlas = FontDataOwnedByAtlas;
	//
	//	GetIO().Fonts->AddFontFromMemoryTTF((void*)fa_solid_900, sizeof(fa_solid_900), font_size, &icons_config, icons_ranges);
	//}
}

#endif
