#pragma once
#include "Asset.h"
#include "../Core/Core.h"
#include "../Resource/Video.h"

namespace VisionGal
{
	struct VideoAsset : public VGAsset
	{
		VideoAsset()
			: VGAsset("Video")
		{
		}

		Ref<VideoClip> videoClip;
	};

	class VG_ENGINE_API VideoAssetLoader : public IAssetLoader
	{
	public:
		VideoAssetLoader() = default;
		~VideoAssetLoader() override = default;
		 
		bool Read(const std::string path, Ref<VGAsset>& asset) override;
	};
}