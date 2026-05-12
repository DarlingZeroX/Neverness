/*
 * VisionGal — GalgameCore 迁移期弃用标记
 *
 * Phase 0：与 SubsystemBus 重构配套；新代码请通过 ISubsystemBus / 各 I*Subsystem 访问能力，
 * 勿再扩展 IGalGameEngine 上的门面 API。
 */

#pragma once

#if defined(_MSC_VER)
#define VG_DEPRECATED_MSG(msg) __declspec(deprecated(msg))
#elif defined(__GNUC__) || defined(__clang__)
#define VG_DEPRECATED_MSG(msg) __attribute__((deprecated(msg)))
#else
#define VG_DEPRECATED_MSG(msg) [[deprecated(msg)]]
#endif

/** 无说明文本的弃用标记（尽量少用，优先 VG_DEPRECATED_MSG） */
#if __cplusplus >= 201402L
#define VG_DEPRECATED [[deprecated]]
#else
#define VG_DEPRECATED VG_DEPRECATED_MSG("deprecated")
#endif
