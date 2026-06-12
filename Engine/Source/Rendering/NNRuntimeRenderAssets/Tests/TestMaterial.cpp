// TestMaterial.cpp — Phase 6 Material System Tests
// Tests:
//   1. Material creation and parameter setting
//   2. Material instance with overrides
//   3. Shader asset loading from memory
//   4. Pipeline cache - PSO reuse
//   5. Asset registry - handle management

#include <NNRuntimeRenderAssets/Material/NNMaterial.h>
#include <NNRuntimeRenderAssets/Material/NNMaterialInstance.h>
#include <NNRuntimeRenderAssets/Shader/NNShaderAsset.h>
#include <NNRuntimeRenderAssets/Shader/NNShaderVariant.h>
#include <NNRuntimeRenderAssets/Cache/NNPipelineCache.h>
#include <NNRuntimeRenderAssets/Cache/NNAssetRegistry.h>
#include <cstdio>
#include <cassert>
#include <cstring>

using namespace NN::Runtime::Assets;
using namespace NN::Runtime::Render;
using namespace NN::Runtime::Core;

static void PrintSeparator(const char* title)
{
    std::printf("\n========================================\n");
    std::printf("  %s\n", title);
    std::printf("========================================\n");
}

// ============================================================================
// Test 1: Material creation and parameters
// ============================================================================
static bool TestMaterialCreation()
{
    PrintSeparator("Test 1: Material Creation");

    auto mat = NNRef<NNMaterial>(new NNMaterial());
    assert(mat);
    assert(mat->GetRefCount() == 1);

    // Set parameters
    mat->SetFloat("Metallic", 0.8f);
    mat->SetFloat("Roughness", 0.2f);
    mat->SetInt("TextureCount", 3);
    mat->SetVector4("BaseColor", 1.0f, 0.5f, 0.2f, 1.0f);

    // Verify parameters
    auto* metallic = mat->GetParam("Metallic");
    assert(metallic != nullptr);
    assert(metallic->Type == NNMaterialParamType::Float);
    assert(metallic->Float == 0.8f);

    auto* roughness = mat->GetParam("Roughness");
    assert(roughness != nullptr);
    assert(roughness->Float == 0.2f);

    auto* texCount = mat->GetParam("TextureCount");
    assert(texCount != nullptr);
    assert(texCount->Type == NNMaterialParamType::Int);
    assert(texCount->Int == 3);

    auto* baseColor = mat->GetParam("BaseColor");
    assert(baseColor != nullptr);
    assert(baseColor->Type == NNMaterialParamType::Vector4);
    assert(baseColor->Vec4[0] == 1.0f);
    assert(baseColor->Vec4[1] == 0.5f);
    assert(baseColor->Vec4[2] == 0.2f);
    assert(baseColor->Vec4[3] == 1.0f);

    assert(mat->GetParamCount() == 4);

    // Set state
    mat->SetCullMode(NNCullMode::None);
    mat->SetBlendEnable(true);
    mat->SetBlendFunc(NNBlendFactor::SrcAlpha, NNBlendFactor::InvSrcAlpha, NNBlendOp::Add);
    mat->SetDepthWrite(false);

    std::printf("  Material created with %zu params\n", mat->GetParamCount());
    std::printf("  Metallic=%.1f, Roughness=%.1f\n", metallic->Float, roughness->Float);
    std::printf("  BaseColor=(%.1f, %.1f, %.1f, %.1f)\n",
                baseColor->Vec4[0], baseColor->Vec4[1], baseColor->Vec4[2], baseColor->Vec4[3]);

    std::printf("  PASSED\n");
    return true;
}

