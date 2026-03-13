#pragma once
#include "../HCorePlatformConfig.h"
#include <HCore/Include/Core/HCoreTypes.h>

/**
 * \file HSystemTimer.h
 *
 * Header for the SDL time management routines.
 */

namespace Horizon
{
	struct H_CORE_PLATFORM_API HSystemTimer
	{
		/**
		* Get the number of milliseconds since SDL library initialization.
		*
		* This value wraps if the program runs for more than ~49 days.
		*
		* This function is not recommended as of SDL 2.0.18; use SDL_GetTicks64()
		* instead, where the value doesn't wrap every ~49 days. There are places in
		* SDL where we provide a 32-bit timestamp that can not change without
		* breaking binary compatibility, though, so this function isn't officially
		* deprecated.
		*
		* \returns an unsigned 32-bit value representing the number of milliseconds
		*          since the SDL library initialized.
		*/
		static uint32 GetTicks(void);

		/**
		 * Get the number of milliseconds since SDL library initialization.
		 *
		 * Note that you should not use the SDL_TICKS_PASSED macro with values
		 * returned by this function, as that macro does clever math to compensate for
		 * the 32-bit overflow every ~49 days that SDL_GetTicks() suffers from. 64-bit
		 * values from this function can be safely compared directly.
		 *
		 * For example, if you want to wait 100 ms, you could do this:
		 *
		 * ```c
		 * const Uint64 timeout = SDL_GetTicks64() + 100;
		 * while (SDL_GetTicks64() < timeout) {
		 *     // ... do work until timeout has elapsed
		 * }
		 * ```
		 *
		 * \returns an unsigned 64-bit value representing the number of milliseconds
		 *          since the SDL library initialized.
		 */
		static uint64 GetTicks64(void);

		/**
		 * Compare 32-bit SDL ticks values, and return true if `A` has passed `B`.
		 *
		 * This should be used with results from SDL_GetTicks(), as this macro
		 * attempts to deal with the 32-bit counter wrapping back to zero every ~49
		 * days, but should _not_ be used with SDL_GetTicks64(), which does not have
		 * that problem.
		 *
		 * For example, with SDL_GetTicks(), if you want to wait 100 ms, you could
		 * do this:
		 *
		 * ```c
		 * const Uint32 timeout = SDL_GetTicks() + 100;
		 * while (!SDL_TICKS_PASSED(SDL_GetTicks(), timeout)) {
		 *     // ... do work until timeout has elapsed
		 * }
		 * ```
		 *
		 * Note that this does not handle tick differences greater
		 * than 2^31 so take care when using the above kind of code
		 * with large timeout delays (tens of days).
		 */

#define H_TICKS_PASSED(A, B)  ((Sint32)((B) - (A)) <= 0)

		 /**
		  * Get the current value of the high resolution counter.
		  *
		  * This function is typically used for profiling.
		  *
		  * The counter values are only meaningful relative to each other. Differences
		  * between values can be converted to times by using
		  * SDL_GetPerformanceFrequency().
		  *
		  * \returns the current counter value.
		  */
		static uint64 GetPerformanceCounter(void);

		/**
		 * Get the count per second of the high resolution counter.
		 *
		 * \returns a platform-specific count per second.
		 */
		static uint64 GetPerformanceFrequency(void);

		/**
		 * Wait a specified number of milliseconds before returning.
		 *
		 * This function waits a specified number of milliseconds before returning. It
		 * waits at least the specified time, but possibly longer due to OS
		 * scheduling.
		 *
		 * \param ms the number of milliseconds to delay
		 * */
		static void Delay(uint32 ms);

		/**
		 * Function prototype for the timer callback function.
		 *
		 * The callback function is passed the current timer interval and returns
		 * the next timer interval. If the returned value is the same as the one
		 * passed in, the periodic alarm continues, otherwise a new alarm is
		 * scheduled. If the callback returns 0, the periodic alarm is cancelled.
		 */
		 //typedef Uint32(SDLCALL* SDL_TimerCallback) (Uint32 interval, void* param);

		 /**
		  * Definition of the timer ID type.
		  */
		  //typedef int SDL_TimerID;

		  /**
		   * Call a callback function at a future time.
		   *
		   * If you use this function, you must pass `SDL_INIT_TIMER` to SDL_Init().
		   *
		   * The callback function is passed the current timer interval and the user
		   * supplied parameter from the SDL_AddTimer() call and should return the next
		   * timer interval. If the value returned from the callback is 0, the timer is
		   * canceled.
		   *
		   * The callback is run on a separate thread.
		   *
		   * Timers take into account the amount of time it took to execute the
		   * callback. For example, if the callback took 250 ms to execute and returned
		   * 1000 (ms), the timer would only wait another 750 ms before its next
		   * iteration.
		   *
		   * Timing may be inexact due to OS scheduling. Be sure to note the current
		   * time with SDL_GetTicks() or SDL_GetPerformanceCounter() in case your
		   * callback needs to adjust for variances.
		   *
		   * \param interval the timer delay, in milliseconds, passed to `callback`
		   * \param callback the SDL_TimerCallback function to call when the specified
		   *                 `interval` elapses
		   * \param param a pointer that is passed to `callback`
		   * \returns a timer ID or 0 if an error occurs; call SDL_GetError() for more
		   *          information.
		   *
		   * \since This function is available since SDL 2.0.0.
		   *
		   * \sa SDL_RemoveTimer
		   */
		   //extern DECLSPEC SDL_TimerID SDLCALL SDL_AddTimer(Uint32 interval,
		   //	SDL_TimerCallback callback,
		   //	void* param);

		   /**
			* Remove a timer created with SDL_AddTimer().
			*
			* \param id the ID of the timer to remove
			* \returns SDL_TRUE if the timer is removed or SDL_FALSE if the timer wasn't
			*          found.
			*
			* \since This function is available since SDL 2.0.0.
			*
			* \sa SDL_AddTimer
			*/
			//bool SDLCALL SDL_RemoveTimer(SDL_TimerID id);
	};
}
