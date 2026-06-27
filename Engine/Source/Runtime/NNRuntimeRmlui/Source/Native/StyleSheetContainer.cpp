#include "Native/RmlNative.h"
#include "Native/StreamFile.h"
#include "Util.h"
#include <iostream>

#include "NNCore/Interface/HLog.h"

RMLUI_CAPI void *rml_StyleSheetContainer_New() {
    return new Rml::StyleSheetContainer();
}

RMLUI_CAPI void rml_StyleSheetContainer_Free(Rml::StyleSheetContainer* container) {
    delete container;
}

RMLUI_CAPI bool rml_StyleSheetContainer_LoadStyleSheetContainer(Rml::StyleSheetContainer *container, const char *file_name) {

	H_LOG_ERROR("未实现接口：rml_StyleSheetContainer_LoadStyleSheetContainer");
	throw "未实现接口：rml_StyleSheetContainer_LoadStyleSheetContainer";
	return false;
	//  auto file_stream = Rml::MakeUnique<Rml::StreamFile>();
	//  file_stream->Open(file_name);
    //return container->LoadStyleSheetContainer(file_stream.get(), 1);
}
