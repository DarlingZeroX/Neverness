add_subdirectory ("NNRuntimeLua")
#add_subdirectory ("NNRuntimeRHI")
add_subdirectory ("NNRuntimeVFS")
add_subdirectory ("NNRuntimePak")
add_subdirectory ("NNRuntimeImGui")
add_subdirectory ("NNRuntimeCore")
# NNNativeEngineAPI 已合并至 NNAPI
add_subdirectory ("NNRuntimeApplication")
# NNRuntimeNativeEngineAPIStub 已合并至 NNAPI
add_subdirectory ("NNAPI")

#add_subdirectory("NNRuntimeManagedBridge")
add_subdirectory ("NNRuntimeEngine")
add_subdirectory("NNRuntimeRenderer2D")
add_subdirectory ("NNRuntimeEngineServices")
#add_subdirectory ("NNRuntimeAssetLegacy")
#add_subdirectory ("NNRuntimeAsset")  # 已移至 Legacy
#add_subdirectory ("NNRuntimeImageCodec")
#add_subdirectory ("NNRuntimeRenderAssets")  # 已移至 Legacy
add_subdirectory ("NNRuntimeRmlui")
# add_subdirectory ("NNEngineLegacy")  # 冻结：缺少 NNRuntimeRmlui/Interface/UIDocumentLegacy.h
add_subdirectory("NNRuntimeMedia")
# add_subdirectory("NNRuntimeMediaAssets")  # 预先存在编译错误，待修复
