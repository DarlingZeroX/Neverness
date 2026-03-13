#include "pch.h"
#include "ImGuiEx/HProfiler.h"

HProflier::HProflier()
{
	__int64 countsPerSec;
	QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSec);
	m_SecondsPerCount = 1.0f / (float)countsPerSec;
}

HProflier& HProflier::Instance()
{
	static HProflier __instance;
	return __instance;
}

void HProflier::Start(const std::string& item)
{
	__int64 countsPerSec;
	QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSec);
	m_SecondsPerCount = 1.0f / (float)countsPerSec;

	m_StartNameOrder.push_back(item);
	__int64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
	m_TimeRecords[item] = (float)(currTime)*m_SecondsPerCount;
}

void HProflier::End(const std::string& item)
{
	__int64 countsPerSec;
	QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSec);
	m_SecondsPerCount = 1.0f / (float)countsPerSec;

	__int64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
	m_TimeRecords[item] = (float)(currTime)*m_SecondsPerCount - m_TimeRecords[item];
}

void HProflier::Reset()
{
	m_TimeRecords.clear();
	m_FrameCount++;
}

void HProflier::DrawFPS(float delatTime)
{
	float fps = 1.0f / delatTime;
	ImGui::InputFloat("FPS: ", &fps, 0, 0, "%0.3f", ImGuiInputTextFlags_ReadOnly);
	ImGui::InputInt("FrameCount: ", &m_FrameCount, 1, 100, ImGuiInputTextFlags_ReadOnly);
}

void HProflier::OnGUI(float delatTime)
{
	if (!ImGui::Begin("Profiler"))
	{
		ImGui::End();
		return;
	}
	DrawFPS(delatTime);

	float frameTime = 0;
	for (std::string& name : m_StartNameOrder)
	{
		auto time = m_TimeRecords[name];
		auto timeStr = name + ":" + std::to_string(time * 1000.0) + "ms";
		ImGui::Text(timeStr.c_str());
		frameTime += time;
	}

	//for (auto [key, value] : m_TimeRecords)
	//{
	//	frameTime += value;
	//	auto time = key + ":" + std::to_string(value * 1000.0) + "ms";
	//	ImGui::Text(time.c_str());
	//}

	auto time = "Frame Time:" + std::to_string(frameTime * 1000.0) + "ms";
	ImGui::Text(time.c_str());

	ImGui::End();

	m_StartNameOrder.clear();
}