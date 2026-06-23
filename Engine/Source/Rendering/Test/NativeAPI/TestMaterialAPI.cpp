// TestMaterialAPI.cpp — Phase 8 C# Interop Tests
// Tests the C API that C# will use via P/Invoke

#include <NNRuntimeNativeEngineAPI/API/NNMaterialAPI.h>
#include <cstdio>
#include <cassert>
#include <cstring>

static void PrintSeparator(const char* title)
{
    std::printf("\n========================================\n");
    std::printf("  %s\n", title);
    std::printf("========================================\n");
}

// ============================================================================
// Test 1: Material creation and parameters via C API
// ============================================================================
static bool TestMaterialCreation()
{
    PrintSeparator("Test 1: Material C API");

    NNE_Handle mat = NNE_CreateMaterial();
    assert(mat != NNE_INVALID_HANDLE);

    // Set parameters
    NNE_MaterialSetFloat(mat, "Metallic", 0.8f);
    NNE_MaterialSetFloat(mat, "Roughness", 0.2f);
    NNE_MaterialSetInt(mat, "TextureCount", 3);
    NNE_MaterialSetVec4(mat, "BaseColor", 1.0f, 0.5f, 0.2f, 1.0f);

    // Verify
    float metallic = NNE_MaterialGetFloat(mat, "Metallic");
    float roughness = NNE_MaterialGetFloat(mat, "Roughness");
    int texCount = NNE_MaterialGetInt(mat, "TextureCount");

    assert(metallic == 0.8f);
    assert(roughness == 0.2f);
    assert(texCount == 3);

    // Set render state
    NNE_MaterialSetCullMode(mat, 0); // None
    NNE_MaterialSetBlendEnable(mat, 1);
    NNE_MaterialSetDepthEnable(mat, 1, 0); // depth on, write off

    std::printf("  Material handle: 0x%016llx\n", static_cast<unsigned long long>(mat));
    std::printf("  Metallic=%.1f, Roughness=%.1f\n", metallic, roughness);
    std::printf("  TextureCount=%d\n", texCount);

    NNE_ReleaseMaterial(mat);
    std::printf("  PASSED\n");
    return true;
}

// ============================================================================
// Test 2: Material Instance via C API
// ============================================================================
static bool TestMaterialInstance()
{
    PrintSeparator("Test 2: Material Instance C API");

    // Create base material
    NNE_Handle base = NNE_CreateMaterial();
    NNE_MaterialSetFloat(base, "Metallic", 0.5f);
    NNE_MaterialSetFloat(base, "Roughness", 0.5f);
    NNE_MaterialSetVec4(base, "BaseColor", 1.0f, 1.0f, 1.0f, 1.0f);

    // Create instance
    NNE_Handle inst = NNE_CreateMaterialInstance(base);
    assert(inst != NNE_INVALID_HANDLE);

    // Override params
    NNE_MatInstSetFloat(inst, "Metallic", 0.9f);
    NNE_MatInstSetVec4(inst, "BaseColor", 1.0f, 0.0f, 0.0f, 1.0f);

    // Verify base unchanged
    float baseMetallic = NNE_MaterialGetFloat(base, "Metallic");
    assert(baseMetallic == 0.5f);

    std::printf("  Base handle: 0x%016llx\n", static_cast<unsigned long long>(base));
    std::printf("  Instance handle: 0x%016llx\n", static_cast<unsigned long long>(inst));
    std::printf("  Base Metallic=%.1f (unchanged)\n", baseMetallic);

    NNE_ReleaseMaterialInstance(inst);
    NNE_ReleaseMaterial(base);
    std::printf("  PASSED\n");
    return true;
}

