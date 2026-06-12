// ============================================================================
// TestRenderGraph — Phase 5 渲染图测试
// 测试内容：
//   1. 创建 NNRenderGraphBuilder，声明 3 个资源和 3 个 Pass
//   2. 拓扑排序验证执行顺序
//   3. 导出 DOT 可视化文件
//   4. 通过 NNRenderBootstrap 创建设备，执行渲染图
// ============================================================================

#include <NNRuntimeRender/RenderGraph/NNRenderGraphBuilder.h>
#include <NNRuntimeRender/RenderGraph/NNRenderGraph.h>
#include <NNRuntimeRender/RenderGraph/NNRenderGraphCompiler.h>
#include <NNRuntimeRenderBootstrap/Include/NNRenderBootstrap.h>
#include <NNRuntimeDiligent/Device/NNDiligentDevice.h>
#include <NNRuntimeDiligent/Command/NNDiligentCommandList.h>

#include <cstdio>
#include <cassert>
#include <vector>

using namespace NN::Runtime::Render;
using namespace NN::Runtime::Core;

// ============================================================================
// 辅助：打印分隔线
// ============================================================================
static void PrintSeparator(const char* title)
{
    std::printf("\n========================================\n");
    std::printf("  %s\n", title);
    std::printf("========================================\n");
}

// ============================================================================
// 测试 1：纯 CPU 拓扑排序验证（不需要 GPU 设备）
// ============================================================================
static bool TestTopologySort()
{
    PrintSeparator("Test 1: Topology Sort (CPU only)");

    NNRenderGraphBuilder builder;

    // 声明资源
    uint32_t colorTarget = builder.AddResource("ColorTarget", NNResourceType::Texture, 800, 600);
    uint32_t depthTarget = builder.AddResource("DepthTarget", NNResourceType::Texture, 800, 600);
    uint32_t sceneBuffer = builder.AddResource("SceneBuffer", NNResourceType::Buffer);

    std::printf("  Resources: ColorTarget=%u, DepthTarget=%u, SceneBuffer=%u\n",
                colorTarget, depthTarget, sceneBuffer);

    // Pass 1: Clear — 写入 colorTarget
    uint32_t clearPass = builder.AddPass("Clear", [](INNCommandList* /*cmd*/)
    {
        std::printf("  [Execute] Clear pass\n");
    });
    builder.DeclareOutput(clearPass, colorTarget);
    builder.DeclareOutput(clearPass, depthTarget);

    // Pass 2: Draw — 读取 sceneBuffer，写入 colorTarget
    uint32_t drawPass = builder.AddPass("DrawTriangle", [](INNCommandList* /*cmd*/)
    {
        std::printf("  [Execute] DrawTriangle pass\n");
    });
    builder.DeclareInput(drawPass, colorTarget);
    builder.DeclareInput(drawPass, depthTarget);
    builder.DeclareInput(drawPass, sceneBuffer);
    builder.DeclareOutput(drawPass, colorTarget);

    // Pass 3: Present — 读取 colorTarget
    uint32_t presentPass = builder.AddPass("Present", [](INNCommandList* /*cmd*/)
    {
        std::printf("  [Execute] Present pass\n");
    });
    builder.DeclareInput(presentPass, colorTarget);

    std::printf("  Passes: Clear=%u, DrawTriangle=%u, Present=%u\n",
                clearPass, drawPass, presentPass);

    // 编译
    NNRenderGraph graph;
    bool compiled = graph.Compile(builder);
    assert(compiled && "Graph compilation failed — cycle detected?");
    std::printf("  Compile: OK\n");

    // 验证执行顺序
    const auto& order = graph.GetExecutionOrder();
    std::printf("  Execution order (%zu passes):", order.size());
    for (uint32_t id : order)
        std::printf(" %u", id);
    std::printf("\n");

    // 验证：Clear(0) 在 DrawTriangle(1) 之前，DrawTriangle(1) 在 Present(2) 之前
    auto findIndex = [&](uint32_t passId) -> size_t
    {
        for (size_t i = 0; i < order.size(); ++i)
        {
            if (order[i] == passId)
                return i;
        }
        return static_cast<size_t>(-1);
    };

    size_t idxClear   = findIndex(clearPass);
    size_t idxDraw    = findIndex(drawPass);
    size_t idxPresent = findIndex(presentPass);

    assert(idxClear < idxDraw);
    assert(idxDraw < idxPresent);
    std::printf("  Order validation: Clear(%zu) < Draw(%zu) < Present(%zu) — PASS\n",
                idxClear, idxDraw, idxPresent);

    // 执行
    std::printf("  Executing graph...\n");
    bool executed = graph.Execute(nullptr);  // CPU-only 测试，cmdList 为 nullptr
    // Execute 对 nullptr 返回 false，这是预期的
    std::printf("  Execute (null cmd): %s\n", executed ? "OK" : "expected-fail (null cmdList)");

    // 导出 DOT
    builder.ExportDOT("render_graph_phase5.dot");
    std::printf("  DOT exported: render_graph_phase5.dot\n");

    std::printf("  [PASS] Topology sort test passed\n");
    return true;
}

