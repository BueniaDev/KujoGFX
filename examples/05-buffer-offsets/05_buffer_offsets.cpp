#include <iostream>
#include <cstdint>
#include <kujogfx_helper.h>
using namespace std;

#include "example_05_shader.inl"

KujoGFX gfx;
KujoGFXHelper helper;

bool init()
{
    return helper.init("KujoGFX-buffer-offsets", 800, 600);
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

struct Vertex
{
    float xcoord = 0.f;
    float ycoord = 0.f;
    float red = 0.f;
    float green = 0.f;
    float blue = 0.f;
};

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

    const Vertex vertices[] =
    {
	// Positions       // Colors
	// Triangle
	{ 0.0f,   0.55f,   1.0f, 0.0f, 0.0f},
	{ 0.25f,  0.05f,   0.0f, 1.0f, 0.0f},
	{-0.25f,  0.05f,   0.0f, 0.0f, 1.0f},

	// Quad
	{-0.25f, -0.05f,   0.0f, 0.0f, 1.0f},
	{ 0.25f, -0.05f,   0.0f, 1.0f, 0.0f},
	{ 0.25f, -0.55f,   1.0f, 0.0f, 0.0f},
	{-0.25f, -0.55f,   1.0f, 1.0f, 1.0f}
    };

    const uint16_t indices[] =
    {
	// Triangle
	0, 1, 2,
	// Quad (first triangle)
	0, 1, 2,
	// Quad (second triangle)
	0, 2, 3
    };

    KujoGFXBuffer vert_buffer;
    vert_buffer.setData(vertices);

    KujoGFXBuffer index_buffer;
    index_buffer.setIndexBuffer();
    index_buffer.setData(indices);

    KujoGFXShader shader(example_05_vertex, example_05_fragment, example_05_locations);
    KujoGFXPipeline pipeline;
    pipeline.shader = shader;
    pipeline.index_type = IndexTypeUint16;
    pipeline.layout.attribs[0].format = VertexFormatFloat2;
    pipeline.layout.attribs[1].format = VertexFormatFloat3;

    KujoGFXPassAction pass_action(KujoGFXColor(0.0, 0.0, 0.0, 1.0));

    helper.run([&]() -> void {

	KujoGFXBindings bind_triangle;
	bind_triangle.vertex_buffers[0] = vert_buffer;
	bind_triangle.index_buffer = index_buffer;

	KujoGFXBindings bind_quad;
	bind_quad.vertex_buffers[0] = vert_buffer;
	bind_quad.vertex_buffer_offsets[0] = (3 * sizeof(Vertex));
	bind_quad.index_buffer = index_buffer;
	bind_quad.index_buffer_offset = (3 * sizeof(uint16_t));

	gfx.beginPass(pass_action);
	gfx.applyPipeline(pipeline);
	gfx.applyBindings(bind_triangle);
	gfx.draw(0, 3, 1);
	gfx.applyBindings(bind_quad);
	gfx.draw(0, 6, 1);
	gfx.endPass();
	gfx.commit();
	gfx.frame();
    });

    gfx.shutdown();
    shutdown();
    return 0;
}
