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
#include <sstream>
#include <cstdint>
#include <cassert>
#include <vector>
#include <deque>
#include <set>
#include <map>
#include <memory>
#include <atomic>
#include <algorithm>
#include <unordered_map>
#include <optional>
#if !defined(KUJOGFX_PLATFORM_EMSCRIPTEN)
#include <vulkan/vulkan.h>
#endif
#if defined(KUJOGFX_PLATFORM_WINDOWS)
#include <windows.h>
#include <comutil.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <dxgidebug.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <vulkan/vulkan_win32.h>
#if !defined(_uuidof)
#define _uuidof __uuidof
#endif
#if !defined(KUJOGFX_USE_GLES)
#define GLAD_WGL_IMPLEMENTATION
#include <external/glad/wgl.h>
#endif
#elif defined(KUJOGFX_PLATFORM_LINUX)
#if defined(KUJOGFX_IS_WAYLAND)
#include <vulkan/vulkan_wayland.h>
#elif defined(KUJOGFX_IS_X11)
#include <X11/Xutil.h>
#include <vulkan/vulkan_xlib.h>
#else
#error "KujoGFX could not determine the windowing platform of your system."
#endif
#define GLAD_EGL_IMPLEMENTATION
#include <external/glad/egl.h>
#endif
#if defined(KUJOGFX_USE_GLES)
#define GLAD_GLES2_IMPLEMENTATION
#define GLAD_EGL_IMPLEMENTATION
#include <external/glad/gles2.h>
#elif defined(KUJOGFX_PLATFORM_EMSCRIPTEN)
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#include <GLES3/gl3.h>
#else
#define GLAD_GL_IMPLEMENTATION
#include <external/glad/gl.h>
#endif
using namespace std;
using namespace std;

namespace kujogfx
{
    enum class KujoGFXLogLevel
    {
	Debug,
	Info,
	Warn,
	Error,
	Fatal
    };

    class KujoGFXLogStream : public ostringstream
    {
	public:
	    KujoGFXLogStream(KujoGFXLogLevel level) : log_level(level)
	    {

	    }

	    ~KujoGFXLogStream()
	    {
		#if defined(NDEBUG)
		bool is_log = (log_level != KujoGFXLogLevel::Debug);
		#else
		bool is_log = true;
		#endif

		if (is_log)
		{
		    const string s = str();
		    cout << levelToString() << ": " << s << endl;
		    cout.flush();

		    if (log_level == KujoGFXLogLevel::Fatal)
		    {
			exit(-1);
		    }
		}
	    }

	private:
	    KujoGFXLogLevel log_level = KujoGFXLogLevel::Info;

	    string levelToString()
	    {
		switch (log_level)
		{
		    case KujoGFXLogLevel::Debug: return "Debug"; break;
		    case KujoGFXLogLevel::Info: return "Info"; break;
		    case KujoGFXLogLevel::Warn: return "Warn"; break;
		    case KujoGFXLogLevel::Error: return "Error"; break;
		    case KujoGFXLogLevel::Fatal: return "Fatal"; break;
		    default: return ""; break;
		}
	    }
    };

    namespace kujogfxlog
    {
	KujoGFXLogStream debug()
	{
	    return KujoGFXLogStream(KujoGFXLogLevel::Debug);
	}

	KujoGFXLogStream info()
	{
	    return KujoGFXLogStream(KujoGFXLogLevel::Info);
	}

	KujoGFXLogStream warn()
	{
	    return KujoGFXLogStream(KujoGFXLogLevel::Warn);
	}

	KujoGFXLogStream error()
	{
	    return KujoGFXLogStream(KujoGFXLogLevel::Error);
	}

	KujoGFXLogStream fatal()
	{
	    return KujoGFXLogStream(KujoGFXLogLevel::Fatal);
	}
    };

    namespace kujogfxutil
    {
	int roundUp(int val, int round_to)
	{
	    return ((val + (round_to - 1)) & ~(round_to - 1));
	}

	uint32_t alignU32(uint32_t val, uint32_t align)
	{
	    assert((align > 0) && ((align & (align - 1)) == 0));
	    return ((val + (align - 1)) & ~(align - 1));
	}
    };

    enum KujoGFXBackendType
    {
	BackendAuto = -1,
	BackendNull = 0,
	BackendOpenGL,
	BackendDirect3D11,
	BackendDirect3D12,
	BackendVulkan,
	BackendCount
    };

    struct KujoGFXPlatformData
    {
	void *window_handle = NULL;
	void *display_handle = NULL;
	void *context_handle = NULL;
    };

    static constexpr uint32_t max_vertex_attribs = 16;
    static constexpr uint32_t max_vertex_buffer_bind_slots = 8;
    static constexpr uint32_t max_uniform_block_bind_slots = 8;

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

    enum KujoGFXIndexType : int
    {
	IndexTypeNone = 0,
	IndexTypeUint16,
	IndexTypeUint32
    };

    enum KujoGFXPrimitiveType : int
    {
	PrimitiveTriangles = 0,
    };

    enum KujoGFXCullMode : int
    {
	CullModeNone = 0,
	CullModeFront,
	CullModeBack
    };

    enum KujoGFXCompareFunc : int
    {
	CompareFuncNever = 0,
	CompareFuncLessEqual,
	CompareFuncAlways
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

    struct KujoGFXDepthAttachment
    {
	KujoGFXLoadOp load_op = LoadOpClear;
	KujoGFXStoreOp store_op = StoreOpDontCare;
	float clear_val = 1.0f;
    };

    struct KujoGFXDepthState
    {
	bool is_write_enabled = false;
	KujoGFXCompareFunc compare_func = CompareFuncNever;
    };

    struct KujoGFXPassAction
    {
	KujoGFXColorAttachment color_attach;
	KujoGFXDepthAttachment depth_attach;

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

    struct KujoGFXSemantic
    {
	string name = "";
	uint32_t index = 0;
    };

    struct KujoGFXShaderCodeDesc
    {
	string entry_name = "";
	vector<uint8_t> glsl_code;
	vector<uint8_t> glsl_es_code;
	vector<uint8_t> hlsl_5_0_code;
	vector<uint8_t> hlsl_4_0_code;
	vector<uint32_t> spv_code;
    };

    struct KujoGFXShaderCode
    {
	string entry_name = "";
	string glsl_code = "";
	string glsl_es_code = "";
	string hlsl_5_0_code = "";
	string hlsl_4_0_code = "";
	vector<uint32_t> spv_code;
    };

    struct KujoGFXShaderLocations
    {
	vector<string> glsl_names;
	vector<KujoGFXSemantic> hlsl_semantics;
	vector<uint32_t> spirv_locations;
    };

    enum KujoGFXUniformStage : int
    {
	UniformStageInvalid = 0,
	UniformStageVertex,
	UniformStageFragment
    };

    enum KujoGFXUniformLayout : int
    {
	UniformLayoutInvalid = 0,
	UniformLayoutNative,
	UniformLayoutStd140
    };

    enum KujoGFXUniformType : int
    {
	UniformTypeInvalid = 0,
	UniformTypeFloat4
    };

    struct KujoGFXGLSLUniform
    {
	KujoGFXUniformType type = UniformTypeInvalid;
	size_t array_count = 0;
	string name = "";
    };

    struct KujoGFXUniformDesc
    {
	KujoGFXUniformStage stage = UniformStageInvalid;
	KujoGFXUniformLayout layout = UniformLayoutInvalid;
	size_t desc_size = 0;
	uint32_t desc_binding = 0;
	vector<KujoGFXGLSLUniform> glsl_uniforms = {};
    };

    class KujoGFXShader
    {
	public:
	    KujoGFXShader() : id(generateID())
	    {
	    }

	    KujoGFXShader(KujoGFXShaderCodeDesc vert, KujoGFXShaderCodeDesc frag, KujoGFXShaderLocations loc, vector<KujoGFXUniformDesc> uniform = {}) : id(generateID())
	    {
		vert_code = convertCode(vert);
		frag_code = convertCode(frag);
		locations = loc;
		uniforms = uniform;
	    }

	    KujoGFXShaderCode vert_code;
	    KujoGFXShaderCode frag_code;
 	    KujoGFXShaderLocations locations;
	    vector<KujoGFXUniformDesc> uniforms;

	    uint32_t getID() const
	    {
		return id;
	    }

	private:
	    uint32_t id;

	    static atomic<uint32_t> next_id;

	    static uint32_t generateID()
	    {
		return next_id++;
	    }

	    KujoGFXShaderCode convertCode(KujoGFXShaderCodeDesc desc)
	    {
		KujoGFXShaderCode code;
		code.entry_name = desc.entry_name;
		code.glsl_code = convertVec(desc.glsl_code);
		code.glsl_es_code = convertVec(desc.glsl_es_code);
		code.hlsl_5_0_code = convertVec(desc.hlsl_5_0_code);
		code.hlsl_4_0_code = convertVec(desc.hlsl_4_0_code);
		code.spv_code = vector<uint32_t>(desc.spv_code.begin(), desc.spv_code.end());
		return code;
	    }

	    string convertVec(vector<uint8_t> &vec)
	    {
		return string(reinterpret_cast<const char*>(vec.data()), vec.size());
	    }
    };

    class KujoGFXData
    {
	public:
	    KujoGFXData()
	    {

	    }

	    void* getData()
	    {
		return data_ptr;
	    }

	    size_t getSize()
	    {
		return data_size;
	    }

	    template<typename T, size_t N>
	    void setData(T (&arr)[N])
	    {
		data_ptr = reinterpret_cast<void*>(arr);
		data_size = (N * sizeof(T));
	    }

	    template<typename T, size_t N>
	    void setData(const T (&arr)[N])
	    {
		data_ptr = reinterpret_cast<void*>(const_cast<T*>(arr));
		data_size = (N * sizeof(T));
	    }

	    template<typename T>
	    void setData(T &arr)
	    {
		data_ptr = reinterpret_cast<void*>(&arr);
		data_size = sizeof(arr);
	    }

	    template<typename T>
	    void setData(const T &arr)
	    {
		data_ptr = reinterpret_cast<void*>(const_cast<T*>(&arr));
		data_size = sizeof(arr);
	    }

	    template<typename T>
	    void setData(vector<T> &vec)
	    {
		data_ptr = reinterpret_cast<void*>(vec.data());
		data_size = (vec.size() * sizeof(T));
	    }

	    template<typename T>
	    void setData(const vector<T> &vec)
	    {
		data_ptr = reinterpret_cast<void*>(const_cast<T*>(vec.data()));
		data_size = (vec.size() * sizeof(T));
	    }

	    template<typename T, size_t N>
	    void setData(array<T, N> &arr)
	    {
		data_ptr = reinterpret_cast<void*>(arr.data());
		data_size = (arr.size() * sizeof(T));
	    }

	    template<typename T, size_t N>
	    void setData(const array<T, N> &arr)
	    {
		data_ptr = reinterpret_cast<void*>(const_cast<T*>(arr.data()));
		data_size = (arr.size() * sizeof(T));
	    }

	private:
	    void *data_ptr = NULL;
	    size_t data_size = 0;
    };

    class KujoGFXBuffer : public KujoGFXData
    {
	public:
	    KujoGFXBuffer() : id(generateID())
	    {
		setVertexBuffer();
	    }

	    bool isVertexBuffer()
	    {
		return is_vertex_buffer;
	    }

	    bool isIndexBuffer()
	    {
		return is_index_buffer;
	    }

	    void setVertexBuffer()
	    {
		is_vertex_buffer = true;
		is_index_buffer = false;
	    }

	    void setIndexBuffer()
	    {
		is_vertex_buffer = false;
		is_index_buffer = true;
	    }

	    uint32_t getID() const
	    {
		return id;
	    }

	private:
	    bool is_vertex_buffer = false;
	    bool is_index_buffer = false;

	    uint32_t id;
	    static atomic<uint32_t> next_id;

	    static uint32_t generateID()
	    {
		return next_id++;
	    }
    };

    class KujoGFXBindings
    {
	public:
	    KujoGFXBindings()
	    {
		vertex_buffer_offsets.fill(0);
		index_buffer_offset = 0;
	    }

	    array<KujoGFXBuffer, max_vertex_buffer_bind_slots> vertex_buffers;
	    array<uint32_t, max_vertex_buffer_bind_slots> vertex_buffer_offsets;
	    KujoGFXBuffer index_buffer;
	    uint32_t index_buffer_offset;
    };

    enum KujoGFXVertexFormat : int
    {
	VertexFormatInvalid = 0,
	VertexFormatFloat2,
	VertexFormatFloat3,
	VertexFormatFloat4
    };

    class KujoGFXVertexAttribute
    {
	public:
	    KujoGFXVertexAttribute()
	    {

	    }

	    KujoGFXVertexFormat format = VertexFormatInvalid;
	    size_t offset = 0;
	    size_t buffer_index = 0;
    };

    class KujoGFXVertexBufferLayout
    {
	public:
	    KujoGFXVertexBufferLayout()
	    {

	    }

	    size_t stride = 0;
    };

    class KujoGFXVertexLayout
    {
	public:
	    KujoGFXVertexLayout()
	    {
		vertex_buffer_layout_active.fill(false);
	    }

	    array<KujoGFXVertexAttribute, max_vertex_attribs> attribs;
	    array<KujoGFXVertexBufferLayout, max_vertex_buffer_bind_slots> buffers;
	    array<bool, max_vertex_buffer_bind_slots> vertex_buffer_layout_active;
    };

    class KujoGFXPipeline
    {
	public:
	    KujoGFXPipeline() : id(generateID())
	    {
		primitive_type = PrimitiveTriangles;
		index_type = IndexTypeNone;
		cull_mode = CullModeNone;
		depth_state.compare_func = CompareFuncAlways;
	    }

	    KujoGFXShader shader;
	    KujoGFXVertexLayout layout;
	    KujoGFXPrimitiveType primitive_type;
	    KujoGFXIndexType index_type;
	    KujoGFXCullMode cull_mode;
	    KujoGFXDepthState depth_state;

	    uint32_t getID() const
	    {
		return id;
	    }

	private:
	    uint32_t id;
	    static atomic<uint32_t> next_id;

	    static uint32_t generateID()
	    {
		return next_id++;
	    }
    };

    atomic<uint32_t> KujoGFXShader::next_id{1};
    atomic<uint32_t> KujoGFXBuffer::next_id{1};
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
	CommandApplyBindings,
	CommandApplyUniforms,
	CommandDraw,
	CommandCommit
    };

    struct KujoGFXCommand
    {
	KujoGFXCommandType cmd_type = CommandNop;
	KujoGFXPass current_pass;
	KujoGFXPipeline current_pipeline;
	KujoGFXDraw current_draw_call;
	KujoGFXBindings current_bindings;
	int current_uniform_slot;
	KujoGFXData current_uniform_data;

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

	KujoGFXCommand(KujoGFXBindings bindings) : cmd_type(CommandApplyBindings), current_bindings(bindings)
	{

	}

	KujoGFXCommand(int ub_slot, KujoGFXData data) : cmd_type(CommandApplyUniforms), current_uniform_slot(ub_slot), current_uniform_data(data)
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

	    virtual bool initBackend(void*, void*)
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

	    virtual void createPipeline(KujoGFXPipeline&)
	    {
		return;
	    }

	    virtual void applyPipeline()
	    {
		return;
	    }

	    virtual void createBuffer(KujoGFXBuffer)
	    {
		return;
	    }

	    virtual void applyBindings(KujoGFXBindings)
	    {
		return;
	    }

	    virtual void applyUniforms(int, KujoGFXData)
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
    };

    #if defined(KUJOGFX_PLATFORM_WINDOWS)
    class KujoGFX_D3D12 : public KujoGFXBackend
    {
	struct D3D12Pipeline
	{
	    ID3D12RootSignature *root_signature = NULL;
	    ID3D12PipelineState *pipeline_state = NULL;
	    D3D12_PRIMITIVE_TOPOLOGY topology;
	    DXGI_FORMAT index_format;
	};

	struct D3D12Buffer
	{
	    ID3D12Resource *buffer = NULL;
	    D3D12_VERTEX_BUFFER_VIEW vertex_view = {};
	    D3D12_INDEX_BUFFER_VIEW index_view = {};
	};

	public:
	    KujoGFX_D3D12()
	    {
		render_targets.fill(NULL);
		command_allocators.fill(NULL);
		fence_values.fill(0);
	    }

	    ~KujoGFX_D3D12()
	    {
	    }

	    bool initBackend(void *window_handle, void*)
	    {
		if (!initD3D12(window_handle))
		{
		    shutdownD3D12();
		    return false;
		}

		return true;
	    }

	    void shutdownBackend()
	    {
		shutdownD3D12();
	    }

	    void *getContextHandle()
	    {
		return reinterpret_cast<void*>(device);
	    }

	private:
	    void *win_handle = NULL;

	    int window_width = 0;
	    int window_height = 0;

	    static constexpr uint32_t frame_count = 3;

	    ID3D12Debug *debug = NULL;
	    IDXGIFactory4 *factory = NULL;
	    ID3D12Device *device = NULL;
	    ID3D12CommandQueue *command_queue = NULL;
	    IDXGISwapChain3 *swapchain = NULL;
	    ID3D12DescriptorHeap *rtv_heap = NULL;
	    uint32_t rtv_descriptor_size = 0;

	    size_t frame_index = 0;

	    array<ID3D12Resource*, frame_count> render_targets;
	    array<ID3D12CommandAllocator*, frame_count> command_allocators;
	    ID3D12GraphicsCommandList *command_list = NULL;
	    ID3D12Fence *fence = NULL;
	    HANDLE fence_event;
	    array<uint64_t, frame_count> fence_values;

	    unordered_map<uint32_t, D3D12Pipeline> pipelines;
	    D3D12Pipeline current_pipeline;

	    unordered_map<uint32_t, D3D12Buffer> buffers;

	    array<uint32_t, max_vertex_buffer_bind_slots> vertex_strides;

	    KujoGFXPass current_pass;

	    bool isFailed(HRESULT hres)
	    {
		if (FAILED(hres))
		{
		    shutdownD3D12();
		    return true;
		}

		return false;
	    }

	    bool fetchWindowRes()
	    {
		HWND handle = reinterpret_cast<HWND>(win_handle);

		RECT win_rect;

		if (!GetWindowRect(handle, &win_rect))
		{
		    kujogfxlog::error() << "Could not fetch window resolution!";
		    return false;
		}

		window_width = (win_rect.right - win_rect.left);
		window_height = (win_rect.bottom - win_rect.top);
		return true;
	    }

	    void safeShutdown(IUnknown *ptr)
	    {
		if (ptr != NULL)
		{
		    ptr->Release();
		    ptr = NULL;
		}
	    }

	    bool initD3D12(void *window_handle)
	    {
		win_handle = window_handle;

		if (!initDevice())
		{
		    return false;
		}

		if (!initCommandQueue())
		{
		    return false;
		}

		if (!initSwapchain())
		{
		    return false;
		}

		if (!initDescriptorHeap())
		{
		    return false;
		}

		if (!initCommandAllocators())
		{
		    return false;
		}

		if (!initCommandList())
		{
		    return false;
		}

		if (!initFence())
		{
		    return false;
		}

		return true;
	    }

	    string printHRes(HRESULT hres)
	    {
		if (SUCCEEDED(hres))
		{
		    return "";
		}

		stringstream res_str;
		_com_error err(hres);
		res_str << " HRESULT error: " << err.ErrorMessage();
		return res_str.str();
	    }

	    string printLastError()
	    {
		return printHRes(HRESULT_FROM_WIN32(GetLastError()));
	    }

	    void shutdownD3D12()
	    {
		waitForGPU();

		for (auto &iter : buffers)
		{
		    auto buffer = iter.second;
		    safeShutdown(buffer.buffer);
		}

		for (auto &iter : pipelines)
		{
		    auto pipeline = iter.second;
		    safeShutdown(pipeline.pipeline_state);
		    safeShutdown(pipeline.root_signature);
		}

		CloseHandle(fence_event);
		safeShutdown(fence);

		safeShutdown(command_list);

		for (auto &allocator : command_allocators)
		{
		    safeShutdown(allocator);
		}

		for (auto &target : render_targets)
		{
		    safeShutdown(target);
		}

		safeShutdown(rtv_heap);
		safeShutdown(swapchain);
		safeShutdown(command_queue);
		safeShutdown(device);
		safeShutdown(factory);
		safeShutdown(debug);
	    }

