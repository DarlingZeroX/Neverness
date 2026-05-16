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

#include "HLocale.h"
#include <SDL3/SDL_locale.h>

namespace Horizon
{
	void HLocale::GetPreferredLocales(HLocale& locale)
	{
		int count = 0;
		SDL_Locale** sdlLocale = SDL_GetPreferredLocales(&count);

		locale.Country = (*sdlLocale)->country;
		locale.Language = (*sdlLocale)->language;

		SDL_free(sdlLocale);
	}
}