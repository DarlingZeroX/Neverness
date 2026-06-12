# Phase 8 Task: C# Interop Layer

## Objective
Extend NNNativeEngineAPI to expose Material, ShaderAsset, and SRP to C# via P/Invoke handles.

## Architecture
```
C# (Managed)
    ↓ P/Invoke
NNNativeEngineAPI (C functions)
    ↓ Handle → NNRef → Interface
NNRuntimeRenderAssets + NNRuntimeSRP (C++)
```

## API Surface

### Device (existing, extend)
```c
NNHandle NNE_CreateDevice(NNE_BackendType backend, void* hwnd, uint32_t w, uint32_t h);
void    NNE_DestroyDevice(NNHandle device);
void    NNE_ResizeDevice(NNHandle device, uint32_t w, uint32_t h);
```

### Shader Asset (new)
```c
NNHandle NNE_LoadShaderFromFile(NNHandle device, const char* path, int stage);
NNHandle NNE_LoadShaderFromMemory(NNHandle device, const char* source, uint32_t len, int stage, const char* name);
void     NNE_ReleaseShader(NNHandle shader);
```

### Material (new)
```c
NNHandle NNE_CreateMaterial();
void     NNE_MaterialSetVS(NNHandle mat, NNHandle shader);
void     NNE_MaterialSetPS(NNHandle mat, NNHandle shader);
void     NNE_MaterialSetFloat(NNHandle mat, const char* name, float value);
void     NNE_MaterialSetInt(NNHandle mat, const char* name, int value);
void     NNE_MaterialSetVec4(NNHandle mat, const char* name, float x, float y, float z, float w);
void     NNE_MaterialSetTexture(NNHandle mat, const char* name, NNHandle texture);
void     NNE_MaterialSetSampler(NNHandle mat, const char* name, NNHandle sampler);
void     NNE_MaterialSetCullMode(NNHandle mat, int mode);
void     NNE_MaterialSetBlend(NNHandle mat, bool enable, int src, int dst, int op);
void     NNE_MaterialSetDepth(NNHandle mat, bool depthEnable, bool depthWrite);
void     NNE_ReleaseMaterial(NNHandle mat);
```

### Material Instance (new)
```c
NNHandle NNE_CreateMaterialInstance(NNHandle baseMat);
void     NNE_MatInstSetFloat(NNHandle inst, const char* name, float value);
void     NNE_MatInstSetTexture(NNHandle inst, const char* name, NNHandle tex);
void     NNE_ReleaseMaterialInstance(NNHandle inst);
```

### Pipeline (new)
```c
NNHandle NNE_CreateForwardPipeline(NNHandle device, uint32_t w, uint32_t h);
void     NNE_DestroyPipeline(NNHandle pipeline);
void     NNE_ExecutePipeline(NNHandle pipeline, NNHandle cmd, const NNE_RenderContextData* ctx);
void     NNE_ResizePipeline(NNHandle pipeline, uint32_t w, uint32_t h);
```

### Render Context (new)
```c
struct NNE_RenderContextData {
    NNE_CameraData Camera;
    NNE_LightData Lights[8];
    int LightCount;
    float DeltaTime;
    float TotalTime;
    uint64_t FrameNumber;
};
```

## Tasks
1. Extend NNNativeEngineAPI with new C functions
2. Update HandleRegistry integration
3. Create C# bindings file
4. Test: C# creates material, sets params, verifies

## Verification
Build NNPhase8Test, verify all handles work.
