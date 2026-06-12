# Phase 5 Task: RenderGraph Experimental Implementation

## Objective
Implement a simple RenderGraph system that manages resource creation, pass ordering, and automatic barrier insertion. Verify with a multi-pass rendering test (clear + draw + present).

## Working Directory
`E:\Neverness\Engine\Source\Experiments`

## Critical Constraints
- NEVER use `using namespace Diligent;` - always `::Diligent::` fully qualified
- `using namespace NN::Runtime::Core;` and `using namespace NN::Runtime::Render;` OK in .cpp
- C++20, MSVC, Windows, ASCII only in source files
- Follow existing code style in NNDiligentDevice.cpp
- This is a PURE C++ layer - no Diligent dependency in the RenderGraph module itself
- All Diligent-specific code goes in NNRuntimeDiligent

## Architecture
```
NNRuntimeRender (interfaces)
├── RenderGraph/          ← NEW: Pure C++ render graph
│   ├── NNResourceNode.h
│   ├── NNPassNode.h
│   ├── NNRenderGraphBuilder.h
│   ├── NNRenderGraph.h
│   └── NNRenderGraphCompiler.h

NNRuntimeDiligent (implementation)
├── RenderGraph/          ← NEW: Diligent-specific graph execution
│   ├── NNDiligentRenderGraphExecutor.h
│   └── Source/RenderGraph/NNDiligentRenderGraphExecutor.cpp
```

## Key Design Principles
1. **Backend-agnostic**: RenderGraph module has NO Diligent dependency
2. **Builder pattern**: User builds graph via fluent API
3. **Automatic barriers**: Compiler inserts resource transitions
4. **Resource lifetime**: Resources are created/destroyed automatically
5. **Simple**: No complex aliasing or memory allocation - just pass ordering + barriers

## Existing Interfaces (DO NOT modify)
- `NNRuntimeRender/Device/INNRenderDevice.h` — Create* methods
- `NNRuntimeRender/Command/INNCommandList.h` — Draw/Dispatch/SetPipelineState etc.
- `NNRuntimeRender/Resources/INNTexture.h` — Texture interface
- `NNRuntimeRender/Resources/INNBuffer.h` — Buffer interface
- `NNRuntimeRender/Pipeline/INNPipelineState.h` — PSO interface
- `NNRuntimeRender/RenderTarget/INNRenderTarget.h` — Render target interface

## Tasks

### Task 1: NNResourceNode
Create `NNRuntimeRender/RenderGraph/NNResourceNode.h`:

```cpp
#pragma once
#include <cstdint>
#include <string>

namespace NN::Runtime::Render
{
    enum class NNResourceType : uint8_t { Texture, Buffer };
    enum class NNResourceAccess : uint8_t { Read, Write, ReadWrite };

    struct NNResourceNode
    {
        uint32_t Id = 0;
        NNResourceType Type = NNResourceType::Texture;
        NNResourceAccess Access = NNResourceAccess::Read;
        const char* Name = nullptr;  // Debug name
        
        // For textures
        uint32_t Width = 0;
        uint32_t Height = 0;
        uint32_t Format = 0;  // NNPixelFormat enum value
        
        // For buffers
        uint32_t Size = 0;
        
        // Lifetime
        uint32_t FirstUsePass = UINT32_MAX;
        uint32_t LastUsePass = 0;
    };
}
```

### Task 2: NNPassNode
Create `NNRuntimeRender/RenderGraph/NNPassNode.h`:

```cpp
#pragma once
#include "NNResourceNode.h"
#include <cstdint>
#include <vector>
#include <functional>

namespace NN::Runtime::Render
{
    class INNCommandList;
    class INNRenderTarget;

    struct NNPassNode
    {
        uint32_t Id = 0;
        const char* Name = nullptr;  // Debug name
        
        // Resource dependencies
        std::vector<uint32_t> InputResources;   // Resource IDs read by this pass
        std::vector<uint32_t> OutputResources;  // Resource IDs written by this pass
        
        // Render target (optional - for graphics passes)
        uint32_t RenderTargetId = 0;  // Resource ID of the render target
        
        // Execution callback
        std::function<void(INNCommandList*)> Execute;
        
        // Topological sort data
        uint32_t InDegree = 0;
        bool Visited = false;
    };
}
```

### Task 3: NNRenderGraphBuilder
Create `NNRuntimeRender/RenderGraph/NNRenderGraphBuilder.h`:

