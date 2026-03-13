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

#include "pch.h"
#include "EditorComponents/ConsolePanel.h"
#include "EditorCore/Localization.h"
#include "HCore/Interface/HStringTools.h"
#include "VGImgui/Include/ImGuiEx/ImNotify.h"

namespace VisionGal::Editor {

	ConsolePanel::ConsolePanel()
	{
        Horizon::HCoreLogger::AddListener([this](Horizon::HLogLevel level, const std::string& message)
            {
                AddLog(level, message);
            });
	}

	void ConsolePanel::Toggle()
	{
		//m_OutputLogPanelOpenFlag = !m_OutputLogPanelOpenFlag;
	}

	void SetOutputLogPanelPosition()
	{
		//auto mv_size = ImGui::GetWindowSize();
        //float width = ImGui::GetIO().DisplaySize.x - 60;
        //
		//// 获取内容浏览器高度尺寸 resize contraints 
		//static float windowHeight = 300;
		//ImGui::SetNextWindowSizeConstraints(ImVec2(width, 0), ImVec2(width, FLT_MAX), [](ImGuiSizeCallbackData* data)
		//	{
		//		// 使用data->CurrentSize会造成下边抖动
		//		windowHeight = data->DesiredSize.y;
		//	}
		//);
        //
		////fixed window position
		//const ImGuiViewport* viewport = ImGui::GetMainViewport();
        //
		//float windowPosY = viewport->Size.y - windowHeight - 40;
		//ImGui::SetNextWindowSize(ImVec2(600, 300), ImGuiCond_FirstUseEver);
	}

	void ConsolePanel::OnGUI()
	{
		if (!m_IsOpen)
			return;

		//SetOutputLogPanelPosition();

		RenderLog();
	}

    void ConsolePanel::OnSideBarUI()
    {
        const float height = ImGui::GetFrameHeight();

        if (ImGui::Button(EditorText{ "Log" ,ICON_FA_WRENCH }.c_str()))
        {
            Toggle();
        }

        ImGui::SameLine();
    }

    void ConsolePanel::AddLog(Horizon::HLogLevel level, const std::string& message)
    {
        std::string category;
	    switch (level)
	    {
	    case Horizon::HLogLevel::Error:
            category = "Error";
            ImGuiEx::PushNotification({ ImGuiExToastType::Error, message });
            break;
	    case Horizon::HLogLevel::Info:
            category = "Info";
            break;
	    case Horizon::HLogLevel::Warn:
            ImGuiEx::PushNotification({ ImGuiExToastType::Warning, message });
            category = "Warn";
            break;
	    case Horizon::HLogLevel::Critical:
            category = "Critical";
            break;
	    }

		std::string formatString = "[" + category + "] " + message;

		m_Mutex.lock(); // Lock the mutex to ensure thread safety
		// Store the line in the vector
		m_Lines.push_back(formatString);
		m_Mutex.unlock(); // Unlock the mutex after adding the log

		// 运行时会有随机异常，暂时不使用
		//AddLog("[%s] %s", category.c_str(), message.c_str());
    }

    std::string ConsolePanel::GetWindowFullName()
    {
        return EditorText{ GetWindowName(), ICON_FA_TERMINAL }.GetText();
    }

    std::string ConsolePanel::GetWindowName()
    {
        return "Console";
    }

    void ConsolePanel::OpenWindow(bool open)
    {
        m_IsOpen = open;
    }

    bool ConsolePanel::IsWindowOpened()
    {
        return m_IsOpen;
    }

