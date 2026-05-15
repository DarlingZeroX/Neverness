/*
 * DialogueRmlPresentation — Rml 数据模型实现
 */

#include "DialogueSystem/DialogueRmlPresentation.h"

namespace VisionGal::GalGame
{
	bool DialogueRmlPresentation::InitialiseDataModel(Rml::Context* context)
	{
		if (!context)
			return false;

		Rml::DataModelConstructor constructor = context->CreateDataModel("dialog");
		if (!constructor)
			return false;

		constructor.Bind("dialog_name", &m_DialogName);
		constructor.Bind("dialog_text", &m_DialogText);
		m_ModelHandle = constructor.GetModelHandle();
		return true;
	}

	void DialogueRmlPresentation::MarkAllVariablesDirty()
	{
		m_ModelHandle.DirtyAllVariables();
	}
}