```cpp
#pragma once
#include "NNResourceNode.h"
#include "NNPassNode.h"
#include <vector>
#include <functional>

namespace NN::Runtime::Render
{
    class NNRenderGraphBuilder
    {
    public:
        NNRenderGraphBuilder() = default;
        
        // Add a resource to the graph
        uint32_t AddResource(const char* name, NNResourceType type, 
                            uint32_t width = 0, uint32_t height = 0, uint32_t format = 0);
        
        // Add a pass to the graph
        uint32_t AddPass(const char* name, 
                        std::function<void(INNCommandList*)> execute);
        
        // Declare pass dependencies
        void DeclareInput(uint32_t passId, uint32_t resourceId);
        void DeclareOutput(uint32_t passId, uint32_t resourceId);
        
        // Set render target for a pass
        void SetRenderTarget(uint32_t passId, uint32_t resourceId);
        
        // Build the graph (must be called before execution)
        bool Build();
        
        // Access built graph
        const std::vector<NNResourceNode>& GetResources() const { return m_Resources; }
        const std::vector<NNPassNode>& GetPasses() const { return m_Passes; }
        const std::vector<uint32_t>& GetExecutionOrder() const { return m_ExecutionOrder; }
        
        // Debug
        void ExportDOT(const char* filename) const;
        
    private:
        std::vector<NNResourceNode> m_Resources;
        std::vector<NNPassNode> m_Passes;
        std::vector<uint32_t> m_ExecutionOrder;  // Topologically sorted pass IDs
        
        bool TopologicalSort();
        void ComputeResourceLifetime();
    };
}
```

### Task 4: NNRenderGraphBuilder Implementation
Create `NNRuntimeRender/Source/RenderGraph/NNRenderGraphBuilder.cpp`:

Key algorithms:
1. **TopologicalSort**: Kahn's algorithm - process passes with in-degree 0 first
2. **ComputeResourceLifetime**: For each resource, find first and last pass that uses it
3. **ExportDOT**: Generate DOT format for Graphviz visualization

```cpp
bool NNRenderGraphBuilder::TopologicalSort()
{
    // Calculate in-degrees
    for (auto& pass : m_Passes)
    {
        pass.InDegree = 0;
        for (uint32_t inputRes : pass.InputResources)
        {
            // Find passes that write to this resource
            for (auto& other : m_Passes)
            {
                if (other.Id != pass.Id)
                {
                    for (uint32_t outputRes : other.OutputResources)
                    {
                        if (outputRes == inputRes)
                            pass.InDegree++;
                    }
                }
            }
        }
    }
    
    // Kahn's algorithm
    std::queue<uint32_t> queue;
    for (auto& pass : m_Passes)
    {
        if (pass.InDegree == 0)
            queue.push(pass.Id);
    }
    
    m_ExecutionOrder.clear();
    while (!queue.empty())
    {
        uint32_t id = queue.front();
        queue.pop();
        m_ExecutionOrder.push_back(id);
        
        // Find passes that depend on this pass's outputs
        for (auto& pass : m_Passes)
        {
            for (uint32_t inputRes : pass.InputResources)
            {
                for (uint32_t outputRes : m_Passes[id].OutputResources)
                {
                    if (outputRes == inputRes)
                    {
                        pass.InDegree--;
                        if (pass.InDegree == 0)
                            queue.push(pass.Id);
                    }
                }
            }
        }
    }
    
    return m_ExecutionOrder.size() == m_Passes.size();
}
```

### Task 5: NNRenderGraph
Create `NNRuntimeRender/RenderGraph/NNRenderGraph.h`:

```cpp
#pragma once
#include "NNRenderGraphBuilder.h"

namespace NN::Runtime::Render
{
    class INNCommandList;
    class INNRenderDevice;

    class NNRenderGraph
    {
    public:
        NNRenderGraph() = default;
        ~NNRenderGraph() = default;
        
        // Build the graph
        bool Compile(NNRenderGraphBuilder& builder);
        
        // Execute the graph
        bool Execute(INNCommandList* cmdList);
        
        // Get execution order for debugging
        const std::vector<uint32_t>& GetExecutionOrder() const { return m_ExecutionOrder; }
        
    private:
        std::vector<NNResourceNode> m_Resources;
        std::vector<NNPassNode> m_Passes;
        std::vector<uint32_t> m_ExecutionOrder;
    };
}
```

### Task 6: NNRenderGraph Implementation
Create `NNRuntimeRender/Source/RenderGraph/NNRenderGraph.cpp`:

```cpp
bool NNRenderGraph::Compile(NNRenderGraphBuilder& builder)
{
    if (!builder.Build())
        return false;
    
    m_Resources = builder.GetResources();
    m_Passes = builder.GetPasses();
    m_ExecutionOrder = builder.GetExecutionOrder();
    return true;
}

bool NNRenderGraph::Execute(INNCommandList* cmdList)
{
    for (uint32_t passId : m_ExecutionOrder)
    {
        auto& pass = m_Passes[passId];
        if (pass.Execute)
            pass.Execute(cmdList);
    }
    return true;
}
```

