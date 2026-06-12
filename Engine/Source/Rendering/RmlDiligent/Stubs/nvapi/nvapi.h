// =============================================================================
// nvapi.h — 最小 NVAPI stub，用于 Diligent D3D11/D3D12 编译
// 只提供 Diligent 使用的 NVAPI 函数声明
// =============================================================================

#pragma once

#include <windows.h>
#include <d3d11.h>
#include <d3d11_2.h>
#include <d3d12.h>

// NVAPI 基本类型
typedef int NvAPI_Status;

#define NVAPI_OK 0
#define NVAPI_ERROR (-1)

// NVAPI 函数声明（Diligent 使用的）
#ifdef __cplusplus
extern "C" {
#endif

NvAPI_Status NvAPI_Initialize(void);
NvAPI_Status NvAPI_Unload(void);

// D3D11 NVAPI 函数
NvAPI_Status NvAPI_D3D11_UpdateTileMappings(
    ID3D11DeviceContext2* pDeviceContext,
    ID3D11Resource* pResource,
    UINT NumRegions,
    const D3D11_TILED_RESOURCE_COORDINATE* pRegionStartCoordinates,
    const D3D11_TILE_REGION_SIZE* pRegionSizes,
    ID3D11Buffer* pResourceHeap,
    UINT NumRanges,
    const UINT* pRangeFlags,
    const UINT* pHeapRangeStartOffsets,
    const UINT* pRangeTileCounts,
    UINT Flags);

NvAPI_Status NvAPI_D3D11_MultiDrawInstancedIndirect(
    ID3D11DeviceContext* pDeviceContext,
    UINT drawCount,
    ID3D11Buffer* pBufferForArgs,
    UINT alignedByteOffsetForArgs,
    UINT byteStrideForArgs);

NvAPI_Status NvAPI_D3D11_MultiDrawIndexedInstancedIndirect(
    ID3D11DeviceContext* pDeviceContext,
    UINT drawCount,
    ID3D11Buffer* pBufferForArgs,
    UINT alignedByteOffsetForArgs,
    UINT byteStrideForArgs);

NvAPI_Status NvAPI_D3D11_TiledResourceBarrier(
    ID3D11DeviceContext* pDeviceContext,
    void* pTiledResourceOrViewAccessBeforeBarrier,
    void* pTiledResourceOrViewAccessAfterBarrier);

NvAPI_Status NvAPI_D3D11_CreateTiledTexture2DArray(
    ID3D11Device* pDevice,
    const D3D11_TEXTURE2D_DESC* pDesc,
    void* pReserved,
    ID3D11Texture2D** ppTexture2DArray);

// D3D12 NVAPI 函数
NvAPI_Status NvAPI_D3D12_UpdateTileMappings(
    ID3D12CommandQueue* pCommandQueue,
    ID3D12Resource* pResource,
    UINT NumResourceRegions,
    const D3D12_TILED_RESOURCE_COORDINATE* pRegionStartCoordinates,
    const D3D12_TILE_REGION_SIZE* pRegionSizes,
    ID3D12Heap* pResourceHeap,
    UINT NumRanges,
    const D3D12_TILE_RANGE_FLAGS* pRangeFlags,
    const UINT* pHeapRangeStartOffsets,
    const UINT* pRangeTileCounts,
    D3D12_TILE_MAPPING_FLAGS Flags);

NvAPI_Status NvAPI_D3D12_CreateHeap(
    ID3D12Device* pDevice,
    const D3D12_HEAP_DESC* pHeapDesc,
    REFIID riid,
    void** ppvHeap);

NvAPI_Status NvAPI_D3D12_ResourceAliasingBarrier(
    ID3D12GraphicsCommandList* pCommandList,
    UINT NumBarriers,
    const D3D12_RESOURCE_BARRIER* pBarriers);

NvAPI_Status NvAPI_D3D12_CreateReservedResource(
    ID3D12Device* pDevice,
    const D3D12_RESOURCE_DESC* pDesc,
    D3D12_RESOURCE_STATES InitialState,
    const D3D12_CLEAR_VALUE* pOptimizedClearValue,
    REFIID riid,
    void** ppvResource,
    BOOL bTexture2DArray,
    ID3D12Heap* pHeap);

#ifdef __cplusplus
}
#endif