	    void beginPass(KujoGFXPass pass)
	    {
		if (!fetchWindowRes())
		{
		    kujogfxlog::fatal() << "Could not start pass!";
		}

		D3D12_VIEWPORT viewport = {};
		viewport.TopLeftX = 0.f;
		viewport.TopLeftY = 0.f;
		viewport.Width = static_cast<float>(window_width);
		viewport.Height = static_cast<float>(window_height);

		D3D12_RECT scissor_rect = {};
		scissor_rect.left = 0;
		scissor_rect.top = 0;
		scissor_rect.right = window_width;
		scissor_rect.bottom = window_height;

		current_pass = pass;

		auto action = current_pass.action;
		auto color_attachment = action.color_attach;

		HRESULT hres = command_allocators[frame_index]->Reset();

		if (FAILED(hres))
		{
		    kujogfxlog::fatal() << "Could not reset command allocator!" << printHRes(hres);
		}

		hres = command_list->Reset(command_allocators[frame_index], NULL);

		if (isFailed(hres))
		{
		    kujogfxlog::fatal() << "Could not reset command list!" << printHRes(hres);
		}

		command_list->RSSetViewports(1, &viewport);
		command_list->RSSetScissorRects(1, &scissor_rect);

		auto barrier = resBarrierTransition(render_targets[frame_index], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		command_list->ResourceBarrier(1, &barrier);

		D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle(rtv_heap->GetCPUDescriptorHandleForHeapStart());
		rtv_handle.ptr += (frame_index * rtv_descriptor_size);

		command_list->OMSetRenderTargets(1, &rtv_handle, false, NULL);

		if (color_attachment.load_op == LoadOpClear)
		{
		    KujoGFXColor color = color_attachment.color;
		    command_list->ClearRenderTargetView(rtv_handle, color, 0, NULL);
		}
	    }

	    void endPass()
	    {
		auto barrier = resBarrierTransition(render_targets[frame_index], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		command_list->ResourceBarrier(1, &barrier);

		HRESULT hres = command_list->Close();

		if (isFailed(hres))
		{
		    kujogfxlog::fatal() << "Could not close command list!" << printHRes(hres);
		}
	    }

	    D3D12_PRIMITIVE_TOPOLOGY_TYPE getTopologyType(KujoGFXPrimitiveType type)
	    {
		switch (type)
		{
		    case PrimitiveTriangles: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE; break;
		    default:
		    {
			kujogfxlog::fatal() << "Unrecoginzed primitive type of " << dec << int(type);
			return (D3D12_PRIMITIVE_TOPOLOGY_TYPE)0;
		    }
		    break;
		}
	    }

	    D3D12_PRIMITIVE_TOPOLOGY getTopology(KujoGFXPrimitiveType type)
	    {
		switch (type)
		{
		    case PrimitiveTriangles: return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST; break;
		    default:
		    {
			kujogfxlog::fatal() << "Unrecoginzed primitive type of " << dec << int(type);
			return (D3D12_PRIMITIVE_TOPOLOGY)0;
		    }
		    break;
		}
	    }

	    DXGI_FORMAT getIndexFormat(KujoGFXIndexType type)
	    {
		switch (type)
		{
		    case IndexTypeNone: return DXGI_FORMAT_UNKNOWN; break;
		    case IndexTypeUint16: return DXGI_FORMAT_R16_UINT; break;
		    case IndexTypeUint32: return DXGI_FORMAT_R32_UINT; break;
		    default:
		    {
			kujogfxlog::fatal() << "Unrecoginzed index format of " << dec << int(type);
			return DXGI_FORMAT_UNKNOWN;
		    }
		    break;
		}
	    }

	    void setPipeline(KujoGFXPipeline pipeline)
	    {
		auto cached_pipeline = pipelines.find(pipeline.getID());

		if (cached_pipeline == pipelines.end())
		{
		    kujogfxlog::fatal() << "Could not find current pipeline!";
		}

		current_pipeline = cached_pipeline->second;
	    }

	    void createPipeline(KujoGFXPipeline &pipeline)
	    {
		D3D12Pipeline new_pipeline;
		auto shader = pipeline.shader;

		string vertex_src = shader.vert_code.hlsl_5_0_code;
		string pixel_src = shader.frag_code.hlsl_5_0_code;
		auto semantics = shader.locations.hlsl_semantics;

		string vertex_log = "";
		string pixel_log = "";

		ID3DBlob *vert_buffer = NULL;
		ID3DBlob *pixel_buffer = NULL;

		if (!compileShader(vert_buffer, vertex_src, "vs_5_0", shader.vert_code.entry_name, vertex_log))
		{
		    stringstream log_str;
		    log_str << "Could not compile vertex shader!" << endl;
		    log_str << "Error log:" << endl;
		    log_str << vertex_log;
		    kujogfxlog::fatal() << log_str.str();
		}

		if (!compileShader(pixel_buffer, pixel_src, "ps_5_0", shader.frag_code.entry_name, pixel_log))
		{
		    stringstream log_str;
		    log_str << "Could not compile pixel shader!" << endl;
		    log_str << "Error log:" << endl;
		    log_str << pixel_log;
		    kujogfxlog::fatal() << log_str.str();
		}

		D3D12_SHADER_BYTECODE vert_bytecode = {};
		vert_bytecode.BytecodeLength = vert_buffer->GetBufferSize();
		vert_bytecode.pShaderBytecode = vert_buffer->GetBufferPointer();

		D3D12_SHADER_BYTECODE pixel_bytecode = {};
		pixel_bytecode.BytecodeLength = pixel_buffer->GetBufferSize();
		pixel_bytecode.pShaderBytecode = pixel_buffer->GetBufferPointer();

		vector<D3D12_INPUT_ELEMENT_DESC> layouts;

		for (size_t attr_index = 0; attr_index < max_vertex_attribs; attr_index++)
		{
		    auto attrib = pipeline.layout.attribs[attr_index];

		    if (attrib.format == VertexFormatInvalid)
		    {
			break;
		    }

		    D3D12_INPUT_ELEMENT_DESC element_desc = {};
		    ZeroMemory(&element_desc, sizeof(element_desc));
		    element_desc.SemanticName = semantics.at(attr_index).name.c_str();
		    element_desc.SemanticIndex = (UINT)semantics.at(attr_index).index;
		    element_desc.Format = getFormat(attrib.format);
		    element_desc.AlignedByteOffset = attrib.offset;
		    element_desc.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
		    layouts.push_back(element_desc);
		}

		D3D12_INPUT_LAYOUT_DESC input_layout_desc = {};
		input_layout_desc.NumElements = layouts.size();
		input_layout_desc.pInputElementDescs = layouts.data();

		D3D12_ROOT_SIGNATURE_DESC root_signature_desc = initRootSignatureDesc(0, NULL, 0, NULL, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		ID3DBlob *signature = NULL;

		HRESULT hres = D3D12SerializeRootSignature(&root_signature_desc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, NULL);

		if (isFailed(hres))
		{
		    kujogfxlog::fatal() << "Could not serialize root signature!" << printHRes(hres);
		}

		hres = device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&new_pipeline.root_signature));

		if (isFailed(hres))
		{
		    kujogfxlog::fatal() << "Could not create root signature!" << printHRes(hres);
		}

		D3D12_GRAPHICS_PIPELINE_STATE_DESC pso_desc = {};
		pso_desc.InputLayout = input_layout_desc;
		pso_desc.pRootSignature = new_pipeline.root_signature;
		pso_desc.VS = vert_bytecode;
		pso_desc.PS = pixel_bytecode;
		pso_desc.PrimitiveTopologyType = getTopologyType(pipeline.primitive_type);
		pso_desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		pso_desc.SampleDesc.Count = 1;
		pso_desc.SampleMask = 0xFFFFFFFF;
		pso_desc.RasterizerState = defaultRasterizerDesc();
		pso_desc.BlendState = defaultBlendDesc();
		pso_desc.NumRenderTargets = 1;

		hres = device->CreateGraphicsPipelineState(&pso_desc, IID_PPV_ARGS(&new_pipeline.pipeline_state));

		if (isFailed(hres))
		{
		    safeShutdown(new_pipeline.root_signature);
		    kujogfxlog::fatal() << "Could not create graphics pipeline state!" << printHRes(hres);
		}

		for (size_t i = 0; i < max_vertex_buffer_bind_slots; i++)
		{
		    if (pipeline.layout.vertex_buffer_layout_active[i])
		    {
			vertex_strides[i] = pipeline.layout.buffers[i].stride;
		    }
		    else
		    {
			vertex_strides[i]= 0;
		    }
		}

		vert_buffer->Release();
		pixel_buffer->Release();

		new_pipeline.topology = getTopology(pipeline.primitive_type);
		new_pipeline.index_format = getIndexFormat(pipeline.index_type);

		current_pipeline = new_pipeline;
		pipelines.insert(make_pair(pipeline.getID(), new_pipeline));
	    }

	    void applyPipeline()
	    {
		auto pipeline_state = current_pipeline.pipeline_state;
		command_list->SetGraphicsRootSignature(current_pipeline.root_signature);
		command_list->SetPipelineState(pipeline_state);
		command_list->IASetPrimitiveTopology(current_pipeline.topology);
	    }

	    void createBuffer(KujoGFXBuffer buffer)
	    {
		D3D12Buffer d3d_buffer;

		auto heap_properties = getHeapProperties(D3D12_HEAP_TYPE_UPLOAD);
		auto buffer_desc = getBufferResourceDesc(buffer.getSize());

		HRESULT hres = device->CreateCommittedResource(&heap_properties, D3D12_HEAP_FLAG_NONE, &buffer_desc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL, IID_PPV_ARGS(&d3d_buffer.buffer));

		if (isFailed(hres))
		{
		    kujogfxlog::fatal() << "Could not create upload heap!";
		}

		D3D12_RANGE read_range = {};
		read_range.Begin = 0;
		read_range.End = 0;

		uint8_t *buffer_data_begin = NULL;

		hres = d3d_buffer.buffer->Map(0, &read_range, reinterpret_cast<void**>(&buffer_data_begin));

		if (isFailed(hres))
		{
		    kujogfxlog::fatal() << "Could not map buffer data!" << endl;
		}

		memcpy(buffer_data_begin, buffer.getData(), buffer.getSize());
		d3d_buffer.buffer->Unmap(0, NULL);

		if (buffer.isIndexBuffer())
		{
		    auto &buffer_view = d3d_buffer.index_view;
		    buffer_view.BufferLocation = d3d_buffer.buffer->GetGPUVirtualAddress();
		    buffer_view.SizeInBytes = buffer.getSize();
		}
		else if (buffer.isVertexBuffer())
		{
		    auto &buffer_view = d3d_buffer.vertex_view;
		    buffer_view.BufferLocation = d3d_buffer.buffer->GetGPUVirtualAddress();
		    buffer_view.SizeInBytes = buffer.getSize();
		}

		buffers.insert(make_pair(buffer.getID(), d3d_buffer));
		return;
	    }

	    D3D12Buffer findBuffer(KujoGFXBuffer buffer)
	    {
		auto iter = buffers.find(buffer.getID());

		if (iter != buffers.end())
		{
		    return iter->second;
		}

		return {NULL, {}, {}};
	    }

	    D3D12_VERTEX_BUFFER_VIEW adjustVertexBufferView(D3D12_VERTEX_BUFFER_VIEW view, uint32_t offs)
	    {
		uint32_t prev_size = view.SizeInBytes;
		uint32_t current_size = prev_size;
		uint32_t current_offs = 0;

		if (offs >= prev_size)
		{
		    current_size = 0;
		    current_offs = prev_size;
		}
		else
		{
		    current_size = (prev_size - offs);
		    current_offs = offs;
		}

		D3D12_VERTEX_BUFFER_VIEW new_view = view;
		new_view.BufferLocation = (view.BufferLocation + current_offs);
		new_view.SizeInBytes = current_size;
		return new_view;
	    }

	    D3D12_INDEX_BUFFER_VIEW adjustIndexBufferView(D3D12_INDEX_BUFFER_VIEW view, uint32_t offs)
	    {
		uint32_t prev_size = view.SizeInBytes;
		uint32_t current_size = prev_size;
		uint32_t current_offs = 0;

		if (offs >= prev_size)
		{
		    current_size = 0;
		    current_offs = prev_size;
		}
		else
		{
		    current_size = (prev_size - offs);
		    current_offs = offs;
		}

		D3D12_INDEX_BUFFER_VIEW new_view = view;
		new_view.BufferLocation = (view.BufferLocation + current_offs);
		new_view.SizeInBytes = current_size;
		return new_view;
	    }

	    void applyBindings(KujoGFXBindings bindings)
	    {
		vector<D3D12_VERTEX_BUFFER_VIEW> buffer_views;

		for (size_t index = 0; index < bindings.vertex_buffers.size(); index++)
		{
		    auto buffer = findBuffer(bindings.vertex_buffers.at(index));

		    uint32_t buffer_offset = bindings.vertex_buffer_offsets.at(index);

		    if (buffer.buffer != NULL)
		    {
			D3D12_VERTEX_BUFFER_VIEW buffer_view = adjustVertexBufferView(buffer.vertex_view, buffer_offset);
			buffer_view.StrideInBytes = vertex_strides[index];
			buffer_views.push_back(buffer_view);
		    }
		}

		command_list->IASetVertexBuffers(0, buffer_views.size(), buffer_views.data());

		auto index_buffer = findBuffer(bindings.index_buffer);

		if (index_buffer.buffer != NULL)
		{
		    uint32_t index_offset = bindings.index_buffer_offset;
		    D3D12_INDEX_BUFFER_VIEW buffer_view = adjustIndexBufferView(index_buffer.index_view, index_offset);
		    buffer_view.Format = current_pipeline.index_format;
		    command_list->IASetIndexBuffer(&buffer_view);
		}
	    }

	    void applyUniforms(int, KujoGFXData)
	    {
		return;
	    }

	    void draw(KujoGFXDraw draw_call)
	    {
		UINT base_element = draw_call.base_element;
		UINT num_elements = draw_call.num_elements;
		UINT num_instances = draw_call.num_instances;
		bool use_indexed_draw = (current_pipeline.index_format != DXGI_FORMAT_UNKNOWN);

		if (use_indexed_draw)
		{
		    command_list->DrawIndexedInstanced(num_elements, num_instances, base_element, 0, 0);
		}
		else
		{
		    command_list->DrawInstanced(num_elements, num_instances, base_element, 0);
		}
	    }

	    void commitFrame()
	    {
		vector<ID3D12CommandList*> command_lists = {command_list};
		command_queue->ExecuteCommandLists(command_lists.size(), command_lists.data());

		HRESULT hres = swapchain->Present(0, 0);

		if (isFailed(hres))
		{
		    kujogfxlog::fatal() << "Could not present swapchain!" << printHRes(hres);
		}

		moveToNextFrame();
	    }

	    bool initDevice()
	    {
		uint32_t factory_flags = 0;

		HRESULT hres = D3D12GetDebugInterface(IID_PPV_ARGS(&debug));

		if (SUCCEEDED(hres))
		{
		    kujogfxlog::info() << "Enabling debug layer...";
		    debug->EnableDebugLayer();
		    factory_flags |= DXGI_CREATE_FACTORY_DEBUG;
		}

		hres = CreateDXGIFactory2(factory_flags, IID_PPV_ARGS(&factory));

		if (isFailed(hres))
		{
		    kujogfxlog::error() << "Could not create DXGIFactory!" << printHRes(hres);
		    return false;
		}

		IDXGIAdapter1 *adapter = NULL;

		int adapter_index = 0;

		bool is_adapter_found = false;

		while (factory->EnumAdapters1(adapter_index, &adapter) != DXGI_ERROR_NOT_FOUND)
		{
		    DXGI_ADAPTER_DESC1 desc;
		    adapter->GetDesc1(&desc);

		    if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		    {
			adapter->Release();
			adapter_index += 1;
			continue;
		    }

		    hres = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), NULL);

		    if (SUCCEEDED(hres))
		    {
			is_adapter_found = true;
			break;
		    }

		    adapter->Release();
		    adapter_index += 1;
		}

		if (!is_adapter_found)
		{
		    kujogfxlog::error() << "Could not find suitable adapter!";
		    return false;
		}

		hres = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device));

		if (isFailed(hres))
		{
		    kujogfxlog::error() << "Could not create device!" << printHRes(hres);
		    return false;
		}

