// TestSceneRender.cpp — Render a 3D cube and save screenshot as BMP
// No RenderPass, no custom CB — simplest possible approach

#include <iostream>
#include <SDL3/SDL.h>
#include <NNRuntimeCore/NNObject.h>
#include <NNRuntimeDiligent/Device/NNDiligentDevice.h>
#include "Graphics/GraphicsEngine/interface/PipelineState.h"
#include "Graphics/GraphicsEngine/interface/Shader.h"
#include "Graphics/GraphicsEngine/interface/GraphicsTypes.h"
#include "Graphics/GraphicsEngine/interface/DeviceContext.h"
#include "Graphics/GraphicsEngine/interface/SwapChain.h"
#include "Graphics/GraphicsEngine/interface/Buffer.h"
#include <cmath>
#include <fstream>
#include <vector>

namespace nnr = NN::Runtime::Render;

struct Vertex { float px,py,pz, r,g,b, nx,ny,nz; };

static const Vertex gCubeVerts[] = {
    {-0.5f,-0.5f, 0.5f, 1,0,0, 0,0,1}, { 0.5f,-0.5f, 0.5f, 1,0,0, 0,0,1},
    { 0.5f, 0.5f, 0.5f, 1,0,0, 0,0,1}, {-0.5f, 0.5f, 0.5f, 1,0,0, 0,0,1},
    { 0.5f,-0.5f,-0.5f, 0,1,0, 0,0,-1}, {-0.5f,-0.5f,-0.5f, 0,1,0, 0,0,-1},
    {-0.5f, 0.5f,-0.5f, 0,1,0, 0,0,-1}, { 0.5f, 0.5f,-0.5f, 0,1,0, 0,0,-1},
    {-0.5f, 0.5f, 0.5f, 0,0,1, 0,1,0}, { 0.5f, 0.5f, 0.5f, 0,0,1, 0,1,0},
    { 0.5f, 0.5f,-0.5f, 0,0,1, 0,1,0}, {-0.5f, 0.5f,-0.5f, 0,0,1, 0,1,0},
    {-0.5f,-0.5f,-0.5f, 1,1,0, 0,-1,0}, { 0.5f,-0.5f,-0.5f, 1,1,0, 0,-1,0},
    { 0.5f,-0.5f, 0.5f, 1,1,0, 0,-1,0}, {-0.5f,-0.5f, 0.5f, 1,1,0, 0,-1,0},
    { 0.5f,-0.5f, 0.5f, 1,0,1, 1,0,0}, { 0.5f,-0.5f,-0.5f, 1,0,1, 1,0,0},
    { 0.5f, 0.5f,-0.5f, 1,0,1, 1,0,0}, { 0.5f, 0.5f, 0.5f, 1,0,1, 1,0,0},
    {-0.5f,-0.5f,-0.5f, 0,1,1, -1,0,0}, {-0.5f,-0.5f, 0.5f, 0,1,1, -1,0,0},
    {-0.5f, 0.5f, 0.5f, 0,1,1, -1,0,0}, {-0.5f, 0.5f,-0.5f, 0,1,1, -1,0,0},
};

static const uint32_t gCubeIdx[] = {
    0,1,2, 0,2,3, 4,5,6, 4,6,7, 8,9,10, 8,10,11,
    12,13,14, 12,14,15, 16,17,18, 16,18,19, 20,21,22, 20,22,23,
};

static const char* gCubeVS = R"(
struct VSInput {
    float3 Pos   : ATTRIB0;
    float3 Color : ATTRIB1;
    float3 Normal: ATTRIB2;
};

struct PSInput {
    float4 Pos    : SV_POSITION;
    float3 Color  : COLOR;
    float3 Normal : NORMAL;
};

