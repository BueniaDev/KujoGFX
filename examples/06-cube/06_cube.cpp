#include <iostream>
#include <cstdint>
#include <chrono>
#include <kujogfx_helper.h>
#include <kujomath.h>
using namespace kujomath;
using namespace std;
using namespace std::chrono;

#include "example_06_shader.inl"

KujoGFX gfx;
KujoGFXHelper helper;

bool init()
{
    return helper.init("KujoGFX-cube", 800, 600);
}

void shutdown()
{
    helper.shutdown();
}

void* getWindowHandle()
{
    return helper.getWindowHandle();
}

void* getDisplayHandle()
{
    return helper.getDisplayHandle();
}

vs_params_t computeVSParams(float rx, float ry)
{
    float width = helper.getWidthF();
    float height = helper.getHeightF();

    KujoMat4x4F proj = perspectiveFovRH(toRadians(60.f), (width / height), 0.01f, 10.f);
    KujoMat4x4F view = lookAtRH(KujoVec3F(0.f, 1.5f, 4.0f), KujoVec3F(0.f, 0.f, 0.f), KujoVec3F(0.f, 1.0, 0.f));
    KujoMat4x4F view_proj = (view * proj);
    KujoMat4x4F rxm = rotateX(toRadians(rx));
    KujoMat4x4F rym = rotateY(toRadians(ry));
    KujoMat4x4F model = (rym * rxm);

    KujoMat4x4F mvp = (model * view_proj);
    vs_params_t params;
    params.mvp = mvp;
    return params;
}

int main()
{
    if (!init())
    {
	return 1;
    }

    KujoGFXPlatformData pform_data;
    pform_data.window_handle = getWindowHandle();
    pform_data.display_handle = getDisplayHandle();

    KujoGFX gfx;
    // gfx.setBackend(BackendDirect3D11);
    // gfx.setBackend(BackendOpenGL);
    gfx.setBackend(BackendVulkan);

    if (!gfx.init(pform_data))
    {
	cout << "Could not initialize KujoGFX." << endl;
	return 1;
    }

    const float vertices[] = 
    {
	-1.0, -1.0, -1.0,     1.0, 0.0, 0.0, 1.0,
	 1.0, -1.0, -1.0,     1.0, 0.0, 0.0, 1.0,
	 1.0,  1.0, -1.0,     1.0, 0.0, 0.0, 1.0,
	-1.0,  1.0, -1.0,     1.0, 0.0, 0.0, 1.0,

	-1.0, -1.0,  1.0,     0.0, 1.0, 0.0, 1.0,
	 1.0, -1.0,  1.0,     0.0, 1.0, 0.0, 1.0,
	 1.0,  1.0,  1.0,     0.0, 1.0, 0.0, 1.0,
	-1.0,  1.0,  1.0,     0.0, 1.0, 0.0, 1.0,

	-1.0, -1.0, -1.0,     0.0, 0.0, 1.0, 1.0,
	-1.0,  1.0, -1.0,     0.0, 0.0, 1.0, 1.0,
	-1.0,  1.0,  1.0,     0.0, 0.0, 1.0, 1.0,
	-1.0, -1.0,  1.0,     0.0, 0.0, 1.0, 1.0,

	 1.0, -1.0, -1.0,     1.0, 0.5, 0.0, 1.0,
	 1.0,  1.0, -1.0,     1.0, 0.5, 0.0, 1.0,
	 1.0,  1.0,  1.0,     1.0, 0.5, 0.0, 1.0,
	 1.0, -1.0,  1.0,     1.0, 0.5, 0.0, 1.0,

	-1.0, -1.0, -1.0,     0.0, 0.5, 1.0, 1.0,
	-1.0, -1.0,  1.0,     0.0, 0.5, 1.0, 1.0,
	 1.0, -1.0,  1.0,     0.0, 0.5, 1.0, 1.0,
	 1.0, -1.0, -1.0,     0.0, 0.5, 1.0, 1.0,

	-1.0,  1.0, -1.0,     1.0, 0.0, 0.5, 1.0,
	-1.0,  1.0,  1.0,     1.0, 0.0, 0.5, 1.0,
	 1.0,  1.0,  1.0,     1.0, 0.0, 0.5, 1.0,
	 1.0,  1.0, -1.0,     1.0, 0.0, 0.5, 1.0
    };

    const uint16_t indices[] =
    {
	0, 1, 2, 0, 2, 3,
	6, 5, 4, 7, 6, 4,
	8, 9, 10, 8, 10, 11,
	14, 13, 12, 15, 14, 12,
	16, 17, 18, 16, 18, 19,
	22, 21, 20, 23, 22, 20
    };

    KujoGFXBuffer vert_buffer;
    vert_buffer.setData(vertices);

    KujoGFXBuffer index_buffer;
    index_buffer.setIndexBuffer();
    index_buffer.setData(indices);

    KujoGFXShader shader(example_06_vertex, example_06_fragment, example_06_locations, example_06_uniforms);

    KujoGFXPipeline pipeline;
    pipeline.shader = shader;
    pipeline.index_type = IndexTypeUint16;
    pipeline.layout.buffers[0].stride = 28;
    pipeline.layout.attribs[0].format = VertexFormatFloat3;
    pipeline.layout.attribs[1].format = VertexFormatFloat4;
    pipeline.cull_mode = CullModeBack;
    pipeline.depth_state.is_write_enabled = true;
    pipeline.depth_state.compare_func = CompareFuncLessEqual;

    KujoGFXBindings bindings;
    bindings.vertex_buffers[0] = vert_buffer;
    bindings.index_buffer = index_buffer;

    KujoGFXPassAction pass_action(KujoGFXColor(0.0, 0.0, 0.0, 1.0));

    float rx = 0.f;
    float ry = 0.f;

    auto prev_time = steady_clock::now();

    helper.run([&]() -> void {
	auto current_time = steady_clock::now();
	double delta_time = duration<double>(current_time - prev_time).count();
	prev_time = current_time;

	float t = (float(delta_time) * 60.f);

	rx += (1.f * t);
	ry += (2.f * t);

	vs_params_t vs_params = computeVSParams(rx, ry);
 	KujoGFXData vs_data;
	vs_data.setData(vs_params);

	gfx.beginPass(pass_action);
	gfx.applyPipeline(pipeline);
	gfx.applyBindings(bindings);
	gfx.applyUniforms(0, vs_data);
	gfx.draw(0, 36, 1);
	gfx.endPass();
	gfx.commit();
	gfx.frame();
    });

    gfx.shutdown();
    shutdown();
    return 0;
}