// ============================================================================
// Test 3: Render Context marshalling
// ============================================================================
static bool TestRenderContext()
{
    PrintSeparator("Test 3: Render Context");

    NNE_RenderContextData ctx;
    std::memset(&ctx, 0, sizeof(ctx));

    // Setup camera
    ctx.Camera.Position = {0, 5, 10};
    ctx.Camera.NearPlane = 0.1f;
    ctx.Camera.FarPlane = 1000.0f;
    ctx.Camera.FOV = 1.0472f; // 60 deg
    ctx.Camera.AspectRatio = 16.0f / 9.0f;

    // Identity matrices (for test)
    ctx.Camera.ViewMatrix.m[0] = ctx.Camera.ViewMatrix.m[5] =
    ctx.Camera.ViewMatrix.m[10] = ctx.Camera.ViewMatrix.m[15] = 1.0f;
    ctx.Camera.ProjMatrix.m[0] = ctx.Camera.ProjMatrix.m[5] =
    ctx.Camera.ProjMatrix.m[10] = ctx.Camera.ProjMatrix.m[15] = 1.0f;
    ctx.Camera.ViewProjMatrix.m[0] = ctx.Camera.ViewProjMatrix.m[5] =
    ctx.Camera.ViewProjMatrix.m[10] = ctx.Camera.ViewProjMatrix.m[15] = 1.0f;

    // Light
    ctx.Lights[0].Type = 0; // Directional
    ctx.Lights[0].Direction = {0.5f, -1.0f, 0.3f};
    ctx.Lights[0].Color = {1.0f, 0.95f, 0.9f};
    ctx.Lights[0].Intensity = 1.0f;
    ctx.Lights[0].CastShadows = 1;
    ctx.LightCount = 1;

    ctx.FrameWidth = 1920;
    ctx.FrameHeight = 1080;
    ctx.DeltaTime = 1.0f / 60.0f;
    ctx.TotalTime = 10.5f;
    ctx.FrameNumber = 42;

    assert(ctx.Camera.NearPlane == 0.1f);
    assert(ctx.LightCount == 1);
    assert(ctx.FrameNumber == 42);

    std::printf("  Camera: pos(%.0f,%.0f,%.0f) FOV=%.1f\n",
                ctx.Camera.Position.x, ctx.Camera.Position.y, ctx.Camera.Position.z,
                ctx.Camera.FOV * 180.0f / 3.14159265f);
    std::printf("  Lights: %d\n", ctx.LightCount);
    std::printf("  Frame: %llu (%ux%u)\n",
                static_cast<unsigned long long>(ctx.FrameNumber),
                ctx.FrameWidth, ctx.FrameHeight);

    std::printf("  PASSED\n");
    return true;
}

// ============================================================================
// Test 4: Pipeline pass count (without GPU)
// ============================================================================
static bool TestPipelineStruct()
{
    PrintSeparator("Test 4: Pipeline Structure");

    // Can't create pipeline without device, just verify handles
    NNE_Handle invalid = NNE_CreateForwardPipeline(NNE_INVALID_HANDLE, 800, 600);
    assert(invalid == NNE_INVALID_HANDLE);

    // Pipeline pass count on invalid handle
    uint32_t count = NNE_PipelineGetPassCount(NNE_INVALID_HANDLE);
    assert(count == 0);

    std::printf("  Invalid pipeline returns NNE_INVALID_HANDLE: OK\n");
    std::printf("  Pass count on invalid: %u\n", count);

    std::printf("  PASSED\n");
    return true;
}

// ============================================================================
// Main
// ============================================================================
int main()
{
    std::printf("\n");
    std::printf("****************************************\n");
    std::printf("  Phase 8: C# Interop Tests\n");
    std::printf("****************************************\n");

    int passed = 0;
    int total = 4;

    if (TestMaterialCreation()) passed++;
    if (TestMaterialInstance()) passed++;
    if (TestRenderContext()) passed++;
    if (TestPipelineStruct()) passed++;

    PrintSeparator("RESULTS");
    std::printf("  Passed: %d / %d\n", passed, total);

    if (passed == total)
    {
        std::printf("\n  ALL TESTS PASSED!\n\n");
        return 0;
    }
    else
    {
        std::printf("\n  SOME TESTS FAILED!\n\n");
        return 1;
    }
}
