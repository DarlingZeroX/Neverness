#include "VideoClip.h"
#include "NNRuntimeCore/Include/Core/VFS.h"
#include <NNCore/Interface/HVector.h>

namespace NN::Runtime
{
	////////////////////	FVideoClip
	uint2 FVideoClip::GetSize() const
	{
		//return { m_VideoDecoder->GetVideoWidth(),m_VideoDecoder->GetVideoHeight() };
		return { m_VideoClip.GetSize().x,m_VideoClip.GetSize().y };
	}

	NN::Core::IVideoDecoder* FVideoClip::GetDecoder()
	{
		//return m_VideoDecoder.get();
		return m_VideoClip.GetDecoder();
	}

	bool FVideoClip::Open(const std::string& filePath)
	{
		return m_VideoClip.Open(VFS::GetInstance(), filePath);
	}

}