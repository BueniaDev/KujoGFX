#ifndef KUJOGFX_H
#define KUJOGFX_H

#if defined(_WIN32) || defined(_WIN64)
#define KUJOGFX_PLATFORM_WINDOWS
#elif defined(__linux__) || defined(__linux)
#define KUJOGFX_PLATFORM_LINUX
#elif defined(__APPLE__) || defined(__MACH__)
#define KUJOGFX_PLATFORM_MACOS
#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
#define KUJOGFX_PLATFORM_BSD
#elif defined(__ANDROID__)
#define KUJOGFX_PLATFORM_ANDROID
#elif defined(__EMSCRIPTEN__)
#define KUJOGFX_PLATFORM_EMSCRIPTEN
#else
#error "KujoGFX could not determine the platform of your system."
#endif

#include <iostream>
#include <cstdint>
#include <cassert>
#include <vector>
#include <deque>
#include <memory>
#include <atomic>
#include <algorithm>
#include <map>
#include <unordered_map>
#include <GL/glew.h>
#include <vulkan/vulkan.h>
#if defined(KUJOGFX_PLATFORM_WINDOWS)
#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <GL/wglew.h>
#include <vulkan/vulkan_win32.h>
#endif
#include "glslang/Public/ShaderLang.h"
#include "glslang/SPIRV/GlslangtoSpv.h"
#include "spirv_cross/spirv_glsl.hpp"
#include "spirv_cross/spirv_hlsl.hpp"
using namespace spirv_cross;
using namespace std;

namespace kujogfx
{
    enum KujoGFXBackendType
    {
	BackendAuto = -1,
	BackendNull = 0,
	BackendOpenGL,
	#if defined(KUJOGFX_PLATFORM_WINDOWS)
	BackendDirect3D11,
	#endif
	BackendVulkan,
	BackendCount
    };

    struct KujoGFXPlatformData
    {
	void *window_handle = NULL;
	void *context_handle = NULL;
    };

    struct KujoGFXColor
    {
	float red = 0.f;
	float green = 0.f;
	float blue = 0.f;
	float alpha = 0.f;

	KujoGFXColor()
	{
	}

	KujoGFXColor(float r, float g, float b, float a = 1.0f) : red(r), green(g), blue(b), alpha(a)
	{
	    red = clamp<float>(red, 0.0, 1.0);
	    green = clamp<float>(green, 0.0, 1.0);
	    blue = clamp<float>(blue, 0.0, 1.0);
	    alpha = clamp<float>(alpha, 0.0, 1.0);
	}

	operator float* ()
	{
	    return reinterpret_cast<float*>(&red);
	}

	operator const float* () const
	{
	    return reinterpret_cast<const float*>(&red);
	}
    };

    enum KujoGFXLoadOp : int
    {
	LoadOpDontCare = 0,
	LoadOpClear,
	LoadOpLoad
    };

    enum KujoGFXStoreOp : int
    {
	StoreOpDontCare = 0,
	StoreOpStore
    };

    enum KujoGFXPrimitiveType : int
    {
	PrimitiveTriangles = 0
    };

    struct KujoGFXColorAttachment
    {
	KujoGFXLoadOp load_op = LoadOpClear;
	KujoGFXStoreOp store_op = StoreOpStore;
	KujoGFXColor color;

	KujoGFXColorAttachment()
	{

	}

	KujoGFXColorAttachment(KujoGFXColor col) : color(col)
	{

	}
    };

    struct KujoGFXPassAction
    {
	KujoGFXColorAttachment color_attach;

	KujoGFXPassAction()
	{
	}

	KujoGFXPassAction(KujoGFXColor col) : color_attach(col)
	{
	}

	KujoGFXPassAction(KujoGFXColorAttachment attach) : color_attach(attach)
	{
	}
    };

    struct KujoGFXPass
    {
	KujoGFXPassAction action;

	KujoGFXPass()
	{

	}
    };

    struct KujoGFXShader
    {
	enum GLSLShaderLang : int
	{
	    GLSL140,
	    GLSL300ES,
	    GLSL100ES
	};

	string vertex_source = "";
	string fragment_source = "";

	vector<uint32_t> vert_spirv;
	vector<uint32_t> frag_spirv;

	string vertex_out_src = "";
	string fragment_out_src = "";

	bool is_compiled = false;

	KujoGFXShader() : is_compiled(false)
	{
	}

	KujoGFXShader(string vert_source, string frag_source)
	{
	    setShaderSource(vert_source, frag_source);
	}

	void setShaderSource(string vert_source, string frag_source)
	{
	    if (is_compiled)
	    {
		if ((vertex_source != vert_source) || (fragment_source != frag_source))
		{
		    is_compiled = false;
		}
	    }

	    vertex_source = vert_source;
	    fragment_source = frag_source;
	}

	bool toSPIRV(EShLanguage shader_type, string source, vector<uint32_t> &spv_code, bool is_molten_vk)
	{
	    glslang::InitializeProcess();

	    glslang::TShader shader(shader_type);
	    glslang::TProgram program;

	    const char *shader_str[1];

	    TBuiltInResource resources = {};
	    initResources(resources);

	    EShMessages messages = static_cast<EShMessages>(EShMsgDefault | EShMsgSpvRules);

	    if (is_molten_vk)
	    {
		messages = static_cast<EShMessages>(messages | EShMsgVulkanRules);
	    }

	    shader_str[0] = source.data();
	    shader.setStrings(shader_str, 1);
	    shader.setEnvInput(glslang::EShSourceGlsl, shader_type, glslang::EShClientOpenGL, 100);
	    shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_0);
	    shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_0);

	    if (!shader.parse(&resources, 100, false, messages))
	    {
		cout << "Could not parse shader!" << endl;
		cout << "Error log: " << endl;
		cout << shader.getInfoLog() << endl;
		cout << shader.getInfoDebugLog() << endl;
		cout.flush();
		return false;
	    }

	    program.addShader(&shader);

	    if (!program.link(messages))
	    {
		cout << "Could not link shader program!" << endl;
		cout << "Error log: " << endl;
		cout << shader.getInfoLog() << endl;
		cout << shader.getInfoDebugLog() << endl;
		cout.flush();
		return false;
	    }

	    if (!program.mapIO())
	    {
		cout << "Could not map shader program I/O!" << endl;
		cout << "Error log: " << endl;
		cout << shader.getInfoLog() << endl;
		cout << shader.getInfoDebugLog() << endl;
		cout.flush();
		return false;
	    }

