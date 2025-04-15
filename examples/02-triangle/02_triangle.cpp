#include <iostream>
#include <cstdint>
#include <SDL3/SDL.h>
#include "kujogfx.h"
using namespace kujogfx;
using namespace std;

SDL_Window *window = NULL;
KujoGFX gfx;

string vert_source = R"(
    #version 450
    layout (location = 0) out vec4 frag_color;

    vec3 positions[3] = vec3[](
	vec3(0.0, 0.5, 0.5),
	vec3(0.5, -0.5, 0.5),
	vec3(-0.5, -0.5, 0.5)
    );

    vec3 colors[3] = vec3[](
	vec3(1.0, 0.0, 0.0),
	vec3(0.0, 1.0, 0.0),
	vec3(0.0, 0.0, 1.0)
    );

    void main()
    {
	gl_Position = vec4(positions[gl_VertexIndex], 1.0);
	frag_color = vec4(colors[gl_VertexIndex], 1.0);
    }
)";

string frag_source = R"(
    #version 450
    layout (location = 0) in vec4 frag_color;
    layout (location = 0) out vec4 out_color;

    void main()
    {
	out_color = frag_color;
    }
)";

bool sdlError(string msg)
{
    cout << msg << " SDL_Error: " << SDL_GetError() << endl;
    return false;
}

bool initSDL()
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
	return sdlError("SDL could not be initialized!");
    }

    auto flags = SDL_WINDOW_RESIZABLE;

    window = SDL_CreateWindow("KujoGFX-triangle", 800, 600, flags);

    if (window == NULL)
    {
	return sdlError("Window could not be created!");
    }

    return true;
}

void shutdownSDL()
{
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void* getWindowHandle(SDL_Window *win)
{
    #if defined(KUJOGFX_PLATFORM_WINDOWS)
    return (void*)SDL_GetPointerProperty(SDL_GetWindowProperties(win), SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);
    #elif defined(KUJOGFX_PLATFORM_MACOS)
    return (void*)SDL_GetPointerProperty(SDL_GetWindowProperties(win), SDL_PROP_WINDOW_COCOA_WINDOW_POINTER, NULL);
    #elif defined(KUJOGFX_PLATFORM_LINUX) || defined(KUJOGFX_PLATFORM_BSD)
    if (SDL_strcmp(SDL_GetCurrentVideoDriver(), "wayland") == 0)
    {
	return (void*)SDL_GetPointerProperty(SDL_GetWindowProperties(win), SDL_PROP_WINDOW_WAYLAND_SURFACE_POINTER, NULL);
    }
    else
    {
	return (void*)&SDL_GetNumberPropery(SDL_GetWindowProperties(win), SDL_PROP_WINDOW_X11_WINDOW_NUMBER, 0);
    }
    #elif defined(KUJOGFX_PLATFORM_ANDROID)
    return (void*)SDL_GetPointerProperty(SDL_GetWindowProperties(win), SDL_PROP_WINDOW_ANDROID_WINDOW_POINTER, NULL);
    #elif defined(KUJOGFX_PLATFORM_EMSCRIPTEN)
    return (void*)SDL_GetStringProperty(SDL_GetWindowProperties(win), SDL_PROP_WINDOW_EMSCRIPTEN_CANVAS_ID_STRING, NULL);
    #else
    return NULL;
    #endif
}

int main(int argc, char *argv[])
{
    if (!initSDL())
    {
	return 1;
    }

    KujoGFXPlatformData pform_data;
    pform_data.window_handle = getWindowHandle(window);

    KujoGFX gfx;
    // gfx.setBackend(BackendOpenGL);
    // gfx.setBackend(BackendVulkan);

    if (!gfx.init(pform_data))
    {
	cout << "Could not initialize KujoGFX." << endl;
	return 1;
    }

    KujoGFXShader shader(vert_source, frag_source);
    KujoGFXPipeline pipeline;
    pipeline.shader = shader;

    KujoGFXPassAction pass_action(KujoGFXColor(0.0, 0.0, 0.0, 1.0));

    bool quit = false;
    SDL_Event event;

    while (!quit)
    {
	while (SDL_PollEvent(&event))
	{
	    switch (event.type)
	    {
		case SDL_EVENT_QUIT: quit = true; break;
	    }
	}

	gfx.beginPass(pass_action);
	gfx.applyPipeline(pipeline);
	gfx.draw(0, 3, 1);
	gfx.endPass();
	gfx.commit();
	gfx.frame();
    }

    gfx.shutdown();
    shutdownSDL();
    return 0;
}
