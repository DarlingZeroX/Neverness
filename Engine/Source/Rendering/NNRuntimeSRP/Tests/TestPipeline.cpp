// TestPipeline.cpp — Phase 7 SRP Tests
// Tests:
//   1. Pipeline pass description creation
//   2. Camera data setup
//   3. Scene data (lights)
//   4. Render context validation
//   5. Forward pipeline structure

#include <NNRuntimeSRP/Pipeline/NNPipelinePass.h>
#include <NNRuntimeSRP/Pipeline/NNForwardPipeline.h>
#include <NNRuntimeSRP/Pipeline/INNRenderPipeline.h>
#include <NNRuntimeSRP/Context/NNRenderContext.h>
#include <NNRuntimeSRP/Context/NNCameraData.h>
#include <NNRuntimeSRP/Context/NNSceneData.h>
#include <cstdio>
#include <cassert>
#include <cstring>
#include <cmath>

using namespace NN::Runtime::SRP;

static void PrintSeparator(const char* title)
{
    std::printf("\n========================================\n");
    std::printf("  %s\n", title);
    std::printf("========================================\n");
}

// ============================================================================
// Test 1: Pipeline Pass Description
// ============================================================================
static bool TestPipelinePassDesc()
{
    PrintSeparator("Test 1: Pipeline Pass Description");

    NNPipelinePassDesc shadowDesc;
    shadowDesc.Name = "Shadow";
    shadowDesc.Type = NNPassType::Shadow;
    shadowDesc.RTVFormat = NNPixelFormat::D32_FLOAT;
    shadowDesc.DSVFormat = NNPixelFormat::D32_FLOAT;
    shadowDesc.Width = 2048;
    shadowDesc.Height = 2048;
    shadowDesc.ClearColor = false;
    shadowDesc.ClearDepth = true;

    assert(shadowDesc.Type == NNPassType::Shadow);
    assert(shadowDesc.Width == 2048);
    assert(shadowDesc.ClearDepth == true);
    assert(shadowDesc.ClearColor == false);

    NNPipelinePassDesc forwardDesc;
    forwardDesc.Name = "Forward";
    forwardDesc.Type = NNPassType::Forward;
    forwardDesc.RTVFormat = NNPixelFormat::RGBA8_UNORM;
    forwardDesc.DSVFormat = NNPixelFormat::D32_FLOAT;
    forwardDesc.Width = 1920;
    forwardDesc.Height = 1080;
    forwardDesc.ClearColor = true;
    forwardDesc.ClearR = 0.1f;
    forwardDesc.ClearG = 0.1f;
    forwardDesc.ClearB = 0.2f;

    assert(forwardDesc.Type == NNPassType::Forward);
    assert(forwardDesc.ClearR == 0.1f);

    std::printf("  Shadow: %s (%ux%u)\n", shadowDesc.Name.c_str(), shadowDesc.Width, shadowDesc.Height);
    std::printf("  Forward: %s (%ux%u)\n", forwardDesc.Name.c_str(), forwardDesc.Width, forwardDesc.Height);
    std::printf("  PASSED\n");
    return true;
}

