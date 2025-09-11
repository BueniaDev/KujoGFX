#include <iostream>
#include <cstdint>
#include "kujogfx_helper.h"
using namespace std;

KujoGFX gfx;
KujoGFXHelper helper;

bool init()
{
    return helper.init("KujoGFX-triangle-vertex", 800, 600);
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

int main(int argc, char *argv[])
{
    if (!init())
    {
	return 1;
    }

    KujoGFXPlatformData pform_data;
    pform_data.window_handle = getWindowHandle();
    pform_data.display_handle = getDisplayHandle();

    KujoGFX gfx;
    gfx.setBackend(BackendOpenGL);
    // gfx.setBackend(BackendVulkan);

    if (!gfx.init(pform_data))
    {
	cout << "Could not initialize KujoGFX." << endl;
	return 1;
    }

    const float vertices[] =
    {
	// positions		// colors
	0.0f, 0.5f, 0.5f,	1.0f, 0.0f, 0.0f, 1.0f,
	0.5f, -0.5f, 0.5f,	0.0f, 1.0f, 0.0f, 1.0f,
	-0.5f, -0.5f, 0.5f,	0.0f, 0.0f, 1.0f, 1.0f
    };

    string vert_source = R"(
	#version 450
	layout (location=0) in vec4 position;
	layout (location=1) in vec4 in_color;
	layout (location=0) out vec4 color;

	void main()
	{
	    gl_Position = position;
	    color = in_color;
	}
    )";

    string frag_source = R"(
	#version 450
	layout (location=0) in vec4 color;
	layout (location=0) out vec4 frag_color;

	void main()
	{
	    frag_color = color;
	}
    )";

    KujoGFXBuffer buffer;
    buffer.setData(vertices);

    KujoGFXShader shader(vert_source, frag_source);
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
