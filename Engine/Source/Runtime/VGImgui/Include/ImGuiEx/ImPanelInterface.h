#pragma once

namespace ImGuiEx {
	struct ImPanelInterface
	{
		virtual ~ImPanelInterface() = default;

		virtual void FrameUpdate() {};
		virtual void FixedUpdate() {};

		virtual bool IsAsync() { return false; }

		virtual void OnGUI() {};
	};
}
