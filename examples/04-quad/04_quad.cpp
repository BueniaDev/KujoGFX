#include <iostream>
#include <cstdint>
#include <kujogfx_helper.h>
using namespace std;

#include "example_04_shader.inl"

KujoGFX gfx;
KujoGFXHelper helper;

bool init()
{
    return helper.init("KujoGFX-quad", 800, 600);
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
	// Positions		// Colors
	-0.5f,  0.5f, 0.5f,	1.0f, 0.0f, 0.0f, 1.0f,
	 0.5f,  0.5f, 0.5f,	0.0f, 1.0f, 0.0f, 1.0f,
	 0.5f, -0.5f, 0.5f,	0.0f, 0.0f, 1.0f, 1.0f,
	-0.5f, -0.5f, 0.5f,     1.0f, 1.0f, 1.0f, 1.0f
    };

    const uint16_t indices[] =
    {
	// First triangle
	0, 1, 2,
	// Second triangle
	0, 2, 3
    };

    KujoGFXBuffer vert_buffer;
    vert_buffer.setData(vertices);

    KujoGFXBuffer index_buffer;
    index_buffer.setIndexBuffer();
    index_buffer.setData(indices);

    KujoGFXShader shader(example_04_vertex, example_04_fragment, example_04_locations);
    KujoGFXPipeline pipeline;
    pipeline.shader = shader;
    pipeline.index_type = IndexTypeUint16;
    pipeline.layout.attribs[0].offset = 0;
    pipeline.layout.attribs[0].format = VertexFormatFloat3;
    pipeline.layout.attribs[1].offset = 12;
    pipeline.layout.attribs[1].format = VertexFormatFloat4;

    KujoGFXBindings bindings;
    bindings.vertex_buffers[0] = vert_buffer;
    bindings.index_buffer = index_buffer;

    KujoGFXPassAction pass_action(KujoGFXColor(0.0, 0.0, 0.0, 1.0));

    helper.run([&]() -> void {
	gfx.beginPass(pass_action);
	gfx.applyPipeline(pipeline);
	gfx.applyBindings(bindings);
	gfx.draw(0, 6, 1);
	gfx.endPass();
	gfx.commit();
	gfx.frame();
    });

    gfx.shutdown();
    shutdown();
    return 0;
}