void main(in VSInput VSIn, out PSInput PSIn) {
    // Simple Y rotation ~30 deg
    float a = 0.5236f;
    float c = cos(a), s = sin(a);
    float3 wp = float3(c*VSIn.Pos.x + s*VSIn.Pos.z, VSIn.Pos.y, -s*VSIn.Pos.x + c*VSIn.Pos.z);
    float3 wn = float3(c*VSIn.Normal.x + s*VSIn.Normal.z, VSIn.Normal.y, -s*VSIn.Normal.x + c*VSIn.Normal.z);

    // Hardcoded VP: perspective from (2.5,2,3) -> origin, 60deg FOV, 16:9
    // Computed externally, baked in
    float4x4 vp = float4x4(
         0.7706f, -0.4447f,  0.4548f,  0.0f,
         0.0000f,  0.7163f,  0.6978f,  0.0f,
        -0.6374f, -0.5386f,  0.5515f,  5.3852f,
         0.0000f,  0.0000f,  0.0000f,  1.0f
    );

    PSIn.Pos = mul(float4(wp, 1.0), vp);
    PSIn.Color = VSIn.Color;
    PSIn.Normal = wn;
}
)";

static const char* gCubePS = R"(
struct PSInput {
    float4 Pos    : SV_POSITION;
    float3 Color  : COLOR;
    float3 Normal : NORMAL;
};

struct PSOutput {
    float4 Color : SV_TARGET;
};

void main(in PSInput PSIn, out PSOutput PSOut) {
    float3 N = normalize(PSIn.Normal);
    float3 L = normalize(float3(0.3, 0.8, -0.5));
    float NdotL = max(dot(N, L), 0.0);
    float3 ambient = PSIn.Color * 0.2;
    float3 diffuse = PSIn.Color * NdotL * 0.8;
    PSOut.Color = float4(ambient + diffuse, 1.0);
}
)";

static bool SaveBMP(const char* path, const uint32_t* px, uint32_t w, uint32_t h) {
    std::ofstream f(path, std::ios::binary); if(!f) return false;
    uint32_t rs=w*3, pad=(4-(rs%4))%4, is=(rs+pad)*h, fs=54+is;
    uint8_t hdr[54]={}; hdr[0]='B';hdr[1]='M';
    *(uint32_t*)(hdr+2)=fs; *(uint32_t*)(hdr+10)=54; *(uint32_t*)(hdr+14)=40;
    *(int32_t*)(hdr+18)=w; *(int32_t*)(hdr+22)=h; *(uint16_t*)(hdr+26)=1; *(uint16_t*)(hdr+28)=24;
    *(uint32_t*)(hdr+34)=is;
    f.write((char*)hdr,54);
    std::vector<uint8_t> row(rs+pad,0);
    for(int32_t y=h-1;y>=0;y--) {
        for(uint32_t x=0;x<w;x++) { uint32_t p=px[y*w+x]; row[x*3]=p&0xFF; row[x*3+1]=(p>>8)&0xFF; row[x*3+2]=(p>>16)&0xFF; }
        f.write((char*)row.data(),rs+pad);
    }
    return true;
}