	    glslang::GlslangToSpv(*program.getIntermediate(shader_type), spv_code);
	    glslang::FinalizeProcess();
	    return true;
	}

	bool toHLSL(vector<uint32_t> spv_code, string &out_hlsl)
	{
	    CompilerHLSL compiler(spv_code);
	    CompilerHLSL::Options hlsl_options;
	    hlsl_options.shader_model = 40;
	    compiler.set_hlsl_options(hlsl_options);

	    out_hlsl = compiler.compile();

	    if (out_hlsl.empty())
	    {
		cout << "Could not compile shader to HLSL!" << endl;
		return false;
	    }

	    return true;
	}

	bool toGLSL(vector<uint32_t> spv_code, GLSLShaderLang shader_lang, string &out_glsl)
	{
	    CompilerGLSL compiler(spv_code);
	    CompilerGLSL::Options glsl_options;

	    switch (shader_lang)
	    {
		case GLSL140:
		{
		    glsl_options.version = 140;
		    glsl_options.es = false;
		}
		break;
		case GLSL300ES:
		{
		    glsl_options.version = 300;
		    glsl_options.es = true;
		}
		break;
		case GLSL100ES:
		{
		    glsl_options.version = 100;
		    glsl_options.es = true;
		}
		break;
		default:
		{
		    cout << "Unrecognized GLSL version type of " << dec << int(shader_lang) << endl;
		    return false;
		}
		break;
	    }

	    glsl_options.vulkan_semantics = false;
	    glsl_options.enable_420pack_extension = false;
	    compiler.set_common_options(glsl_options);

	    out_glsl = compiler.compile();

	    if (out_glsl.empty())
	    {
		cout << "Could not compile shader to GLSL!" << endl;
		return false;
	    }

	    return true;
	}

	bool translateHLSL()
	{
	    if (is_compiled)
	    {
		return true;
	    }

	    if (vertex_source.empty())
	    {
		cout << "Vertex shader is empty" << endl;
		return false;
	    }

	    if (fragment_source.empty())
	    {
		cout << "Fragment shader is empty" << endl;
		return false;
	    }

	    if (!toSPIRV(EShLangVertex, vertex_source, vert_spirv, false))
	    {
		return false;
	    }

	    if (!toSPIRV(EShLangFragment, fragment_source, frag_spirv, false))
	    {
		return false;
	    }

	    if (!toHLSL(vert_spirv, vertex_out_src))
	    {
		return false;
	    }

	    if (!toHLSL(frag_spirv, fragment_out_src))
	    {
		return false;
	    }

	    is_compiled = true;
	    return true;
	}

	bool translateGLSL()
	{
	    #if defined(KUJOGFX_PLATFORM_ANDROID)
	    GLSLShaderLang shader_lang = GLSL300ES;
	    #elif defined(KUJOGFX_PLATFORM_EMSCRIPTEN)
	    GLSLShaderLang shader_lang = GLSL300ES;
	    #else
	    GLSLShaderLang shader_lang = GLSL140;
	    #endif

	    return translateGLSL(shader_lang);
	}

	bool translateGLSL(GLSLShaderLang shader_lang)
	{
	    if (is_compiled)
	    {
		return true;
	    }

	    if (vertex_source.empty())
	    {
		cout << "Vertex shader is empty" << endl;
		return false;
	    }

	    if (fragment_source.empty())
	    {
		cout << "Fragment shader is empty" << endl;
		return false;
	    }

	    if (!toSPIRV(EShLangVertex, vertex_source, vert_spirv, false))
	    {
		return false;
	    }

	    if (!toSPIRV(EShLangFragment, fragment_source, frag_spirv, false))
	    {
		return false;
	    }

	    if (!toGLSL(vert_spirv, shader_lang, vertex_out_src))
	    {
		return false;
	    }

	    if (!toGLSL(frag_spirv, shader_lang, fragment_out_src))
	    {
		return false;
	    }

	    is_compiled = true;
	    return true;
	}

	bool translateSPIRV()
	{
	    #if defined(KUJOGFX_PLATFORM_MACOS)
	    bool is_molten_vk = true;
	    #else
	    bool is_molten_vk = false;
	    #endif

	    return translateSPIRV(is_molten_vk);
	}

	bool translateSPIRV(bool is_molten_vk)
	{
	    if (is_compiled)
	    {
		return true;
	    }

	    if (vertex_source.empty())
	    {
		cout << "Vertex shader is empty" << endl;
		return false;
	    }

	    if (fragment_source.empty())
	    {
		cout << "Fragment shader is empty" << endl;
		return false;
	    }

	    if (!toSPIRV(EShLangVertex, vertex_source, vert_spirv, is_molten_vk))
	    {
		cout << "Could not compile vertex shader!" << endl;
		return false;
	    }

	    if (!toSPIRV(EShLangFragment, fragment_source, frag_spirv, is_molten_vk))
	    {
		cout << "Could not compile fragment shader!" << endl;
		return false;
	    }

	    is_compiled = true;
	    return true;
	}

	void initResources(TBuiltInResource &resources)
	{
	    resources.maxLights = 32;
	    resources.maxClipPlanes = 6;
	    resources.maxTextureUnits = 32;
	    resources.maxTextureCoords = 32;
	    resources.maxVertexAttribs = 64;
	    resources.maxVertexUniformComponents = 4096;
	    resources.maxVaryingFloats = 64;
	    resources.maxVertexTextureImageUnits = 32;
	    resources.maxCombinedTextureImageUnits = 80;
	    resources.maxTextureImageUnits = 32;
	    resources.maxFragmentUniformComponents = 4096;
	    resources.maxDrawBuffers = 32;
	    resources.maxVertexUniformVectors = 128;
	    resources.maxVaryingVectors = 8;
	    resources.maxFragmentUniformVectors = 16;
	    resources.maxVertexOutputVectors = 16;
	    resources.maxFragmentInputVectors = 15;
	    resources.minProgramTexelOffset = -8;
	    resources.maxProgramTexelOffset = 7;
	    resources.maxClipDistances = 8;
	    resources.maxComputeWorkGroupCountX = 65535;
	    resources.maxComputeWorkGroupCountY = 65535;
	    resources.maxComputeWorkGroupCountZ = 65535;
	    resources.maxComputeWorkGroupSizeX = 1024;
	    resources.maxComputeWorkGroupSizeY = 1024;
	    resources.maxComputeWorkGroupSizeZ = 64;
	    resources.maxComputeUniformComponents = 1024;
	    resources.maxComputeTextureImageUnits = 16;
	    resources.maxComputeImageUniforms = 8;
	    resources.maxComputeAtomicCounters = 8;
	    resources.maxComputeAtomicCounterBuffers = 1;
	    resources.maxVaryingComponents = 60;
	    resources.maxVertexOutputComponents = 64;
	    resources.maxGeometryInputComponents = 64;
	    resources.maxGeometryOutputComponents = 128;
	    resources.maxFragmentInputComponents = 128;
	    resources.maxImageUnits = 8;
	    resources.maxCombinedImageUnitsAndFragmentOutputs = 8;
	    resources.maxCombinedShaderOutputResources = 8;
	    resources.maxImageSamples = 0;
	    resources.maxVertexImageUniforms = 0;
	    resources.maxTessControlImageUniforms = 0;
	    resources.maxTessEvaluationImageUniforms = 0;
	    resources.maxGeometryImageUniforms = 0;
	    resources.maxFragmentImageUniforms = 8;
	    resources.maxCombinedImageUniforms = 8;
	    resources.maxGeometryTextureImageUnits = 16;
	    resources.maxGeometryOutputVertices = 256;
	    resources.maxGeometryTotalOutputComponents = 1024;
	    resources.maxGeometryUniformComponents = 1024;
	    resources.maxGeometryVaryingComponents = 64;
	    resources.maxTessControlInputComponents = 128;
	    resources.maxTessControlOutputComponents = 128;
	    resources.maxTessControlTextureImageUnits = 16;
	    resources.maxTessControlUniformComponents = 1024;
	    resources.maxTessControlTotalOutputComponents = 4096;
	    resources.maxTessEvaluationInputComponents = 128;
	    resources.maxTessEvaluationOutputComponents = 128;
	    resources.maxTessEvaluationTextureImageUnits = 16;
	    resources.maxTessEvaluationUniformComponents = 1024;
	    resources.maxTessPatchComponents = 120;
	    resources.maxPatchVertices = 32;
	    resources.maxTessGenLevel = 64;
	    resources.maxViewports = 16;
	    resources.maxVertexAtomicCounters = 0;
	    resources.maxTessControlAtomicCounters = 0;
	    resources.maxTessEvaluationAtomicCounters = 0;
	    resources.maxGeometryAtomicCounters = 0;
	    resources.maxFragmentAtomicCounters = 8;
	    resources.maxCombinedAtomicCounters = 8;
	    resources.maxAtomicCounterBindings = 1;
	    resources.maxVertexAtomicCounterBuffers = 0;
	    resources.maxTessControlAtomicCounterBuffers = 0;
	    resources.maxTessEvaluationAtomicCounterBuffers = 0;
	    resources.maxGeometryAtomicCounterBuffers = 0;
	    resources.maxFragmentAtomicCounterBuffers = 1;
	    resources.maxCombinedAtomicCounterBuffers = 1;
	    resources.maxAtomicCounterBufferSize = 16384;
	    resources.maxTransformFeedbackBuffers = 4;
	    resources.maxTransformFeedbackInterleavedComponents = 64;
	    resources.maxCullDistances = 8;
	    resources.maxCombinedClipAndCullDistances = 8;
	    resources.maxSamples = 4;
	    resources.maxMeshOutputVerticesNV = 256;
	    resources.maxMeshOutputPrimitivesNV = 512;
	    resources.maxMeshWorkGroupSizeX_NV = 32;
	    resources.maxMeshWorkGroupSizeY_NV = 1;
	    resources.maxMeshWorkGroupSizeZ_NV = 1;
	    resources.maxTaskWorkGroupSizeX_NV = 32;
	    resources.maxTaskWorkGroupSizeY_NV = 1;
	    resources.maxTaskWorkGroupSizeZ_NV = 1;
	    resources.maxMeshViewCountNV = 4;
	    resources.limits.nonInductiveForLoops = 1;
	    resources.limits.whileLoops = 1;
	    resources.limits.doWhileLoops = 1;
	    resources.limits.generalUniformIndexing = 1;
	    resources.limits.generalAttributeMatrixVectorIndexing = 1;
	    resources.limits.generalVaryingIndexing = 1;
	    resources.limits.generalSamplerIndexing = 1;
	    resources.limits.generalVariableIndexing = 1;
	    resources.limits.generalConstantMatrixVectorIndexing = 1;
	}
    };

    class KujoGFXPipeline
    {
	public:
	    KujoGFXPipeline() : id(generateID())
	    {
	    }

	    KujoGFXShader shader;
	    KujoGFXPrimitiveType primitive_type;
	    const uint32_t id;

	private:
	    static atomic<uint32_t> next_id;

	    static uint32_t generateID()
	    {
		return next_id++;
	    }
    };

    atomic<uint32_t> KujoGFXPipeline::next_id{1};

    struct KujoGFXDraw
    {
	int base_element = 0;
	int num_elements = 0;
	int num_instances = 0;

	KujoGFXDraw() : base_element(0), num_elements(0), num_instances(0)
	{

	}

	KujoGFXDraw(int base, int num_elem, int num_inst) : base_element(base), num_elements(num_elem), num_instances(num_inst)
	{

	}
    };

    enum KujoGFXCommandType : int
    {
	CommandNop = 0,
	CommandBeginPass,
	CommandEndPass,
	CommandApplyPipeline,
	CommandDraw,
	CommandCommit
    };

    struct KujoGFXCommand
    {
	KujoGFXCommandType cmd_type = CommandNop;
	KujoGFXPass current_pass;
	KujoGFXPipeline current_pipeline;
	KujoGFXDraw current_draw_call;

	KujoGFXCommand() : cmd_type(CommandNop)
	{

	}

	KujoGFXCommand(KujoGFXCommandType cmd_type) : cmd_type(cmd_type)
	{

	}

	KujoGFXCommand(KujoGFXPass pass) : cmd_type(CommandBeginPass), current_pass(pass)
	{

	}

	KujoGFXCommand(KujoGFXPipeline pipeline) : cmd_type(CommandApplyPipeline), current_pipeline(pipeline)
	{

	}

	KujoGFXCommand(KujoGFXDraw draw_call) : cmd_type(CommandDraw), current_draw_call(draw_call)
	{

	}
    };

    class KujoGFXBackend
    {
	public:
	    KujoGFXBackend()
	    {

	    }

	    ~KujoGFXBackend()
	    {

	    }

	    virtual bool initBackend(void*)
	    {
		return true;
	    }

	    virtual void shutdownBackend()
	    {
		return;
	    }

	    virtual void* getContextHandle()
	    {
		return NULL;
	    }

	    virtual void beginPass(KujoGFXPass)
	    {
		return;
	    }

	    virtual void endPass()
	    {
		return;
	    }

	    virtual void setPipeline(KujoGFXPipeline)
	    {
		return;
	    }

	    virtual void createPipeline(KujoGFXPipeline)
	    {
		return;
	    }

	    virtual void applyPipeline()
	    {
		return;
	    }

	    virtual void draw(KujoGFXDraw)
	    {
		return;
	    }

	    virtual void commitFrame()
	    {
		return;
	    }
    };

    class KujoGFX_Null : public KujoGFXBackend
    {
	public:
	    KujoGFX_Null()
	    {

	    }

	    ~KujoGFX_Null()
	    {

	    }

	    bool initBackend(void*)
	    {
		return true;
	    }

	    void shutdownBackend()
	    {
		return;
	    }

	    void *getContextHandle()
	    {
		return NULL;
	    }
    };

    #if defined(KUJOGFX_PLATFORM_WINDOWS)
    class KujoGFX_D3D11 : public KujoGFXBackend
    {
	struct D3D11Pipeline
	{
	    ID3D11VertexShader *vert_shader = NULL;
	    ID3D11PixelShader *pixel_shader = NULL;
	};

	public:
	    KujoGFX_D3D11()
	    {

	    }

	    ~KujoGFX_D3D11()
	    {

	    }

	    bool initBackend(void *window_handle)
	    {
		if (!initD3D11(window_handle))
		{
		    return false;
		}

		return true;
	    }

	    void shutdownBackend()
	    {
		shutdownD3D11();
	    }

	    void *getContextHandle()
	    {
		return reinterpret_cast<void*>(d3d11_dev_con);
	    }

	private:
	    void *win_handle = NULL;

	    int window_width = 0;
	    int window_height = 0;

	    IDXGISwapChain *swapchain;
	    ID3D11Device *d3d11_device;
	    ID3D11DeviceContext *d3d11_dev_con;
	    ID3D11RenderTargetView *render_target_view;

	    unordered_map<uint32_t, D3D11Pipeline> pipelines;
	    D3D11Pipeline current_pipeline;

	    KujoGFXPass current_pass;

	    bool fetchWindowRes()
	    {
		HWND handle = reinterpret_cast<HWND>(win_handle);

		RECT win_rect;

		if (!GetWindowRect(handle, &win_rect))
		{
		    return false;
		}

		window_width = (win_rect.right - win_rect.left);
		window_height = (win_rect.bottom - win_rect.top);
		return true;
	    }

	    bool initD3D11(void *window_handle)
	    {
		win_handle = window_handle;

		if (!fetchWindowRes())
		{
		    cout << "Could not fetch window resolution!" << endl;
		    return false;
		}

		HWND handle = reinterpret_cast<HWND>(win_handle);

		DXGI_MODE_DESC buffer_desc;
		ZeroMemory(&buffer_desc, sizeof(DXGI_MODE_DESC));

		buffer_desc.Width = window_width;
		buffer_desc.Height = window_height;
		buffer_desc.RefreshRate.Numerator = 60;
		buffer_desc.RefreshRate.Denominator = 1;
		buffer_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		buffer_desc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		buffer_desc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

		DXGI_SWAP_CHAIN_DESC swapchain_desc;

		ZeroMemory(&swapchain_desc, sizeof(DXGI_SWAP_CHAIN_DESC));

		swapchain_desc.BufferDesc = buffer_desc;
		swapchain_desc.SampleDesc.Count = 1;
		swapchain_desc.SampleDesc.Quality = 0;
		swapchain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapchain_desc.BufferCount = 1;
		swapchain_desc.OutputWindow = handle;
		swapchain_desc.Windowed = TRUE;
		swapchain_desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

		HRESULT hres = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, NULL, 0, D3D11_SDK_VERSION, &swapchain_desc, &swapchain, &d3d11_device, NULL, &d3d11_dev_con);

		if (hres != S_OK)
		{
		    cout << "Direct3D 11 could not be initialized!" << endl;
		    return false;
		}

		ID3D11Texture2D *back_buffer;
		hres = swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&back_buffer);

		hres = d3d11_device->CreateRenderTargetView(back_buffer, NULL, &render_target_view);
		back_buffer->Release();
		return true;
	    }

	    void shutdownD3D11()
	    {
		for (auto &iter : pipelines)
		{
		    auto pipeline = iter.second;
		    if (pipeline.vert_shader != NULL)
		    {
			pipeline.vert_shader->Release();
			pipeline.vert_shader = NULL;
		    }

		    if (pipeline.pixel_shader != NULL)
		    {
			pipeline.pixel_shader->Release();
			pipeline.pixel_shader = NULL;
		    }
		}

		render_target_view->Release();
		swapchain->Release();
		d3d11_device->Release();
		d3d11_dev_con->Release();
	    }


	    void beginPass(KujoGFXPass pass)
	    {
		if (!fetchWindowRes())
		{
		    cout << "Could not fetch window resolution!" << endl;
		    throw runtime_error("KujoGFX_D3D11 error");
		}

		current_pass = pass;

		auto action = current_pass.action;
		auto color_attachment = action.color_attach;

		d3d11_dev_con->OMSetRenderTargets(1, &render_target_view, NULL);

		D3D11_VIEWPORT viewport;
		ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = window_width;
		viewport.Height = window_height;

		d3d11_dev_con->RSSetViewports(1, &viewport);

		D3D11_RECT scissor_rect;
		scissor_rect.left = 0;
		scissor_rect.top = 0;
		scissor_rect.right = window_width;
		scissor_rect.bottom = window_height;

		d3d11_dev_con->RSSetScissorRects(1, &scissor_rect);

		if (color_attachment.load_op == LoadOpClear)
		{
		    KujoGFXColor color = color_attachment.color;
		    d3d11_dev_con->ClearRenderTargetView(render_target_view, color);
		}
	    }

	    void endPass()
	    {
		return;
	    }

	    void setPipeline(KujoGFXPipeline pipeline)
	    {
		auto cached_pipeline = pipelines.find(pipeline.id);

		if (cached_pipeline == pipelines.end())
		{
		    cout << "Could not find current pipeline!" << endl;
		    throw runtime_error("KujoGFX_D3D11 error");
		}

		current_pipeline = cached_pipeline->second;
	    }

	    void createPipeline(KujoGFXPipeline pipeline)
	    {
		D3D11Pipeline new_pipeline;
		auto shader = pipeline.shader;

		if (!shader.translateHLSL())
		{
		    cout << "Could not compile shaders!" << endl;
		    throw runtime_error("KujoGFX_D3D11 error");
		}

		string vertex_src = shader.vertex_out_src;
		string pixel_src = shader.fragment_out_src;

		string vertex_log = "";
		string pixel_log = "";

		ID3DBlob *vert_buffer = NULL;
		ID3DBlob *pixel_buffer = NULL;

		if (!compileShader(vert_buffer, vertex_src, "vs_4_0", vertex_log))
		{
		    cout << "Could not compile vertex shader!" << endl;
		    cout << "Error log:" << endl;
		    cout << vertex_log << endl;
		    throw runtime_error("KujoGFX_D3D11 error");
		}

		if (!compileShader(pixel_buffer, pixel_src, "ps_4_0", pixel_log))
		{
		    cout << "Could not compile pixel shader!" << endl;
		    cout << "Error log:" << endl;
		    cout << pixel_log << endl;
		    throw runtime_error("KujoGFX_D3D11 error");
		}

		HRESULT hres = d3d11_device->CreateVertexShader(vert_buffer->GetBufferPointer(), vert_buffer->GetBufferSize(), NULL, &new_pipeline.vert_shader);

		if (FAILED(hres))
		{
		    cout << "Could not create vertex shader!" << endl;
		    throw runtime_error("KujoGFX_D3D11 error");
		}

		hres = d3d11_device->CreatePixelShader(pixel_buffer->GetBufferPointer(), pixel_buffer->GetBufferSize(), NULL, &new_pipeline.pixel_shader);

		if (FAILED(hres))
		{
		    cout << "Could not create pixel shader!" << endl;
		    throw runtime_error("KujoGFX_D3D11 error");
		}

		vert_buffer->Release();
		pixel_buffer->Release();
		current_pipeline = new_pipeline;
		pipelines.insert(make_pair(pipeline.id, new_pipeline));
	    }

	    void applyPipeline()
	    {
		d3d11_dev_con->VSSetShader(current_pipeline.vert_shader, 0, 0);
		d3d11_dev_con->PSSetShader(current_pipeline.pixel_shader, 0, 0);
		d3d11_dev_con->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	    }

	    void draw(KujoGFXDraw draw_call)
	    {
		UINT base_element = draw_call.base_element;
		UINT num_elements = draw_call.num_elements;
		UINT num_instances = draw_call.num_instances;

		if (num_instances > 1)
		{
		    d3d11_dev_con->DrawInstanced(num_elements, num_instances, base_element, 0);
		}
		else
		{
		    d3d11_dev_con->Draw(num_elements, base_element);
		}
	    }

	    void commitFrame()	
	    {
		swapchain->Present(0, 0);
	    }

	    bool compileShader(ID3DBlob* &shader, string source, string target, string &err_logs)
	    {
		ID3DBlob *log_blob = NULL;
		HRESULT res = D3DCompile(source.c_str(), source.length(), NULL, NULL, NULL, "main", target.c_str(), 0, 0, &shader, &log_blob);

		if (FAILED(res))
		{
		    err_logs = string(reinterpret_cast<const char*>(log_blob->GetBufferPointer()), log_blob->GetBufferSize());
		    return false;
		}

		return true;
	    }
    };
    #endif

    // TODO: Implement platform-specific code for the following platforms:
    // MacOS
    // Linux
    // BSD
    // Android
    // Emscripten

    class KujoGFX_OpenGL : public KujoGFXBackend
    {
	struct GLPipeline
	{
	    GLuint program;
	};

	public:
	    KujoGFX_OpenGL()
	    {

	    }

	    ~KujoGFX_OpenGL()
	    {

	    }

	    bool initBackend(void* window_handle)
	    {
		if (!initOpenGL(window_handle))
		{
		    return false;
		}

		return true;
	    }

	    void shutdownBackend()
	    {
		shutdownOpenGL();
	    }

	    void *getContextHandle()
	    {
		#if defined(KUJOGFX_PLATFORM_WINDOWS)
		return reinterpret_cast<void*>(m_hrc);
		#else
		return NULL;
		#endif
	    }

	private:
	    static constexpr int gl_major_version = 3;
	    static constexpr int gl_minor_version = 1;

	    int window_width = 0;
	    int window_height = 0;
	    void *win_handle = NULL;

	    KujoGFXPass current_pass;

	    unordered_map<uint32_t, GLPipeline> pipelines;
	    GLPipeline current_pipeline;

	    #if defined(KUJOGFX_PLATFORM_LINUX)
	    enum DisplayType : int
	    {
		None = 0,
		X11,
		Wayland
	    };

	    DisplayType display_type = None;

	    string getEnvVar(string var_name)
	    {
		auto var_val = getenv(var_name.c_str());

		if (var_val != NULL)
		{
		    return var_val;
		}

		return "";
	    }

	    bool isWayland()
	    {
		return (display_type == Wayland);
	    }

	    bool isX11()
	    {
		return (display_type == X11);
	    }

	    bool detectX11OrWayland()
	    {
		if (display_type != None)
		{
		    return true;
		}

		string session_type = getEnvVar("XDG_SESSION_TYPE");
		string display = getEnvVar("DISPLAY");
		string wayland_display = getEnvVar("WAYLAND_DISPLAY");

		if (session_type == "wayland")
		{
		    display_type = Wayland;
		}
		else if (session_type == "x11")
		{
		    display_type = X11;
		}
		else if (!wayland_display.empty())
		{
		    display_type = Wayland;
		}
		else if (!display.empty())
		{
		    display_type = X11;
		}
		else
		{
		    display_type = None;
		}

		if (display_type == None)
		{
		    cout << "Could not determine session type!" << endl;
		    return false;
		}

		return true;
	    }

	    #endif

	    #if defined(KUJOGFX_PLATFORM_WINDOWS)
	    HGLRC m_hrc;
	    HDC m_hdc;

	    bool createWGLContext(HDC pDC)
	    {
		PIXELFORMATDESCRIPTOR pfd;
		memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
		pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
		pfd.nVersion = 1;
		pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
		pfd.iPixelType = PFD_TYPE_RGBA;
		pfd.cColorBits = 32;
		pfd.cDepthBits = 32;
		pfd.iLayerType = PFD_MAIN_PLANE;

		int nPixelFormat = ChoosePixelFormat(pDC, &pfd);

		if (nPixelFormat == 0)
		{
		    return false;
		}

		BOOL bResult = SetPixelFormat(pDC, nPixelFormat, &pfd);

		if (!bResult)
		{
		    return false;
		}

		HGLRC tempContext = wglCreateContext(pDC);
		wglMakeCurrent(pDC, tempContext);

		GLenum err = glewInit();

		if (err != GLEW_OK)
		{
		    cout << "GLEW could not be initialized! GLEW_error: " << glewGetErrorString(err) << endl;
		    return false;
		}

		int attribs[] = 
		{
		    WGL_CONTEXT_MAJOR_VERSION_ARB, gl_major_version,
		    WGL_CONTEXT_MINOR_VERSION_ARB, gl_minor_version,
		    WGL_CONTEXT_FLAGS_ARB, 0,
		    0
		};

		if (wglewIsSupported("WGL_ARB_create_context") == 1)
		{
		    m_hrc = wglCreateContextAttribsARB(pDC, 0, attribs);
		    wglMakeCurrent(NULL, NULL);
		    wglDeleteContext(tempContext);
		    wglMakeCurrent(pDC, m_hrc);
		}
		else
		{
		    m_hrc = tempContext;
		}

		if (!m_hrc)
		{
		    return false;
		}

		cout << "OpenGL version found: " << glGetString(GL_VERSION) << endl;
		return true;
	    }

	    void deleteWGLContext()
	    {
		wglMakeCurrent(NULL, NULL);

		if (m_hrc)
		{
		    wglDeleteContext(m_hrc);
		    m_hrc = NULL;
		}
	    }

	    #endif

	    bool fetchWindowRes()
	    {
		#if defined(KUJOGFX_PLATFORM_WINDOWS)
		HWND handle = reinterpret_cast<HWND>(win_handle);

		RECT win_rect;

		if (!GetWindowRect(handle, &win_rect))
		{
		    return false;
		}

		window_width = (win_rect.right - win_rect.left);
		window_height = (win_rect.bottom - win_rect.top);
		#else
		#error "OpenGL window resolution fetch is unimplemented for this platform"
		#endif

		return true;
	    }

	    bool initOpenGL(void *window_handle)
	    {
		win_handle = window_handle;

		if (!fetchWindowRes())
		{
		    cout << "Could not fetch window resolution!" << endl;
		    return false;
		}

		#if defined(KUJOGFX_PLATFORM_WINDOWS)
		HWND handle = reinterpret_cast<HWND>(win_handle);
		m_hdc = GetDC(handle);
		if (!createWGLContext(m_hdc))
		{
		    return false;
		}
		#else
		#error "OpenGL context creation is unimplemented for this platform"
		#endif

		return true;
	    }

	    void shutdownOpenGL()
	    {
		for (auto &iter : pipelines)
		{
		    auto pipeline = iter.second;
		    if (isProgramDelete(pipeline.program))
		    {
			glDeleteProgram(pipeline.program);
		    }
		}

		#if defined(KUJOGFX_PLATFORM_WINDOWS)
		deleteWGLContext();
		#else
		#error "OpenGL context deletion is unimplemented for this platform"
		#endif
	    }

	    bool isProgramDelete(GLuint program)
	    {
		if (!glIsProgram(program))
		{
		    return false;
		}

		int is_delete = 0;
		glGetProgramiv(program, GL_DELETE_STATUS, &is_delete);
		return (is_delete != 0);
	    }

	    void beginPass(KujoGFXPass pass)
	    {
		if (!fetchWindowRes())
		{
		    cout << "Could not fetch window resolution!" << endl;
		    throw runtime_error("KujoGFX_OpenGL error");
		}

		glViewport(0, 0, window_width, window_height);
		glScissor(0, 0, window_width, window_height);

		current_pass = pass;

		auto action = current_pass.action;
		auto color_attachment = action.color_attach;

		if (color_attachment.load_op == LoadOpClear)
		{
		    KujoGFXColor color = color_attachment.color;
		    glClearColor(color.red, color.green, color.blue, color.alpha);
		}

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	    }

	    void endPass()
	    {
		return;
	    }

	    void setPipeline(KujoGFXPipeline pipeline)
	    {
		auto cached_pipeline = pipelines.find(pipeline.id);

		if (cached_pipeline == pipelines.end())
		{
		    cout << "Could not find current pipeline!" << endl;
		    throw runtime_error("KujoGFX_OpenGL error");
		}

		current_pipeline = cached_pipeline->second;
	    }

	    void createPipeline(KujoGFXPipeline pipeline)
	    {
		GLPipeline new_pipeline;
		auto shader = pipeline.shader;

		if (!shader.translateGLSL())
		{
		    cout << "Could not compile shaders!" << endl;
		    throw runtime_error("KujoGFX_OpenGL error");
		}

		string vert_source = shader.vertex_out_src;
		string frag_source = shader.fragment_out_src;
		string vert_log = "";
		string frag_log = "";

		GLuint vertex_shader;
		GLuint fragment_shader;
		GLuint shader_program;

		if (!compileShader(vertex_shader, GL_VERTEX_SHADER, vert_source, vert_log))
		{
		    cout << "Could not compile vertex shader!" << endl;
		    cout << "Error log: " << endl;
		    cout << vert_log << endl;
		    throw runtime_error("KujoGFX_OpenGL error");
		}

		if (!compileShader(fragment_shader, GL_FRAGMENT_SHADER, frag_source, frag_log))
		{
		    cout << "Could not compile fragment shader!" << endl;
		    cout << "Error log: " << endl;
		    cout << frag_log << endl;
		    throw runtime_error("KujoGFX_OpenGL error");
		}

		string program_log = "";

		shader_program = glCreateProgram();
		glAttachShader(shader_program, vertex_shader);
		glAttachShader(shader_program, fragment_shader);

		glLinkProgram(shader_program);

		int success = 0;
		glGetProgramiv(shader_program, GL_LINK_STATUS, &success);

		if (!success)
		{
		    GLint log_length;
		    glGetProgramiv(shader_program, GL_INFO_LOG_LENGTH, &log_length);

		    vector<char> str_data(log_length, 0);
		    glGetProgramInfoLog(shader_program, log_length, NULL, str_data.data());

		    program_log.resize(log_length, 0);
		    copy(str_data.begin(), str_data.end(), program_log.begin());

		    cout << "Could not link program!" << endl;
		    cout << "Error log: " << endl;
		    cout << program_log << endl;
		    throw runtime_error("KujoGFX_OpenGL error");
		}

		glDeleteShader(vertex_shader);
		glDeleteShader(fragment_shader);

		new_pipeline.program = shader_program;
		pipelines.insert(make_pair(pipeline.id, new_pipeline));
		current_pipeline = new_pipeline;
	    }

	    void applyPipeline()
	    {
		glUseProgram(current_pipeline.program);
	    }

	    bool compileShader(GLuint &shader, GLenum shader_type, string source, string &log_str)
	    {
		auto c_str = source.c_str();

		shader = glCreateShader(shader_type);
		glShaderSource(shader, 1, &c_str, NULL);
		glCompileShader(shader);

		int success = 0;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

		if (!success)
		{
		    GLint log_length;
		    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);

		    vector<char> str_data(log_length, 0);

		    glGetShaderInfoLog(shader, log_length, NULL, str_data.data());

		    log_str.resize(log_length, 0);
		    copy(str_data.begin(), str_data.end(), log_str.begin());

		    return false;
		}

		return true;
	    }

	    void draw(KujoGFXDraw draw_cmd)
	    {
		int base_elements = draw_cmd.base_element;
		int num_elements = draw_cmd.num_elements;
		int num_instances = draw_cmd.num_instances;

		if (num_instances > 1)
		{
		    glDrawArraysInstanced(GL_TRIANGLES, base_elements, num_elements, num_instances);
		}
		else
		{
		    glDrawArrays(GL_TRIANGLES, base_elements, num_elements);
		}
	    }

	    void commitFrame()
	    {
		#if defined(KUJOGFX_PLATFORM_WINDOWS)
		SwapBuffers(m_hdc);
		#else
		#error "OpenGL buffer swap is unimplemented for this platform"
		#endif
	    }
    };

    class KujoGFX_Vulkan : public KujoGFXBackend
    {
	struct VulkanPipeline
	{
	    VkShaderModule vert_module = VK_NULL_HANDLE;
	    VkShaderModule frag_module = VK_NULL_HANDLE;
	    VkPipeline pipeline = VK_NULL_HANDLE;
	};

	public:
	    KujoGFX_Vulkan()
	    {

	    }

	    ~KujoGFX_Vulkan()
	    {

	    }

	    bool initBackend(void* window_handle)
	    {
		if (!initVulkan(window_handle))
		{
		    return false;
		}

		return true;
	    }

	    void shutdownBackend()
	    {
		shutdownVulkan();
	    }

	    void *getContextHandle()
	    {
		return reinterpret_cast<void*>(instance);
	    }

	private:
	    VkInstance instance;
	    VkSurfaceKHR surface;
	    VkPhysicalDevice physical_device;
	    VkDevice device;
	    VkQueue graphics_queue;
	    VkQueue present_queue;
	    VkSwapchainKHR swapchain;
	    vector<VkImage> swapchain_images;
	    vector<VkImageView> swapchain_image_views;
	    VkFramebuffer swapchain_framebuffer = VK_NULL_HANDLE;
	    VkFormat swapchain_image_format;
	    VkExtent2D swapchain_extent;
	    VkRenderPass render_pass = VK_NULL_HANDLE;
	    VkPipelineLayout pipeline_layout;
	    VkPipeline graphics_pipeline = VK_NULL_HANDLE;
	    VkCommandPool command_pool;
	    vector<VkCommandBuffer> present_command_buffers;
	    VkCommandBuffer command_buffer = VK_NULL_HANDLE;
	    uint32_t image_index = 0;
	    uint32_t instance_version = 0;
	    uint32_t api_version = 0;

	    unordered_map<uint32_t, VulkanPipeline> pipelines;
	    VulkanPipeline current_pipeline;

	    static constexpr int max_frames_in_flight = 2;

	    VkSemaphore image_available_semaphore;
	    VkSemaphore render_finished_semaphore;
	    VkFence in_flight_fence;

	    VkPipelineDynamicStateCreateInfo dynamic_state;
	    VkPipelineViewportStateCreateInfo viewport_state;

	    uint32_t graphics_queue_family = 0;
	    uint32_t present_queue_family = 0;

	    uint32_t window_width = 0;
	    uint32_t window_height = 0;

	    void *win_handle = NULL;
	    KujoGFXPass current_pass;

	    #if defined(KUJOGFX_PLATFORM_LINUX)
	    enum DisplayType : int
	    {
		None = 0,
		X11,
		Wayland
	    };

	    DisplayType display_type = None;

	    string getEnvVar(string var_name)
	    {
		auto var_val = getenv(var_name.c_str());

		if (var_val != NULL)
		{
		    return var_val;
		}

		return "";
	    }

	    bool isWayland()
	    {
		return (display_type == Wayland);
	    }

	    bool isX11()
	    {
		return (display_type == X11);
	    }

	    bool detectX11OrWayland()
	    {
		if (display_type != None)
		{
		    return true;
		}

		string session_type = getEnvVar("XDG_SESSION_TYPE");
		string display = getEnvVar("DISPLAY");
		string wayland_display = getEnvVar("WAYLAND_DISPLAY");

		if (session_type == "wayland")
		{
		    display_type = Wayland;
		}
		else if (session_type == "x11")
		{
		    display_type = X11;
		}
		else if (!wayland_display.empty())
		{
		    display_type = Wayland;
		}
		else if (!display.empty())
		{
		    display_type = X11;
		}
		else
		{
		    display_type = None;
		}

		if (display_type == None)
		{
		    cout << "Could not determine session type!" << endl;
		    return false;
		}

		return true;
	    }

	    #endif

	    bool initVulkan(void *window_handle)
	    {
		win_handle = window_handle;

		if (!createInstance())
		{
		    return false;
		}

		if (!createSurface())
		{
		    return false;
		}

		if (!findPhysicalDevice())
		{
		    return false;
		}

		if (!findQueueFamilies())
		{
		    return false;
		}

		if (!createLogicalDevice())
		{
		    return false;
		}

		if (!createSwapchain())
		{
		    return false;
		}

		if (!createPipelineLayout())
		{
		    return false;
		}

		if (!createCommandQueues())
		{
		    return false;
		}

		if (!createSyncObjects())
		{
		    return false;
		}

		return true;
	    }

	    void cleanupSwapchain()
	    {
		for (auto &image_view : swapchain_image_views)
		{
		    vkDestroyImageView(device, image_view, NULL);
		}

		vkDestroySwapchainKHR(device, swapchain, NULL);
	    }

	    void shutdownVulkan()
	    {
		vkDeviceWaitIdle(device);

		for (auto &iter : pipelines)
		{
		    auto &pipeline = iter.second;

		    if (pipeline.vert_module != VK_NULL_HANDLE)
		    {
			vkDestroyShaderModule(device, pipeline.vert_module, NULL);
			pipeline.vert_module = NULL;
		    }

		    if (pipeline.frag_module != VK_NULL_HANDLE)
		    {
			vkDestroyShaderModule(device, pipeline.frag_module, NULL);
			pipeline.frag_module = NULL;
		    }

		    if (pipeline.pipeline != VK_NULL_HANDLE)
		    {
			vkDestroyPipeline(device, pipeline.pipeline, NULL);
			pipeline.pipeline = NULL;
		    }
		}

		vkDestroyPipelineLayout(device, pipeline_layout, NULL);
		vkDestroySemaphore(device, image_available_semaphore, NULL);
		vkDestroySemaphore(device, render_finished_semaphore, NULL);
		vkDestroyFence(device, in_flight_fence, NULL);
		vkFreeCommandBuffers(device, command_pool, present_command_buffers.size(), present_command_buffers.data());
		vkDestroyCommandPool(device, command_pool, NULL);

		cleanupSwapchain();
		vkDestroyDevice(device, NULL);
		vkDestroySurfaceKHR(instance, surface, NULL);
		vkDestroyInstance(instance, NULL);
	    }

	    void recreateSwapchain()
	    {
		vkDeviceWaitIdle(device);

		cleanupSwapchain();

		if (!fetchWindowRes())
		{
		    cout << "Could not fetch window resolution!" << endl;
		    throw runtime_error("KujoGFX_Vulkan error");
		}

		if (!createSwapchain())
		{
		    cout << "Could not recreate swapchain!" << endl;
		    throw runtime_error("KujoGFX_Vulkan error");
		}
	    }

	    VkAttachmentLoadOp convertLoadOp(KujoGFXLoadOp load_op)
	    {
		switch (load_op)
		{
		    case LoadOpLoad: return VK_ATTACHMENT_LOAD_OP_LOAD; break;
		    case LoadOpClear: return VK_ATTACHMENT_LOAD_OP_CLEAR; break;
		    default: return VK_ATTACHMENT_LOAD_OP_DONT_CARE; break;
		}
	    }

	    VkAttachmentStoreOp convertStoreOp(KujoGFXStoreOp store_op)
	    {
		switch (store_op)
		{
		    case StoreOpStore: return VK_ATTACHMENT_STORE_OP_STORE; break;
		    default: return VK_ATTACHMENT_STORE_OP_DONT_CARE; break;
		}
	    }

	    VkClearColorValue convertClearColor(KujoGFXColor color)
	    {
		return {{color.red, color.green, color.blue, color.alpha}};
	    }

	    bool createRenderPass()
	    {
		VkAttachmentDescription color_attachment = {};
		color_attachment.format = swapchain_image_format;
		color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
		color_attachment.loadOp = convertLoadOp(current_pass.action.color_attach.load_op);
		color_attachment.storeOp = convertStoreOp(current_pass.action.color_attach.store_op);
		color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference color_attachment_ref = {};
		color_attachment_ref.attachment = 0;
		color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &color_attachment_ref;

		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo render_pass_info = {};
		render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		render_pass_info.attachmentCount = 1;
		render_pass_info.pAttachments = &color_attachment;
		render_pass_info.subpassCount = 1;
		render_pass_info.pSubpasses = &subpass;
		render_pass_info.dependencyCount = 1;
		render_pass_info.pDependencies = &dependency;

		VkResult err = vkCreateRenderPass(device, &render_pass_info, NULL, &render_pass);

		if (err != VK_SUCCESS)
		{
		    cout << "Could not create render pass!" << endl;
		    return false;
		}

		return true;
	    }

	    bool createFramebuffer()
	    {
		VkFramebufferCreateInfo framebuffer_info = {};
		framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebuffer_info.pNext = NULL;
		framebuffer_info.renderPass = render_pass;
		framebuffer_info.attachmentCount = 1;
		framebuffer_info.width = swapchain_extent.width;
		framebuffer_info.height = swapchain_extent.height;
		framebuffer_info.layers = 1;
		framebuffer_info.pAttachments = &swapchain_image_views[image_index];

		VkResult err = vkCreateFramebuffer(device, &framebuffer_info, NULL, &swapchain_framebuffer);

		if (err != VK_SUCCESS)
		{
		    cout << "Could not create framebuffer!" << endl;
		    return false;
		}

		return true;
	    }

	    void beginPass(KujoGFXPass pass)
	    {
		current_pass = pass;

		if (!createRenderPass())
		{
		    cout << "createRenderPass() failed!" << endl;
		    throw runtime_error("KujoGFX_Vulkan error");
		}

		vkWaitForFences(device, 1, &in_flight_fence, VK_TRUE, UINT64_MAX);

		VkResult err = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, image_available_semaphore, VK_NULL_HANDLE, &image_index);

		if (err == VK_ERROR_OUT_OF_DATE_KHR)
		{
		    recreateSwapchain();
		    return;
		}
		else if ((err != VK_SUCCESS) && (err != VK_SUBOPTIMAL_KHR))
		{
		    cout << "Could not acquire swapchain image!" << endl;
		    throw runtime_error("KujoGFX_Vulkan error");
		}

		vkResetFences(device, 1, &in_flight_fence);

		if (!createFramebuffer())
		{
		    cout << "createFramebuffer() failed!" << endl;
		    throw runtime_error("KujoGFX_Vulkan error");
		}

		command_buffer = present_command_buffers[image_index];
		vkResetCommandBuffer(command_buffer, 0);

		VkCommandBufferBeginInfo begin_info = {};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin_info.flags = 0;
		begin_info.pInheritanceInfo = NULL;

		err = vkBeginCommandBuffer(command_buffer, &begin_info);

		if (err != VK_SUCCESS)
		{
		    cout << "Could not begin command buffer!" << endl;
		    throw runtime_error("KujoGFX_Vulkan error");
		}

		VkClearValue clear_val;
		clear_val.color = convertClearColor(current_pass.action.color_attach.color);

		VkRenderPassBeginInfo render_pass_info = {};
		render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		render_pass_info.pNext = NULL;
		render_pass_info.renderPass = render_pass;
		render_pass_info.renderArea.offset = {0, 0};
		render_pass_info.renderArea.extent = swapchain_extent;
		render_pass_info.framebuffer = swapchain_framebuffer;
		render_pass_info.clearValueCount = 1;
		render_pass_info.pClearValues = &clear_val;

		vkCmdBeginRenderPass(command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
	    }

	    void endPass()
	    {
		vkCmdEndRenderPass(command_buffer);
		VkResult err = vkEndCommandBuffer(command_buffer);

		if (err != VK_SUCCESS)
		{
		    cout << "Could not begin command buffer!" << endl;
		    throw runtime_error("KujoGFX_Vulkan error");
		}

		VkSubmitInfo submit_info = {};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		submit_info.waitSemaphoreCount = 1;
		submit_info.pWaitSemaphores = &image_available_semaphore;
		submit_info.pWaitDstStageMask = &wait_stage;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &command_buffer;
		submit_info.signalSemaphoreCount = 1;
		submit_info.pSignalSemaphores = &render_finished_semaphore;

		err = vkQueueSubmit(graphics_queue, 1, &submit_info, in_flight_fence);

		if (err != VK_SUCCESS)
		{
		    cout << "Could not submit draw commands!" << endl;
		    throw runtime_error("KujoGFX_Vulkan error");
		}

		VkPresentInfoKHR present_info = {};
		present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		present_info.waitSemaphoreCount = 1;
		present_info.pWaitSemaphores = &render_finished_semaphore;
		present_info.swapchainCount = 1;
		present_info.pSwapchains = &swapchain;
		present_info.pImageIndices = &image_index;
		present_info.pResults = NULL;

		err = vkQueuePresentKHR(present_queue, &present_info);

		if ((err == VK_ERROR_OUT_OF_DATE_KHR) || (err == VK_SUBOPTIMAL_KHR))
		{
		    recreateSwapchain();
		}
		else if (err != VK_SUCCESS)
		{
		    cout << "Could not render swapchain image!" << endl;
		    throw runtime_error("KujoGFX_Vulkan error");
		}
	    }

	    VkShaderModule createShaderModule(vector<uint32_t> &code)
	    {
		VkShaderModuleCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		create_info.codeSize = (code.size() * sizeof(uint32_t));
		create_info.pCode = code.data();

		VkShaderModule shader_module;

		VkResult err = vkCreateShaderModule(device, &create_info, NULL, &shader_module);

		if (err != VK_SUCCESS)
		{
		    cout << "Could not create shader module!" << endl;
		    throw runtime_error("KujoGFX_Vulkan error");
		}

		return shader_module;
	    }

	    void setPipeline(KujoGFXPipeline pipeline)
	    {
		auto cached_pipeline = pipelines.find(pipeline.id);

		if (cached_pipeline == pipelines.end())
		{
		    cout << "Could not find current pipeline!" << endl;
		    throw runtime_error("KujoGFX_Vulkan error");
		}

		current_pipeline = cached_pipeline->second;
	    }

	    void createPipeline(KujoGFXPipeline pipeline)
	    {
		VulkanPipeline new_pipeline;
		auto shader = pipeline.shader;

		if (!shader.translateSPIRV())
		{
		    cout << "Could not compile shader!" << endl;
		    throw runtime_error("KujoGFX_Vulkan error");
		}

		new_pipeline.vert_module = createShaderModule(shader.vert_spirv);
		new_pipeline.frag_module = createShaderModule(shader.frag_spirv);

		VkPipelineShaderStageCreateInfo vert_shader_info = {};
		vert_shader_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vert_shader_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vert_shader_info.module = new_pipeline.vert_module;
		vert_shader_info.pName = "main";

		VkPipelineShaderStageCreateInfo frag_shader_info = {};
		frag_shader_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		frag_shader_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		frag_shader_info.module = new_pipeline.frag_module;
		frag_shader_info.pName = "main";

		vector<VkPipelineShaderStageCreateInfo> shader_stages = 
		{
		    vert_shader_info,
		    frag_shader_info
		};

		vector<VkDynamicState> dynamic_states = 
		{
		    VK_DYNAMIC_STATE_VIEWPORT,
		    VK_DYNAMIC_STATE_SCISSOR
		};

		VkPipelineDynamicStateCreateInfo dynamic_state = {};
		dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamic_state.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size());
		dynamic_state.pDynamicStates = dynamic_states.data();

		VkPipelineVertexInputStateCreateInfo vertex_input_info = {};
		vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertex_input_info.vertexBindingDescriptionCount = 0;
		vertex_input_info.pVertexBindingDescriptions = NULL;
		vertex_input_info.vertexAttributeDescriptionCount = 0;
		vertex_input_info.pVertexAttributeDescriptions = NULL;

		VkPipelineInputAssemblyStateCreateInfo input_assembly = {};
		input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		input_assembly.primitiveRestartEnable = VK_FALSE;

		VkPipelineViewportStateCreateInfo viewport_state = {};
		viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewport_state.viewportCount = 1;
		viewport_state.scissorCount = 1;

		VkPipelineRasterizationStateCreateInfo rasterizer = {};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;

		VkPipelineMultisampleStateCreateInfo multisampling = {};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		VkPipelineColorBlendAttachmentState color_blend_attachment = {};
		color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		color_blend_attachment.blendEnable = VK_FALSE;

		VkPipelineColorBlendStateCreateInfo color_blending = {};
		color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		color_blending.logicOpEnable = false;
		color_blending.attachmentCount = 1;
		color_blending.pAttachments = &color_blend_attachment;
	
		VkGraphicsPipelineCreateInfo pipeline_info = {};
		pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipeline_info.stageCount = 2;
		pipeline_info.pStages = shader_stages.data();
		pipeline_info.pVertexInputState = &vertex_input_info;
		pipeline_info.pInputAssemblyState = &input_assembly;
		pipeline_info.pViewportState = &viewport_state;
		pipeline_info.pRasterizationState = &rasterizer;
		pipeline_info.pMultisampleState = &multisampling;
		pipeline_info.pColorBlendState = &color_blending;
		pipeline_info.pDynamicState = &dynamic_state;
		pipeline_info.layout = pipeline_layout;
		pipeline_info.renderPass = render_pass;
		pipeline_info.subpass = 0;

		VkResult err = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipeline_info, NULL, &new_pipeline.pipeline);

		if (err != VK_SUCCESS)
		{
		    cout << "Could not create graphics pipeline!" << endl;
		    throw runtime_error("KujoGFX_Vulkan error");
		}

		pipelines.insert(make_pair(pipeline.id, new_pipeline));
		current_pipeline = new_pipeline;
	    }

	    void applyPipeline()
	    {
		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = static_cast<float>(swapchain_extent.height);
		viewport.width = static_cast<float>(swapchain_extent.width);
		viewport.height = -static_cast<float>(swapchain_extent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor = {};
		scissor.offset = {0, 0};
		scissor.extent = swapchain_extent;

		vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, current_pipeline.pipeline);
		vkCmdSetViewport(command_buffer, 0, 1, &viewport);
		vkCmdSetScissor(command_buffer, 0, 1, &scissor);
	    }

	    void draw(KujoGFXDraw draw)
	    {
		int base_element = draw.base_element;
		int num_elements = draw.num_elements;
		int num_instances = draw.num_instances; 
		vkCmdDraw(command_buffer, num_elements, num_instances, base_element, 0);
	    }

	    void commitFrame()
	    {
		vkDeviceWaitIdle(device);

		if (swapchain_framebuffer != VK_NULL_HANDLE)
		{
		    vkDestroyFramebuffer(device, swapchain_framebuffer, NULL);
		    swapchain_framebuffer = VK_NULL_HANDLE;
		}

		if (render_pass != VK_NULL_HANDLE)
		{
		    vkDestroyRenderPass(device, render_pass, NULL);
		    render_pass = VK_NULL_HANDLE;
		}
	    }

	    bool createInstance()
	    {
		uint32_t instance_version = VK_API_VERSION_1_0;

		auto instance_ver_func = (PFN_vkEnumerateInstanceVersion)vkGetInstanceProcAddr(NULL, "vkEnumerateInstanceVersion");
		
		if (instance_ver_func != NULL)
		{
		    vkEnumerateInstanceVersion(&instance_version);
		}

		if (instance_version >= VK_API_VERSION_1_1)
		{
		    cout << "Using Vulkan 1.1" << endl;
		    api_version = VK_API_VERSION_1_1;
		}
		else
		{
		    cout << "Falling back to Vulkan 1.0" << endl;
		    api_version = VK_API_VERSION_1_0;
		}

		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "KujoGFX_Vulkan";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "KujoGFX_VulkanBackend";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = api_version;

		uint32_t extension_count = 0;
		VkResult err = vkEnumerateInstanceExtensionProperties(NULL, &extension_count, NULL);

		if (err != VK_SUCCESS)
		{
		    cout << "Could not enumerate extension count!" << endl;
		    return false;
		}

		if (extension_count == 0)
		{
		    cout << "No extensions supported!" << endl;
		    return false;
		}

		vector<VkExtensionProperties> extensions(extension_count);
		err = vkEnumerateInstanceExtensionProperties(NULL, &extension_count, extensions.data());

		if (err != VK_SUCCESS)
		{
		    cout << "Could not enumerate extensions!" << endl;
		    return false;
		}

		vector<const char*> extension_names;

		vector<const char*> desired_extensions =
		{
		    "VK_EXT_debug_report",
		    "VK_EXT_debug_utils",
		    #if defined(KUJOGFX_PLATFORM_WINDOWS)
		    VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
		    #elif defined(KUJOGFX_PLATFORM_LINUX) || defined(KUJOGFX_PLATFORM_BSD)
		    VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME,
		    VK_KHR_XLIB_SURFACE_EXTENSION_NAME,
		    VK_KHR_XCB_SURFACE_EXTENSION_NAME,
		    #elif defined(KUJOGFX_PLATFORM_MACOS)
		    VK_MVK_MACOS_SURFACE_EXTENSION_NAME,
		    #else
		    #error "Vulkan extension collection is unimplemented for this platform"
		    #endif
		    VK_KHR_SURFACE_EXTENSION_NAME
		};

		if (api_version == VK_API_VERSION_1_0)
		{
		    desired_extensions.push_back(VK_KHR_MAINTENANCE1_EXTENSION_NAME);
		}

		cout << "Available extensions: " << endl;

		for (const auto &ext : extensions)
		{
		    cout << "\t" << ext.extensionName << endl;
		}

		for (const auto &extension : extensions)
		{
		    string ext_name = extension.extensionName;

		    for (const auto &name : desired_extensions)
		    {
			if (name == ext_name)
			{
			    extension_names.push_back(name);
			}
		    }
		}

		cout << "Supported extensions: " << endl;

		for (const auto &name : extension_names)
		{
		    cout << "\t" << name << endl;
		}

		uint32_t layer_count;
		err = vkEnumerateInstanceLayerProperties(&layer_count, NULL);

		if (err != VK_SUCCESS)
		{
		    cout << "Could not enumerate layer count!" << endl;
		    return false;
		}

		vector<VkLayerProperties> layers(layer_count);
		err = vkEnumerateInstanceLayerProperties(&layer_count, layers.data());

		vector<const char*> desired_layers = 
		{
		    "VK_LAYER_KHRONOS_validation"
		};

		vector<const char*> layer_names;

		for (const auto &layer : layers)
		{
		    string layer_name = layer.layerName;

		    for (const auto &name : desired_layers)
		    {
			if (name == layer_name)
			{
			    layer_names.push_back(name);
			}
		    }
		}

		cout << "Supported layers: " << endl;

		for (const auto &name : layer_names)
		{
		    cout << "\t" << name << endl;
		}

		VkInstanceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;
		createInfo.enabledExtensionCount = extension_names.size();
		createInfo.ppEnabledExtensionNames = extension_names.data();
		createInfo.enabledLayerCount = layer_names.size();
		createInfo.ppEnabledLayerNames = layer_names.data();

		err = vkCreateInstance(&createInfo, NULL, &instance);

		if (err != VK_SUCCESS)
		{
		    cout << "Could not create instance!" << endl;
		    return false;
		}

		return true;
	    }

	    bool fetchWindowRes()
	    {
		#if defined(KUJOGFX_PLATFORM_WINDOWS)
		HWND handle = reinterpret_cast<HWND>(win_handle);

		RECT win_rect;

		if (!GetWindowRect(handle, &win_rect))
		{
		    return false;
		}

		window_width = (win_rect.right - win_rect.left);
		window_height = (win_rect.bottom - win_rect.top);
		#else
		#error "Vulkan window resolution fetch is unimplemented for this platform"
		#endif
		return true;
	    }

	    bool createSurface()
	    {
		if (!fetchWindowRes())
		{
		    cout << "Could not fetch window resolution!" << endl;
		    return false;
		}

		#if defined(KUJOGFX_PLATFORM_WINDOWS)
		HWND handle = reinterpret_cast<HWND>(win_handle);

		HINSTANCE hInstance = GetModuleHandle(NULL);

		VkWin32SurfaceCreateInfoKHR win32_create_info;
		win32_create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		win32_create_info.pNext = NULL;
		win32_create_info.flags = 0;
		win32_create_info.hinstance = hInstance;
		win32_create_info.hwnd = handle;

		VkResult err = vkCreateWin32SurfaceKHR(instance, &win32_create_info, NULL, &surface);

		if (err != VK_SUCCESS)
		{
		    cout << "Could not create surface!" << endl;
		    return false;
		}

		#else
		#error "Vulkan surface creation is unimplemented for this platform"
		#endif

		return true;
	    }

	    bool findPhysicalDevice()
	    {
		uint32_t device_count = 0;
		vkEnumeratePhysicalDevices(instance, &device_count, NULL);

		if (device_count == 0)
		{
		    cout << "Could not find a suitable device!" << endl;
		    return false;
		}

		vector<VkPhysicalDevice> devices(device_count);
		vkEnumeratePhysicalDevices(instance, &device_count, devices.data());

		multimap<int, VkPhysicalDevice> candidates;

		for (const auto &device : devices)
		{
		    int score = rateDeviceSuitability(device);
		    candidates.insert(make_pair(score, device));
		}

		if (candidates.rbegin()->first > 0)
		{
		    physical_device = candidates.rbegin()->second;
		}
		else
		{
		    cout << "Could not find a suitable device!" << endl;
		    return false;
		}

		uint32_t extension_count = 0;
		vkEnumerateDeviceExtensionProperties(physical_device, NULL, &extension_count, NULL);

		if (extension_count == 0)
		{
		    cout << "Physical device doesn't support any extensions!" << endl;
		    return false;
		}

		vector<VkExtensionProperties> device_extensions(extension_count);
		vkEnumerateDeviceExtensionProperties(physical_device, NULL, &extension_count, device_extensions.data());

		bool is_swapchain_extension_found = false;

		for (const auto &extension : device_extensions)
		{
		    string ext_name = extension.extensionName;
		    if (ext_name == VK_KHR_SWAPCHAIN_EXTENSION_NAME)
		    {
			is_swapchain_extension_found = true;
			break;
		    }
		}

		if (!is_swapchain_extension_found)
		{
		    cout << "Physical device doesn't support swapchains!" << endl;
		    return false;
		}

		return true;
	    }

	    int rateDeviceSuitability(VkPhysicalDevice device)
	    {
		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(device, &deviceProperties);

		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

		int score = 0;

		// Discrete GPUs have a significant performance advantage
		if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{
		    score += 1000;
		}

		// Maximum possible size of textures affects graphics quality
		score += deviceProperties.limits.maxImageDimension2D;

		/*
		cout << "Current device: " << deviceProperties.deviceName << endl;
		cout << "Device score: " << dec << score << endl;
		cout << endl;
		*/

		return score;
	    }

	    bool findQueueFamilies()
	    {
		uint32_t queue_family_count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, NULL);

		if (queue_family_count == 0)
		{
		    cout << "Physical device has no queue families!" << endl;
		    return false;
		}

		vector<VkQueueFamilyProperties> queue_families(queue_family_count);
		vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families.data());

		cout << "Physical device has " << dec << queue_family_count << " queue families" << endl;

		bool found_graphics_queue_family = false;
		bool found_present_queue_family = false;

		for (uint32_t i = 0; i < queue_family_count; i++)
		{
		    VkBool32 present_support = false;
		    vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface, &present_support);

		    if ((queue_families[i].queueCount > 0) && (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT))
		    {
			graphics_queue_family = i;
			found_graphics_queue_family = true;

			if (present_support)
			{
			    present_queue_family = i;
			    found_present_queue_family = true;
			    break;
			}
		    }

		    if (!found_present_queue_family && present_support)
		    {
			present_queue_family = i;
			found_present_queue_family = true;
		    }
		}

		if (!found_graphics_queue_family || !found_present_queue_family)
		{
		    cout << "Could not find a valid queue family!" << endl;
		    return false;
		}

		cout << "Found queue family of " << dec << int(graphics_queue_family) << " that supports graphics and queue family of " << dec << int(present_queue_family) << " that supports presentation" << endl;
		return true;
	    }

	    bool createLogicalDevice()
	    {
		float queue_priority = 1.0f;

		VkDeviceQueueCreateInfo queue_create_info[2] = {};

		queue_create_info[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_create_info[0].queueFamilyIndex = graphics_queue_family;
		queue_create_info[0].queueCount = 1;
		queue_create_info[0].pQueuePriorities = &queue_priority;

		queue_create_info[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_create_info[1].queueFamilyIndex = present_queue_family;
		queue_create_info[1].queueCount = 1;
		queue_create_info[1].pQueuePriorities = &queue_priority;

		VkDeviceCreateInfo device_create_info = {};
		device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		device_create_info.pQueueCreateInfos = queue_create_info;

		if (graphics_queue_family == present_queue_family)
		{
		    device_create_info.queueCreateInfoCount = 1;
		}
		else
		{
		    device_create_info.queueCreateInfoCount = 2;
		}

		vector<const char*> device_extensions =
		{
		    VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

		device_create_info.enabledExtensionCount = device_extensions.size();
		device_create_info.ppEnabledExtensionNames = device_extensions.data();

		VkResult err = vkCreateDevice(physical_device, &device_create_info, NULL, &device);

		if (err != VK_SUCCESS)
		{
		    cout << "Could not create logical device!" << endl;
		    return false;
		}

		vkGetDeviceQueue(device, graphics_queue_family, 0, &graphics_queue);
		vkGetDeviceQueue(device, present_queue_family, 0, &present_queue);
		return true;
	    }

	    bool createSwapchain()
	    {
		VkSurfaceCapabilitiesKHR surface_capabilities;
		VkResult err = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &surface_capabilities);

		if (err != VK_SUCCESS)
		{
		    cout << "Could not acquire presentation surface capabilities!" << endl;
		    return false;
		}

		uint32_t format_count = 0;

		err = vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, NULL);

		if (err != VK_SUCCESS)
		{
		    cout << "Could not acquire surface format count!" << endl;
		    return false;
		}

		if (format_count == 0)
		{
		    cout << "No surface formats supported!" << endl;
		    return false;
		}

		vector<VkSurfaceFormatKHR> surface_formats(format_count);

		err = vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, surface_formats.data());

		if (err != VK_SUCCESS)
		{
		    cout << "Could not get supported surface formats!" << endl;
		    return false;
		}

		uint32_t present_mode_count = 0;

		err = vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, NULL);

		if (err != VK_SUCCESS)
		{
		    cout << "Could not get number of supported presentation modes!" << endl;
		    return false;
		}

		if (present_mode_count == 0)
		{
		    cout << "No presentation modes supported!" << endl;
		    return false;
		}

		vector<VkPresentModeKHR> present_modes(present_mode_count);

		err = vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, present_modes.data());

		if (err != VK_SUCCESS)
		{
		    cout << "Could not get supported presentation modes!" << endl;
		    return false;
		}

		uint32_t image_count = (surface_capabilities.minImageCount + 1);

		if ((surface_capabilities.maxImageCount != 0) && (image_count > surface_capabilities.maxImageCount))
		{
		    image_count = surface_capabilities.maxImageCount;
		}

		// cout << "Using " << dec << image_count << " images for swapchain!" << endl;

		VkSurfaceFormatKHR surface_format = chooseSurfaceFormat(surface_formats);

		swapchain_image_format = surface_format.format;
		swapchain_extent = chooseSwapExtent(surface_capabilities);
		

		if (!(surface_capabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT))
		{
		    cout << "Warning: swapchain image does not support VK_IMAGE_TRANSFER_DST usage" << endl;
		}

		VkSurfaceTransformFlagBitsKHR surface_transform;

		if (surface_capabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
		{
		    surface_transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		}
		else
		{
		    surface_transform = surface_capabilities.currentTransform;
		}

		VkPresentModeKHR present_mode = choosePresentMode(present_modes);

		VkSwapchainCreateInfoKHR create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		create_info.surface = surface;
		create_info.minImageCount = image_count;
		create_info.imageFormat = surface_format.format;
		create_info.imageColorSpace = surface_format.colorSpace;
		create_info.imageExtent = swapchain_extent;
		create_info.imageArrayLayers = 1;
		create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		create_info.queueFamilyIndexCount = 0;
		create_info.pQueueFamilyIndices = NULL;
		create_info.preTransform = surface_transform;
		create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		create_info.presentMode = present_mode;
		create_info.clipped = VK_TRUE;
		create_info.oldSwapchain = VK_NULL_HANDLE;

		err = vkCreateSwapchainKHR(device, &create_info, NULL, &swapchain);

		if (err != VK_SUCCESS)
		{
		    cout << "Could not create swapchain!" << endl;
		    return false;
		}

		uint32_t actual_image_count = 0;

		err = vkGetSwapchainImagesKHR(device, swapchain, &actual_image_count, NULL);

		if (err != VK_SUCCESS)
		{
		    cout << "Could not get swapchain image count!" << endl;
		    return false;
		}

		if (actual_image_count == 0)
		{
		    cout << "No swapchain images found!" << endl;
		    return false;
		}

		swapchain_images.resize(actual_image_count);

		err = vkGetSwapchainImagesKHR(device, swapchain, &actual_image_count, swapchain_images.data());

		if (err != VK_SUCCESS)
		{
		    cout << "Could not acquire swapchain images!" << endl;
		    return false;
		}

		if (!createImageViews())
		{
		    return false;
		}

		return true;
	    }

	    bool createImageViews()
	    {
		swapchain_image_views.resize(swapchain_images.size());

		for (size_t i = 0; i < swapchain_images.size(); i++)
		{
		    VkImageViewCreateInfo create_info = {};
		    create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		    create_info.image = swapchain_images[i];
		    create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		    create_info.format = swapchain_image_format;
		    create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		    create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		    create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		    create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		    create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		    create_info.subresourceRange.baseMipLevel = 0;
		    create_info.subresourceRange.levelCount = 1;
		    create_info.subresourceRange.baseArrayLayer = 0;
		    create_info.subresourceRange.layerCount = 1;

		    VkResult err = vkCreateImageView(device, &create_info, NULL, &swapchain_image_views[i]);

		    if (err != VK_SUCCESS)
		    {
			cout << "Could not create image views!" << endl;
			return false;
		    }
		}

		return true;
	    }

	    bool createCommandQueues()
	    {
		VkCommandPoolCreateInfo pool_create_info = {};
		pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		pool_create_info.queueFamilyIndex = present_queue_family;

		VkResult err = vkCreateCommandPool(device, &pool_create_info, NULL, &command_pool);

		if (err != VK_SUCCESS)
		{
		    cout << "Could not create command queue!" << endl;
		    return false;
		}

		present_command_buffers.resize(max_frames_in_flight);

		VkCommandBufferAllocateInfo alloc_info = {};
		alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		alloc_info.commandPool = command_pool;
		alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		alloc_info.commandBufferCount = uint32_t(present_command_buffers.size());

		err = vkAllocateCommandBuffers(device, &alloc_info, present_command_buffers.data());

		if (err != VK_SUCCESS)
		{
		    cout << "Could not allocate presentation command buffers!" << endl;
		    return false;
		}

		return true;
	    }

	    bool createSyncObjects()
	    {
		VkSemaphoreCreateInfo semaphore_info = {};
		semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fence_info = {};
		fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		VkResult err = vkCreateSemaphore(device, &semaphore_info, NULL, &image_available_semaphore);

		if (err != VK_SUCCESS)
		{
		    cout << "Could not create image semaphore!" << endl;
		    return false;
		}

		err = vkCreateSemaphore(device, &semaphore_info, NULL, &render_finished_semaphore);

		if (err != VK_SUCCESS)
		{
		    cout << "Could not create render semaphore!" << endl;
		    return false;
		}

		err = vkCreateFence(device, &fence_info, NULL, &in_flight_fence);

		if (err != VK_SUCCESS)
		{
		    cout << "Could not create fence!" << endl;
		    return false;
		}

		return true;
	    }

	    bool createPipelineLayout()
	    {
		VkPipelineLayoutCreateInfo pipeline_layout_info = {};
		pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

		VkResult err = vkCreatePipelineLayout(device, &pipeline_layout_info, NULL, &pipeline_layout);

		if (err != VK_SUCCESS)
		{
		    cout << "Could not create pipeline layout!" << endl;
		    return false;
		}

		return true;
	    }

	    VkSurfaceFormatKHR chooseSurfaceFormat(const vector<VkSurfaceFormatKHR> &available_formats)
	    {
		if ((available_formats.size() == 1) && (available_formats[0].format == VK_FORMAT_UNDEFINED))
		{
		    return {VK_FORMAT_R8G8B8A8_UNORM, VK_COLORSPACE_SRGB_NONLINEAR_KHR};
		}

		for (const auto &format : available_formats)
		{
		    if (format.format == VK_FORMAT_R8G8B8A8_UNORM)
		    {
			return format;
		    }
		}

		return available_formats[0];
	    }

	    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &surface_capabilities)
	    {
		if (int(surface_capabilities.currentExtent.width) == -1)
		{
		    VkExtent2D swapchain_extent = {};

		    auto min_extent = surface_capabilities.minImageExtent;
		    auto max_extent = surface_capabilities.maxImageExtent;

		    swapchain_extent.width = clamp(window_width, min_extent.width, max_extent.width);
		    swapchain_extent.height = clamp(window_height, min_extent.height, max_extent.height);
		    return swapchain_extent;
		}
		else
		{
		    return surface_capabilities.currentExtent;
		}
	    }

	    VkPresentModeKHR choosePresentMode(const vector<VkPresentModeKHR> present_modes)
	    {
		for (const auto &mode : present_modes)
		{
		    if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
		    {
			return mode;
		    }
		}

		return VK_PRESENT_MODE_FIFO_KHR;
	    }
    };

    class KujoGFX
    {
	public:
	    KujoGFX()
	    {

	    }

	    ~KujoGFX()
	    {

	    }

	    void setBackend(KujoGFXBackendType type = BackendAuto)
	    {
		manual_backend_type = type;
	    }

	    bool init(KujoGFXPlatformData data)
	    {
		if (is_initialized)
		{
		    return true;
		}

		if (!validatePlatformData(data))
		{
		    return false;
		}

		detectBackend();

		assert(backend != NULL);

		if (platform_data.context_handle == NULL)
		{
		    platform_data.context_handle = backend->getContextHandle();
		}

		is_initialized = true;
		return true;
	    }

	    void shutdown()
	    {
		if (!is_initialized)
		{
		    return;
		}

		assert(backend != NULL);
		backend->shutdownBackend();

		if (platform_data.context_handle != NULL)
		{
		    platform_data.context_handle = NULL;
		}

		if (platform_data.window_handle != NULL)
		{
		    platform_data.window_handle = NULL;
		}

		is_initialized = false;
	    }

	    void beginPass(KujoGFXPassAction pass_action)
	    {
		KujoGFXPass pass;
		pass.action = pass_action;
		beginPass(pass);
	    }

	    void beginPass(KujoGFXPass pass)
	    {
		KujoGFXCommand command(pass);
		commands.push_back(command);
	    }

	    void endPass()
	    {
		KujoGFXCommand command(CommandEndPass);
		commands.push_back(command);
	    }

	    void applyPipeline(KujoGFXPipeline pipeline)
	    {
		KujoGFXCommand command(pipeline);
		commands.push_back(command);
	    }

	    void draw(int base_element, int num_elements, int num_instances)
	    {
		KujoGFXCommand command(KujoGFXDraw(base_element, num_elements, num_instances));
		commands.push_back(command);
	    }

	    void commit()
	    {
		KujoGFXCommand command(CommandCommit);
		commands.push_back(command);
	    }

	    void frame()
	    {
		while (!commands.empty())
		{
		    KujoGFXCommand command = commands.front();
		    commands.pop_front();
		    processCommand(command);
		}
	    }

	private:
	    KujoGFXBackendType manual_backend_type = BackendAuto;
	    KujoGFXBackendType backend_type = BackendAuto;
	    KujoGFXPlatformData platform_data;
	    unique_ptr<KujoGFXBackend> backend;

	    deque<KujoGFXCommand> commands;

	    unordered_map<uint32_t, KujoGFXPipeline> pipeline_cache;

	    bool is_initialized = false;

	    bool validatePlatformData(KujoGFXPlatformData data)
	    {
		if (data.window_handle == NULL)
		{
		    cout << "Window handle is not set" << endl;
		    return false;
		}

		if ((platform_data.context_handle != NULL) && (data.context_handle != NULL))
		{
		    cout << "Only window handle can be set after initialization!" << endl;
		    return false;
		}

		platform_data = data;
		return true;
	    }

	    void detectBackend()
	    {
		multimap<int, KujoGFXBackendType> backend_candidates;

		for (int i = 0; i < KujoGFXBackendType::BackendCount; i++)
		{
		    KujoGFXBackendType type = (KujoGFXBackendType)i;
		    int score = rateBackendSuitability(type);
		    // cout << "Score of " << backendTypeToString(type) << " backend: " << dec << int(score) << endl;
		    backend_candidates.insert(make_pair(score, type));
		}

		KujoGFXBackend *backend_ptr = NULL;

		for (auto it = backend_candidates.rbegin(); it != backend_candidates.rend(); it++)
		{
		    auto iter = *it;
		    auto type = iter.second;

		    switch (type)
		    {
			case BackendOpenGL: backend_ptr = new KujoGFX_OpenGL(); break;
			#if defined(KUJOGFX_PLATFORM_WINDOWS)
			case BackendDirect3D11: backend_ptr = new KujoGFX_D3D11(); break;
			#endif
			case BackendVulkan: backend_ptr = new KujoGFX_Vulkan(); break;
			default: backend_ptr = new KujoGFX_Null(); break;
		    }

		    if (backend_ptr != NULL)
		    {
			if (backend_ptr->initBackend(platform_data.window_handle))
			{
			    platform_data.context_handle = backend_ptr->getContextHandle();
			    backend = unique_ptr<KujoGFXBackend>(backend_ptr);
			    backend_type = type;
			    break;
			}
		    }
		}
	    }

	    string backendTypeToString(KujoGFXBackendType type)
	    {
		switch (type)
		{
		    case BackendNull: return "Null"; break;
		    case BackendOpenGL: return "OpenGL"; break;
		    case BackendVulkan: return "Vulkan"; break;
		    #if defined(KUJOGFX_PLATFORM_WINDOWS)
		    case BackendDirect3D11: return "Direct3D 11"; break;
		    #endif
		    default: return "unknown"; break;
		}
	    }

	    int rateBackendSuitability(KujoGFXBackendType type)
	    {
		int score = 0;

		if (manual_backend_type != BackendAuto)
		{
		    if (manual_backend_type == type)
		    {
			score += 100;
		    }
		}

		auto isBackendSelected = [&](KujoGFXBackendType backend_val) -> bool
		{
		    if ((manual_backend_type != BackendAuto) && (manual_backend_type == type))
		    {
			return false;
		    }
		    else
		    {
			return (type == backend_val);
		    }
		};

		#if defined(KUJOGFX_PLATFORM_WINDOWS)

		uint16_t win_version = getWindowsVersion();

		if (win_version >= 0x0602)
		{
		    if (isBackendSelected(BackendDirect3D11))
		    {
			score += 30;
		    }

		    if (isBackendSelected(BackendVulkan))
		    {
			score += 20;
		    }

		    if (isBackendSelected(BackendOpenGL))
		    {
			score += 10;
		    }
		}
		else if (win_version >= 0x0601)
		{
		    if (isBackendSelected(BackendDirect3D11))
		    {
			score += 20;
		    }

		    if (isBackendSelected(BackendOpenGL))
		    {
			score += 10;
		    }
		}
		else if (isBackendSelected(BackendOpenGL))
		{
		    score += 10;
		}

		#elif defined(KUJOGFX_PLATFORM_LINUX) || defined(KUJOGFX_PLATFORM_BSD)

		if (isBackendSelected(BackendVulkan))
		{
		    score += 20;
		}
		else if (isBackendSelected(BackendOpenGL))
		{
		    score += 10;
		}

		#elif defined(KUJOGFX_PLATFORM_MACOS)

		if (isBackendSelected(BackendVulkan))
		{
		    score += 10;
		}

		#elif defined(KUJOGFX_PLATFORM_ANDROID)

		if (isBackendSelected(BackendVulkan))
		{
		    score += 20;
		}
		else if (isBackendSelected(BackendOpenGL))
		{
		    score += 10;
		}

		#elif defined(KUJOGFX_PLATFORM_EMSCRIPTEN)
		if (isBackendSelected(BackendOpenGL))
		{
		    score += 10;
		}
		#endif
		return score;
	    }

	    uint16_t getWindowsVersion()
	    {
		#if defined(KUJOGFX_PLATFORM_WINDOWS)
		RTL_OSVERSIONINFOW os_vers;
		ZeroMemory(&os_vers, sizeof(os_vers));
		os_vers.dwOSVersionInfoSize = sizeof(os_vers);
		const HMODULE hmodule = GetModuleHandle("ntdll.dll");

		if (hmodule == NULL)
		{
		    cout << "Could not fetch ntdll handle!" << endl;
		    return 0;
		}

		FARPROC (WINAPI *rtlGetVersion_ptr)(PRTL_OSVERSIONINFOW) = reinterpret_cast<FARPROC (WINAPI*)(PRTL_OSVERSIONINFOW)>(GetProcAddress(hmodule, "RtlGetVersion"));

		if (rtlGetVersion_ptr == NULL)
		{
		    cout << "Could not fetch address of RtlGetVersion()" << endl;
		    return 0;
		}

		rtlGetVersion_ptr(&os_vers);

		if (os_vers.dwMajorVersion == 0)
		{
		    cout << "Call to rtlGetVersion() failed!" << endl;
		    return 0;
		}

		uint16_t major_version = uint8_t(os_vers.dwMajorVersion);
		uint16_t minor_version = uint8_t(os_vers.dwMinorVersion);
		return ((major_version << 8) | minor_version);
		#else
		return 0;
		#endif
	    }

	    void processCommand(KujoGFXCommand cmd)
	    {
		assert(backend != NULL);

		switch (cmd.cmd_type)
		{
		    case CommandNop: break;
		    case CommandBeginPass: backend->beginPass(cmd.current_pass); break;
		    case CommandEndPass: backend->endPass(); break;
		    case CommandCommit: backend->commitFrame(); break;
		    case CommandApplyPipeline: applyPipelineCmd(cmd.current_pipeline); break;
		    case CommandDraw: backend->draw(cmd.current_draw_call); break;
		    default:
		    {
			cout << "Unrecognized command of " << dec << int(cmd.cmd_type) << endl;
			throw runtime_error("KujoGFX error");
		    }
		    break;
		}
	    }

	    void applyPipelineCmd(KujoGFXPipeline pipeline)
	    {
		auto cached_pipeline = pipeline_cache.find(pipeline.id);

		if (cached_pipeline != pipeline_cache.end())
		{
		    backend->setPipeline(cached_pipeline->second);
		}
		else
		{
		    backend->createPipeline(pipeline);
		    pipeline_cache.insert(make_pair(pipeline.id, pipeline));
		}

		backend->applyPipeline();
	    }
    };
};


#endif // KUJOGFX_H