// ============================================================================
// Test 2: Material instance with overrides
// ============================================================================
static bool TestMaterialInstance()
{
    PrintSeparator("Test 2: Material Instance");

    // Create base material
    auto base = NNRef<NNMaterial>(new NNMaterial());
    base->SetFloat("Metallic", 0.5f);
    base->SetFloat("Roughness", 0.5f);
    base->SetVector4("BaseColor", 1.0f, 1.0f, 1.0f, 1.0f);

    // Create instance
    auto instance = NNRef<NNMaterialInstance>(new NNMaterialInstance(base));
    assert(instance);
    assert(instance->GetBaseMaterial() == base);

    // Override some params
    instance->SetFloat("Metallic", 0.9f);
    instance->SetVector4("BaseColor", 1.0f, 0.0f, 0.0f, 1.0f);

    // Verify: overridden params
    auto* instMetallic = instance->GetParam("Metallic");
    assert(instMetallic != nullptr);
    assert(instMetallic->Float == 0.9f);

    auto* instColor = instance->GetParam("BaseColor");
    assert(instColor != nullptr);
    assert(instColor->Vec4[0] == 1.0f);
    assert(instColor->Vec4[1] == 0.0f);

    // Verify: non-overridden params come from base
    auto* instRoughness = instance->GetParam("Roughness");
    assert(instRoughness != nullptr);
    assert(instRoughness->Float == 0.5f);

    // Verify: base material unchanged
    auto* baseMetallic = base->GetParam("Metallic");
    assert(baseMetallic != nullptr);
    assert(baseMetallic->Float == 0.5f); // Still 0.5, not 0.9

    std::printf("  Instance Metallic=%.1f (override)\n", instMetallic->Float);
    std::printf("  Instance Roughness=%.1f (from base)\n", instRoughness->Float);
    std::printf("  Base Metallic=%.1f (unchanged)\n", baseMetallic->Float);

    std::printf("  PASSED\n");
    return true;
}

// ============================================================================
// Test 3: Shader asset from memory
// ============================================================================
static bool TestShaderAsset()
{
    PrintSeparator("Test 3: Shader Asset");

    // Create shader source in memory
    const char* vsSource = R"(
        struct VSInput { float3 Pos : POSITION; };
        struct VSOutput { float4 Pos : SV_POSITION; };
        VSOutput main(VSInput input) {
            VSOutput output;
            output.Pos = float4(input.Pos, 1.0);
            return output;
        }
    )";

    const char* psSource = R"(
        struct PSInput { float4 Pos : SV_POSITION; };
        float4 main(PSInput input) : SV_TARGET {
            return float4(1.0, 0.5, 0.2, 1.0);
        }
    )";

    auto vsAsset = NNShaderAsset::LoadFromMemory(vsSource, static_cast<uint32_t>(strlen(vsSource)),
                                                   NNShaderStage::Vertex, "TestVS");
    auto psAsset = NNShaderAsset::LoadFromMemory(psSource, static_cast<uint32_t>(strlen(psSource)),
                                                   NNShaderStage::Pixel, "TestPS");

    assert(vsAsset);
    assert(psAsset);

    assert(vsAsset->GetStage() == NNShaderStage::Vertex);
    assert(psAsset->GetStage() == NNShaderStage::Pixel);

    assert(strcmp(vsAsset->GetName(), "TestVS") == 0);
    assert(strcmp(psAsset->GetName(), "TestPS") == 0);

    assert(vsAsset->HasSource());
    assert(!vsAsset->HasByteCode());

    std::printf("  VS asset: %s (%u bytes)\n", vsAsset->GetName(),
                static_cast<uint32_t>(strlen(vsAsset->GetSource())));
    std::printf("  PS asset: %s (%u bytes)\n", psAsset->GetName(),
                static_cast<uint32_t>(strlen(psAsset->GetSource())));

    std::printf("  PASSED\n");
    return true;
}