### Task 7: Update NNRuntimeRender CMakeLists.txt
Add new source files to `NNRuntimeRender/CMakeLists.txt`:

```cmake
add_library(NNRuntimeRender STATIC
    Source/RenderGraph/NNRenderGraphBuilder.cpp
    Source/RenderGraph/NNRenderGraph.cpp
)
```

### Task 8: Test — TestRenderGraph.cpp
Create `NNRuntimeDiligent/Tests/TestRenderGraph.cpp`:

Test that:
1. Create device via NNRenderBootstrap
2. Create render graph with 3 passes:
   - Pass 1: Clear render target (color texture)
   - Pass 2: Draw triangle
   - Pass 3: Present
3. Compile the graph
4. Execute the graph
5. Verify execution order is correct
6. Export DOT file for visualization

```cpp
#include <NNRuntimeRender/RenderGraph/NNRenderGraphBuilder.h>
#include <NNRuntimeRender/RenderGraph/NNRenderGraph.h>
#include <NNRuntimeRenderBootstrap/Include/NNRenderBootstrap.h>
#include <NNRuntimeDiligent/Device/NNDiligentDevice.h>
#include <NNRuntimeDiligent/Command/NNDiligentCommandList.h>

int main()
{
    // Create device
    auto device = NNRenderBootstrap::CreateDevice(createInfo);
    auto* dilDevice = static_cast<NNDiligent::NNDiligentDevice*>(device.Get());
    
    // Build render graph
    NNRenderGraphBuilder builder;
    
    // Resources
    uint32_t colorTarget = builder.AddResource("ColorTarget", NNResourceType::Texture, 800, 600);
    
    // Passes
    uint32_t clearPass = builder.AddPass("Clear", [&](INNCommandList* cmd) {
        auto* diliCmd = static_cast<NNDiligent::NNDiligentCommandList*>(cmd);
        diliCmd->ClearRenderTarget(0.2f, 0.3f, 0.4f, 1.0f);
    });
    builder.DeclareOutput(clearPass, colorTarget);
    
    uint32_t drawPass = builder.AddPass("DrawTriangle", [&](INNCommandList* cmd) {
        auto* diliCmd = static_cast<NNDiligent::NNDiligentCommandList*>(cmd);
        diliCmd->SetPipelineState(pso.Get());
        NNDrawAttribs da{};
        da.VertexCount = 3;
        diliCmd->Draw(da);
    });
    builder.DeclareInput(drawPass, colorTarget);
    
    uint32_t presentPass = builder.AddPass("Present", [&](INNCommandList* cmd) {
        auto* diliCmd = static_cast<NNDiligent::NNDiligentCommandList*>(cmd);
        diliCmd->Present();
    });
    builder.DeclareInput(presentPass, colorTarget);
    
    // Compile and execute
    NNRenderGraph graph;
    graph.Compile(builder);
    graph.Execute(dilDevice->GetImmediateCommandList());
    
    // Export DOT
    builder.ExportDOT("render_graph.dot");
    
    return 0;
}
```

Add to top-level CMakeLists.txt:
```cmake
add_executable(NNPhase5Test NNRuntimeDiligent/Tests/TestRenderGraph.cpp)
target_include_directories(NNPhase5Test PRIVATE
    ${DILIGENT_ENGINE_DIR}/DiligentCore
    ${CMAKE_CURRENT_SOURCE_DIR}/NNRuntimeRenderBootstrap/Include
)
target_link_libraries(NNPhase5Test PRIVATE
    NNRuntimeRender
    NNRuntimeDiligent
    NNRuntimeRenderBootstrap
    NNRuntimeCore
    SDL3::SDL3 NevernessCore-Core
)
target_compile_definitions(NNPhase5Test PRIVATE PLATFORM_WIN32=1)
if(VULKAN_SUPPORTED)
    target_compile_definitions(NNPhase5Test PRIVATE VULKAN_SUPPORTED=1)
endif()
```

## Build Command
```powershell
cd E:\Neverness\Build\Experiments-Test
cmake E:\Neverness\Engine\Source\Experiments -G "Visual Studio 18 2026" -A x64 -DCMAKE_TOOLCHAIN_FILE=E:/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build . --target NNRuntimeRender --config Debug
cmake --build . --target NNPhase5Test --config Debug
```

## Run Command
```powershell
& "E:\Neverness\Build\Experiments-Test\Debug\NNPhase5Test.exe"
```

## Verification
- NNRuntimeRender compiles with new RenderGraph source files
- NNPhase5Test links and runs
- Test builds a 3-pass graph (Clear → Draw → Present)
- Graph compiles successfully (topological sort works)
- Graph executes passes in correct order
- DOT file is generated for visualization
- No crashes