int main(int argc, char* argv[])
{
    const char* outPath = "screenshot.bmp";
    int maxFrames = 30;
    for (int i=1;i<argc;i++) {
        if(strcmp(argv[i],"--out")==0&&i+1<argc) outPath=argv[++i];
        if(strcmp(argv[i],"--frames")==0&&i+1<argc) maxFrames=std::atoi(argv[++i]);
    }

    if(!SDL_Init(SDL_INIT_VIDEO)) { std::cerr<<"SDL: "<<SDL_GetError()<<std::endl; return 1; }
    SDL_Window* wnd = SDL_CreateWindow("NNRuntime Scene",1280,720,SDL_WINDOW_RESIZABLE);
    if(!wnd) { SDL_Quit(); return 1; }

    nnr::NNRenderDeviceCreateInfo ci{};
    ci.Window=wnd; ci.Width=1280; ci.Height=720; ci.EnableValidation=false; ci.Backend=nnr::NNRenderBackendType::Auto;

    auto* dd = new NNDiligent::NNDiligentDevice();
    dd->AddRef();
    if(!dd->Initialize(ci)) { dd->Release(); SDL_DestroyWindow(wnd); SDL_Quit(); return 1; }

    auto* dev = dd->GetDiligentDevice();
    auto* ctx = dd->GetDiligentContext();
    auto* sc  = dd->GetDiligentSwapChain();
    std::cout<<"Backend: "<<dd->GetDeviceInfo().DeviceName<<std::endl;

    // VB
    ::Diligent::BufferDesc vbd; vbd.Name="VB"; vbd.Size=sizeof(gCubeVerts); vbd.BindFlags=::Diligent::BIND_VERTEX_BUFFER; vbd.Usage=::Diligent::USAGE_IMMUTABLE;
    ::Diligent::BufferData vbd2; vbd2.pData=gCubeVerts; vbd2.DataSize=sizeof(gCubeVerts);
    ::Diligent::RefCntAutoPtr<::Diligent::IBuffer> VB;
    dev->CreateBuffer(vbd,&vbd2,&VB);

    // IB
    ::Diligent::BufferDesc ibd; ibd.Name="IB"; ibd.Size=sizeof(gCubeIdx); ibd.BindFlags=::Diligent::BIND_INDEX_BUFFER; ibd.Usage=::Diligent::USAGE_IMMUTABLE;
    ::Diligent::BufferData ibd2; ibd2.pData=gCubeIdx; ibd2.DataSize=sizeof(gCubeIdx);
    ::Diligent::RefCntAutoPtr<::Diligent::IBuffer> IB;
    dev->CreateBuffer(ibd,&ibd2,&IB);

    // Shaders
    ::Diligent::ShaderCreateInfo sci;
    sci.SourceLanguage=::Diligent::SHADER_SOURCE_LANGUAGE_HLSL;
    sci.Desc.UseCombinedTextureSamplers=true;

    ::Diligent::RefCntAutoPtr<::Diligent::IShader> VS,PS;
    sci.Desc.ShaderType=::Diligent::SHADER_TYPE_VERTEX; sci.EntryPoint="main"; sci.Desc.Name="VS"; sci.Source=gCubeVS;
    dev->CreateShader(sci,&VS);
    sci.Desc.ShaderType=::Diligent::SHADER_TYPE_PIXEL; sci.Desc.Name="PS"; sci.Source=gCubePS;
    dev->CreateShader(sci,&PS);

    // PSO — no RenderPass, use swap chain format directly
    ::Diligent::GraphicsPipelineStateCreateInfo psoCI;
    psoCI.PSODesc.Name="CubePSO"; psoCI.PSODesc.PipelineType=::Diligent::PIPELINE_TYPE_GRAPHICS;
    psoCI.GraphicsPipeline.NumRenderTargets=1;
    psoCI.GraphicsPipeline.RTVFormats[0]=sc->GetDesc().ColorBufferFormat;
    psoCI.GraphicsPipeline.DSVFormat=sc->GetDesc().DepthBufferFormat;
    psoCI.GraphicsPipeline.PrimitiveTopology=::Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    ::Diligent::LayoutElement elems[]={
        ::Diligent::LayoutElement{0,0,3,::Diligent::VT_FLOAT32,false,0},
        ::Diligent::LayoutElement{1,0,3,::Diligent::VT_FLOAT32,false,12},
        ::Diligent::LayoutElement{2,0,3,::Diligent::VT_FLOAT32,false,24},
    };
    psoCI.GraphicsPipeline.InputLayout.LayoutElements=elems;
    psoCI.GraphicsPipeline.InputLayout.NumElements=3;
    psoCI.GraphicsPipeline.RasterizerDesc.CullMode=::Diligent::CULL_MODE_BACK;
    psoCI.GraphicsPipeline.DepthStencilDesc.DepthEnable=true;
    psoCI.pVS=VS; psoCI.pPS=PS;

    ::Diligent::RefCntAutoPtr<::Diligent::IPipelineState> PSO;
    dev->CreateGraphicsPipelineState(psoCI,&PSO);
    if(!PSO) { std::cerr<<"PSO FAILED"<<std::endl; dd->Release(); SDL_DestroyWindow(wnd); SDL_Quit(); return 1; }
    std::cout<<"PSO OK"<<std::endl;

    std::cout<<"Rendering "<<maxFrames<<" frames..."<<std::endl;

    uint32_t* screenshot=nullptr; uint32_t scrW=0, scrH=0;

    for(int frame=0;frame<maxFrames;frame++)
    {
        SDL_Event evt;
        while(SDL_PollEvent(&evt)) {
            if(evt.type==SDL_EVENT_QUIT) goto done;
            if(evt.type==SDL_EVENT_KEY_DOWN&&evt.key.key==SDLK_ESCAPE) goto done;
            if(evt.type==SDL_EVENT_WINDOW_RESIZED) { int w=evt.window.data1,h=evt.window.data2; if(w>0&&h>0) sc->Resize(w,h); }
        }

        // Clear
        auto* pRTV = sc->GetCurrentBackBufferRTV();
        auto* pDSV = sc->GetDepthBufferDSV();
        ctx->SetRenderTargets(1, &pRTV, pDSV, ::Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        float clr[4] = {0.15f, 0.15f, 0.25f, 1.0f};
        ctx->ClearRenderTarget(pRTV, clr, ::Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        ctx->ClearDepthStencil(pDSV, ::Diligent::CLEAR_DEPTH_FLAG, 1.0f, 0, ::Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        // Set viewport
        ::Diligent::Viewport vp;
        vp.TopLeftX=0; vp.TopLeftY=0;
        vp.Width=static_cast<float>(sc->GetDesc().Width);
        vp.Height=static_cast<float>(sc->GetDesc().Height);
        vp.MinDepth=0; vp.MaxDepth=1;
        ctx->SetViewports(1,&vp,sc->GetDesc().Width,sc->GetDesc().Height);

        ctx->SetPipelineState(PSO);

        ::Diligent::IBuffer* vbs[]={VB}; uint64_t offs[]={0};
        ctx->SetVertexBuffers(0,1,vbs,offs,::Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION,::Diligent::SET_VERTEX_BUFFERS_FLAG_RESET);
        ctx->SetIndexBuffer(IB,0,::Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        ::Diligent::DrawIndexedAttribs da; da.NumIndices=36; da.IndexType=::Diligent::VT_UINT32; da.Flags=::Diligent::DRAW_FLAG_VERIFY_ALL;
        ctx->DrawIndexed(da);

        // Screenshot on last frame
        if(frame==maxFrames-1) {
            scrW=sc->GetDesc().Width; scrH=sc->GetDesc().Height;
            ::Diligent::TextureDesc stgDesc;
            stgDesc.Name="Staging"; stgDesc.Type=::Diligent::RESOURCE_DIM_TEX_2D;
            stgDesc.Width=scrW; stgDesc.Height=scrH;
            stgDesc.Format=sc->GetDesc().ColorBufferFormat;
            stgDesc.Usage=::Diligent::USAGE_STAGING; stgDesc.CPUAccessFlags=::Diligent::CPU_ACCESS_READ;
            ::Diligent::RefCntAutoPtr<::Diligent::ITexture> staging;
            dev->CreateTexture(stgDesc,nullptr,&staging);

            ::Diligent::ITexture* pBB = pRTV->GetTexture();
            ::Diligent::CopyTextureAttribs cpy;
            cpy.pSrcTexture=pBB; cpy.SrcTextureTransitionMode=::Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION;
            cpy.pDstTexture=staging; cpy.DstTextureTransitionMode=::Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION;
            ctx->CopyTexture(cpy);

            // Wait for GPU
            ctx->WaitForIdle();

            ::Diligent::MappedTextureSubresource mapped;
            ctx->MapTextureSubresource(staging,0,0,::Diligent::MAP_READ,::Diligent::MAP_FLAG_NONE,nullptr,mapped);
            if(mapped.pData) {
                screenshot=new uint32_t[scrW*scrH];
                const uint8_t* src=static_cast<const uint8_t*>(mapped.pData);
                for(uint32_t y=0;y<scrH;y++) std::memcpy(&screenshot[y*scrW], src+y*mapped.Stride, scrW*4);
                std::cout<<"Screenshot captured"<<std::endl;
            }
            ctx->UnmapTextureSubresource(staging,0,0);
        }

        sc->Present();
        if(frame%10==0) std::cout<<"Frame "<<frame<<" OK"<<std::endl;
    }

done:
    if(screenshot) {
        if(SaveBMP(outPath,screenshot,scrW,scrH))
            std::cout<<"Saved: "<<outPath<<" ("<<scrW<<"x"<<scrH<<")"<<std::endl;
        else std::cerr<<"Save failed!"<<std::endl;
        delete[] screenshot;
    }

    PSO=nullptr; VB=nullptr; IB=nullptr; VS=nullptr; PS=nullptr;
    dd->Release(); SDL_DestroyWindow(wnd); SDL_Quit();
    std::cout<<"=== Done ==="<<std::endl;
    return 0;
}