    void ConsolePanel::RenderLog()
	{
        if (!m_IsOpen)
            return;

		if (!ImGui::Begin(GetWindowFullName().c_str()))
		{
			ImGui::End();
			return;
		}

        //if (ImGui::SmallButton("[Debug] Add 5 entries"))
        //{
        //    static int counter = 0;
        //    const char* categories[3] = { "info", "warn", "error" };
        //    const char* words[] = { "Bumfuzzled", "Cattywampus", "Snickersnee", "Abibliophobia", "Absquatulate", "Nincompoop", "Pauciloquent" };
        //    for (int n = 0; n < 5; n++)
        //    {
        //        const char* category = categories[counter % IM_ARRAYSIZE(categories)];
        //        const char* word = words[counter % IM_ARRAYSIZE(words)];
        //        AddLog("[%05d] [%s] Hello, current time is %.1f, here's a word: '%s'\n",
        //            ImGui::GetFrameCount(), category, ImGui::GetTime(), word);
        //        counter++;
        //    }
        //}

        // Options menu
        if (ImGui::BeginPopup(EditorText{ "Options" }.c_str()))
        {
            ImGui::Checkbox("Auto-scroll", &AutoScroll);
            ImGui::EndPopup();
        }

        // Main window
        if (ImGui::Button(EditorText{ "Options" }.c_str()))
            ImGui::OpenPopup(EditorText{ "Options" }.c_str());
        ImGui::SameLine();
        bool clear = ImGui::Button(EditorText{ "Clear" }.c_str());
        ImGui::SameLine();
        bool copy = ImGui::Button(EditorText{ "Copy" }.c_str());
        ImGui::SameLine();
        Filter.Draw(EditorText{ "Filter" }.c_str(), -100.0f);

        ImGui::Separator();

        if (ImGui::BeginChild("scrolling", ImVec2(0, 0), ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar))
        {
            if (clear)
                Clear();
            if (copy)
                ImGui::LogToClipboard();

            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
            const char* buf = Buf.begin();
            const char* buf_end = Buf.end();
            if (Filter.IsActive())
            {
                // In this example we don't use the clipper when Filter is enabled.
                // This is because we don't have random access to the result of our filter.
                // A real application processing logs with ten of thousands of entries may want to store the result of
                // search/filter.. especially if the filtering function is not trivial (e.g. reg-exp).
                for (auto& line : m_Lines)
                {
                    if (Filter.PassFilter(line.c_str()))
                        ImGui::TextUnformatted(line.c_str());
                }
            }
            else
            {
                // The simplest and easy way to display the entire buffer:
                //   ImGui::TextUnformatted(buf_begin, buf_end);
                // And it'll just work. TextUnformatted() has specialization for large blob of text and will fast-forward
                // to skip non-visible lines. Here we instead demonstrate using the clipper to only process lines that are
                // within the visible area.
                // If you have tens of thousands of items and their processing cost is non-negligible, coarse clipping them
                // on your side is recommended. Using ImGuiListClipper requires
                // - A) random access into your data
                // - B) items all being the  same height,
                // both of which we can handle since we have an array pointing to the beginning of each line of text.
                // When using the filter (in the block of code above) we don't have random access into the data to display
                // anymore, which is why we don't use the clipper. Storing or skimming through the search result would make
                // it possible (and would be recommended if you want to search through tens of thousands of entries).
                ImGuiListClipper clipper;
                clipper.Begin(m_Lines.size());
                while (clipper.Step())
                {
                    for (int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd; line_no++)
                    {
                        ImGui::TextUnformatted(m_Lines[line_no].c_str());
                    }
                }
                clipper.End();
            }
            ImGui::PopStyleVar();

            // Keep up at the bottom of the scroll region if we were already at the bottom at the beginning of the frame.
            // Using a scrollbar or mouse-wheel will take away from the bottom edge.
            if (AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
                ImGui::SetScrollHereY(1.0f);
        }

        ImGui::EndChild();
        ImGui::End();
	}

    void ConsolePanel::AddLog(const char* fmt, ...)
    {
        std::string formatString;
        {
            va_list args;
            va_start(args, fmt);

            {
                va_list argsCopy;
                va_copy(argsCopy, args); // 做个拷贝传给 Format
                formatString = Horizon::HStringTools::Format(fmt, argsCopy);

                va_end(argsCopy); // 结束拷贝
            }

            va_end(args);
        }

        m_Lines.push_back(formatString);
    }

    void ConsolePanel::Clear()
    {
        Buf.clear();
        m_Lines.clear();
    }

}