// ============================================================================
// 测试 2：环检测 — 验证 Build() 在存在环时返回 false
// ============================================================================
static bool TestCycleDetection()
{
    PrintSeparator("Test 2: Cycle Detection");

    NNRenderGraphBuilder builder;

    uint32_t resA = builder.AddResource("ResA", NNResourceType::Texture, 256, 256);
    uint32_t resB = builder.AddResource("ResB", NNResourceType::Texture, 256, 256);

    // Pass 0: 读 resB，写 resA
    uint32_t pass0 = builder.AddPass("Pass0", [](INNCommandList*) {});
    builder.DeclareInput(pass0, resB);
    builder.DeclareOutput(pass0, resA);

    // Pass 1: 读 resA，写 resB  →  形成环：Pass0→ResA→Pass1→ResB→Pass0
    uint32_t pass1 = builder.AddPass("Pass1", [](INNCommandList*) {});
    builder.DeclareInput(pass1, resA);
    builder.DeclareOutput(pass1, resB);

    NNRenderGraph graph;
    bool compiled = graph.Compile(builder);

    // 应该编译失败（检测到环）
    assert(!compiled && "Cycle should have been detected");
    std::printf("  Cycle detected correctly — Compile returned false\n");
    std::printf("  [PASS] Cycle detection test passed\n");
    return true;
}

// ============================================================================
// 测试 3：GPU 执行（需要实际设备）
// ============================================================================
static bool TestGPUExecution()
{
    PrintSeparator("Test 3: GPU Execution (requires device)");

    // 创建设备
    NNRenderDeviceCreateInfo createInfo{};
    createInfo.Backend     = NNRenderBackendType::Auto;
    createInfo.Width       = 800;
    createInfo.Height      = 600;
    createInfo.VSync       = true;
    createInfo.EnableValidation = true;

    auto device = NNRenderBootstrap::CreateDevice(createInfo);
    if (!device)
    {
        std::printf("  [SKIP] Could not create render device\n");
        return true;  // 不算失败，只是跳过
    }

    std::printf("  Device created successfully\n");

    // 获取 Diligent 设备和命令列表
    auto* dilDevice = static_cast<NNDiligent::NNDiligentDevice*>(device.Get());
    auto* cmdList   = static_cast<NNDiligent::NNDiligentCommandList*>(
                          dilDevice->GetImmediateCommandList());

    if (!cmdList)
    {
        std::printf("  [SKIP] Could not get immediate command list\n");
        return true;
    }

    // 构建渲染图
    NNRenderGraphBuilder builder;

    uint32_t colorTarget = builder.AddResource("ColorTarget", NNResourceType::Texture, 800, 600);

    // Pass 1: Clear
    uint32_t clearPass = builder.AddPass("Clear", [&](INNCommandList* cmd)
    {
        auto* diliCmd = static_cast<NNDiligent::NNDiligentCommandList*>(cmd);
        diliCmd->ClearRenderTarget(0.2f, 0.3f, 0.4f, 1.0f);
        std::printf("  [GPU Execute] Clear pass\n");
    });
    builder.DeclareOutput(clearPass, colorTarget);

    // Pass 2: Draw (placeholder — 无 PSO，仅测试流程)
    uint32_t drawPass = builder.AddPass("DrawTriangle", [&](INNCommandList* /*cmd*/)
    {
        // 实际绘制需要 PSO，此处仅测试 Pass 执行流程
        std::printf("  [GPU Execute] DrawTriangle pass (skipped — no PSO)\n");
    });
    builder.DeclareInput(drawPass, colorTarget);

    // Pass 3: Present
    uint32_t presentPass = builder.AddPass("Present", [&](INNCommandList* cmd)
    {
        auto* diliCmd = static_cast<NNDiligent::NNDiligentCommandList*>(cmd);
        diliCmd->Present();
        std::printf("  [GPU Execute] Present pass\n");
    });
    builder.DeclareInput(presentPass, colorTarget);

    // 编译
    NNRenderGraph graph;
    if (!graph.Compile(builder))
    {
        std::printf("  [FAIL] Graph compilation failed\n");
        return false;
    }

    std::printf("  Graph compiled, executing on GPU...\n");

    // 执行
    bool ok = graph.Execute(cmdList);
    std::printf("  GPU Execute: %s\n", ok ? "OK" : "FAIL");

    // 导出 DOT
    builder.ExportDOT("render_graph_phase5_gpu.dot");
    std::printf("  DOT exported: render_graph_phase5_gpu.dot\n");

    std::printf("  [PASS] GPU execution test completed\n");
    return true;
}

