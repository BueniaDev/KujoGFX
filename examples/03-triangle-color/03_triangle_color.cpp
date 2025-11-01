#include <iostream>
#include <cstdint>
#include <kujogfx_helper.h>
using namespace std;

#include "example_03_shader.inl"

KujoGFX gfx;
KujoGFXHelper helper;

bool init()
{
    return helper.init("KujoGFX-triangle-color", 800, 600);
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
	0.0f, 0.5f, 0.5f,	1.0f, 0.0f, 0.0f, 1.0f,
	0.5f, -0.5f, 0.5f,	0.0f, 1.0f, 0.0f, 1.0f,
	-0.5f, -0.5f, 0.5f,	0.0f, 0.0f, 1.0f, 1.0f
    };

    KujoGFXBuffer buffer;
    buffer.setData(vertices);

    KujoGFXShader shader(example_03_vertex, example_03_fragment, example_03_locations);
    KujoGFXPipeline pipeline;
    pipeline.shader = shader;
    pipeline.layout.attribs[0].format = VertexFormatFloat3;
    pipeline.layout.attribs[1].format = VertexFormatFloat4;

    KujoGFXBindings bindings;
    bindings.vertex_buffers[0] = buffer;

    KujoGFXPassAction pass_action(KujoGFXColor(0.0, 0.0, 0.0, 1.0));

    helper.run([&]() -> void {
	gfx.beginPass(pass_action);
	gfx.applyPipeline(pipeline);
	gfx.applyBindings(bindings);
	gfx.draw(0, 3, 1);
	gfx.endPass();
	gfx.commit();
	gfx.frame();
    });

    gfx.shutdown();
    shutdown();
    return 0;
}