		adapter->Release();
		return true;
	    }

	    bool initCommandQueue()
	    {
		D3D12_COMMAND_QUEUE_DESC command_queue_desc = {};
		command_queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		command_queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

		HRESULT hres = device->CreateCommandQueue(&command_queue_desc, IID_PPV_ARGS(&command_queue));

		if (isFailed(hres))
		{
		    kujogfxlog::error() << "Could not create command queue!" << printHRes(hres);
		    return false;
		}

		return true;
	    }

	    bool initSwapchain()
	    {
		if (!fetchWindowRes())
		{
		    return false;
		}

		HWND handle = reinterpret_cast<HWND>(win_handle);

		DXGI_SWAP_CHAIN_DESC1 swapchain_desc = {};
		swapchain_desc.Width = window_width;
		swapchain_desc.Height = window_height;
		swapchain_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapchain_desc.SampleDesc.Count = 1;
		swapchain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapchain_desc.BufferCount = frame_count;
		swapchain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

		IDXGISwapChain1 *temp_swapchain = NULL;

		HRESULT hres = factory->CreateSwapChainForHwnd(command_queue, handle, &swapchain_desc, NULL, NULL, &temp_swapchain);

		if (isFailed(hres))
		{
		    kujogfxlog::error() << "Could not create swapchain!" << printHRes(hres);
		    return false;
		}

		swapchain = static_cast<IDXGISwapChain3*>(temp_swapchain);

		frame_index = swapchain->GetCurrentBackBufferIndex();

		return true;
	    }

	    bool initDescriptorHeap()
	    {
		D3D12_DESCRIPTOR_HEAP_DESC heap_desc = {};
		heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		heap_desc.NumDescriptors = frame_count;
		heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

		HRESULT hres = device->CreateDescriptorHeap(&heap_desc, IID_PPV_ARGS(&rtv_heap));

		if (isFailed(hres))
		{
		    kujogfxlog::error() << "Could not create descriptor heap!" << printHRes(hres);
		    return false;
		}

		rtv_descriptor_size = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle(rtv_heap->GetCPUDescriptorHandleForHeapStart());

		for (size_t i = 0; i < frame_count; i++)
		{
		    hres = swapchain->GetBuffer(i, IID_PPV_ARGS(&render_targets[i]));

		    if (isFailed(hres))
		    {
			kujogfxlog::error() << "Could not create render targets!" << printHRes(hres);
			return false;
		    }

		    device->CreateRenderTargetView(render_targets[i], NULL, rtv_handle);
		    rtv_handle.ptr += rtv_descriptor_size;
		}

		return true;
	    }

	    bool initCommandAllocators()
	    {
		for (size_t i = 0; i < frame_count; i++)
		{
		    HRESULT hres = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&command_allocators[i]));

		    if (isFailed(hres))
		    {
			kujogfxlog::error() << "Could not create command allocators!" << printHRes(hres);
			return false;
		    }
		}

		return true;
	    }

	    bool initCommandList()
	    {
		HRESULT hres = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, command_allocators[frame_index], NULL, IID_PPV_ARGS(&command_list));

		if (isFailed(hres))
		{
		    kujogfxlog::error() << "Could not create command list!" << printHRes(hres);
		    return false;
		}

		command_list->Close();
		return true;
	    }

	    bool initFence()
	    {
		HRESULT hres = device->CreateFence(fence_values[frame_index], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));

		if (isFailed(hres))
		{
		    kujogfxlog::error() << "Could not create fence!" << printHRes(hres);
		    return false;
		}

		fence_values[frame_index] += 1;

		fence_event = CreateEvent(NULL, false, false, NULL);

		if (fence_event == NULL)
		{
		    kujogfxlog::error() << "Could not create fence event!" << printLastError();
		    return false;
		}

		waitForGPU();

		return true;
	    }

	    void waitForGPU()
	    {
		HRESULT hres = command_queue->Signal(fence, fence_values[frame_index]);

		if (isFailed(hres))
		{
		    kujogfxlog::fatal() << "Could not signal fence in command queue!" << printHRes(hres);
		}

		hres = fence->SetEventOnCompletion(fence_values[frame_index], fence_event);

		if (isFailed(hres))
		{
		    kujogfxlog::fatal() << "Could not set fence event!" << printHRes(hres);
		}

		WaitForSingleObjectEx(fence_event, INFINITE, false);
		fence_values[frame_index] += 1;
	    }

	    void moveToNextFrame()
	    {
		const uint64_t current_fence_value = fence_values[frame_index];

		HRESULT hres = command_queue->Signal(fence, current_fence_value);

		if (isFailed(hres))
		{
		    kujogfxlog::fatal() << "Could not signal fence in command queue!" << printHRes(hres);
		}

		frame_index = swapchain->GetCurrentBackBufferIndex();

		if (fence->GetCompletedValue() < fence_values[frame_index])
		{
		    hres = fence->SetEventOnCompletion(fence_values[frame_index], fence_event);

		    if (isFailed(hres))
		    {
			kujogfxlog::fatal() << "Could not set fence event!" << printHRes(hres);
		    }

		    WaitForSingleObjectEx(fence_event, INFINITE, false);
		}

		fence_values[frame_index] = (current_fence_value + 1);
	    }

	    D3D12_RESOURCE_BARRIER resBarrierTransition(ID3D12Resource *resource, D3D12_RESOURCE_STATES state_before, D3D12_RESOURCE_STATES state_after, uint32_t sub_resource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, D3D12_RESOURCE_BARRIER_FLAGS flags = D3D12_RESOURCE_BARRIER_FLAG_NONE)
	    {
		D3D12_RESOURCE_BARRIER barrier = {};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = flags;
		barrier.Transition.pResource = resource;
		barrier.Transition.StateBefore = state_before;
		barrier.Transition.StateAfter = state_after;
		barrier.Transition.Subresource = sub_resource;
		return barrier;
	    }

	    D3D12_ROOT_SIGNATURE_DESC initRootSignatureDesc(uint32_t num_params, const D3D12_ROOT_PARAMETER *params, uint32_t num_static_samplers, const D3D12_STATIC_SAMPLER_DESC *static_samplers = NULL, D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_NONE)
	    {
		D3D12_ROOT_SIGNATURE_DESC rs_desc = {};
		rs_desc.NumParameters = num_params;
		rs_desc.pParameters = params;
		rs_desc.NumStaticSamplers = num_static_samplers;
		rs_desc.pStaticSamplers = static_samplers;
		rs_desc.Flags = flags;
		return rs_desc;
	    }

	    D3D12_RASTERIZER_DESC defaultRasterizerDesc()
	    {
		D3D12_RASTERIZER_DESC rasterizer_desc = {};
		rasterizer_desc.FillMode = D3D12_FILL_MODE_SOLID;
		rasterizer_desc.CullMode = D3D12_CULL_MODE_BACK;
		rasterizer_desc.FrontCounterClockwise = false;
		rasterizer_desc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
		rasterizer_desc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
		rasterizer_desc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
		rasterizer_desc.DepthClipEnable = true;
		rasterizer_desc.MultisampleEnable = false;
		rasterizer_desc.AntialiasedLineEnable = false;
		rasterizer_desc.ForcedSampleCount = 0;
		rasterizer_desc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
		return rasterizer_desc;
	    }

	    D3D12_BLEND_DESC defaultBlendDesc()
	    {
		const D3D12_RENDER_TARGET_BLEND_DESC target_desc = {
		    false, false,
		    D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
		    D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
		    D3D12_LOGIC_OP_NOOP,
		    D3D12_COLOR_WRITE_ENABLE_ALL,
		};

		D3D12_BLEND_DESC blend_desc = {};
		blend_desc.AlphaToCoverageEnable = false;
		blend_desc.IndependentBlendEnable = false;

		for (uint32_t i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; i++)
		{
		    blend_desc.RenderTarget[i] = target_desc;   
		}

		return blend_desc;
	    }

	    bool compileShader(ID3DBlob* &shader, string source, string target, string entry_point, string &err_logs)
	    {
		ID3DBlob *log_blob = NULL;
		HRESULT res = D3DCompile(source.c_str(), source.length(), NULL, NULL, NULL, entry_point.c_str(), target.c_str(), 0, 0, &shader, &log_blob);

		if (FAILED(res))
		{
		    err_logs = string(reinterpret_cast<const char*>(log_blob->GetBufferPointer()), log_blob->GetBufferSize());
		    return false;
		}

		return true;
	    }

	    DXGI_FORMAT getFormat(KujoGFXVertexFormat format)
	    {
		switch (format)
		{
		    case VertexFormatFloat2: return DXGI_FORMAT_R32G32_FLOAT; break;
		    case VertexFormatFloat3: return DXGI_FORMAT_R32G32B32_FLOAT; break;
		    case VertexFormatFloat4: return DXGI_FORMAT_R32G32B32A32_FLOAT; break;
		    case VertexFormatInvalid: return DXGI_FORMAT_UNKNOWN; break;
		    default:
		    {
			kujogfxlog::fatal() << "Unrecoginzed vertex format of " << dec << int(format);
			return DXGI_FORMAT_UNKNOWN;
		    }
		    break;
		}
	    }

	    D3D12_HEAP_PROPERTIES getHeapProperties(D3D12_HEAP_TYPE type, uint32_t creation_node_mask = 1, uint32_t node_mask = 1)
	    {
		D3D12_HEAP_PROPERTIES properties = {};
		properties.Type = type;
		properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		properties.CreationNodeMask = creation_node_mask;
		properties.VisibleNodeMask = node_mask;
		return properties;
	    }

	    D3D12_RESOURCE_DESC getBufferResourceDesc(uint64_t width, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE, uint64_t alignment = 0)
	    {
		D3D12_RESOURCE_DESC resource_desc = {};
		resource_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		resource_desc.Alignment = alignment;
		resource_desc.Width = width;
		resource_desc.Height = 1;
		resource_desc.DepthOrArraySize = 1;
		resource_desc.MipLevels = 1;
		resource_desc.Format = DXGI_FORMAT_UNKNOWN;
		resource_desc.SampleDesc.Count = 1;
		resource_desc.SampleDesc.Quality = 0;
		resource_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		resource_desc.Flags = flags;
		return resource_desc;
	    }
    };

    class KujoGFX_D3D11 : public KujoGFXBackend
    {
	struct D3D11ConstBuffer
	{
	    KujoGFXUniformStage stage = UniformStageInvalid;
	    uint32_t binding = 0;
	    ID3D11Buffer *buffer = NULL;
	};

	struct D3D11Pipeline
	{
	    ID3D11VertexShader *vert_shader = NULL;
	    ID3D11PixelShader *pixel_shader = NULL;
	    ID3D11InputLayout *vert_layout = NULL;
	    ID3D11RasterizerState *raster_state = NULL;
	    ID3D11DepthStencilState *depth_stencil_state = NULL;
	    D3D_PRIMITIVE_TOPOLOGY topology;
	    DXGI_FORMAT index_format;
	    vector<D3D11ConstBuffer> cb_buffers;
	};

	public:
	    KujoGFX_D3D11()
	    {

	    }

	    ~KujoGFX_D3D11()
	    {

	    }

	    bool initBackend(void *window_handle, void*)
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
	    ID3D11Debug *d3d11_debug;
	    ID3D11InfoQueue *d3d11_debug_queue;
	    ID3D11DeviceContext *d3d11_dev_con;
	    ID3D11RenderTargetView *render_target_view;
	    ID3D11DepthStencilView *depth_stencil_view;
	    ID3D11Texture2D *depth_stencil_buffer;

	    unordered_map<uint32_t, ID3D11Buffer*> buffers;

	    vector<D3D11_INPUT_ELEMENT_DESC> layouts;
	    array<UINT, max_vertex_buffer_bind_slots> vb_strides;

	    unordered_map<uint32_t, D3D11Pipeline> pipelines;
	    D3D11Pipeline current_pipeline;

	    KujoGFXPass current_pass;

	    string hresToString(HRESULT hres)
	    {
		if (hres == S_OK)
		{
		    return "";
		}

		_com_error err(hres);
		return err.ErrorMessage();
	    }

	    uint16_t getWindowsVersion()
	    {
		RTL_OSVERSIONINFOW os_vers;
		ZeroMemory(&os_vers, sizeof(os_vers));
		os_vers.dwOSVersionInfoSize = sizeof(os_vers);
		const HMODULE hmodule = GetModuleHandle("ntdll.dll");

		if (hmodule == NULL)
		{
		    kujogfxlog::error() << "Could not fetch ntdll handle!";
		    return 0;
		}

		FARPROC (WINAPI *rtlGetVersion_ptr)(PRTL_OSVERSIONINFOW) = reinterpret_cast<FARPROC (WINAPI*)(PRTL_OSVERSIONINFOW)>(GetProcAddress(hmodule, "RtlGetVersion"));

		if (rtlGetVersion_ptr == NULL)
		{
		    kujogfxlog::error() << "Could not fetch address of RtlGetVersion()";
		    return 0;
		}

		rtlGetVersion_ptr(&os_vers);

		if (os_vers.dwMajorVersion == 0)
		{
		    kujogfxlog::error() << "Call to rtlGetVersion() failed!";
		    return 0;
		}

		uint16_t major_version = uint8_t(os_vers.dwMajorVersion);
		uint16_t minor_version = uint8_t(os_vers.dwMinorVersion);
		return ((major_version << 8) | minor_version);
	    }

	    bool isWin10OrGreater()
	    {
		return (getWindowsVersion() >= 0x0A00);
	    }

	    bool fetchWindowRes()
	    {
		HWND handle = reinterpret_cast<HWND>(win_handle);

		RECT win_rect;

		if (!GetWindowRect(handle, &win_rect))
		{
		    kujogfxlog::error() << "Could not fetch window resolution!";
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
		swapchain_desc.OutputWindow = handle;
		swapchain_desc.Windowed = TRUE;

		if (isWin10OrGreater())
		{
		    swapchain_desc.BufferCount = 2;
		    swapchain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		}
		else
		{
		    swapchain_desc.BufferCount = 1;
		    swapchain_desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		}

		UINT flags = D3D11_CREATE_DEVICE_DEBUG;

		HRESULT hres = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, flags, NULL, 0, D3D11_SDK_VERSION, &swapchain_desc, &swapchain, &d3d11_device, NULL, &d3d11_dev_con);

		if (FAILED(hres))
		{
		    kujogfxlog::error() << "Direct3D 11 could not be initialized!";
		    return false;
		}

		ID3D11Texture2D *back_buffer;
		hres = swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&back_buffer);

		if (FAILED(hres))
		{
		    kujogfxlog::error() << "Could not fetch swapchain buffer!";
		    return false;
		}

		hres = d3d11_device->CreateRenderTargetView(back_buffer, NULL, &render_target_view);

		if (FAILED(hres))
		{
		    kujogfxlog::error() << "Could not create render target view!";
		    return false;
		}

		back_buffer->Release();

		D3D11_TEXTURE2D_DESC depth_stencil_desc;
		depth_stencil_desc.Width = window_width;
		depth_stencil_desc.Height = window_height;
		depth_stencil_desc.MipLevels = 1;
		depth_stencil_desc.ArraySize = 1;
		depth_stencil_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depth_stencil_desc.SampleDesc.Count = 1;
		depth_stencil_desc.SampleDesc.Quality = 0;
		depth_stencil_desc.Usage = D3D11_USAGE_DEFAULT;
		depth_stencil_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depth_stencil_desc.CPUAccessFlags = 0;
		depth_stencil_desc.MiscFlags = 0;

		hres = d3d11_device->CreateTexture2D(&depth_stencil_desc, NULL, &depth_stencil_buffer);

		if (FAILED(hres))
		{
		    kujogfxlog::error() << "Could not create depth stencil buffer!";
		    return false;
		}

		hres = d3d11_device->CreateDepthStencilView(depth_stencil_buffer, NULL, &depth_stencil_view);

		if (FAILED(hres))
		{
		    kujogfxlog::error() << "Could not create depth stencil view!";
		    return false;
		}

		return true;
	    }

	    void shutdownD3D11()
	    {
		for (auto &iter : buffers)
		{
		    auto buffer = iter.second;
		    if (buffer != NULL)
		    {
			buffer->Release();
			buffer = NULL;
		    }
		}

		for (auto &iter : pipelines)
		{
		    auto pipeline = iter.second;

		    for (auto &buffer : pipeline.cb_buffers)
		    {
			if (buffer.buffer != NULL)
			{
			    buffer.buffer->Release();
			    buffer.buffer = NULL;
			}
		    }

		    if (pipeline.depth_stencil_state != NULL)
		    {
			pipeline.depth_stencil_state->Release();
			pipeline.depth_stencil_state = NULL;
		    }

		    if (pipeline.raster_state != NULL)
		    {
			pipeline.raster_state->Release();
			pipeline.raster_state = NULL;
		    }

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

		    if (pipeline.vert_layout != NULL)
		    {
			pipeline.vert_layout->Release();
			pipeline.vert_layout = NULL;
		    }
		}

		depth_stencil_view->Release();
		depth_stencil_buffer->Release();
		render_target_view->Release();
		swapchain->Release();
		d3d11_device->Release();
		d3d11_dev_con->Release();
	    }


	    void beginPass(KujoGFXPass pass)
	    {
		if (!fetchWindowRes())
		{
		    kujogfxlog::fatal() << "Could not start pass!";
		}

		current_pass = pass;

		auto action = current_pass.action;
		auto color_attachment = action.color_attach;
		auto depth_attachment = action.depth_attach;

		d3d11_dev_con->OMSetRenderTargets(1, &render_target_view, depth_stencil_view);

		D3D11_VIEWPORT viewport;
		ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = window_width;
		viewport.Height = window_height;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;

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

		uint32_t depth_flags = 0;
		float depth_clear = 0.f;
		uint8_t stencil_clear = 0;

		if (depth_attachment.load_op == LoadOpClear)
		{
		    depth_flags |= D3D11_CLEAR_DEPTH;
		    depth_clear = depth_attachment.clear_val;
		}

		d3d11_dev_con->ClearDepthStencilView(depth_stencil_view, depth_flags, depth_clear, stencil_clear);
	    }

	    void endPass()
	    {
		return;
	    }

	    D3D11_PRIMITIVE_TOPOLOGY getTopology(KujoGFXPrimitiveType type)
	    {
		switch (type)
		{
		    case PrimitiveTriangles: return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST; break;
		    default:
		    {
			kujogfxlog::fatal() << "Unrecoginzed primitive type of " << dec << int(type);
			return (D3D11_PRIMITIVE_TOPOLOGY)0;
		    }
		    break;
		}
	    }

	    DXGI_FORMAT getFormat(KujoGFXVertexFormat format)
	    {
		switch (format)
		{
		    case VertexFormatFloat2: return DXGI_FORMAT_R32G32_FLOAT; break;
		    case VertexFormatFloat3: return DXGI_FORMAT_R32G32B32_FLOAT; break;
		    case VertexFormatFloat4: return DXGI_FORMAT_R32G32B32A32_FLOAT; break;
		    case VertexFormatInvalid: return DXGI_FORMAT_UNKNOWN; break;
		    default:
		    {
			kujogfxlog::fatal() << "Unrecoginzed vertex format of " << dec << int(format);
			return DXGI_FORMAT_UNKNOWN;
		    }
		    break;
		}
	    }

	    DXGI_FORMAT getIndexFormat(KujoGFXIndexType type)
	    {
		switch (type)
		{
		    case IndexTypeNone: return DXGI_FORMAT_UNKNOWN; break;
		    case IndexTypeUint16: return DXGI_FORMAT_R16_UINT; break;
		    case IndexTypeUint32: return DXGI_FORMAT_R32_UINT; break;
		    default:
		    {
			kujogfxlog::fatal() << "Unrecoginzed index format of " << dec << int(type);
			return DXGI_FORMAT_UNKNOWN;
		    }
		    break;
		}
	    }

	    D3D11_CULL_MODE getCullMode(KujoGFXCullMode cull_mode)
	    {
		switch (cull_mode)
		{
		    case CullModeNone: return D3D11_CULL_NONE; break;
		    case CullModeFront: return D3D11_CULL_FRONT; break;
		    case CullModeBack: return D3D11_CULL_BACK; break;
		    default:
		    {
			kujogfxlog::fatal() << "Unimplemented cull mode of " << dec << int(cull_mode);
			return (D3D11_CULL_MODE)0;
		    }
		    break;
		}
	    }

	    D3D11_COMPARISON_FUNC getCompareFunc(KujoGFXCompareFunc comp_func)
	    {
		switch (comp_func)
		{
		    case CompareFuncNever: return D3D11_COMPARISON_NEVER; break;
		    case CompareFuncLessEqual: return D3D11_COMPARISON_LESS_EQUAL; break;
		    case CompareFuncAlways: return D3D11_COMPARISON_ALWAYS; break;
		    default:
		    {
			kujogfxlog::fatal() << "Unimplemented comparision func of " << dec << int(comp_func);
			return (D3D11_COMPARISON_FUNC)0;
		    }
		    break;
		}
	    }

	    void setPipeline(KujoGFXPipeline pipeline)
	    {
		auto cached_pipeline = pipelines.find(pipeline.getID());

		if (cached_pipeline == pipelines.end())
		{
		    kujogfxlog::fatal() << "Could not find current pipeline!";
		}

		current_pipeline = cached_pipeline->second;
	    }

	    void createPipeline(KujoGFXPipeline &pipeline)
	    {
		D3D11Pipeline new_pipeline;
		auto shader = pipeline.shader;

		string vertex_src = shader.vert_code.hlsl_4_0_code;
		string pixel_src = shader.frag_code.hlsl_4_0_code;
		auto semantics = shader.locations.hlsl_semantics;
		auto uniforms = shader.uniforms;

		size_t uniform_size = min<size_t>(max_uniform_block_bind_slots, uniforms.size());

		string vertex_log = "";
		string pixel_log = "";

		ID3DBlob *vert_buffer = NULL;
		ID3DBlob *pixel_buffer = NULL;

		if (!compileShader(vert_buffer, vertex_src, "vs_4_0", shader.vert_code.entry_name, vertex_log))
		{
		    stringstream log_str;
		    log_str << "Could not compile vertex shader!" << endl;
		    log_str << "Error log:" << endl;
		    log_str << vertex_log;
		    kujogfxlog::fatal() << log_str.str();
		}

		if (!compileShader(pixel_buffer, pixel_src, "ps_4_0", shader.frag_code.entry_name, pixel_log))
		{
		    stringstream log_str;
		    log_str << "Could not compile pixel shader!" << endl;
		    log_str << "Error log:" << endl;
		    log_str << pixel_log;
		    kujogfxlog::fatal() << log_str.str();
		}

		HRESULT hres = d3d11_device->CreateVertexShader(vert_buffer->GetBufferPointer(), vert_buffer->GetBufferSize(), NULL, &new_pipeline.vert_shader);

		if (FAILED(hres))
		{
		    kujogfxlog::fatal() << "Could not create vertex shader!";
		}

		hres = d3d11_device->CreatePixelShader(pixel_buffer->GetBufferPointer(), pixel_buffer->GetBufferSize(), NULL, &new_pipeline.pixel_shader);

		if (FAILED(hres))
		{
		    kujogfxlog::fatal() << "Could not create pixel shader!";
		}

		for (size_t attr_index = 0; attr_index < max_vertex_attribs; attr_index++)
		{
		    auto attrib = pipeline.layout.attribs[attr_index];

		    if (attrib.format == VertexFormatInvalid)
		    {
			break;
		    }

		    D3D11_INPUT_ELEMENT_DESC element_desc;
		    ZeroMemory(&element_desc, sizeof(element_desc));
		    element_desc.SemanticName = semantics.at(attr_index).name.c_str();
		    element_desc.SemanticIndex = (UINT)semantics.at(attr_index).index;
		    element_desc.Format = getFormat(attrib.format);
		    element_desc.AlignedByteOffset = attrib.offset;
		    element_desc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		    layouts.push_back(element_desc);
		}

		if (!layouts.empty())
		{
		    HRESULT hres = d3d11_device->CreateInputLayout(layouts.data(), layouts.size(), vert_buffer->GetBufferPointer(), vert_buffer->GetBufferSize(), &new_pipeline.vert_layout);

		    if (FAILED(hres))
		    {
			kujogfxlog::fatal() << "Could not create input layout!";
		    }
		}

		D3D11_RASTERIZER_DESC raster_desc;
		ZeroMemory(&raster_desc, sizeof(raster_desc));
		raster_desc.FillMode = D3D11_FILL_SOLID;
		raster_desc.CullMode = getCullMode(pipeline.cull_mode);
		raster_desc.FrontCounterClockwise = false;
		raster_desc.DepthBias = 0;
		raster_desc.DepthBiasClamp = 0.f;
		raster_desc.SlopeScaledDepthBias = 0.f;
		raster_desc.DepthClipEnable = true;
		raster_desc.ScissorEnable = true;
		raster_desc.MultisampleEnable = false;
		raster_desc.AntialiasedLineEnable = false;

		hres = d3d11_device->CreateRasterizerState(&raster_desc, &new_pipeline.raster_state);

		if (FAILED(hres))
		{
		    kujogfxlog::fatal() << "Could not create rasterizer state!";
		}

		D3D11_DEPTH_STENCIL_DESC depth_stencil_desc;
		ZeroMemory(&depth_stencil_desc, sizeof(depth_stencil_desc));
		depth_stencil_desc.DepthEnable = true;
		depth_stencil_desc.DepthWriteMask = (pipeline.depth_state.is_write_enabled) ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
		depth_stencil_desc.DepthFunc = getCompareFunc(pipeline.depth_state.compare_func);

		hres = d3d11_device->CreateDepthStencilState(&depth_stencil_desc, &new_pipeline.depth_stencil_state);

		if (FAILED(hres))
		{
		    kujogfxlog::fatal() << "Could not create depth/stencil state!";
		}

		for (size_t i = 0; i < max_vertex_buffer_bind_slots; i++)
		{
		    if (pipeline.layout.vertex_buffer_layout_active[i])
		    {
			vb_strides[i] = (UINT)pipeline.layout.buffers[i].stride;
		    }
		    else
		    {
			vb_strides[i] = 0;
		    }
		}

		new_pipeline.topology = getTopology(pipeline.primitive_type);
		new_pipeline.index_format = getIndexFormat(pipeline.index_type);

		for (size_t i = 0; i < uniform_size; i++)
		{
		    ID3D11Buffer *cb_buffer = NULL;
		    auto &uniform_block = uniforms.at(i);
		    auto stage = uniform_block.stage;

		    if (stage == UniformStageInvalid)
		    {
			continue;
		    }

		    D3D11_BUFFER_DESC cb_buffer_desc;
		    ZeroMemory(&cb_buffer_desc, sizeof(cb_buffer_desc));
		    cb_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
		    cb_buffer_desc.ByteWidth = kujogfxutil::roundUp(int(uniform_block.desc_size), 16);
		    cb_buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

		    HRESULT hres = d3d11_device->CreateBuffer(&cb_buffer_desc, NULL, &cb_buffer);

		    if (FAILED(hres))
		    {
			kujogfxlog::fatal() << "Could not create constant buffer!";
		    }

		    new_pipeline.cb_buffers.push_back({stage, uniform_block.desc_binding, cb_buffer});
		}

		vert_buffer->Release();
		pixel_buffer->Release();
		current_pipeline = new_pipeline;
		pipelines.insert(make_pair(pipeline.getID(), new_pipeline));
	    }

	    void applyPipeline()
	    {
		array<ID3D11Buffer*, 8> vs_buffers = {};
		array<ID3D11Buffer*, 8> ps_buffers = {};

		for (size_t i = 0; i < current_pipeline.cb_buffers.size(); i++)
		{
		    auto &buffer = current_pipeline.cb_buffers.at(i);
		    uint32_t register_b_n = buffer.binding;

		    switch (buffer.stage)
		    {
			case UniformStageVertex:
			{
			    if (register_b_n >= 8)
			    {
				kujogfxlog::fatal() << "Invalid vertex buffer binding of " << dec << register_b_n;
			    }

			    vs_buffers.at(register_b_n) = buffer.buffer;
			}
			break;
			case UniformStageFragment:
			{
			    if (register_b_n >= 8)
			    {
				kujogfxlog::fatal() << "Invalid fragment buffer binding of " << dec << register_b_n;
			    }

			    ps_buffers.at(register_b_n) = buffer.buffer;
			}
			break;
			default:
			{
			    kujogfxlog::fatal() << "Unrecognized buffer stage of " << dec << int(buffer.stage);
			}
			break;
		    }
		}

		d3d11_dev_con->RSSetState(current_pipeline.raster_state);
		d3d11_dev_con->OMSetDepthStencilState(current_pipeline.depth_stencil_state, 0);
		d3d11_dev_con->IASetInputLayout(current_pipeline.vert_layout);
		d3d11_dev_con->VSSetShader(current_pipeline.vert_shader, 0, 0);
		d3d11_dev_con->VSSetConstantBuffers(0, vs_buffers.size(), vs_buffers.data());
		d3d11_dev_con->PSSetShader(current_pipeline.pixel_shader, 0, 0);
		d3d11_dev_con->PSSetConstantBuffers(0, ps_buffers.size(), ps_buffers.data());
		d3d11_dev_con->IASetPrimitiveTopology(current_pipeline.topology);
	    }

	    D3D11_USAGE getUsage(KujoGFXBuffer)
	    {
		return D3D11_USAGE_DEFAULT;
	    }

	    D3D11_BIND_FLAG getBindFlags(KujoGFXBuffer buffer)
	    {
		if (buffer.isIndexBuffer())
		{
		    return D3D11_BIND_INDEX_BUFFER;
		}
		else if (buffer.isVertexBuffer())
		{
		    return D3D11_BIND_VERTEX_BUFFER;
		}
		else
		{
		    return (D3D11_BIND_FLAG)0;
		}
	    }

	    D3D11_CPU_ACCESS_FLAG getCPUAccessFlags(KujoGFXBuffer)
	    {
		return (D3D11_CPU_ACCESS_FLAG)0;
	    }

	    D3D11_RESOURCE_MISC_FLAG getMiscFlags(KujoGFXBuffer)
	    {
		return (D3D11_RESOURCE_MISC_FLAG)0;
	    }

	    void createBuffer(KujoGFXBuffer buffer)
	    {
		ID3D11Buffer *d3d_buffer = NULL;
		D3D11_BUFFER_DESC buffer_desc;
		ZeroMemory(&buffer_desc, sizeof(buffer_desc));

		buffer_desc.Usage = getUsage(buffer);
		buffer_desc.ByteWidth = (UINT)buffer.getSize();
		buffer_desc.BindFlags = getBindFlags(buffer);
		buffer_desc.CPUAccessFlags = getCPUAccessFlags(buffer);
		buffer_desc.MiscFlags = getMiscFlags(buffer);

		D3D11_SUBRESOURCE_DATA buffer_data;
		ZeroMemory(&buffer_data, sizeof(buffer_data));
		buffer_data.pSysMem = buffer.getData();

		HRESULT hres = d3d11_device->CreateBuffer(&buffer_desc, &buffer_data, &d3d_buffer);

		if (FAILED(hres))
		{
		    kujogfxlog::fatal() << "Failed to create buffer!";
		}

		buffers.insert(make_pair(buffer.getID(), d3d_buffer));
	    }

	    ID3D11Buffer *findBuffer(KujoGFXBuffer buffer)
	    {
		auto iter = buffers.find(buffer.getID());

		if (iter == buffers.end())
		{
		    return NULL;
		}

		return iter->second;
	    }

	    void applyBindings(KujoGFXBindings bindings)
	    {
		vector<ID3D11Buffer*> vertex_buffers;
		vector<UINT> vertex_buffer_strides;
		vector<UINT> vertex_buffer_offsets;

		for (size_t index = 0; index < bindings.vertex_buffers.size(); index++)
		{
		    auto &buffer = bindings.vertex_buffers.at(index);
		    ID3D11Buffer *vert_buffer = findBuffer(buffer);

		    if (vert_buffer != NULL)
		    {
			vertex_buffers.push_back(vert_buffer);
			vertex_buffer_strides.push_back(vb_strides[index]);
			vertex_buffer_offsets.push_back(bindings.vertex_buffer_offsets[index]);
		    }
		}

		d3d11_dev_con->IASetVertexBuffers(0, vertex_buffers.size(), vertex_buffers.data(), vertex_buffer_strides.data(), vertex_buffer_offsets.data());

		ID3D11Buffer *index_buffer = findBuffer(bindings.index_buffer);

		if (index_buffer != NULL)
		{
		    d3d11_dev_con->IASetIndexBuffer(index_buffer, current_pipeline.index_format, bindings.index_buffer_offset);
		}
	    }

	    void applyUniforms(int ub_slot, KujoGFXData data)
	    {
		assert((ub_slot >= 0) && (ub_slot < current_pipeline.cb_buffers.size()));
		auto &buffer = current_pipeline.cb_buffers.at(ub_slot).buffer;
		assert(buffer != NULL);
		d3d11_dev_con->UpdateSubresource(buffer, 0, NULL, data.getData(), 0, 0);
	    }

	    void draw(KujoGFXDraw draw_call)
	    {
		uint32_t base_element = draw_call.base_element;
		uint32_t num_elements = draw_call.num_elements;
		uint32_t num_instances = draw_call.num_instances;
		bool use_indexed_draw = (current_pipeline.index_format != DXGI_FORMAT_UNKNOWN);
		bool use_instanced_draw = (num_instances > 1);

		if (use_indexed_draw)
		{
		    if (use_instanced_draw)
		    {
			d3d11_dev_con->DrawIndexedInstanced(num_elements, num_instances, base_element, 0, 0);
		    }
		    else
		    {
			d3d11_dev_con->DrawIndexed(num_elements, base_element, 0);
		    }
		}
		else
		{
		    if (use_instanced_draw)
		    {
			d3d11_dev_con->DrawInstanced(num_elements, num_instances, base_element, 0);
		    }
		    else
		    {
			d3d11_dev_con->Draw(num_elements, base_element);
		    }
		}
	    }

	    void commitFrame()	
	    {
		swapchain->Present(0, 0);
	    }

	    bool compileShader(ID3DBlob* &shader, string source, string target, string entry_point, string &err_logs)
	    {
		ID3DBlob *log_blob = NULL;
		HRESULT res = D3DCompile(source.c_str(), source.length(), NULL, NULL, NULL, entry_point.c_str(), target.c_str(), 0, 0, &shader, &log_blob);

		if (FAILED(res))
		{
		    err_logs = string(reinterpret_cast<const char*>(log_blob->GetBufferPointer()), log_blob->GetBufferSize());
		    return false;
		}

		return true;
	    }
    };
    #endif

    // TODO list:
    // Implement platform-specific code for the following platforms:
    //   MacOS
    //   BSD
    //   Android
    //   Emscripten
    // Add support for index buffers

    class KujoGFX_OpenGL : public KujoGFXBackend
    {
	struct GLAttrib
	{
	    int8_t vb_index = -1;
	    uint8_t size = 0;
	    GLenum type;
	    uint8_t stride = 0;
	    int offset = 0;
	    bool is_active = false;
	};

	struct GLUniform
	{
	    KujoGFXUniformType type;
	    uint16_t count = 0;
	    uint16_t offset = 0;
	    GLint gl_loc = 0;
	};

	struct GLUniformBlock
	{
	    vector<GLUniform> uniforms;
	};

	struct GLPipeline
	{
	    GLuint program;
	    array<GLAttrib, max_vertex_attribs> attribs;
	    GLenum primitive_type;
	    GLenum index_type;
	    vector<GLUniformBlock> uniform_blocks;
	};

	public:
	    KujoGFX_OpenGL()
	    {

	    }

	    ~KujoGFX_OpenGL()
	    {

	    }

	    bool initBackend(void *window_handle, void *display_handle)
	    {
		if (!initOpenGL(window_handle, display_handle))
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
		#if defined(KUJOGFX_PLATFORM_WINDOWS) && !defined(KUJOGFX_USE_GLES)
		return reinterpret_cast<void*>(m_hrc);
		#elif defined(KUJOGFX_PLATFORM_LINUX)
		return reinterpret_cast<void*>(&m_context);
		#elif defined(KUJOGFX_PLATFORM_EMSCRIPTEN)
		return NULL;
		#else
		#error "OpenGL context handle is unimplemented for this platform"
		return NULL;
		#endif
	    }

	private:
	    #if defined(KUJOGFX_PLATFORM_EMSCRIPTEN) || defined(KUJOGFX_PLATFORM_ANDROID) || defined(KUJOGFX_USE_GLES)
	    static constexpr int gl_major_version = 3;
	    static constexpr int gl_minor_version = 0;
	    static constexpr bool use_gles = true;
	    #else
	    static constexpr int gl_major_version = 3;
	    static constexpr int gl_minor_version = 3;
	    static constexpr bool use_gles = false;
	    #endif

	    int window_width = 0;
	    int window_height = 0;
	    void *win_handle = NULL;
	    void *disp_handle = NULL;

	    unordered_map<uint32_t, GLuint> buffers;
	    uint32_t index_buffer_offset = 0;

	    size_t gl_max_vertex_attribs = 0;

	    bool loadGL()
	    {
		#if defined(KUJOGFX_USE_GLES)
		return (gladLoaderLoadGLES2() != 0);
		#elif !defined(KUJOGFX_PLATFORM_EMSCRIPTEN)
		return (gladLoaderLoadGL() != 0);
		#else
		return true;
		#endif
	    }

	    void unloadGL()
	    {
		#if defined(KUJOGFX_USE_GLES)
		gladLoaderUnloadGLES2();
		#elif !defined(KUJOGFX_PLATFORM_EMSCRIPTEN)
		gladLoaderUnloadGL();
		#else
		return;
		#endif
	    }

	    GLuint gl_vao;

	    KujoGFXPass current_pass;

	    unordered_map<uint32_t, GLPipeline> pipelines;
	    GLPipeline current_pipeline;

	    bool createGLContext()
	    {
		#if defined(KUJOGFX_PLATFORM_WINDOWS) && !defined(KUJOGFX_USE_GLES)
		return createWGLContext();
		#elif defined(KUJOGFX_PLATFORM_LINUX)
		return createEGLContext();
		#elif defined(KUJOGFX_PLATFORM_EMSCRIPTEN)
		return createEmscriptenGLContext();
		#else
		#error "OpenGL context creation is unimplemented for this platform"
		return true;
		#endif
	    }

	    void deleteGLContext()
	    {
		#if defined(KUJOGFX_PLATFORM_WINDOWS) && !defined(KUJOGFX_USE_GLES)
		deleteWGLContext();
		#elif defined(KUJOGFX_PLATFORM_LINUX)
		deleteEGLContext();
		#elif defined(KUJOGFX_PLATFORM_EMSCRIPTEN)
		#else
		#error "OpenGL context creation is unimplemented for this platform"
		#endif
	    }

	    #if defined(KUJOGFX_PLATFORM_WINDOWS) && !defined(KUJOGFX_USE_GLES)
	    HGLRC m_hrc;
	    HDC m_hdc;

	    string errorToString()
	    {
		DWORD error = GetLastError();

		if (error == 0)
		{
		    return "";
		}

		LPVOID message_buffer;
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&message_buffer, 0, NULL);

		string message(static_cast<LPCTSTR>(message_buffer));
		LocalFree(message_buffer);
		return message;
	    }
	

	    bool createWGLContext()
	    {
		m_hdc = GetDC(reinterpret_cast<HWND>(win_handle));

		PIXELFORMATDESCRIPTOR pfd;
		memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
		pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
		pfd.nVersion = 1;
		pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
		pfd.iPixelType = PFD_TYPE_RGBA;
		pfd.cColorBits = 32;
		pfd.cAlphaBits = 8;
		pfd.cDepthBits = 24;
		pfd.iLayerType = PFD_MAIN_PLANE;

		int nPixelFormat = ChoosePixelFormat(m_hdc, &pfd);

		if (nPixelFormat == 0)
		{
		    kujogfxlog::error() << "Could not choose pixel format! Win32 error: " << errorToString();
		    return false;
		}

		BOOL bResult = SetPixelFormat(m_hdc, nPixelFormat, &pfd);

		if (!bResult)
		{
		    kujogfxlog::error() << "Could not set pixel format!";
		    return false;
		}

		HGLRC tempContext = wglCreateContext(m_hdc);
		wglMakeCurrent(m_hdc, tempContext);

		int version = gladLoaderLoadWGL(m_hdc);

		if (version == 0)
		{
		    wglMakeCurrent(NULL, NULL);
		    wglDeleteContext(tempContext);
		    kujogfxlog::error() << "Could not load WGL functions!";
		    return false;
		}

		int attribs[] = 
		{
		    WGL_CONTEXT_MAJOR_VERSION_ARB, gl_major_version,
		    WGL_CONTEXT_MINOR_VERSION_ARB, gl_minor_version,
		    #ifndef NDEBUG
		    WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB,
		    #else
		    WGL_CONTEXT_FLAGS_ARB, 0,
		    #endif
		    0
		};

		m_hrc = wglCreateContextAttribsARB(m_hdc, 0, attribs);
		wglMakeCurrent(NULL, NULL);
		wglDeleteContext(tempContext);
		wglMakeCurrent(m_hdc, m_hrc);

		if (!m_hrc)
		{
		    kujogfxlog::error() << "Could not create WGL context!";
		    return false;
		}

		if (!loadGL())
		{
		    wglMakeCurrent(NULL, NULL);
		    wglDeleteContext(m_hrc);
		    kujogfxlog::error() << "Could not load OpenGL functions!";
		    return false;
		}

		kujogfxlog::info() << "OpenGL version found: " << glGetString(GL_VERSION);
		return true;
	    }

	    void deleteWGLContext()
	    {
		unloadGL();
		wglMakeCurrent(NULL, NULL);

		if (m_hrc)
		{
		    wglDeleteContext(m_hrc);
		    m_hrc = NULL;
		}
	    }

	    #elif defined(KUJOGFX_PLATFORM_LINUX)
	    EGLDisplay m_display;
	    EGLSurface m_surface;
	    EGLContext m_context;

	    bool createEGLContext()
	    {
		int egl_version = gladLoaderLoadEGL(NULL);

		if (egl_version == 0)
		{
		    kujogfxlog::error() << "Could not load EGL!";
		    return false;
		}

		#if defined(KUJOGFX_PLATFORM_WINDOWS)
		disp_handle = GetDC(reinterpret_cast<HWND>(win_handle));
		#endif

		auto display_type = reinterpret_cast<EGLNativeDisplayType>(disp_handle);
		auto window_type = reinterpret_cast<EGLNativeWindowType>(win_handle);

		m_display = eglGetDisplay(display_type);

		if (m_display == EGL_NO_DISPLAY)
		{
		    kujogfxlog::error() << "Could not fetch EGL display!";
		    return false;
		}

		if (!eglInitialize(m_display, NULL, NULL))
		{
		    kujogfxlog::error() << "Could not initialize EGL!";
		    return false;
		}

		egl_version = gladLoaderLoadEGL(m_display);

		if (egl_version == 0)
		{
		    kujogfxlog::error() << "Could not reload EGL!";
		    return false;
		}

		#if defined(KUJOGFX_USE_GLES)
		EGLenum bind_api =  EGL_OPENGL_ES_API;
		EGLint attrib_bit = EGL_OPENGL_ES2_BIT;
		#else
		EGLenum bind_api = EGL_OPENGL_API;
		EGLint attrib_bit = EGL_OPENGL_BIT;
		#endif


		if (!eglBindAPI(bind_api))
		{
		    kujogfxlog::error() << "Could not bind OpenGL API!";
		    return false;
		}

		EGLint config_attrib[] = 
		{
		    EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		    EGL_CONFORMANT, attrib_bit,
		    EGL_RENDERABLE_TYPE, attrib_bit,
		    EGL_COLOR_BUFFER_TYPE, EGL_RGB_BUFFER,
		    EGL_RED_SIZE, 8,
		    EGL_GREEN_SIZE, 8,
		    EGL_BLUE_SIZE, 8,
		    EGL_ALPHA_SIZE, 8,
		    EGL_NONE
		};

		EGLint config_count;

		EGLConfig config;

		if (!eglChooseConfig(m_display, config_attrib, &config, 1, &config_count))
		{
		    kujogfxlog::error() << "Could not select configuration!";
		    return false;
		}

		if (config_count != 1)
		{
		    kujogfxlog::error() << "Could not find appropriate configuration!";
		    return false;
		}

		EGLint surface_attribs[] =
		{
		    EGL_GL_COLORSPACE, EGL_GL_COLORSPACE_LINEAR,
		    EGL_RENDER_BUFFER, EGL_BACK_BUFFER,
		    EGL_NONE
		};

		m_surface = eglCreateWindowSurface(m_display, config, window_type, surface_attribs);

		if (m_surface == EGL_NO_SURFACE)
		{
		    kujogfxlog::error() << "Could not create EGL surface!";
		    return false;
		}

		EGLint context_attribs[] =
		{
		    EGL_CONTEXT_MAJOR_VERSION_KHR, gl_major_version,
		    EGL_CONTEXT_MINOR_VERSION_KHR, gl_minor_version,
		    #if !defined(KUJOGFX_USE_GLES)
		    EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,
		    #endif
		    #ifndef NDEBUG
		    EGL_CONTEXT_OPENGL_DEBUG, EGL_TRUE,
		    #endif
		    EGL_NONE
		};

		m_context = eglCreateContext(m_display, config, EGL_NO_CONTEXT, context_attribs);

		if (m_context == EGL_NO_CONTEXT)
		{
		    kujogfxlog::error() << "Could not create EGL context!";
		    return false;
		}

		if (!eglMakeCurrent(m_display, m_surface, m_surface, m_context))
		{
		    kujogfxlog::error() << "Could not make EGL context current!";
		    return false;
		}

		if (!loadGL())
		{
		    kujogfxlog::error() << "Could not load OpenGL functions!";
		    return false;
		}

		kujogfxlog::info() << "OpenGL version found: " << glGetString(GL_VERSION) << endl;
		return true;
	    }

	    void deleteEGLContext()
	    {
		unloadGL();
		eglMakeCurrent(m_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
		eglDestroyContext(m_display, m_context);
		eglDestroySurface(m_display, m_surface);
		eglTerminate(m_display);

		gladLoaderUnloadEGL();
	    }

	    #elif defined(KUJOGFX_PLATFORM_EMSCRIPTEN)
	    
	    bool createEmscriptenGLContext()
	    {
		const char *canvas_name = reinterpret_cast<const char*>(win_handle);

		EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx;
		EmscriptenWebGLContextAttributes attribs;
		emscripten_webgl_init_context_attributes(&attribs);
		attribs.majorVersion = 2;
		attribs.minorVersion = 0;
		ctx = emscripten_webgl_create_context(canvas_name, &attribs);

		if (ctx == 0)
		{
		    kujogfxlog::error() << "Could not create Emscripten WebGL context!";
		    return false;
		}
	
		emscripten_webgl_make_context_current(ctx);
		return true;
	    }

	    #endif

	    bool fetchWindowRes()
	    {
		#if defined(KUJOGFX_PLATFORM_WINDOWS)
		HWND handle = reinterpret_cast<HWND>(win_handle);

		RECT win_rect;

		if (!GetWindowRect(handle, &win_rect))
		{
		    kujogfxlog::error() << "Could not fetch window resolution!";
		    return false;
		}

		window_width = (win_rect.right - win_rect.left);
		window_height = (win_rect.bottom - win_rect.top);
		#elif defined(KUJOGFX_PLATFORM_LINUX)
		#if defined(KUJOGFX_IS_X11)
		Display *dpy = reinterpret_cast<Display*>(disp_handle);
		Window win = reinterpret_cast<Window>(win_handle);

		XWindowAttributes attrib;
		XGetWindowAttributes(dpy, win, &attrib);

		window_width = attrib.width;
		window_height = attrib.height;
		#else
		#error "OpenGL window resolution fetch is unimplemented on Wayland"
		#endif
		#elif defined(KUJOGFX_PLATFORM_EMSCRIPTEN)

		const char *canvas_name = reinterpret_cast<const char*>(win_handle);

		EMSCRIPTEN_RESULT res = emscripten_get_canvas_element_size(canvas_name, &window_width, &window_height);

		if (res != EMSCRIPTEN_RESULT_SUCCESS)
		{
		    kujogfxlog::error() << "Could not fetch canvas size" << endl;
		    return false;
		}

		#else
		#error "OpenGL window resolution fetch is unimplemented for this platform"
		#endif

		return true;
	    }

	    bool initOpenGL(void *window_handle, void *display_handle)
	    {
		win_handle = window_handle;
		disp_handle = display_handle;

		if (!fetchWindowRes())
		{
		    return false;
		}

		if (!createGLContext())
		{
		    kujogfxlog::error() << "Could not create OpenGL context!";
		    return false;
		}

		// TODO: Figure out alternative for MacOS, which doesn't support any debug extensions
		/*
		#if !defined(NDEBUG)
		auto debug_cb = [](GLenum source, GLenum type, GLuint, GLenum severity, GLsizei, const GLchar *msg, const void*) -> void
		{
		    cout << "Source: " << hex << int(source) << endl;
		    cout << "Type: " << hex << int(type) << endl;
		    cout << "Severity: " << hex << int(severity) << endl;
		    cout << "Message: " << string(msg) << endl;
		    cout << endl;
		};

		glDebugMessageCallback(debug_cb, NULL);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		#endif
		*/

		if (!initLimits())
		{
		    kujogfxlog::error() << "Could not initialize OpenGL limits!";
		    return false;
		}

		glGenVertexArrays(1, &gl_vao);
		glBindVertexArray(gl_vao);

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_ALWAYS);
		glDepthMask(GL_FALSE);
		glDisable(GL_STENCIL_TEST);
		glDisable(GL_BLEND);
		glDisable(GL_CULL_FACE);
		glFrontFace(GL_CW);
		glCullFace(GL_BACK);
		glEnable(GL_SCISSOR_TEST);

		return true;
	    }

	    string glErrorToString(GLenum code)
	    {
		stringstream err_str;

		switch (code)
		{
		    case GL_INVALID_ENUM: err_str << "Invalid enum"; break;
		    case GL_INVALID_VALUE: err_str << "Invalid value"; break;
		    case GL_INVALID_OPERATION: err_str << "Invalid operation"; break;
		    case GL_OUT_OF_MEMORY: err_str << "Out of memory"; break;
		    case GL_INVALID_FRAMEBUFFER_OPERATION: err_str << "Invalid framebuffer operation" << endl;
		    default:
		    {
			err_str << "Error code of " << hex << int(code);
		    }
		    break; 
		}

		return err_str.str();
	    }

	    bool checkErrors(string msg)
	    {
		GLenum error_code = glGetError();

		if (error_code != GL_NO_ERROR)
		{
		    kujogfxlog::error() << msg << " OpenGL error: " << glErrorToString(error_code);
		    return false;
		}

		return true;
	    }

	    bool initLimits()
	    {
		if (!checkErrors("Could not initialize OpenGL!"))
		{
		    return false;
		}

		GLint gl_int_val;
		glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &gl_int_val);

		if (!checkErrors("Could not fetch max vertex attributes!"))
		{
		    return false;
		}

		gl_max_vertex_attribs = (uint32_t)gl_int_val;

		if (gl_max_vertex_attribs > max_vertex_attribs)
		{
		    gl_max_vertex_attribs = max_vertex_attribs;
		}

		kujogfxlog::info() << "Maximum vertex attributes: " << dec << int(gl_max_vertex_attribs) << endl;

		return true;
	    }

	    void shutdownOpenGL()
	    {
		for (auto &iter : buffers)
		{
		    auto buffer = iter.second;

		    if (glIsBuffer(buffer))
		    {
			glDeleteBuffers(1, &buffer);
		    }
		}

		for (auto &iter : pipelines)
		{
		    auto pipeline = iter.second;
		    if (isProgramDelete(pipeline.program))
		    {
			glDeleteProgram(pipeline.program);
		    }
		}

		glBindVertexArray(0);

		if (gl_vao)
		{
		    glDeleteVertexArrays(1, &gl_vao);
		}

		deleteGLContext();
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
		    kujogfxlog::fatal() << "Could not fetch window resolution!";
		}

		glViewport(0, 0, window_width, -window_height);
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

	    uint8_t getSize(KujoGFXVertexFormat format)
	    {
		switch (format)
		{
		    case VertexFormatFloat2: return 2; break;
		    case VertexFormatFloat3: return 3; break;
		    case VertexFormatFloat4: return 4; break;
		    case VertexFormatInvalid: return 0; break;
		    default:
		    {
			kujogfxlog::fatal() << "Unrecoginzed size fetch vertex format of " << dec << int(format);
			return 0;
		    }
		    break;
		}
	    }

	    GLenum getType(KujoGFXVertexFormat format)
	    {
		switch (format)
		{
		    case VertexFormatFloat2:
		    case VertexFormatFloat3:
		    case VertexFormatFloat4: return GL_FLOAT; break;
		    case VertexFormatInvalid: return 0; break;
		    default:
		    {
			kujogfxlog::fatal() << "Unrecoginzed type fetch vertex format of " << dec << int(format);
			return 0;
		    }
		    break;
		}
	    }

	    GLenum getPrimitiveType(KujoGFXPrimitiveType type)
	    {
		switch (type)
		{
		    case PrimitiveTriangles: return GL_TRIANGLES; break;
		    default:
		    {
			kujogfxlog::fatal() << "Unrecoginzed primitive type of " << dec << int(type);
			return 0;
		    }
		    break;
		}
	    }

	    GLenum getIndexType(KujoGFXIndexType type)
	    {
		switch (type)
		{
		    case IndexTypeNone: return 0; break;
		    case IndexTypeUint16: return GL_UNSIGNED_SHORT; break;
		    case IndexTypeUint32: return GL_UNSIGNED_INT; break;
		    default:
		    {
			kujogfxlog::fatal() << "Unrecoginzed index type of " << dec << int(type);
			return 0;
		    }
		    break;
		}
	    }

	    uint32_t getUniformAlignment(KujoGFXUniformType type, size_t array_count, KujoGFXUniformLayout ub_layout)
	    {
		if (ub_layout == UniformLayoutNative)
		{
		    return 1;
		}
		else
		{
		    assert(array_count > 0);

		    if (array_count == 1)
		    {
			kujogfxlog::fatal() << "Unimplemented: alignment array count of 1";
			return 0;
		    }
		    else
		    {
			return 16;
		    }
		}
	    }

	    uint32_t getUniformSize(KujoGFXUniformType type, size_t array_count, KujoGFXUniformLayout ub_layout)
	    {
		assert(array_count > 0);

		if (array_count == 1)
		{
		    kujogfxlog::fatal() << "Unimplemented: size array count of 1" << endl;
		    return 0;
		}
		else
		{
		    if (ub_layout == UniformLayoutNative)
		    {
			kujogfxlog::fatal() << "Unimplemented: size layout of UniformLayoutNative" << endl;
			return 0;
		    }
		    else
		    {
			switch (type)
			{
			    case UniformTypeFloat4:
			    {
				return (16 * uint32_t(array_count));
			    }
			    break;
			    default:
			    {
				kujogfxlog::fatal() << "Unimplemented UniformLayoutStd140 uniform type of " << dec << int(type);
				return 0;
			    }
			    break;
			}
		    }
		}
	    }

	    GLenum getCompareFunc(KujoGFXCompareFunc comp_func)
	    {
		switch (comp_func)
		{
		    case CompareFuncNever: return GL_NEVER; break;
		    case CompareFuncLessEqual: return GL_LEQUAL; break;
		    case CompareFuncAlways: return GL_ALWAYS; break;
		    default:
		    {
			kujogfxlog::fatal() << "Unimplemented comparision func of " << dec << int(comp_func);
			return 0;
		    }
		    break;
		}
	    }

	    void setPipeline(KujoGFXPipeline pipeline)
	    {
		auto cached_pipeline = pipelines.find(pipeline.getID());

		if (cached_pipeline == pipelines.end())
		{
		    kujogfxlog::fatal() << "Could not find current pipeline!";
		}

		current_pipeline = cached_pipeline->second;
	    }

	    void createPipeline(KujoGFXPipeline &pipeline)
	    {
		GLPipeline new_pipeline;
		auto shader = pipeline.shader;

		string vert_source = (use_gles) ? shader.vert_code.glsl_es_code : shader.vert_code.glsl_code;
		string frag_source = (use_gles) ? shader.frag_code.glsl_es_code : shader.frag_code.glsl_code;
		auto names = shader.locations.glsl_names;
		auto uniforms = shader.uniforms;

		size_t uniform_size = min<size_t>(max_uniform_block_bind_slots, uniforms.size());

		string vert_log = "";
		string frag_log = "";

		GLuint vertex_shader;
		GLuint fragment_shader;
		GLuint shader_program;

		if (!compileShader(vertex_shader, GL_VERTEX_SHADER, vert_source, vert_log))
		{
		    stringstream log_str;
		    log_str << "Could not compile vertex shader!" << endl;
		    log_str << "Error log: " << endl;
		    log_str << vert_log;
		    kujogfxlog::fatal() << log_str.str();
		}

		if (!compileShader(fragment_shader, GL_FRAGMENT_SHADER, frag_source, frag_log))
		{
		    stringstream log_str;
		    log_str << "Could not compile fragment shader!" << endl;
		    log_str << "Error log: " << endl;
		    log_str << frag_log;
		    kujogfxlog::fatal() << log_str.str();
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

		    stringstream log_str;
		    log_str << "Could not link program!" << endl;
		    log_str << "Error log: " << endl;
		    log_str << program_log;
		    kujogfxlog::fatal() << log_str.str();
		}

		glDeleteShader(vertex_shader);
		glDeleteShader(fragment_shader);

		// NOTE: GLSL compilers may remove unused vertex attributes, so we can't rely
		// on the 'prepoulated' vertex_buffer_layout_active[] state and need to
		// manually fill out that array from scratch with the necessary info
		// after the GLSL compilation
		for (size_t i = 0; i < max_vertex_buffer_bind_slots; i++)
		{
		    pipeline.layout.vertex_buffer_layout_active[i] = false;
		}

		for (size_t attr = 0; attr < max_vertex_attribs; attr++)
		{
		    new_pipeline.attribs[attr].vb_index = -1;
		}

		for (size_t attr = 0; attr < gl_max_vertex_attribs; attr++)
		{
		    auto attrib = pipeline.layout.attribs[attr];

		    if (attrib.format == VertexFormatInvalid)
		    {
			break;
		    }

		    auto buffer = pipeline.layout.buffers[attrib.buffer_index];

		    GLint attr_loc = attr;

		    if (!names[attr].empty())
		    {
			attr_loc = glGetAttribLocation(shader_program, names[attr].c_str());
		    }

		    if (attr_loc != -1)
		    {
			assert(attr_loc < (GLint)gl_max_vertex_attribs);
			auto &gl_attr = new_pipeline.attribs[attr_loc];
			assert(gl_attr.vb_index == -1);
			gl_attr.vb_index = int8_t(attrib.buffer_index);
			assert(gl_attr.stride > 0);
			gl_attr.stride = uint8_t(buffer.stride);
			gl_attr.offset = attrib.offset;
			gl_attr.size = getSize(attrib.format);
			gl_attr.type = getType(attrib.format);
			pipeline.layout.vertex_buffer_layout_active[attr] = true;
		    }
		}

		new_pipeline.primitive_type = getPrimitiveType(pipeline.primitive_type);
		new_pipeline.index_type = getIndexType(pipeline.index_type);
		new_pipeline.program = shader_program;

		if (pipeline.cull_mode == CullModeNone)
		{
		    glDisable(GL_CULL_FACE);
		}
		else
		{
		    glEnable(GL_CULL_FACE);
		    GLenum cull_mode = (pipeline.cull_mode == CullModeFront) ? GL_FRONT : GL_BACK;
		    glCullFace(cull_mode);
		}

		glDepthFunc(getCompareFunc(pipeline.depth_state.compare_func));
		glDepthMask(pipeline.depth_state.is_write_enabled);

		uint32_t uniform_offs = 0;

		for (size_t ub_index = 0; ub_index < uniform_size; ub_index++)
		{
		    GLUniformBlock gl_block;
		    auto &uniform_block = uniforms.at(ub_index);
		    auto stage = uniform_block.stage;

		    if (stage == UniformStageInvalid)
		    {
			continue;
		    }

		    auto glsl_uniforms = uniform_block.glsl_uniforms;
		    size_t glsl_uniform_size = min<size_t>(16, glsl_uniforms.size());

		    for (size_t u_index = 0; u_index < glsl_uniform_size; u_index++)
		    {
			auto &glsl_uniform = glsl_uniforms.at(u_index);

			if (glsl_uniform.type == UniformTypeInvalid)
			{
			    continue;
			}

			uint32_t u_align = getUniformAlignment(glsl_uniform.type, glsl_uniform.array_count, uniform_block.layout);
			uint32_t u_size = getUniformSize(glsl_uniform.type, glsl_uniform.array_count, uniform_block.layout);
			uniform_offs = kujogfxutil::alignU32(uniform_offs, u_align);

			GLUniform gl_uniform;
			gl_uniform.type = glsl_uniform.type;
			gl_uniform.count = uint16_t(glsl_uniform.array_count);
			gl_uniform.offset = uint16_t(uniform_offs);
			assert(!glsl_uniform.name.empty());
			gl_uniform.gl_loc = glGetUniformLocation(new_pipeline.program, glsl_uniform.name.c_str());

			if (gl_uniform.gl_loc == -1)
			{
			    kujogfxlog::warn() << "Uniform block name of " << glsl_uniform.name << " was not found in provided shader.";
			}

			gl_block.uniforms.push_back(gl_uniform);


			uniform_offs += u_size;
		    }

		    new_pipeline.uniform_blocks.push_back(gl_block);
		}

		pipelines.insert(make_pair(pipeline.getID(), new_pipeline));
		current_pipeline = new_pipeline;
	    }

	    void applyPipeline()
	    {
		glUseProgram(current_pipeline.program);
	    }

	    GLenum getTarget(KujoGFXBuffer buffer)
	    {
		if (buffer.isIndexBuffer())
		{
		    return GL_ELEMENT_ARRAY_BUFFER;
		}
		else if (buffer.isVertexBuffer())
		{
		    return GL_ARRAY_BUFFER;
		}
		else
		{
		    return (GLenum)0;
		}
	    }

	    GLenum getUsage(KujoGFXBuffer)
	    {
		return GL_STATIC_DRAW;
	    }

	    void createBuffer(KujoGFXBuffer buffer)
	    {
		auto target = getTarget(buffer);
		auto usage = getUsage(buffer);

		GLuint gl_buffer;

		glGenBuffers(1, &gl_buffer);
		glBindBuffer(target, gl_buffer);
		glBufferData(target, buffer.getSize(), NULL, usage);

		auto data = buffer.getData();

		if (data != NULL)
		{
		    glBufferSubData(target, 0, buffer.getSize(), data);
		}

		buffers.insert(make_pair(buffer.getID(), gl_buffer));
	    }

	    GLuint findBuffer(KujoGFXBuffer buffer)
	    {
		auto iter = buffers.find(buffer.getID());

		if (iter == buffers.end())
		{
		    return (GLuint)0;
		}

		return iter->second;
	    }

	    void applyBindings(KujoGFXBindings bindings)
	    {
		for (size_t attr = 0; attr < gl_max_vertex_attribs; attr++)
		{
		    auto &attrib = current_pipeline.attribs[attr];

		    bool is_enable_attrib = false;

		    if (attrib.vb_index >= 0)
		    {
			auto &buffer = bindings.vertex_buffers.at(attrib.vb_index);

			GLuint vert_buffer = findBuffer(buffer);

			if (glIsBuffer(vert_buffer))
			{
			    is_enable_attrib = true;
			}
		    }

		    if (is_enable_attrib)
		    {
			uint32_t buffer_offset = bindings.vertex_buffer_offsets.at(attrib.vb_index);
			void *offset = reinterpret_cast<void*>(attrib.offset + buffer_offset);
			glVertexAttribPointer(attr, attrib.size, attrib.type, GL_FALSE, attrib.stride, offset);
			glEnableVertexAttribArray(attr);
		    }
		    else
		    {
			glDisableVertexAttribArray(attr);
		    }
		}

		index_buffer_offset = bindings.index_buffer_offset;

		GLuint index_buffer = findBuffer(bindings.index_buffer);

		if (glIsBuffer(index_buffer))
		{
		    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
		}
	    }

	    void applyUniforms(int ub_slot, KujoGFXData data)
	    {
		assert((ub_slot >= 0) && (ub_slot < current_pipeline.uniform_blocks.size()));

		auto &ub_block = current_pipeline.uniform_blocks.at(ub_slot);

		for (size_t i = 0; i < ub_block.uniforms.size(); i++)
		{
		    auto &uniform = ub_block.uniforms.at(i);
		    assert(uniform.type != UniformTypeInvalid);

		    if (uniform.gl_loc == -1)
		    {
			continue;
		    }

		    uint8_t *data_ptr = reinterpret_cast<uint8_t*>(data.getData());

		    float *ptr_float = reinterpret_cast<float*>(data_ptr + uniform.offset);

		    switch (uniform.type)
		    {
			case UniformTypeInvalid: break;
			case UniformTypeFloat4:
			{
			    glUniform4fv(uniform.gl_loc, uniform.count, ptr_float);
			}
			break;
			default:
			{
			    kujogfxlog::fatal() << "Unimplemented uniform type of " << dec << int(uniform.type);
			}
			break;
		    }
		}
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
		int base_element = draw_cmd.base_element;
		int num_elements = draw_cmd.num_elements;
		int num_instances = draw_cmd.num_instances;

		bool use_instanced_draw = (num_instances > 1);

		if (current_pipeline.index_type != 0)
		{
		    const int i_size = (current_pipeline.index_type == GL_UNSIGNED_SHORT) ? 2 : 4;
		    const void* indices = reinterpret_cast<const void*>((base_element * i_size) + index_buffer_offset);
		    if (use_instanced_draw)
		    {
			glDrawElementsInstanced(current_pipeline.primitive_type, num_elements, current_pipeline.index_type, indices, num_instances);
		    }
		    else
		    {
			glDrawElements(current_pipeline.primitive_type, num_elements, current_pipeline.index_type, indices);
		    }
		}
		else
		{
		    if (use_instanced_draw)
		    {
			glDrawArraysInstanced(current_pipeline.primitive_type, base_element, num_elements, num_instances);
		    }
		    else
		    {
			glDrawArrays(current_pipeline.primitive_type, base_element, num_elements);
		    }
		}
	    }

	    void commitFrame()
	    {
		#if defined(KUJOGFX_PLATFORM_WINDOWS) && !defined(KUJOGFX_USE_GLES)
		SwapBuffers(m_hdc);
		#elif defined(KUJOGFX_PLATFORM_EMSCRIPTEN)
		#else
		eglSwapBuffers(m_display, m_surface);
		#endif
	    }

	    void printErrors()
	    {
		GLenum err;

		while (true)
		{
		    err = glGetError();

		    if (err == GL_NO_ERROR)
		    {
			break;
		    }

		    kujogfxlog::error() << "OpenGL error code of " << hex << int(err) << " detected" << endl;
		}
	    }
    };

    #if !defined(KUJOGFX_PLATFORM_EMSCRIPTEN)
    class KujoGFX_Vulkan : public KujoGFXBackend
    {
	struct VulkanMemory
	{
	    VkDeviceMemory memory = VK_NULL_HANDLE;
	    uint32_t offset = 0;
	};

	struct VulkanBuffer
	{
	    VkBuffer buffer = VK_NULL_HANDLE;
	    VulkanMemory memory;
	};

	struct VulkanPipeline
	{
	    VkPipeline pipeline = VK_NULL_HANDLE;
	    VkPipelineLayout layout = VK_NULL_HANDLE;
	    VkIndexType index_type = VK_INDEX_TYPE_UINT16;
	    bool is_index_active = false;
	};

	public:
	    KujoGFX_Vulkan()
	    {
		api_version = getAPIVersion();
	    }

	    ~KujoGFX_Vulkan()
	    {

	    }

	    bool initBackend(void *window_handle, void *display_handle)
	    {
		if (!initVulkan(window_handle, display_handle))
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
		return NULL;
	    }

	private:
	    void *win_handle = NULL;
	    void *disp_handle = NULL;
	    uint32_t window_width = 0;
	    uint32_t window_height = 0;

	    static constexpr int max_frames_in_flight = 2;

	    VkInstance instance = VK_NULL_HANDLE;
	    VkSurfaceKHR surface = VK_NULL_HANDLE;
	    VkPhysicalDevice physical_device = VK_NULL_HANDLE;
	    VkDevice device = VK_NULL_HANDLE;
	    VkQueue graphics_queue = VK_NULL_HANDLE;
	    VkQueue present_queue = VK_NULL_HANDLE;
	    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
	    vector<VkImage> swapchain_images;
	    vector<VkImageView> swapchain_image_views;
	    VkImage depth_image;
	    VkImageView depth_image_view;
	    VulkanMemory depth_image_memory;
	    vector<VkFramebuffer> swapchain_framebuffers;
	    VkFormat swapchain_image_format;
	    VkExtent2D swapchain_extent;
	    VkRenderPass render_pass = VK_NULL_HANDLE;
	    VkCommandPool command_pool = VK_NULL_HANDLE;
	    vector<VkCommandBuffer> command_buffers;
	    VkCommandBuffer command_buffer;
	    vector<VkSemaphore> image_available_semaphores;
	    vector<VkSemaphore> render_finished_semaphores;
	    vector<VkFence> in_flight_fences;
	    uint32_t graphics_queue_family = 0;
	    uint32_t present_queue_family = 0;
	    uint32_t api_version = 0;
	    uint32_t current_frame = 0;
	    uint32_t image_index = 0;

	    unordered_map<uint32_t, VulkanPipeline> pipelines;
	    VulkanPipeline current_pipeline;

	    unordered_map<uint32_t, VulkanBuffer> buffers;

	    KujoGFXPass current_pass;

	    bool has_khr_maintenance_1 = false;

	    uint32_t getAPIVersion()
	    {
		auto fn_vkEnumerateInstanceVersion = PFN_vkEnumerateInstanceVersion(vkGetInstanceProcAddr(NULL, "vkEnumerateInstanceVersion"));

		if (fn_vkEnumerateInstanceVersion != NULL)
		{
		    uint32_t instance_version = 0;
		    fn_vkEnumerateInstanceVersion(&instance_version);

		    if (instance_version >= VK_API_VERSION_1_1)
		    {
			kujogfxlog::debug() << "Using Vulkan 1.1";
			return VK_API_VERSION_1_1;
		    }
		}

		kujogfxlog::debug() << "Falling back to Vulkan 1.0";
		return VK_API_VERSION_1_0;
	    }

	    VkBool32 convertBoolVk(bool value)
	    {
		if (value)
		{
		    return VK_TRUE;
		}

		return VK_FALSE;
	    }

	    bool assertVk(bool cond, bool is_shutdown = true)
	    {
		if (cond)
		{
		    return false;
		}

		if (is_shutdown)
		{
		    shutdownVulkan();
		}

		return true;
	    }

	    bool hasFailed(VkResult err, bool is_shutdown = true)
	    {
		return assertVk((err == VK_SUCCESS), is_shutdown);
	    }

	    bool initVulkan(void *window_handle, void *display_handle)
	    {
		win_handle = window_handle;
		disp_handle = display_handle;

		if (assertVk(createInstance()))
		{
		    return false;
		}

		if (assertVk(createSurface()))
		{
		    return false;
		}

		if (assertVk(pickPhysicalDevice()))
		{
		    return false;
		}

		if (assertVk(checkSwapchainSupport()))
		{
		    return false;
		}

		if (assertVk(findQueueFamilies()))
		{
		    return false;
		}

		if (assertVk(createLogicalDevice()))
		{
		    return false;
		}

		if (assertVk(createSwapchain()))
		{
		    return false;
		}

		if (assertVk(createCommandQueues()))
		{
		    return false;
		}

		if (assertVk(createSyncObjects()))
		{
		    return false;
		}

		return true;
	    }

	    void shutdownVulkan()
	    {
		vkDeviceWaitIdle(device);

		for (auto &semaphore : image_available_semaphores)
		{
		    if (semaphore != VK_NULL_HANDLE)
		    {
			vkDestroySemaphore(device, semaphore, NULL);
			semaphore = VK_NULL_HANDLE;
		    }
		}

		for (auto &semaphore : render_finished_semaphores)
		{
		    if (semaphore != VK_NULL_HANDLE)
		    {
			vkDestroySemaphore(device, semaphore, NULL);
			semaphore = VK_NULL_HANDLE;
		    }
		}

		for (auto &fence : in_flight_fences)
		{
		    if (fence != VK_NULL_HANDLE)
		    {
			vkDestroyFence(device, fence, NULL);
			fence = VK_NULL_HANDLE;
		    }
		}

		if (command_pool != VK_NULL_HANDLE)
		{
		    vkDestroyCommandPool(device, command_pool, NULL);
		    command_pool = VK_NULL_HANDLE;
		}

		for (auto &iter : buffers)
		{
		    auto buffer = iter.second;

		    if (buffer.buffer != VK_NULL_HANDLE)
		    {
			vkDestroyBuffer(device, buffer.buffer, NULL);
			buffer.buffer = VK_NULL_HANDLE;
		    }

		    if (buffer.memory.memory != VK_NULL_HANDLE)
		    {
			vkFreeMemory(device, buffer.memory.memory, NULL);
			buffer.memory.memory = VK_NULL_HANDLE;
		    }
		}

		buffers.clear();

		for (auto &iter : pipelines)
		{
		    auto pipeline = iter.second;

		    if (pipeline.pipeline != VK_NULL_HANDLE)
		    {
			vkDestroyPipeline(device, pipeline.pipeline, NULL);
			pipeline.pipeline = VK_NULL_HANDLE;
		    }

		    if (pipeline.layout != VK_NULL_HANDLE)
		    {
			vkDestroyPipelineLayout(device, pipeline.layout, NULL);
			pipeline.layout = VK_NULL_HANDLE;
		    }
		}

		pipelines.clear();

		cleanupSwapchain();

		if (device != VK_NULL_HANDLE)
		{
		    vkDestroyDevice(device, NULL);
		    device = VK_NULL_HANDLE;
		}

		if (surface != VK_NULL_HANDLE)
		{
		    vkDestroySurfaceKHR(instance, surface, NULL);
		    surface = VK_NULL_HANDLE;
		}

		if (instance != VK_NULL_HANDLE)
		{
		    vkDestroyInstance(instance, NULL);
		    instance = VK_NULL_HANDLE;
		}
	    }

	    void cleanupSwapchain()
	    {
		if (depth_image_view != VK_NULL_HANDLE)
		{
		    vkDestroyImageView(device, depth_image_view, NULL);
		    depth_image_view = VK_NULL_HANDLE;
		}

		if (depth_image != VK_NULL_HANDLE)
		{
		    vkDestroyImage(device, depth_image, NULL);
		    depth_image = VK_NULL_HANDLE;
		}

		if (depth_image_memory.memory != VK_NULL_HANDLE)
		{
		    vkFreeMemory(device, depth_image_memory.memory, NULL);
		    depth_image_memory.memory = VK_NULL_HANDLE;
		}

		for (auto &image_view : swapchain_image_views)
		{
		    if (image_view != VK_NULL_HANDLE)
		    {
			vkDestroyImageView(device, image_view, NULL);
			image_view = VK_NULL_HANDLE;
		    }
		}

		if (swapchain != VK_NULL_HANDLE)
		{
		    vkDestroySwapchainKHR(device, swapchain, NULL);
		    swapchain = VK_NULL_HANDLE;
		}
	    }

	    void recreateSwapchain()
	    {
		vkDeviceWaitIdle(device);
		cleanupSwapchain();

		if (assertVk(fetchWindowRes()))
		{
		    kujogfxlog::fatal() << "Could not fetch window resolution!";
		}

		if (assertVk(createSwapchain()))
		{
		    kujogfxlog::fatal() << "Could not recreate swapchain!";
		}
	    }

	    uint32_t findMemoryType(uint32_t type_filter, VkMemoryPropertyFlags properties)
	    {
		VkPhysicalDeviceMemoryProperties mem_properties;
		vkGetPhysicalDeviceMemoryProperties(physical_device, &mem_properties);

		for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++)
		{
		    if ((type_filter & (1 << i)) && ((mem_properties.memoryTypes[i].propertyFlags & properties) == properties))
		    {
			return i;
		    }
		}

		assertVk(false);
		kujogfxlog::fatal() << "Could not find suitable memory type!";
		return 0;
	    }

	    VkResult allocateMemoryVk(VkMemoryRequirements requirements, VkMemoryPropertyFlags properties, VkDeviceMemory &memory, uint32_t &offset)
	    {
		VkMemoryAllocateInfo alloc_info = {};
		alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		alloc_info.allocationSize = requirements.size;
		alloc_info.memoryTypeIndex = findMemoryType(requirements.memoryTypeBits, properties);

		VkResult err = vkAllocateMemory(device, &alloc_info, NULL, &memory);

		if (hasFailed(err, false))
		{
		    return err;
		}

		offset = 0;
		return VK_SUCCESS;
	    }

	    VkResult createBufferVk(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &buffer_memory, uint32_t &offset)
	    {
		VkBufferCreateInfo buffer_info = {};
		buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buffer_info.size = size;
		buffer_info.usage = usage;
		buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VkResult err = vkCreateBuffer(device, &buffer_info, NULL, &buffer);

		if (hasFailed(err, false))
		{
		    kujogfxlog::error() << "Could not create buffer!";
		    return err;
		}

		VkMemoryRequirements mem_requirements;
		vkGetBufferMemoryRequirements(device, buffer, &mem_requirements);

		err = allocateMemoryVk(mem_requirements, properties, buffer_memory, offset);

		if (hasFailed(err, false))
		{
		    kujogfxlog::error() << "Could not allocate buffer memory!";
		    return err;
		}

		vkBindBufferMemory(device, buffer, buffer_memory, 0);

		return VK_SUCCESS;
	    }

	    VkResult createBufferVk(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VulkanBuffer &buffer)
	    {
		return createBufferVk(size, usage, properties, buffer.buffer, buffer.memory.memory, buffer.memory.offset);
	    }

	    VkResult copyBufferVk(VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size)
	    {
		VkCommandBufferAllocateInfo alloc_info = {};
		alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		alloc_info.commandPool = command_pool;
		alloc_info.commandBufferCount = 1;

		VkCommandBuffer cmd_buffer;
		VkResult err = vkAllocateCommandBuffers(device, &alloc_info, &cmd_buffer);

		if (hasFailed(err, false))
		{
		    return err;
		}

		VkCommandBufferBeginInfo begin_info = {};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		vkBeginCommandBuffer(cmd_buffer, &begin_info);

		VkBufferCopy copy_region = {};
		copy_region.size = size;
		vkCmdCopyBuffer(cmd_buffer, src_buffer, dst_buffer, 1, &copy_region);

		vkEndCommandBuffer(cmd_buffer);

		VkSubmitInfo submit_info = {};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &cmd_buffer;

		vkQueueSubmit(graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
		vkQueueWaitIdle(graphics_queue);

		vkFreeCommandBuffers(device, command_pool, 1, &cmd_buffer);
		return VK_SUCCESS;
	    }

	    VkResult copyBufferVk(VulkanBuffer src_buffer, VulkanBuffer dst_buffer, VkDeviceSize size)
	    {
		return copyBufferVk(src_buffer.buffer, dst_buffer.buffer, size);
	    }

	    VkResult createImageVk(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage &image, VkDeviceMemory &image_memory, uint32_t &image_offset)
	    {
		VkImageCreateInfo image_info = {};
		image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		image_info.imageType = VK_IMAGE_TYPE_2D;
		image_info.extent.width = width;
		image_info.extent.height = height;
		image_info.extent.depth = 1;
		image_info.mipLevels = 1;
		image_info.arrayLayers = 1;
		image_info.format = format;
		image_info.tiling = tiling;
		image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		image_info.usage = usage;
		image_info.samples = VK_SAMPLE_COUNT_1_BIT;
		image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VkResult err = vkCreateImage(device, &image_info, NULL, &image);

		if (hasFailed(err, false))
		{
		    kujogfxlog::error() << "Could not create image!";
		    return err;
		}

		VkMemoryRequirements mem_requirements;
		vkGetImageMemoryRequirements(device, image, &mem_requirements);

		err = allocateMemoryVk(mem_requirements, properties, image_memory, image_offset);

		if (hasFailed(err, false))
		{
		    kujogfxlog::error() << "Could not allocate image memory!";
		    return err;
		}

		vkBindImageMemory(device, image, image_memory, image_offset);
		return VK_SUCCESS;
	    }

	    VkResult createImageVk(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage &image, VulkanMemory &memory)
	    {
		return createImageVk(width, height, format, tiling, usage, properties, image, memory.memory, memory.offset);
	    }

	    VkResult createImageViewVk(VkImage image, VkFormat format, VkImageAspectFlags aspect_flags, VkImageView &image_view)
	    {
		VkImageViewCreateInfo view_info = {};
		view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		view_info.image = image;
		view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		view_info.format = format;
		view_info.subresourceRange.aspectMask = aspect_flags;
		view_info.subresourceRange.baseMipLevel = 1;
		view_info.subresourceRange.levelCount = 1;
		view_info.subresourceRange.baseArrayLayer = 0;
		view_info.subresourceRange.layerCount = 1;

		VkResult err = vkCreateImageView(device, &view_info, NULL, &image_view);

		if (hasFailed(err, false))
		{
		    kujogfxlog::error() << "Could not create image view!";
		    return err;
		}

		return VK_SUCCESS;
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

		VkAttachmentDescription depth_attachment = {};
		depth_attachment.format = findDepthFormat();
		depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depth_attachment.loadOp = convertLoadOp(current_pass.action.depth_attach.load_op);
		depth_attachment.storeOp = convertStoreOp(current_pass.action.depth_attach.store_op);
		color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		color_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference color_attachment_ref = {};
		color_attachment_ref.attachment = 0;
		color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depth_attachment_ref = {};
		depth_attachment_ref.attachment = 1;
		depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &color_attachment_ref;
		subpass.pDepthStencilAttachment = &depth_attachment_ref;

		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = (VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT);
		dependency.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dependency.dstStageMask = (VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT);
		dependency.dstAccessMask = (VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT);

		vector<VkAttachmentDescription> attachments = {
		    color_attachment,
		    depth_attachment
		};

		VkRenderPassCreateInfo render_pass_info = {};
		render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		render_pass_info.attachmentCount = uint32_t(attachments.size());
		render_pass_info.pAttachments = attachments.data();
		render_pass_info.subpassCount = 1;
		render_pass_info.pSubpasses = &subpass;
		render_pass_info.dependencyCount = 1;
		render_pass_info.pDependencies = &dependency;

		VkResult err = vkCreateRenderPass(device, &render_pass_info, NULL, &render_pass);

		if (hasFailed(err, false))
		{
		    kujogfxlog::error() << "Could not create render pass!";
		    return false;
		}

		return true;
	    }

	    bool createFramebuffers()
	    {
		swapchain_framebuffers.resize(swapchain_image_views.size());

		for (size_t i = 0; i < swapchain_image_views.size(); i++)
		{
		    vector<VkImageView> attachments = {
			swapchain_image_views[i],
			depth_image_view
		    };

		    VkFramebufferCreateInfo framebuffer_info = {};
		    framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		    framebuffer_info.renderPass = render_pass;
		    framebuffer_info.attachmentCount = uint32_t(attachments.size());
		    framebuffer_info.pAttachments = attachments.data();
		    framebuffer_info.width = swapchain_extent.width;
		    framebuffer_info.height = swapchain_extent.height;
		    framebuffer_info.layers = 1;

		    VkResult err = vkCreateFramebuffer(device, &framebuffer_info, NULL, &swapchain_framebuffers[i]);

		    if (hasFailed(err, false))
		    {
			kujogfxlog::error() << "Could not create framebuffers!";
			return false;
		    }
		}

		return true;
	    }

	    void beginPass(KujoGFXPass pass)
	    {
		current_pass = pass;

		if (assertVk(createRenderPass()))
		{
		    kujogfxlog::fatal() << "Could not start render pass!";
		    return;
		}

		vkWaitForFences(device, 1, &in_flight_fences[current_frame], VK_TRUE, UINT64_MAX);
		vkResetFences(device, 1, &in_flight_fences[current_frame]);

		VkResult err = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, image_available_semaphores[current_frame], VK_NULL_HANDLE, &image_index);

		if (err == VK_ERROR_OUT_OF_DATE_KHR)
		{
		    recreateSwapchain();
		    return;
		}
		else if ((err != VK_SUCCESS) && (err != VK_SUBOPTIMAL_KHR))
		{
		    kujogfxlog::fatal() << "Could not acquire swapchain images!";
		    return;
		}

		if (assertVk(createFramebuffers()))
		{
		    kujogfxlog::fatal() << "Could not start render pass!";
		    return;
		}

		command_buffer = command_buffers[current_frame];

		vkResetCommandBuffer(command_buffer, 0);

		VkCommandBufferBeginInfo begin_info = {};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin_info.flags = 0;
		begin_info.pInheritanceInfo = NULL;

		err = vkBeginCommandBuffer(command_buffer, &begin_info);

		if (hasFailed(err))
		{
		    kujogfxlog::fatal() << "Could not begin command buffer!";
		    return;
		}

		array<VkClearValue, 2> clear_values;
		clear_values[0].color = convertClearColor(current_pass.action.color_attach.color);
		clear_values[1].depthStencil = {current_pass.action.depth_attach.clear_val, 0};

		VkRenderPassBeginInfo render_pass_info = {};
		render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		render_pass_info.pNext = NULL;
		render_pass_info.renderPass = render_pass;
		render_pass_info.renderArea.offset = {0, 0};
		render_pass_info.renderArea.extent = swapchain_extent;
		render_pass_info.framebuffer = swapchain_framebuffers[image_index];
		render_pass_info.clearValueCount = uint32_t(clear_values.size());
		render_pass_info.pClearValues = clear_values.data();

		vkCmdBeginRenderPass(command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
	    }

	    void endPass()
	    {
		vkCmdEndRenderPass(command_buffer);
		VkResult err = vkEndCommandBuffer(command_buffer);

		if (hasFailed(err))
		{
		    kujogfxlog::fatal() << "Could not begin command buffer!";
		    return;
		}

		VkSubmitInfo submit_info = {};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		submit_info.waitSemaphoreCount = 1;
		submit_info.pWaitSemaphores = &image_available_semaphores[current_frame];
		submit_info.pWaitDstStageMask = &wait_stage;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &command_buffer;
		submit_info.signalSemaphoreCount = 1;
		submit_info.pSignalSemaphores = &render_finished_semaphores[current_frame];

		err = vkQueueSubmit(graphics_queue, 1, &submit_info, in_flight_fences[current_frame]);

		if (hasFailed(err))
		{
		    kujogfxlog::fatal() << "Could not submit draw commands!";
		    return;
		}

		VkPresentInfoKHR present_info = {};
		present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		present_info.waitSemaphoreCount = 1;
		present_info.pWaitSemaphores = &render_finished_semaphores[current_frame];
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
		    kujogfxlog::fatal() << "Could not render swapchain image!";
		}

		current_frame = ((current_frame + 1) % max_frames_in_flight);
	    }

	    VkShaderModule createShaderModule(vector<uint32_t> &code)
	    {
		VkShaderModuleCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		create_info.codeSize = (code.size() * sizeof(uint32_t));
		create_info.pCode = code.data();

		VkShaderModule shader_module;
		VkResult err = vkCreateShaderModule(device, &create_info, NULL, &shader_module);

		if (hasFailed(err))
		{
		    kujogfxlog::fatal() << "Could not create shader module!";
		}

		return shader_module;
	    }

	    VkPrimitiveTopology getTopology(KujoGFXPrimitiveType type)
	    {
		switch (type)
		{
		    case PrimitiveTriangles: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; break;
		    default:
		    {
			assertVk(false);
			kujogfxlog::fatal() << "Unrecognized primitive type of " << dec << int(type);
			return (VkPrimitiveTopology)0;
		    }
		    break;
		}
	    }

	    VkCullModeFlags getCullMode(KujoGFXCullMode mode)
	    {
		switch (mode)
		{
		    case CullModeNone: return VK_CULL_MODE_NONE; break;
		    case CullModeFront: return VK_CULL_MODE_FRONT_BIT; break;
		    case CullModeBack: return VK_CULL_MODE_BACK_BIT; break;
		    default:
		    {
			assertVk(false);
			kujogfxlog::fatal() << "Unrecognized cull mode of " << dec << int(mode);
			return (VkCullModeFlags)0;
		    }
		    break;
		}
	    }

	    VkFormat getFormat(KujoGFXVertexFormat format)
	    {
		switch (format)
		{
		    case VertexFormatFloat2: return VK_FORMAT_R32G32_SFLOAT; break;
		    case VertexFormatFloat3: return VK_FORMAT_R32G32B32_SFLOAT; break;
		    case VertexFormatFloat4: return VK_FORMAT_R32G32B32A32_SFLOAT; break;
		    case VertexFormatInvalid: return VK_FORMAT_UNDEFINED; break;
		    default:
		    {
			assertVk(false);
			kujogfxlog::fatal() << "Unrecognized vertex format of " << dec << int(format);
			return VK_FORMAT_UNDEFINED;
		    }
		    break;
		}
	    }

	    VkIndexType getIndexType(KujoGFXIndexType type)
	    {
		switch (type)
		{
		    case IndexTypeNone: return VK_INDEX_TYPE_UINT16; break;
		    case IndexTypeUint16: return VK_INDEX_TYPE_UINT16; break;
		    case IndexTypeUint32: return VK_INDEX_TYPE_UINT32; break;
		    default:
		    {
			assertVk(false);
			kujogfxlog::fatal() << "Unrecognized index type of " << dec << int(type);
			return (VkIndexType)0;
		    }
		    break;
		}
	    }

	    VkCompareOp getCompareOp(KujoGFXCompareFunc func)
	    {
		switch (func)
		{
		    case CompareFuncNever: return VK_COMPARE_OP_NEVER; break;
		    case CompareFuncLessEqual: return VK_COMPARE_OP_LESS_OR_EQUAL; break;
		    case CompareFuncAlways: return VK_COMPARE_OP_ALWAYS; break;
		    default:
		    {
			assertVk(false);
			kujogfxlog::fatal() << "Unrecognized compare func of " << dec << int(func);
			return (VkCompareOp)0;
		    }
		    break;
		}
	    }

	    vector<KujoGFXUniformDesc> getUniforms(KujoGFXPipeline pipeline)
	    {
		auto uniforms = pipeline.shader.uniforms;
		size_t uniform_size = min<size_t>(max_uniform_block_bind_slots, uniforms.size());
		return vector<KujoGFXUniformDesc>(uniforms.begin(), (uniforms.begin() + uniform_size));
	    }

	    void setPipeline(KujoGFXPipeline pipeline)
	    {
		auto cached_pipeline = pipelines.find(pipeline.getID());

		if (assertVk(cached_pipeline != pipelines.end()))
		{
		    kujogfxlog::fatal() << "Could not find current pipeline!";
		    return;
		}

		current_pipeline = cached_pipeline->second;
	    }

	    void createPipeline(KujoGFXPipeline &pipeline)
	    {
		VulkanPipeline new_pipeline;
		auto shader = pipeline.shader;
		auto locations = shader.locations.spirv_locations;
		auto uniforms = getUniforms(pipeline);

		// Create shader modules
		VkShaderModule vert_module = createShaderModule(shader.vert_code.spv_code);
		VkShaderModule frag_module = createShaderModule(shader.frag_code.spv_code);

		VkPipelineShaderStageCreateInfo vert_stage_info = {};
		vert_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vert_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vert_stage_info.module = vert_module;
		vert_stage_info.pName = shader.vert_code.entry_name.c_str();

		VkPipelineShaderStageCreateInfo frag_stage_info = {};
		frag_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		frag_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		frag_stage_info.module = frag_module;
		frag_stage_info.pName = shader.frag_code.entry_name.c_str();

		vector<VkPipelineShaderStageCreateInfo> shader_stages = {
		    vert_stage_info,
		    frag_stage_info
		};

		// Set up dynamic states
		vector<VkDynamicState> dynamic_states = {
		    VK_DYNAMIC_STATE_VIEWPORT,
		    VK_DYNAMIC_STATE_SCISSOR
		};

		VkPipelineDynamicStateCreateInfo dynamic_state = {};
		dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamic_state.dynamicStateCount = uint32_t(dynamic_states.size());
		dynamic_state.pDynamicStates = dynamic_states.data();

		// Set up vertex input descriptions
		vector<VkVertexInputBindingDescription> vertex_bindings;
		vector<VkVertexInputAttributeDescription> vertex_attribs;

		for (size_t i = 0; i < max_vertex_buffer_bind_slots; i++)
		{
		    if (pipeline.layout.vertex_buffer_layout_active[i])
		    {
			VkVertexInputBindingDescription description;
			description.binding = i;
			description.stride = pipeline.layout.buffers[i].stride;
			description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			vertex_bindings.push_back(description);
		    }
		}

		for (size_t attr_index = 0; attr_index < max_vertex_attribs; attr_index++)
		{
		    auto attrib = pipeline.layout.attribs[attr_index];

		    if (attrib.format == VertexFormatInvalid)
		    {
			break;
		    }

		    VkVertexInputAttributeDescription description;
		    description.binding = attrib.buffer_index;
		    description.location = locations.at(attr_index);
		    description.format = getFormat(attrib.format);
		    description.offset = attrib.offset;

		    vertex_attribs.push_back(description);
		}

		VkPipelineVertexInputStateCreateInfo vertex_input_info = {};
		vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertex_input_info.vertexBindingDescriptionCount = uint32_t(vertex_bindings.size());
		vertex_input_info.pVertexBindingDescriptions = vertex_bindings.data();
		vertex_input_info.vertexAttributeDescriptionCount = uint32_t(vertex_attribs.size());
		vertex_input_info.pVertexAttributeDescriptions = vertex_attribs.data();

		VkPipelineInputAssemblyStateCreateInfo input_assembly = {};
		input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		input_assembly.topology = getTopology(pipeline.primitive_type);
		input_assembly.primitiveRestartEnable = VK_FALSE;

		VkPipelineViewportStateCreateInfo viewport_state = {};
		viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewport_state.viewportCount = 1;
		viewport_state.scissorCount = 1;

		VkPipelineRasterizationStateCreateInfo raster_state = {};
		raster_state.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		raster_state.depthClampEnable = VK_FALSE;
		raster_state.rasterizerDiscardEnable = VK_FALSE;
		raster_state.polygonMode = VK_POLYGON_MODE_FILL;
		raster_state.lineWidth = 1.0f;
		raster_state.cullMode = getCullMode(pipeline.cull_mode);
		raster_state.frontFace = VK_FRONT_FACE_CLOCKWISE;
		raster_state.depthBiasEnable = VK_FALSE;
		raster_state.depthBiasConstantFactor = 0.f;
		raster_state.depthBiasClamp = 0.f;
		raster_state.depthBiasSlopeFactor = 0.f;

		VkPipelineMultisampleStateCreateInfo multisample_state = {};
		multisample_state.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisample_state.sampleShadingEnable = VK_FALSE;
		multisample_state.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisample_state.minSampleShading = 1.0f;
		multisample_state.pSampleMask = NULL;
		multisample_state.alphaToCoverageEnable = VK_FALSE;
		multisample_state.alphaToOneEnable = VK_FALSE;

		VkPipelineColorBlendAttachmentState color_blend_attachment = {};
		color_blend_attachment.colorWriteMask = (VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT);
		color_blend_attachment.blendEnable = VK_FALSE;
		color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
		color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;

		VkPipelineColorBlendStateCreateInfo color_blend_state = {};
		color_blend_state.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		color_blend_state.logicOpEnable = VK_FALSE;
		color_blend_state.attachmentCount = 1;
		color_blend_state.pAttachments = &color_blend_attachment;

		VkPipelineDepthStencilStateCreateInfo depth_stencil_state = {};
		depth_stencil_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depth_stencil_state.depthTestEnable = VK_TRUE;
		depth_stencil_state.depthWriteEnable = convertBoolVk(pipeline.depth_state.is_write_enabled);
		depth_stencil_state.depthCompareOp = getCompareOp(pipeline.depth_state.compare_func);
		depth_stencil_state.depthBoundsTestEnable = VK_FALSE;
		depth_stencil_state.stencilTestEnable = VK_FALSE;

		VkPipelineLayoutCreateInfo pipeline_layout_info = {};
		pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

		VkResult err = vkCreatePipelineLayout(device, &pipeline_layout_info, NULL, &new_pipeline.layout);

		if (hasFailed(err))
		{
		    kujogfxlog::fatal() << "Could not create pipeline layout!";
		}

		VkGraphicsPipelineCreateInfo pipeline_info = {};
		pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipeline_info.stageCount = 2;
		pipeline_info.pStages = shader_stages.data();
		pipeline_info.pVertexInputState = &vertex_input_info;
		pipeline_info.pInputAssemblyState = &input_assembly;
		pipeline_info.pViewportState = &viewport_state;
		pipeline_info.pRasterizationState = &raster_state;
		pipeline_info.pMultisampleState = &multisample_state;
		pipeline_info.pColorBlendState = &color_blend_state;
		pipeline_info.pDepthStencilState = &depth_stencil_state;
		pipeline_info.pDynamicState = &dynamic_state;
		pipeline_info.layout = new_pipeline.layout;
		pipeline_info.renderPass = render_pass;
		pipeline_info.subpass = 0;

		err = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipeline_info, NULL, &new_pipeline.pipeline);

		if (hasFailed(err))
		{
		    kujogfxlog::fatal() << "Could not create graphics pipeline!";
		}

		new_pipeline.index_type = getIndexType(pipeline.index_type);

		// Hack to determine if index buffer is active (because Vulkan doesn't have native support for a 'no indices provided' type
		// (at least, not without out-of-scope extensions))
		new_pipeline.is_index_active = (pipeline.index_type != IndexTypeNone);

		vkDestroyShaderModule(device, vert_module, NULL);
		vkDestroyShaderModule(device, frag_module, NULL);

		pipelines.insert(make_pair(pipeline.getID(), new_pipeline));
		current_pipeline = new_pipeline;
	    }

	    void applyPipeline()
	    {
		VkViewport viewport = {};
		viewport.x = 0.f;
		viewport.y = static_cast<float>(swapchain_extent.height);
		viewport.width = static_cast<float>(swapchain_extent.width);
		viewport.height = -static_cast<float>(swapchain_extent.height);
		viewport.minDepth = 0.f;
		viewport.maxDepth = 1.f;

		VkRect2D scissor = {};
		scissor.offset = {0, 0};
		scissor.extent = swapchain_extent;

		vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, current_pipeline.pipeline);
		vkCmdSetViewport(command_buffer, 0, 1, &viewport);
		vkCmdSetScissor(command_buffer, 0, 1, &scissor);
	    }

	    VkBufferUsageFlags getUsage(KujoGFXBuffer buffer)
	    {
		VkBufferUsageFlags flags = 0;

		if (buffer.isIndexBuffer())
		{
		    flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		}
		else if (buffer.isVertexBuffer())
		{
		    flags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		}

		return flags;
	    }

	    void createBuffer(KujoGFXBuffer buffer)
	    {
		VulkanBuffer staging_buffer;
		VkResult err = createBufferVk(buffer.getSize(),
		    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		    (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT),
		    staging_buffer);

		if (hasFailed(err))
		{
		    kujogfxlog::fatal() << "Could not create staging buffer!";
		}

		void *mem_data = NULL;
		vkMapMemory(device, staging_buffer.memory.memory, staging_buffer.memory.offset, buffer.getSize(), 0, &mem_data);
		memcpy(mem_data, buffer.getData(), buffer.getSize());
		vkUnmapMemory(device, staging_buffer.memory.memory);

		VulkanBuffer main_buffer;
		err = createBufferVk(buffer.getSize(),
		    (VK_BUFFER_USAGE_TRANSFER_DST_BIT | getUsage(buffer)),
		    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		    main_buffer);

		if (hasFailed(err))
		{
		    kujogfxlog::fatal() << "Could not create main buffer!";
		}

		err = copyBufferVk(staging_buffer, main_buffer, buffer.getSize());

		if (hasFailed(err))
		{
		    kujogfxlog::fatal() << "Could not copy buffer data!";
		}

		vkDestroyBuffer(device, staging_buffer.buffer, NULL);
		vkFreeMemory(device, staging_buffer.memory.memory, NULL);

		buffers.insert(make_pair(buffer.getID(), main_buffer));
	    }

	    VulkanBuffer findBuffer(KujoGFXBuffer buffer)
	    {
		auto iter = buffers.find(buffer.getID());

		if (iter != buffers.end())
		{
		    return iter->second;
		}

		return {VK_NULL_HANDLE, {VK_NULL_HANDLE, 0}};
	    }

	    void applyBindings(KujoGFXBindings bindings)
	    {
		vector<VkBuffer> vertex_buffers;
		vector<VkDeviceSize> vertex_offsets;

		for (size_t index = 0; index < bindings.vertex_buffers.size(); index++)
		{
		    auto buffer = findBuffer(bindings.vertex_buffers.at(index));

		    if (buffer.buffer != VK_NULL_HANDLE)
		    {
			vertex_buffers.push_back(buffer.buffer);
			vertex_offsets.push_back(bindings.vertex_buffer_offsets.at(index));
		    }
		}

		vkCmdBindVertexBuffers(command_buffer, 0, vertex_buffers.size(), vertex_buffers.data(), vertex_offsets.data());

		auto index_buffer = findBuffer(bindings.index_buffer);

		if (index_buffer.buffer != VK_NULL_HANDLE)
		{
		    vkCmdBindIndexBuffer(command_buffer, index_buffer.buffer, bindings.index_buffer_offset, current_pipeline.index_type);
		}
	    }

	    void applyUniforms(int, KujoGFXData)
	    {
		return;
	    }

	    void draw(KujoGFXDraw draw)
	    {
		int base_element = draw.base_element;
		int num_elements = draw.num_elements;
		int num_instances = draw.num_instances;

		if (current_pipeline.is_index_active)
		{
		    vkCmdDrawIndexed(command_buffer, num_elements, num_instances, base_element, 0, 0);
		}
		else
		{
		    vkCmdDraw(command_buffer, num_elements, num_instances, base_element, 0);
		}
	    }

	    void commitFrame()
	    {
		vkDeviceWaitIdle(device);

		for (auto &framebuffer : swapchain_framebuffers)
		{
		    if (framebuffer != VK_NULL_HANDLE)
		    {
			vkDestroyFramebuffer(device, framebuffer, NULL);
			framebuffer = VK_NULL_HANDLE;
		    }
		}

		if (render_pass != VK_NULL_HANDLE)
		{
		    vkDestroyRenderPass(device, render_pass, NULL);
		    render_pass = VK_NULL_HANDLE;
		}
	    }

	    vector<const char*> getDesiredExtensions()
	    {
		vector<const char*> desired_extensions = {
		    "VK_EXT_debug_report",
		    "VK_EXT_debug_utils",
		    #if defined(KUJOGFX_PLATFORM_WINDOWS)
		    VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
		    #elif defined(KUJOGFX_PLATFORM_LINUX) || defined(KUJOGFX_PLATFORM_BSD)
		    #if defined(KUJOGFX_IS_WAYLAND)
		    VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME,
		    #elif defined(KUJOGFX_IS_X11)
		    VK_KHR_XLIB_SURFACE_EXTENSION_NAME,
		    #endif
		    #elif defined(KUJOGFX_PLATFORM_MACOS)
		    VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME,
		    VK_MVK_MACOS_SURFACE_EXTENSION_NAME,
		    #else
		    #error "Vulkan extension collection is unimplemented for this platform"
		    #endif
		    VK_KHR_SURFACE_EXTENSION_NAME
		};

		if (api_version < VK_API_VERSION_1_1)
		{
		    desired_extensions.push_back(VK_KHR_MAINTENANCE1_EXTENSION_NAME);
		}

		return desired_extensions;
	    }

	    vector<const char*> getDesiredLayers()
	    {
		vector<const char*> desired_layers = {
		    "VK_LAYER_KHRONOS_validation"
		};

		return desired_layers;
	    }

	    bool getExtensions(vector<const char*> &extensions)
	    {
		uint32_t ext_count = 0;
		VkResult err = vkEnumerateInstanceExtensionProperties(NULL, &ext_count, NULL);

		if (err != VK_SUCCESS)
		{
		    kujogfxlog::error() << "Could not fetch extension properties count!";
		    return false;
		}

		vector<VkExtensionProperties> ext_properties(ext_count);

		err = vkEnumerateInstanceExtensionProperties(NULL, &ext_count, ext_properties.data());

		if (err != VK_SUCCESS)
		{
		    kujogfxlog::error() << "Could not fetch extension properties!";
		    return false;
		}

		vector<const char*> desired_extensions = getDesiredExtensions();

		for (const auto &property : ext_properties)
		{
		    string ext_name = property.extensionName;

		    for (const auto &ext : desired_extensions)
		    {
			if (ext_name == ext)
			{
			    extensions.push_back(ext);
			    break;
			}
		    }
		}

		return true;
	    }

	    bool getLayers(vector<const char*> &layers)
	    {
		uint32_t layer_count = 0;
		VkResult err = vkEnumerateInstanceLayerProperties(&layer_count, NULL);

		if (err != VK_SUCCESS)
		{
		    kujogfxlog::error() << "Could not fetch layer properties count!";
		    return false;
		}

		vector<VkLayerProperties> layer_properties(layer_count);
		err = vkEnumerateInstanceLayerProperties(&layer_count, layer_properties.data());

		if (err != VK_SUCCESS)
		{
		    kujogfxlog::error() << "Could not fetch layer properties!";
		    return false;
		}

		vector<const char*> desired_layers = getDesiredLayers();

		for (const auto &property : layer_properties)
		{
		    string layer_name = property.layerName;

		    for (const auto &layer : desired_layers)
		    {
			if (layer_name == layer)
			{
			    layers.push_back(layer);
			    break;
			}
		    }
		}

		return true;
	    }

	    bool createInstance()
	    {
		VkApplicationInfo app_info = {};
		app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		app_info.pApplicationName = "KujoGFX-Vulkan";
		app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		app_info.pEngineName = "KujoGFX-Vulkan";
		app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		app_info.apiVersion = api_version;

		vector<const char*> extensions;

		if (!getExtensions(extensions))
		{
		    return false;
		}

		vector<const char*> layers;

		if (!getLayers(layers))
		{
		    return false;
		}

		stringstream ext_str;
		ext_str << "Available extensions: " << endl;

		for (const auto &extension : extensions)
		{
		    ext_str << "\t" << extension << endl;
		}

		kujogfxlog::debug() << ext_str.str();

		stringstream layer_str;
		layer_str << "Available layers: " << endl;

		for (const auto &layer : layers)
		{
		    layer_str << "\t" << layer << endl;
		}

		kujogfxlog::debug() << layer_str.str();

		VkInstanceCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		create_info.pApplicationInfo = &app_info;

		#if defined(KUJOGFX_PLATFORM_MACOS)
		create_info.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
		#endif

		create_info.enabledExtensionCount = uint32_t(extensions.size());
		create_info.ppEnabledExtensionNames = extensions.data();

		create_info.enabledLayerCount = uint32_t(layers.size());
		create_info.ppEnabledLayerNames = layers.data();

		VkResult err = vkCreateInstance(&create_info, NULL, &instance);

		if (hasFailed(err))
		{
		    kujogfxlog::error() << "Could not create instance!";
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
		#elif defined(KUJOGFX_PLATFORM_LINUX)
		#if defined(KUJOGFX_IS_X11)
		Display *dpy = reinterpret_cast<Display*>(disp_handle);
		Window win = reinterpret_cast<Window>(win_handle);

		XWindowAttributes attrib;
		XGetWindowAttributes(dpy, win, &attrib);

		window_width = attrib.width;
		window_height = attrib.height;
		#else
		#error "Vulkan window resolution fetch is unimplemented on Wayland"
		#endif
		#else
		#error "Vulkan window resolution fetch is unimplemented for this platform"
		#endif
		return true;
	    }

	    bool createSurface()
	    {
		if (assertVk(fetchWindowRes()))
		{
		    kujogfxlog::error() << "Could not fetch window resolution!";
		    return false;
		}

		VkResult err = VK_SUCCESS;

		#if defined(KUJOGFX_PLATFORM_WINDOWS)
		HWND handle = reinterpret_cast<HWND>(win_handle);

		HINSTANCE hInstance = GetModuleHandle(NULL);

		VkWin32SurfaceCreateInfoKHR win32_create_info;
		win32_create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		win32_create_info.pNext = NULL;
		win32_create_info.flags = 0;
		win32_create_info.hinstance = hInstance;
		win32_create_info.hwnd = handle;

		err = vkCreateWin32SurfaceKHR(instance, &win32_create_info, NULL, &surface);

		#elif defined(KUJOGFX_PLATFORM_LINUX)
		#if defined(KUJOGFX_IS_X11)
		Display *dpy = reinterpret_cast<Display*>(disp_handle);
		Window win = reinterpret_cast<Window>(win_handle);

		VkXlibSurfaceCreateInfoKHR xlib_create_info;
		xlib_create_info.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
		xlib_create_info.pNext = NULL;
		xlib_create_info.flags = 0;
		xlib_create_info.dpy = dpy;
		xlib_create_info.window = win;

		err = vkCreateXlibSurfaceKHR(instance, &xlib_create_info, NULL, &surface);

		#else
		#error "Vulkan surface creation is unimplemented on Wayland"
		#endif
		#else
		#error "Vulkan surface creation is unimplemented for this platform"
		#endif

		if (hasFailed(err))
		{
		    cout << "Could not create surface!" << endl;
		    return false;
		}

		return true;
	    }

	    int rateDeviceSuitability(VkPhysicalDevice device)
	    {
		VkPhysicalDeviceProperties device_properties;
		vkGetPhysicalDeviceProperties(device, &device_properties);

		int score = 0;

		// Discrete GPUs have a significant performance advantage
		if (device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{
		    score += 1000;
		}

		// Maximum possible size of textures affects image quality
		score += device_properties.limits.maxImageDimension2D;
		return score;
	    }

	    bool pickPhysicalDevice()
	    {
		uint32_t device_count = 0;
		VkResult err = vkEnumeratePhysicalDevices(instance, &device_count, NULL);

		if (hasFailed(err))
		{
		    kujogfxlog::error() << "Could not fetch number of physical devices!";
		    return false;
		}

		if (assertVk(device_count != 0))
		{
		    kujogfxlog::error() << "Could not find any GPUs with Vulkan support!";
		    return false;
		}

		vector<VkPhysicalDevice> devices(device_count);
		err = vkEnumeratePhysicalDevices(instance, &device_count, devices.data());

		if (hasFailed(err))
		{
		    kujogfxlog::error() << "Could not fetch physical devices!";
		    return false;
		}

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
		    kujogfxlog::error() << "Could not find suitable physical device!";
		    assertVk(false);
		    return false;
		}

		return true;
	    }

	    bool checkSwapchainSupport()
	    {
		uint32_t ext_count = 0;
		VkResult err = vkEnumerateDeviceExtensionProperties(physical_device, NULL, &ext_count, NULL);

		if (hasFailed(err))
		{
		    kujogfxlog::error() << "Could not fetch physical device extension properties count!";
		    return false;
		}

		if (assertVk(ext_count != 0))
		{
		    kujogfxlog::error() << "Physical device does not support any extensions!";
		    return false;
		}

		vector<VkExtensionProperties> ext_properties(ext_count);
		err = vkEnumerateDeviceExtensionProperties(physical_device, NULL, &ext_count, ext_properties.data());

		if (hasFailed(err))
		{
		    kujogfxlog::error() << "Could not fetch physical device extension properties!";
		    return false;
		}

		bool has_found_swapchain = false;

		for (const auto &property : ext_properties)
		{
		    string ext_name = property.extensionName;

		    if (ext_name == VK_KHR_SWAPCHAIN_EXTENSION_NAME)
		    {
			kujogfxlog::debug() << "Swapchain extension found!";
			has_found_swapchain = true;
			break;
		    }
		}

		if (assertVk(has_found_swapchain))
		{
		    kujogfxlog::error() << "Physical device does not support swapchains!";
		    return false;
		}

		return true;
	    }

	    bool findQueueFamilies()
	    {
		uint32_t queue_family_count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, NULL);

		if (assertVk(queue_family_count != 0))
		{
		    kujogfxlog::error() << "Physical device has no queue families!";
		    return false;
		}

		vector<VkQueueFamilyProperties> queue_family_properties(queue_family_count);
		vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_family_properties.data());

		bool has_graphics_queue_family = false;
		bool has_present_queue_family = false;

		for (size_t index = 0; index < queue_family_count; index++)
		{
		    VkBool32 present_support = VK_FALSE;
		    VkResult err = vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, index, surface, &present_support);

		    if (hasFailed(err))
		    {
			kujogfxlog::error() << "Could not check if physical device has present support!" << endl;
			return false;
		    }

		    if ((queue_family_properties[index].queueCount > 0) && (queue_family_properties[index].queueFlags & VK_QUEUE_GRAPHICS_BIT))
		    {
			graphics_queue_family = index;
			has_graphics_queue_family = true;

			if (present_support)
			{
			    present_queue_family = index;
			    has_present_queue_family = true;
			    break;
			}
		    }

		    if (!has_present_queue_family && present_support)
		    {
			present_queue_family = index;
			has_present_queue_family = true;
		    }
		}

		if (assertVk(has_graphics_queue_family && has_present_queue_family))
		{
		    kujogfxlog::error() << "Could not find valid graphics queue family!";
		    return false;
		}

		return true;
	    }

	    bool createLogicalDevice()
	    {
		float queue_priority = 1.0f;

		array<VkDeviceQueueCreateInfo, 2> queue_create_info = {{}};

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
		device_create_info.pQueueCreateInfos = queue_create_info.data();

		if (graphics_queue_family == present_queue_family)
		{
		    device_create_info.queueCreateInfoCount = 1;
		}
		else
		{
		    device_create_info.queueCreateInfoCount = 2;
		}

		const char *device_extensions = VK_KHR_SWAPCHAIN_EXTENSION_NAME;

		device_create_info.enabledExtensionCount = 1;
		device_create_info.ppEnabledExtensionNames = &device_extensions;

		VkResult err = vkCreateDevice(physical_device, &device_create_info, NULL, &device);

		if (hasFailed(err))
		{
		    kujogfxlog::error() << "Could not create logical device!";
		    return false;
		}

		vkGetDeviceQueue(device, graphics_queue_family, 0, &graphics_queue);
		vkGetDeviceQueue(device, present_queue_family, 0, &present_queue);

		return true;
	    }

	    bool createSwapchain()
	    {
		VkSurfaceCapabilitiesKHR capabilities;

		VkResult err = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &capabilities);

		if (hasFailed(err))
		{
		    kujogfxlog::error() << "Could not acquire presentation surface capabilities!";
		    return false;
		}

		uint32_t format_count = 0;
		err = vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, NULL);

		if (hasFailed(err))
		{
		    kujogfxlog::error() << "Could not fetch number of supported surface formats!";
		    return false;
		}

		if (assertVk(format_count != 0))
		{
		    kujogfxlog::error() << "No supported surface formats found!";
		    return false;
		}

		vector<VkSurfaceFormatKHR> surface_formats(format_count);

		err = vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, surface_formats.data());

		if (hasFailed(err))
		{
		    kujogfxlog::error() << "Could not fetch supported surface formats!";
		    return false;
		}

		uint32_t present_mode_count = 0;

		err = vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, NULL);

		if (hasFailed(err))
		{
		    kujogfxlog::error() << "Could not fetch supported presentation modes!";
		    return false;
		}

		if (assertVk(present_mode_count != 0))
		{
		    kujogfxlog::error() << "No supported presentation modes found!";
		    return false;
		}

		vector<VkPresentModeKHR> present_modes(present_mode_count);

		err = vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, present_modes.data());

		if (hasFailed(err))
		{
		    kujogfxlog::error() << "Could not fetch supported presentation modes!";
		    return false;
		}

		uint32_t image_count = (capabilities.minImageCount + 1);

		if ((capabilities.maxImageCount != 0) && (image_count > capabilities.maxImageCount))
		{
		    image_count = capabilities.maxImageCount;
		}

		VkSurfaceFormatKHR surface_format = chooseSurfaceFormat(surface_formats);

		swapchain_image_format = surface_format.format;
		swapchain_extent = chooseSwapExtent(capabilities);

		if (!(capabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT))
		{
		    cout << "Warning: swapchain image does not support VK_IMAGE_TRANSFER_DST usage" << endl;
		}

		VkSurfaceTransformFlagBitsKHR surface_transform;

		if (capabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
		{
		    surface_transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		}
		else
		{
		    surface_transform = capabilities.currentTransform;
		}

		VkPresentModeKHR present_mode = choosePresentMode(present_modes);

		VkSwapchainCreateInfoKHR create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		create_info.surface = surface;
		create_info.minImageCount = image_count;
		create_info.imageFormat = swapchain_image_format;
		create_info.imageColorSpace = surface_format.colorSpace;
		create_info.imageExtent = swapchain_extent;
		create_info.imageArrayLayers = 1;
		create_info.imageUsage = (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
		create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		create_info.queueFamilyIndexCount = 0;
		create_info.pQueueFamilyIndices = NULL;
		create_info.preTransform = surface_transform;
		create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		create_info.presentMode = present_mode;
		create_info.clipped = VK_TRUE;
		create_info.oldSwapchain = VK_NULL_HANDLE;

		err = vkCreateSwapchainKHR(device, &create_info, NULL, &swapchain);

		if (hasFailed(err))
		{
		    kujogfxlog::error() << "Could not create swapchain!";
		    return false;
		}

		uint32_t actual_image_count = 0;

		err = vkGetSwapchainImagesKHR(device, swapchain, &actual_image_count, NULL);

		if (hasFailed(err))
		{
		    kujogfxlog::error() << "Could not fetch swapchain images count!";
		    return false;
		}

		if (assertVk(actual_image_count != 0))
		{
		    kujogfxlog::error() << "No swapchain images found!";
		    return false;
		}

		swapchain_images.resize(actual_image_count);

		err = vkGetSwapchainImagesKHR(device, swapchain, &actual_image_count, swapchain_images.data());

		if (hasFailed(err))
		{
		    kujogfxlog::error() << "Could not acquire swapchain images!";
		    return false;
		}

		if (assertVk(createImageViews()))
		{
		    return false;
		}

		if (assertVk(createDepthResources()))
		{
		    return false;
		}

		return true;
	    }

	    VkSurfaceFormatKHR chooseSurfaceFormat(const vector<VkSurfaceFormatKHR> &available_formats)
	    {
		if ((available_formats.size() == 1) && (available_formats[0].format == VK_FORMAT_UNDEFINED))
		{
		    return {VK_FORMAT_B8G8R8A8_UNORM, VK_COLORSPACE_SRGB_NONLINEAR_KHR};
		}

		for (const auto &format : available_formats)
		{
		    if (format.format == VK_FORMAT_B8G8R8A8_UNORM)
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

	    VkFormat findSupportedFormat(const vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
	    {
		for (auto &format : candidates)
		{
		    VkFormatProperties properties;
		    vkGetPhysicalDeviceFormatProperties(physical_device, format, &properties);

		    if ((tiling == VK_IMAGE_TILING_LINEAR) && ((properties.linearTilingFeatures & features) == features))
		    {
			return format;
		    }
		    else if ((tiling == VK_IMAGE_TILING_OPTIMAL) && ((properties.optimalTilingFeatures & features) == features))
		    {
			return format;
		    }
		}

		assertVk(false);
		kujogfxlog::fatal() << "Could not find supported format!";
		return VK_FORMAT_UNDEFINED;
	    }

	    VkFormat findDepthFormat()
	    {
		return findSupportedFormat(
		    {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
		    VK_IMAGE_TILING_OPTIMAL,
		    VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
		);
	    }

	    bool hasStencilComponent(VkFormat format)
	    {
		return ((format == VK_FORMAT_D32_SFLOAT_S8_UINT) || (format == VK_FORMAT_D24_UNORM_S8_UINT));
	    }

	    bool createImageViews()
	    {
		swapchain_image_views.resize(swapchain_images.size());

		for (size_t i = 0; i < swapchain_images.size(); i++)
		{
		    VkResult err = createImageViewVk(swapchain_images.at(i), swapchain_image_format, VK_IMAGE_ASPECT_COLOR_BIT, swapchain_image_views[i]);

		    if (hasFailed(err))
		    {
			kujogfxlog::error() << "Could not create swapchain image views!";
			return false;
		    }
		}

		return true;
	    }

	    bool createDepthResources()
	    {
		VkFormat depth_format = findDepthFormat();

		VkResult err = createImageVk(swapchain_extent.width, swapchain_extent.height, depth_format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depth_image, depth_image_memory);

		if (hasFailed(err))
		{
		    kujogfxlog::error() << "Could not create depth image!";
		    return false;
		}

		err = createImageViewVk(depth_image, depth_format, VK_IMAGE_ASPECT_DEPTH_BIT, depth_image_view);

		if (hasFailed(err))
		{
		    kujogfxlog::error() << "Could not create depth image view!";
		    return false;
		}

		return true;
	    }

	    bool createCommandQueues()
	    {
		VkCommandPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		pool_info.queueFamilyIndex = present_queue_family;

		VkResult err = vkCreateCommandPool(device, &pool_info, NULL, &command_pool);

		if (hasFailed(err))
		{
		    kujogfxlog::error() << "Could not create command pool!";
		    return false;
		}

		command_buffers.resize(max_frames_in_flight);

		VkCommandBufferAllocateInfo alloc_info = {};
		alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		alloc_info.commandPool = command_pool;
		alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		alloc_info.commandBufferCount = uint32_t(command_buffers.size());

		err = vkAllocateCommandBuffers(device, &alloc_info, command_buffers.data());

		if (hasFailed(err))
		{
		    kujogfxlog::error() << "Could not allocate command buffers!";
		    return false;
		}

		return true;
	    }

	    bool createSyncObjects()
	    {
		image_available_semaphores.resize(max_frames_in_flight);
		render_finished_semaphores.resize(max_frames_in_flight);
		in_flight_fences.resize(max_frames_in_flight);

		VkSemaphoreCreateInfo semaphore_info = {};
		semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fence_info = {};
		fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < max_frames_in_flight; i++)
		{
		    VkResult err = vkCreateSemaphore(device, &semaphore_info, NULL, &image_available_semaphores[i]);

		    if (hasFailed(err))
		    {
			kujogfxlog::error() << "Could not create image semaphores!";
			return false;
		    }

		    err = vkCreateSemaphore(device, &semaphore_info, NULL, &render_finished_semaphores[i]);

		    if (hasFailed(err))
		    {
			kujogfxlog::error() << "Could not create render semaphores!";
			return false;
		    }

		    err = vkCreateFence(device, &fence_info, NULL, &in_flight_fences[i]);

		    if (hasFailed(err))
		    {
			kujogfxlog::error() << "Could not create fences!";
			return false;
		    }
		}

		return true;
	    }
    };
    #endif

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

	    void applyBindings(KujoGFXBindings bindings)
	    {
		KujoGFXCommand command(bindings);
		commands.push_back(command);
	    }

	    void applyUniforms(int ub_slot, KujoGFXData data)
	    {
		KujoGFXCommand command(ub_slot, data);
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
	    KujoGFXPipeline current_pipeline;

	    unordered_map<uint32_t, KujoGFXBuffer> buffer_cache;

	    bool is_initialized = false;

	    bool validatePlatformData(KujoGFXPlatformData data)
	    {
		if (data.window_handle == NULL)
		{
		    kujogfxlog::error() << "Window handle is not set";
		    return false;
		}

		if ((platform_data.context_handle != NULL) && (data.context_handle != NULL))
		{
		    kujogfxlog::error() << "Only window handle can be set after initialization!";
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
			case BackendDirect3D12: backend_ptr = new KujoGFX_D3D12(); break;
			#endif
			#if !defined(KUJOGFX_PLATFORM_EMSCRIPTEN)
			case BackendVulkan: backend_ptr = new KujoGFX_Vulkan(); break;
			#endif
			default: backend_ptr = new KujoGFX_Null(); break;
		    }

		    if (backend_ptr != NULL)
		    {
			if (backend_ptr->initBackend(platform_data.window_handle, platform_data.display_handle))
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
		    #if !defined(KUJOGFX_PLATFORM_EMSCRIPTEN)
		    case BackendVulkan: return "Vulkan"; break;
		    #endif
		    #if defined(KUJOGFX_PLATFORM_WINDOWS)
		    case BackendDirect3D11: return "Direct3D 11"; break;
		    case BackendDirect3D12: return "Direct3D 12"; break;
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
		    if (isBackendSelected(BackendDirect3D12))
		    {
			score += 40;
		    }

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
		    kujogfxlog::error() << "Could not fetch ntdll handle!";
		    return 0;
		}

		FARPROC (WINAPI *rtlGetVersion_ptr)(PRTL_OSVERSIONINFOW) = reinterpret_cast<FARPROC (WINAPI*)(PRTL_OSVERSIONINFOW)>(GetProcAddress(hmodule, "RtlGetVersion"));

		if (rtlGetVersion_ptr == NULL)
		{
		    kujogfxlog::error() << "Could not fetch address of RtlGetVersion()";
		    return 0;
		}

		rtlGetVersion_ptr(&os_vers);

		if (os_vers.dwMajorVersion == 0)
		{
		    kujogfxlog::error() << "Call to rtlGetVersion() failed!";
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
		switch (cmd.cmd_type)
		{
		    case CommandNop: break;
		    case CommandBeginPass: beginPassCmd(cmd.current_pass); break;
		    case CommandEndPass: endPassCmd(); break;
		    case CommandCommit: commitFrameCmd(); break;
		    case CommandApplyPipeline: applyPipelineCmd(cmd.current_pipeline); break;
		    case CommandApplyBindings: applyBindingsCmd(cmd.current_bindings); break;
		    case CommandApplyUniforms: applyUniformsCmd(cmd.current_uniform_slot, cmd.current_uniform_data); break;
		    case CommandDraw: drawCmd(cmd.current_draw_call); break;
		    default:
		    {
			kujogfxlog::fatal() << "Unrecognized command of " << dec << int(cmd.cmd_type);
			throw runtime_error("KujoGFX error");
		    }
		    break;
		}
	    }

	    void beginPassCmd(KujoGFXPass pass)
	    {
		assert(backend != NULL);
		backend->beginPass(pass);
	    }

	    void endPassCmd()
	    {
		assert(backend != NULL);
		backend->endPass();
	    }

	    void commitFrameCmd()
	    {
		assert(backend != NULL);
		backend->commitFrame();
	    }

	    size_t vertexFormatByteSize(KujoGFXVertexFormat format)
	    {
		switch (format)
		{
		    case VertexFormatFloat2: return 8; break;
		    case VertexFormatFloat3: return 12; break;
		    case VertexFormatFloat4: return 16; break;
		    case VertexFormatInvalid: return 0; break;
		    default: return 0; break;
		}
	    }

	    KujoGFXPipeline createPipeline(KujoGFXPipeline pipeline)
	    {
		KujoGFXPipeline init_pipeline = pipeline;

		for (size_t i = 0; i < max_vertex_buffer_bind_slots; i++)
		{
		    auto attrib = init_pipeline.layout.attribs[i];
		    size_t buffer_index = attrib.buffer_index;

		    if (attrib.format != VertexFormatInvalid)
		    {
			init_pipeline.layout.vertex_buffer_layout_active[buffer_index] = true;
		    }
		}

		array<int, max_vertex_buffer_bind_slots> auto_offs = {{0}};

		bool use_auto_offs = true;

		for (auto &attrib : pipeline.layout.attribs)
		{
		    if (attrib.offset != 0)
		    {
			use_auto_offs = false;
		    }
		}

		for (size_t i = 0; i < max_vertex_attribs; i++)
		{
		    auto &attrib = init_pipeline.layout.attribs[i];

		    if (attrib.format == VertexFormatInvalid)
		    {
			break;
		    }

		    size_t buffer_index = 0;

		    if (use_auto_offs)
		    {
			attrib.offset = auto_offs[buffer_index];
		    }

		    auto_offs[buffer_index] += vertexFormatByteSize(attrib.format);
		}

		for (size_t buf_index = 0; buf_index < max_vertex_buffer_bind_slots; buf_index++)
		{
		    auto &buffer = init_pipeline.layout.buffers[buf_index];

		    if (buffer.stride == 0)
		    {
			buffer.stride = auto_offs[buf_index];
		    }
		}

		return init_pipeline;
	    }

	    void applyPipelineCmd(KujoGFXPipeline pipeline)
	    {
		assert(backend != NULL);
		auto cached_pipeline = pipeline_cache.find(pipeline.getID());

		if (cached_pipeline != pipeline_cache.end())
		{
		    backend->setPipeline(cached_pipeline->second);
		    current_pipeline = cached_pipeline->second;
		}
		else
		{
		    auto init_pipeline = createPipeline(pipeline);
		    backend->createPipeline(init_pipeline);
		    pipeline_cache.insert(make_pair(pipeline.getID(), init_pipeline));
		    current_pipeline = init_pipeline;
		}

		backend->applyPipeline();
	    }

	    bool isBufferCached(KujoGFXBuffer &buffer)
	    {
		return (buffer_cache.find(buffer.getID()) != buffer_cache.end());
	    }

	    void insertBuffer(KujoGFXBuffer &buffer)
	    {
		if (isBufferCached(buffer))
		{
		    return;
		}

		buffer_cache.insert(make_pair(buffer.getID(), buffer));
	    }

	    void setupBuffer(KujoGFXBuffer &buffer)
	    {
		if ((buffer.getData() == NULL) || isBufferCached(buffer))
		{
		    return;
		}

		backend->createBuffer(buffer);
		insertBuffer(buffer);
	    }

	    void applyBindingsCmd(KujoGFXBindings bindings)
	    {
		assert(backend != NULL);
		for (size_t i = 0; i < bindings.vertex_buffers.size(); i++)
		{
		    if (!current_pipeline.layout.vertex_buffer_layout_active[i])
		    {
			continue;
		    }

		    setupBuffer(bindings.vertex_buffers.at(i));
		}

		setupBuffer(bindings.index_buffer);
		backend->applyBindings(bindings);
	    }

	    void applyUniformsCmd(int ub_slot, KujoGFXData data)
	    {
		assert(backend != NULL);
		backend->applyUniforms(ub_slot, data);
	    }

	    void drawCmd(KujoGFXDraw draw)
	    {
		assert(backend != NULL);
		backend->draw(draw);
	    }
    };
};


#endif // KUJOGFX_H