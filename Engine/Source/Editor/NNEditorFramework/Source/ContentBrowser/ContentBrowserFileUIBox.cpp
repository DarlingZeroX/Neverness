#include "ContentBrowser/ContentBrowserFileUIBox.h"
#include "NNEditorFramework/Include/EditorCore/Localization.h"

namespace VisionGal::Editor {
	ContentBrowserFileUIBox::ContentBrowserFileUIBox()
	{
		thumbnialTextSizeY = 35.0f;
		thumbnailImageSizeY = 90.0f;

		padding = 6.0f;

		Size = ImVec2(thumbnailImageSizeY, thumbnailImageSizeY + thumbnialTextSizeY);

		CellSize = Size.x + padding;

		TextSize = ImVec2(CellSize, thumbnialTextSizeY);
		ImageSize = ImVec2(Size.x, Size.y - thumbnialTextSizeY);
		assetTypeColor = IM_COL32(100, 100, 100, 255);
	}

	ContentBrowserFileUIBox& ContentBrowserFileUIBox::Get()
	{
		static ContentBrowserFileUIBox thumbnial;
		return thumbnial;
	}

	void ContentBrowserFileUIBox::Draw(ContentBrowser* browser, ImDrawList* draw_list, ContentBrowserItem& item,
		const ImVec2& p0, const ImVec2& p1)
	{
		const ImVec2 textPosStart = ImVec2(p0.x, p0.y + Size.y - thumbnialTextSizeY);
		const ImVec2 assetTypeStart = ImVec2(p0.x, p0.y + Size.y - 18);
		const ImVec2 textPosEnd = ImVec2(textPosStart.x + TextSize.x, textPosStart.y + TextSize.y);

		const ImVec2 imagePosStart = ImVec2(p0.x, p0.y);
		const ImVec2 imagePosEnd = ImVec2(imagePosStart.x + ImageSize.x, imagePosStart.y + ImageSize.y);

		const ImVec2 thumbnailPosEnd = ImVec2(imagePosStart.x + Size.x, imagePosStart.y + Size.y);

		// background shadow
		draw_list->AddRectFilled(imagePosStart, ImVec2(thumbnailPosEnd.x + 3, thumbnailPosEnd.y + 3), IM_COL32(2, 2, 2, 255), 4.0f);

		// fill thumbnail
		ImGui::PushClipRect(p0, p1, true);

		draw_list->AddRectFilled(imagePosStart, imagePosEnd, IM_COL32(0, 0, 0, 255));
		if (item.iconView != nullptr)
			draw_list->AddImage((ImTextureID)item.iconView, imagePosStart, imagePosEnd);			///////////// ��ʾͼ��
		draw_list->AddRectFilled(textPosStart, textPosEnd, IM_COL32(10, 10, 10, 255));

		if (item.Renameing)
		{
			// fill thumbnail
			ImGuiEx::InputText("##Renaming", item.Name);

			//if (ImGui::IsItemDeactivatedAfterEdit())
			if (ImGui::IsItemDeactivated() || (!ImGui::IsItemHovered() && ImGui::IsMouseClicked(0)))
			{
				item.Renameing = false;

				browser->RenameDirectoryItem(item, item.Name);
			}
		}
		else
		{
			draw_list->AddText(textPosStart, IM_COL32_WHITE, item.Name.c_str());

			if (!item.IsDirectory)
				draw_list->AddText(assetTypeStart, assetTypeColor, EditorText{ item.AssetType }.c_str());

		}

		// frame
		if (ImGui::IsItemHovered() || item.Selected)
			draw_list->AddRect(imagePosStart, thumbnailPosEnd, IM_COL32(100, 100, 100, 200), 3, 0, 2);
		ImGui::PopClipRect();
	}
}
