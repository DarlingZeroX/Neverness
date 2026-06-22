#pragma once

/**
 * @file NNAPIConfig.h
 * @brief NNAPI 模块编译配置。
 *
 * NNAPI 是纯 STATIC 库（ABI 头文件 + Stub），不导出函数。
 * 导出函数（NNNativeApi_GetDefaultTable 等）由 NNRuntimeEngineServices（SHARED）提供。
 */
