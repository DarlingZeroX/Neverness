/*
 * GalGameContextSnapshot 实现
 */

#include "GalGameContextSnapshot.h"
#include "GalGameContext.h"

namespace VisionGal::GalGame
{
	GalGameContextSnapshot GalGameContextSnapshot::Capture(const GalGameContext& src)
	{
		GalGameContextSnapshot out;
		out.runtimeState = src.runtimeState;
		out.archiveData = src.archiveData;
		return out;
	}

	void GalGameContextSnapshot::Apply(GalGameContext& dst) const
	{
		dst.runtimeState = runtimeState;
		if (archiveData)
			dst.archiveData = archiveData;
	}
}
