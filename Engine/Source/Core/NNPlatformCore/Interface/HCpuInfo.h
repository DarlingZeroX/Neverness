#pragma once
#include "../HCorePlatformConfig.h"

/**
 *  \file HCpuInfo.h
 *
 *  CPU feature detection encapsulate for SDL.
 */

namespace NN::Core
{
	struct H_CORE_PLATFORM_API HCpuInfo
	{
		/**
		* Get the number of CPU cores available.
		*
		* \returns the total number of logical CPU cores. On CPUs that include
		*          technologies such as hyperthreading, the number of logical cores
		*          may be more than the number of physical cores.
		*/
		static int GetCPUCount(void);

		/**
		* Determine the L1 cache line size of the CPU.
		*
		* This is useful for determining multi-threaded structure padding or SIMD
		* prefetch sizes.
		*
		* \returns the L1 cache line size of the CPU, in bytes.
		*/
		static int  GetCPUCacheLineSize(void);

		/**
		 * Determine whether the CPU has AltiVec features.
		 *
		 * This always returns false on CPUs that aren't using PowerPC instruction
		 * sets.
		 *
		 * \returns SDL_TRUE if the CPU has AltiVec features or SDL_FALSE if not.
		 *
		 * \since This function is available since SDL 2.0.0.
		 *
		 * \sa SDL_Has3DNow
		 * \sa SDL_HasAVX
		 * \sa SDL_HasAVX2
		 * \sa SDL_HasMMX
		 * \sa SDL_HasRDTSC
		 * \sa SDL_HasSSE
		 * \sa SDL_HasSSE2
		 * \sa SDL_HasSSE3
		 * \sa SDL_HasSSE41
		 * \sa SDL_HasSSE42
		 */
		static bool HasAltiVec(void);

		/**
		 * Determine whether the CPU has MMX features.
		 *
		 * This always returns false on CPUs that aren't using Intel instruction sets.
		 *
		 * \returns SDL_TRUE if the CPU has MMX features or SDL_FALSE if not.
		 *
		 * \since This function is available since SDL 2.0.0.
		 *
		 * \sa SDL_Has3DNow
		 * \sa SDL_HasAltiVec
		 * \sa SDL_HasAVX
		 * \sa SDL_HasAVX2
		 * \sa SDL_HasRDTSC
		 * \sa SDL_HasSSE
		 * \sa SDL_HasSSE2
		 * \sa SDL_HasSSE3
		 * \sa SDL_HasSSE41
		 * \sa SDL_HasSSE42
		 */
		static bool HasMMX(void);

		/**
		 * Determine whether the CPU has SSE features.
		 *
		 * This always returns false on CPUs that aren't using Intel instruction sets.
		 *
		 * \returns SDL_TRUE if the CPU has SSE features or SDL_FALSE if not.
		 */
		static bool HasSSE(void);

		/**
		 * Determine whether the CPU has SSE2 features.
		 *
		 * This always returns false on CPUs that aren't using Intel instruction sets.
		 *
		 * \returns SDL_TRUE if the CPU has SSE2 features or SDL_FALSE if not.
		 */
		static bool HasSSE2(void);

		/**
		 * Determine whether the CPU has SSE3 features.
		 *
		 * This always returns false on CPUs that aren't using Intel instruction sets.
		 *
		 * \returns SDL_TRUE if the CPU has SSE3 features or SDL_FALSE if not.
		 */
		static bool HasSSE3(void);

		/**
		 * Determine whether the CPU has SSE4.1 features.
		 *
		 * This always returns false on CPUs that aren't using Intel instruction sets.
		 *
		 * \returns SDL_TRUE if the CPU has SSE4.1 features or SDL_FALSE if not.
		 */
		static bool HasSSE41(void);

		/**
		 * Determine whether the CPU has SSE4.2 features.
		 *
		 * This always returns false on CPUs that aren't using Intel instruction sets.
		 *
		 * \returns SDL_TRUE if the CPU has SSE4.2 features or SDL_FALSE if not.
		 */
		static bool HasSSE42(void);

		/**
		 * Determine whether the CPU has AVX features.
		 *
		 * This always returns false on CPUs that aren't using Intel instruction sets.
		 *
		 * \returns SDL_TRUE if the CPU has AVX features or SDL_FALSE if not.
		 */
		static bool HasAVX(void);

		/**
		 * Determine whether the CPU has AVX2 features.
		 *
		 * This always returns false on CPUs that aren't using Intel instruction sets.
		 *
		 * \returns SDL_TRUE if the CPU has AVX2 features or SDL_FALSE if not.
		 */
		static bool HasAVX2(void);

		/**
		 * Determine whether the CPU has AVX-512F (foundation) features.
		 *
		 * This always returns false on CPUs that aren't using Intel instruction sets.
		 *
		 * \returns SDL_TRUE if the CPU has AVX-512F features or SDL_FALSE if not.
		 */
		static bool HasAVX512F(void);

		/**
		 * Determine whether the CPU has ARM SIMD (ARMv6) features.
		 *
		 * This is different from ARM NEON, which is a different instruction set.
		 *
		 * This always returns false on CPUs that aren't using ARM instruction sets.
		 *
		 * \returns SDL_TRUE if the CPU has ARM SIMD features or SDL_FALSE if not.
		 */
		static bool HasARMSIMD(void);

		/**
		 * Determine whether the CPU has NEON (ARM SIMD) features.
		 *
		 * This always returns false on CPUs that aren't using ARM instruction sets.
		 *
		 * \returns SDL_TRUE if the CPU has ARM NEON features or SDL_FALSE if not.
		 */
		static bool HasNEON(void);

		/**
		 * Get the amount of RAM configured in the system.
		 *
		 * \returns the amount of RAM configured in the system in MB.
		 */
		static int GetSystemRAM(void);
	};
}