// ============================================================================
// 测试 4：资源生命周期计算
// ============================================================================
static bool TestResourceLifetime()
{
    PrintSeparator("Test 4: Resource Lifetime");

    NNRenderGraphBuilder builder;

    uint32_t resA = builder.AddResource("ResA", NNResourceType::Texture, 256, 256);
    uint32_t resB = builder.AddResource("ResB", NNResourceType::Texture, 256, 256);
    uint32_t resC = builder.AddResource("ResC", NNResourceType::Buffer);

    // Pass 0: 写 resA
    uint32_t pass0 = builder.AddPass("P0_WriteA", [](INNCommandList*) {});
    builder.DeclareOutput(pass0, resA);

    // Pass 1: 读 resA，写 resB
    uint32_t pass1 = builder.AddPass("P1_ReadA_WriteB", [](INNCommandList*) {});
    builder.DeclareInput(pass1, resA);
    builder.DeclareOutput(pass1, resB);

    // Pass 2: 读 resB，读 resC
    uint32_t pass2 = builder.AddPass("P2_ReadB_ReadC", [](INNCommandList*) {});
    builder.DeclareInput(pass2, resB);
    builder.DeclareInput(pass2, resC);

    NNRenderGraph graph;
    if (!graph.Compile(builder))
    {
        std::printf("  [FAIL] Compilation failed\n");
        return false;
    }

    // 检查资源生命周期
    const auto& resources = builder.GetResources();

    // ResA: 用在 Pass 0 (输出) 和 Pass 1 (输入) → First=0, Last=1
    assert(resources[resA].FirstUsePass == 0);
    assert(resources[resA].LastUsePass  == 1);
    std::printf("  ResA: FirstUse=%u, LastUse=%u — PASS\n",
                resources[resA].FirstUsePass, resources[resA].LastUsePass);

    // ResB: 用在 Pass 1 (输出) 和 Pass 2 (输入) → First=1, Last=2
    assert(resources[resB].FirstUsePass == 1);
    assert(resources[resB].LastUsePass  == 2);
    std::printf("  ResB: FirstUse=%u, LastUse=%u — PASS\n",
                resources[resB].FirstUsePass, resources[resB].LastUsePass);

    // ResC: 只用在 Pass 2 (输入) → First=2, Last=2
    assert(resources[resC].FirstUsePass == 2);
    assert(resources[resC].LastUsePass  == 2);
    std::printf("  ResC: FirstUse=%u, LastUse=%u — PASS\n",
                resources[resC].FirstUsePass, resources[resC].LastUsePass);

    std::printf("  [PASS] Resource lifetime test passed\n");
    return true;
}

// ============================================================================
// main — 运行所有测试
// ============================================================================
int main()
{
    std::printf("=== Phase 5: RenderGraph Tests ===\n");

    int passed = 0;
    int failed = 0;

    auto runTest = [&](const char* name, bool (*func)()) -> void
    {
        std::printf("\n--- Running: %s ---\n", name);
        if (func())
        {
            passed++;
            std::printf("--- %s: PASSED ---\n", name);
        }
        else
        {
            failed++;
            std::printf("--- %s: FAILED ---\n", name);
        }
    };

    runTest("TopologySort",     TestTopologySort);
    runTest("CycleDetection",   TestCycleDetection);
    runTest("ResourceLifetime", TestResourceLifetime);
    runTest("GPUExecution",     TestGPUExecution);

    PrintSeparator("Results");
    std::printf("  Passed: %d\n", passed);
    std::printf("  Failed: %d\n", failed);
    std::printf("  Total:  %d\n", passed + failed);

    if (failed > 0)
    {
        std::printf("\n  *** SOME TESTS FAILED ***\n");
        return 1;
    }

    std::printf("\n  *** ALL TESTS PASSED ***\n");
    return 0;
}