// ============================================================================
// Test 2: Camera Data
// ============================================================================
static bool TestCameraData()
{
    PrintSeparator("Test 2: Camera Data");

    NNCameraData camera;

    // Setup perspective camera
    Vector3 eye = {0, 5, 10};
    Vector3 target = {0, 0, 0};
    Vector3 up = {0, 1, 0};
    float fov = 60.0f * 3.14159265f / 180.0f;
    float aspect = 16.0f / 9.0f;

    camera.SetupPerspective(eye, target, up, fov, aspect, 0.1f, 1000.0f);

    // Verify
    assert(camera.Position.x == 0.0f);
    assert(camera.Position.y == 5.0f);
    assert(camera.Position.z == 10.0f);
    assert(camera.NearPlane == 0.1f);
    assert(camera.FarPlane == 1000.0f);

    // Verify matrices are not all zeros
    bool hasNonZero = false;
    for (int i = 0; i < 16; ++i)
    {
        if (camera.ViewMatrix.m[i] != 0.0f) hasNonZero = true;
    }
    assert(hasNonZero);

    hasNonZero = false;
    for (int i = 0; i < 16; ++i)
    {
        if (camera.ProjMatrix.m[i] != 0.0f) hasNonZero = true;
    }
    assert(hasNonZero);

    // Verify ViewProj = Proj * View
    Matrix4x4 expectedVP = camera.ProjMatrix * camera.ViewMatrix;
    for (int i = 0; i < 16; ++i)
    {
        float diff = std::abs(camera.ViewProjMatrix.m[i] - expectedVP.m[i]);
        assert(diff < 0.001f);
    }

    std::printf("  Camera pos: (%.1f, %.1f, %.1f)\n", eye.x, eye.y, eye.z);
    std::printf("  FOV: %.0f deg, Aspect: %.2f\n", fov * 180.0f / 3.14159265f, aspect);
    std::printf("  Near=%.1f, Far=%.1f\n", camera.NearPlane, camera.FarPlane);
    std::printf("  ViewProj matrix verified\n");

    // Test ortho
    NNCameraData orthoCam;
    orthoCam.SetupOrtho({0, 10, 0}, {0, 0, 0}, {0, 0, -1},
                        -10, 10, -10, 10, 0.1f, 100.0f);

    hasNonZero = false;
    for (int i = 0; i < 16; ++i)
    {
        if (orthoCam.ProjMatrix.m[i] != 0.0f) hasNonZero = true;
    }
    assert(hasNonZero);

    std::printf("  Ortho camera verified\n");
    std::printf("  PASSED\n");
    return true;
}

// ============================================================================
// Test 3: Scene Data (Lights)
// ============================================================================
static bool TestSceneData()
{
    PrintSeparator("Test 3: Scene Data");

    NNSceneData scene;

    // Add directional light
    scene.AddDirectionalLight({0.5f, -1.0f, 0.3f}, {1.0f, 0.95f, 0.9f}, 1.0f);

    assert(scene.Lights.size() == 1);
    assert(scene.Lights[0].Type == NNLightType::Directional);
    assert(scene.Lights[0].Intensity == 1.0f);

    // Get main light
    const auto* mainLight = scene.GetMainLight();
    assert(mainLight != nullptr);
    assert(mainLight->Type == NNLightType::Directional);

    // Setup shadow camera
    scene.Lights[0].CastShadows = true;
    scene.Lights[0].ShadowMapSize = 1024;
    scene.Lights[0].SetupShadowCamera({0, 0, 0}, 20.0f);

    // Verify light view-proj is not all zeros
    bool hasNonZero = false;
    for (int i = 0; i < 16; ++i)
    {
        if (scene.Lights[0].LightViewProj.m[i] != 0.0f) hasNonZero = true;
    }
    assert(hasNonZero);

    // Ambient
    scene.Ambient.SkyColor = {0.5f, 0.7f, 1.0f};
    scene.Ambient.SkyIntensity = 0.3f;

    std::printf("  Lights: %zu\n", scene.Lights.size());
    std::printf("  Main light dir: (%.1f, %.1f, %.1f)\n",
                mainLight->Direction.x, mainLight->Direction.y, mainLight->Direction.z);
    std::printf("  Shadow map: %ux%u\n", scene.Lights[0].ShadowMapSize, scene.Lights[0].ShadowMapSize);
    std::printf("  Ambient sky: (%.1f, %.1f, %.1f) @ %.1f\n",
                scene.Ambient.SkyColor.x, scene.Ambient.SkyColor.y, scene.Ambient.SkyColor.z,
                scene.Ambient.SkyIntensity);
    std::printf("  PASSED\n");
    return true;
}