// ============================================================================
// Test 4: Pipeline cache
// ============================================================================
static bool TestPipelineCache()
{
    PrintSeparator("Test 4: Pipeline Cache Key Hashing");

    // Test that identical keys produce same hash
    NNPipelineKey key1;
    key1.VS = reinterpret_cast<INNShader*>(0x1000);
    key1.PS = reinterpret_cast<INNShader*>(0x2000);
    key1.VertexLayout.Stride = 12;
    key1.RTVFormat = NNPixelFormat::RGBA8_UNORM;
    key1.DSVFormat = NNPixelFormat::D32_FLOAT;
    key1.SampleCount = 1;

    NNPipelineKey key2 = key1;

    assert(key1.Hash() == key2.Hash());
    assert(key1 == key2);

    // Different key should have different hash
    NNPipelineKey key3 = key1;
    key3.RTVFormat = NNPixelFormat::RGBA8_SRGB;

    assert(key1.Hash() != key3.Hash());
    assert(!(key1 == key3));

    std::printf("  Key1 hash: 0x%016llx\n", static_cast<unsigned long long>(key1.Hash()));
    std::printf("  Key2 hash: 0x%016llx (same)\n", static_cast<unsigned long long>(key2.Hash()));
    std::printf("  Key3 hash: 0x%016llx (different format)\n", static_cast<unsigned long long>(key3.Hash()));

    std::printf("  PASSED\n");
    return true;
}

// ============================================================================
// Test 5: Asset registry
// ============================================================================
static bool TestAssetRegistry()
{
    PrintSeparator("Test 5: Asset Registry");

    auto registry = NNRef<NNAssetRegistry>(new NNAssetRegistry());
    assert(registry);

    // Create assets
    auto mat1 = NNRef<NNMaterial>(new NNMaterial());
    mat1->SetFloat("Metallic", 0.8f);

    auto mat2 = NNRef<NNMaterial>(new NNMaterial());
    mat2->SetFloat("Roughness", 0.3f);

    // Register
    NNRenderHandle h1 = registry->RegisterMaterial(mat1);
    NNRenderHandle h2 = registry->RegisterMaterial(mat2);

    assert(h1 != NN_INVALID_HANDLE);
    assert(h2 != NN_INVALID_HANDLE);
    assert(h1 != h2);

    // Get back
    auto* getMat1 = registry->GetMaterial(h1);
    auto* getMat2 = registry->GetMaterial(h2);

    assert(getMat1 == mat1.Get());
    assert(getMat2 == mat2.Get());

    // Stats
    assert(registry->GetMaterialCount() == 2);
    assert(registry->GetTotalCount() == 2);

    // Release
    registry->Release(h1);
    assert(registry->GetMaterialCount() == 1);
    assert(registry->GetMaterial(h1) == nullptr);
    assert(registry->GetMaterial(h2) != nullptr);

    std::printf("  Registered 2 materials\n");
    std::printf("  Handle1=0x%016llx, Handle2=0x%016llx\n",
                static_cast<unsigned long long>(h1), static_cast<unsigned long long>(h2));
    std::printf("  After release: %zu materials\n", registry->GetMaterialCount());

    std::printf("  PASSED\n");
    return true;
}

// ============================================================================
// Test 6: Shader variant key
// ============================================================================
static bool TestShaderVariantKey()
{
    PrintSeparator("Test 6: Shader Variant Key");

    NNShaderVariantKey key1;
    key1.FeatureMask = 0x01;
    key1.MaterialFlags = 0x10;

    NNShaderVariantKey key2;
    key2.FeatureMask = 0x01;
    key2.MaterialFlags = 0x10;

    NNShaderVariantKey key3;
    key3.FeatureMask = 0x02;
    key3.MaterialFlags = 0x10;

    assert(key1.Hash() == key2.Hash());
    assert(key1 == key2);
    assert(key1.Hash() != key3.Hash());
    assert(!(key1 == key3));

    std::printf("  Key1 hash: 0x%016llx\n", static_cast<unsigned long long>(key1.Hash()));
    std::printf("  Key3 hash: 0x%016llx (different)\n", static_cast<unsigned long long>(key3.Hash()));

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
    std::printf("  Phase 6: Material System Tests\n");
    std::printf("****************************************\n");

    int passed = 0;
    int total = 6;

    if (TestMaterialCreation()) passed++;
    if (TestMaterialInstance()) passed++;
    if (TestShaderAsset()) passed++;
    if (TestPipelineCache()) passed++;
    if (TestAssetRegistry()) passed++;
    if (TestShaderVariantKey()) passed++;

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
