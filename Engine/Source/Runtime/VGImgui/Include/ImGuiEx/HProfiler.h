#pragma once
#include "../imconfig.h"
#include <map>
#include <string>
#include <vector>
#include <Windows.h>
#include "../imgui/imgui.h"

class IMGUI_API HProflier
{
	HProflier();
public:
	static HProflier& Instance();
public:
	template <class _Fn>
	void Profile(const std::string& item, _Fn _Func)
	{
		Start(item);
		_Func();
		End(item);
	}

	void Start(const std::string& item);

	void End(const std::string& item);

	void Reset();

	void DrawFPS(float delatTime);

	void OnGUI(float delatTime);
private:
	float m_SecondsPerCount;
	std::map<std::string, float> m_TimeRecords;
	int m_FrameCount = 0;
	std::vector<std::string> m_StartNameOrder;
};