// ============================================================================
// Test 4: Render Context Validation
// ============================================================================
static bool TestRenderContext()
{
    PrintSeparator("Test 4: Render Context");

    NNRenderContext ctx;

    // Invalid context (no device)
    assert(!ctx.IsValid());

    // Set minimum fields
    ctx.Device = reinterpret_cast<INNRenderDevice*>(0x1); // Dummy
    ctx.PipelineCache = reinterpret_cast<NNPipelineCache*>(0x2); // Dummy
    ctx.FrameWidth = 1920;
    ctx.FrameHeight = 1080;

    assert(ctx.IsValid());

    // Camera
    ctx.Camera.SetupPerspective({0, 5, 10}, {0, 0, 0}, {0, 1, 0},
                                 60.0f * 3.14159265f / 180.0f, 16.0f / 9.0f, 0.1f, 1000.0f);

    // Scene
    ctx.Scene.AddDirectionalLight({0, -1, 0}, {1, 1, 1}, 1.0f);
    ctx.FrameNumber = 42;
    ctx.DeltaTime = 1.0f / 60.0f;
    ctx.TotalTime = 10.5f;

    assert(ctx.FrameNumber == 42);
    assert(ctx.DeltaTime > 0.016f && ctx.DeltaTime < 0.017f);

    std::printf("  Context valid: %s\n", ctx.IsValid() ? "true" : "false");
    std::printf("  Frame: %llu, DT: %.4f\n",
                static_cast<unsigned long long>(ctx.FrameNumber), ctx.DeltaTime);
    std::printf("  Lights: %zu\n", ctx.Scene.Lights.size());
    std::printf("  PASSED\n");
    return true;
}

// ============================================================================
// Test 5: Forward Pipeline Structure
// ============================================================================
static bool TestForwardPipeline()
{
    PrintSeparator("Test 5: Forward Pipeline Structure");

    // Create pipeline (no device, just test structure)
    auto pipeline = NNRef<NNForwardPipeline>(new NNForwardPipeline());
    assert(pipeline);

    assert(strcmp(pipeline->GetName(), "ForwardPipeline") == 0);
    assert(pipeline->GetPassCount() == 0); // Not initialized yet

    // Verify passes are accessible
    auto& shadowPass = pipeline->GetShadowPass();
    auto& forwardPass = pipeline->GetForwardPass();
    auto& imguiPass = pipeline->GetImGuiPass();

    // Passes exist (just not set up)
    assert(shadowPass.GetShadowMapSize() == 2048); // Default

    std::printf("  Pipeline: %s\n", pipeline->GetName());
    std::printf("  Passes before init: %u\n", pipeline->GetPassCount());
    std::printf("  Shadow map default size: %u\n", shadowPass.GetShadowMapSize());
    std::printf("  PASSED\n");
    return true;
}

// ============================================================================
// Test 6: Matrix math
// ============================================================================
static bool TestMatrixMath()
{
    PrintSeparator("Test 6: Matrix Math");

    // Identity
    Matrix4x4 id = Matrix4x4::Identity();
    assert(id.m[0] == 1.0f && id.m[5] == 1.0f && id.m[10] == 1.0f && id.m[15] == 1.0f);
    assert(id.m[1] == 0.0f && id.m[2] == 0.0f);

    // Multiply by identity = same
    Matrix4x4 persp = Matrix4x4::Perspective(1.0f, 16.0f/9.0f, 0.1f, 100.0f);
    Matrix4x4 result = id * persp;
    for (int i = 0; i < 16; ++i)
    {
        assert(std::abs(result.m[i] - persp.m[i]) < 0.001f);
    }

    // Vector transform
    Vector4 v = {1, 0, 0, 1};
    Vector4 tv = id * v;
    assert(std::abs(tv.x - 1.0f) < 0.001f);
    assert(std::abs(tv.y - 0.0f) < 0.001f);

    // Cross product
    Vector3 a = {1, 0, 0};
    Vector3 b = {0, 1, 0};
    Vector3 c = a.Cross(b);
    assert(std::abs(c.z - 1.0f) < 0.001f);

    // Normalize
    Vector3 n = Vector3{3, 4, 0}.Normalized();
    assert(std::abs(n.Length() - 1.0f) < 0.001f);

    std::printf("  Identity * Perspective = Perspective: OK\n");
    std::printf("  Identity * Vec4 = Vec4: OK\n");
    std::printf("  Cross product: OK\n");
    std::printf("  Normalize: OK (len=%.3f)\n", n.Length());
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
    std::printf("  Phase 7: SRP Tests\n");
    std::printf("****************************************\n");

    int passed = 0;
    int total = 6;

    if (TestPipelinePassDesc()) passed++;
    if (TestCameraData()) passed++;
    if (TestSceneData()) passed++;
    if (TestRenderContext()) passed++;
    if (TestForwardPipeline()) passed++;
    if (TestMatrixMath()) passed++;

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
