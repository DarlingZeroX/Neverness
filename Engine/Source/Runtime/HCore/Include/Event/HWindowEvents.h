// Copyright (c) 2025 梦旅缘心
// This file is part of VisionGal and is licensed under the MIT License.
// See the LICENSE file in the project root for details.

#pragma once
#include "../../Interface/HCoreTypes.h"
#include "HEventBase.h"
#include <HCore/Include/Meta/Macros.h>

namespace Horizon::Events
{
	/**
     *  \brief Event subtype for window events
     */
	enum class HMetaEnable() HWindowEventType
    {
        /// <summary>
        /// Window Event
        /// </summary>
        NONE,           /**< Never used */
        SHOWN,          /**< Window has been shown */
        HIDDEN,         /**< Window has been hidden */
        EXPOSED,        /**< Window has been exposed and should be
                                             redrawn */
        MOVED,          /**< Window has been moved to data1, data2
                                         */
        RESIZED,        /**< Window has been resized to data1xdata2 */
        SIZE_CHANGED,   /**< The window size has changed, either as
                                             a result of an API call or through the
                                             system or user changing the window size. */
        MINIMIZED,      /**< Window has been minimized */
        MAXIMIZED,      /**< Window has been maximized */
        RESTORED,       /**< Window has been restored to normal size
                                             and position */
        ENTER,          /**< Window has gained mouse focus */
        LEAVE,          /**< Window has lost mouse focus */
        FOCUS_GAINED,   /**< Window has gained keyboard focus */
        FOCUS_LOST,     /**< Window has lost keyboard focus */
        CLOSE,          /**< The window manager requests that the window be closed */
        TAKE_FOCUS,     /**< Window is being offered a focus (should SetWindowInputFocus() on itself or a subwindow, or ignore) */
        HIT_TEST,       /**< Window had a hit test that wasn't SDL_HITTEST_NORMAL. */
        ICCPROF_CHANGED,/**< The ICC profile of the window's display has changed. */
        DISPLAY_CHANGED, /**< Window has been moved to display data1. */

        /// <summary>
        /// Drop Event
        /// </summary>
        DROP_BEGIN,        /**< The system requests a file open */
        DROP_FILE,         /**< text/plain drag-and-drop event */
        DROP_TEXT,         /**< A new set of drops is beginning (NULL filename) */
        DROP_COMPLETE,     /**< Current set of drops is now complete (NULL filename) */
    } ;

	/**
	 *  \brief Window state change event data (event.window.*)
	 */
	struct HWindowEvent : public HEvent
	{
        ~HWindowEvent() override = default;

		uint32 timestamp = 0;              /**< In milliseconds, populated using SDL_GetTicks() */
		uint32 windowID = 0;               /**< The associated window */
        HWindowEventType eventType;        /**< ::SDL_WindowEventID */
		uint8 padding1= 0;
		uint8 padding2= 0;
		uint8 padding3= 0;
		int32 data1 = 0;                    /**< event dependent data */
		int32 data2 = 0;                    /**< event dependent data */
        std::string file;

        std::uint32_t GetEventType() const override { return static_cast<unsigned>(eventType); }
        const char* GetName() const override { return ""; }

        virtual std::string ToString() const { return GetName(); }

		EVENT_CLASS_GLOBAL_CATEGORY(Window) 
	} ;

}