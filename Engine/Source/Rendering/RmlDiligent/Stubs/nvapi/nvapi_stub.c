// =============================================================================
// nvapi_stub.c — NVAPI 函数 stub 实现
// 用于 Diligent D3D11/D3D12 编译，返回 NVAPI_ERROR 表示 NVAPI 不可用
// =============================================================================

#include "nvapi.h"

// NVAPI 基本函数
NvAPI_Status NvAPI_Initialize(void) { return NVAPI_ERROR; }
NvAPI_Status NvAPI_Unload(void) { return NVAPI_ERROR; }

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
    UINT Flags) { return NVAPI_ERROR; }

NvAPI_Status NvAPI_D3D11_MultiDrawInstancedIndirect(
    ID3D11DeviceContext* pDeviceContext,
    UINT drawCount,
    ID3D11Buffer* pBufferForArgs,
    UINT alignedByteOffsetForArgs,
    UINT byteStrideForArgs) { return NVAPI_ERROR; }

NvAPI_Status NvAPI_D3D11_MultiDrawIndexedInstancedIndirect(
    ID3D11DeviceContext* pDeviceContext,
    UINT drawCount,
    ID3D11Buffer* pBufferForArgs,
    UINT alignedByteOffsetForArgs,
    UINT byteStrideForArgs) { return NVAPI_ERROR; }

NvAPI_Status NvAPI_D3D11_TiledResourceBarrier(
    ID3D11DeviceContext* pDeviceContext,
    void* pTiledResourceOrViewAccessBeforeBarrier,
    void* pTiledResourceOrViewAccessAfterBarrier) { return NVAPI_ERROR; }

NvAPI_Status NvAPI_D3D11_CreateTiledTexture2DArray(
    ID3D11Device* pDevice,
    const D3D11_TEXTURE2D_DESC* pDesc,
    void* pReserved,
    ID3D11Texture2D** ppTexture2DArray) { return NVAPI_ERROR; }

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
    D3D12_TILE_MAPPING_FLAGS Flags) { return NVAPI_ERROR; }

NvAPI_Status NvAPI_D3D12_CreateHeap(
    ID3D12Device* pDevice,
    const D3D12_HEAP_DESC* pHeapDesc,
    REFIID riid,
    void** ppvHeap) { return NVAPI_ERROR; }

NvAPI_Status NvAPI_D3D12_ResourceAliasingBarrier(
    ID3D12GraphicsCommandList* pCommandList,
    UINT NumBarriers,
    const D3D12_RESOURCE_BARRIER* pBarriers) { return NVAPI_ERROR; }

NvAPI_Status NvAPI_D3D12_CreateReservedResource(
    ID3D12Device* pDevice,
    const D3D12_RESOURCE_DESC* pDesc,
    D3D12_RESOURCE_STATES InitialState,
    const D3D12_CLEAR_VALUE* pOptimizedClearValue,
    REFIID riid,
    void** ppvResource,
    BOOL bTexture2DArray,
    ID3D12Heap* pHeap) { return NVAPI_ERROR; }
