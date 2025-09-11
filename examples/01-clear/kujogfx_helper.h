#ifndef KUJOGFX_HELPER_H
#define KUJOGFX_HELPER_H

#include <iostream>
#include <cstdint>
#include <functional>
#include "kujogfx.h"
#if defined(KUJOGFX_PLATFORM_EMSCRIPTEN)
#else
#include <SDL3/SDL.h>
#endif
using namespace kujogfx;
using namespace std;

class KujoGFXHelper
{
    public:
	KujoGFXHelper()
	{

	}

	~KujoGFXHelper()
	{

	}

	bool init(string name, size_t width, size_t height)
	{
	    #if defined(KUJOGFX_PLATFORM_EMSCRIPTEN)
	    return initEmscipten();
	    #else
	    return initSDL(name, width, height);
	    #endif
	}

	void shutdown()
	{
	    #if defined(KUJOGFX_PLATFORM_EMSCRIPTEN)
	    shutdownEmscripten();
	    #else
	    shutdownSDL();
	    #endif
	}

	void* getWindowHandle()
	{
	    #if defined(KUJOGFX_PLATFORM_EMSCRIPTEN)
	    return getWindowHandleEmscripten();
	    #else
	    return getWindowHandleSDL(window);
	    #endif
	}

	void* getDisplayHandle()
	{
	    #if defined(KUJOGFX_PLATFORM_EMSCRIPTEN)
	    return getDisplayHandleEmscripten();
	    #else
	    return getDisplayHandleSDL(window);
	    #endif
	}

	void run(function<void()> func)
	{
	    if (!func)
	    {
		return;
	    }

	    #if defined(KUJOGFX_PLATFORM_EMSCRIPTEN)
	    runEmscripten(func);
	    #else
	    runSDL(func);
	    #endif
	}

	#if defined(KUJOGFX_PLATFORM_EMSCRIPTEN)
	static constexpr bool use_emscripten = true;

	bool initEmscripten()
	{
	    return true;
	}

	void shutdownEmscripten()
	{
	}

	void* getWindowHandleEmscripten()
	{
	    return NULL;
	}

	void *getDisplayHandleEmscripten()
	{
	    return NULL;
	}

	void runEmscripten(function<void()>)
	{
	}

	#else
	static constexpr bool use_emscripten = false;

	SDL_Window *window = NULL;

	bool sdlError(string msg)
	{
	    cout << msg << " SDL_Error: " << SDL_GetError() << endl;
	    return false;
	}

	bool initSDL(string name, size_t width, size_t height)
	{
	    if (!SDL_Init(SDL_INIT_VIDEO))
	    {
		return sdlError("SDL could not be initialized!");
	    }

	    window = SDL_CreateWindow(name.c_str(), width, height, SDL_WINDOW_RESIZABLE);

	    if (window == NULL)
	    {
		return sdlError("Window could not be created!");
	    }

	    return true;
	}

	void shutdownSDL()
	{
	    if (window != NULL)
	    {
		SDL_DestroyWindow(window);
		window = NULL;
	    }

	    SDL_Quit();
	}

	void* getWindowHandleSDL(SDL_Window *win)
	{
	    #if defined(KUJOGFX_PLATFORM_WINDOWS)
	    return (void*)SDL_GetPointerProperty(SDL_GetWindowProperties(win), SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);
	    #elif defined(KUJOGFX_PLATFORM_MACOS)
	    return (void*)SDL_GetPointerProperty(SDL_GetWindowProperties(win), SDL_PROP_WINDOW_COCOA_WINDOW_POINTER, NULL);
	    #elif defined(KUJOGFX_PLATFORM_LINUX) || defined(KUJOGFX_PLATFORM_BSD)
	    #if defined(KUJOGFX_IS_WAYLAND)
	    return (void*)SDL_GetPointerProperty(SDL_GetWindowProperties(win), SDL_PROP_WINDOW_WAYLAND_SURFACE_POINTER, NULL);
	    #elif defined(KUJOGFX_IS_X11)
	    return (void*)SDL_GetNumberProperty(SDL_GetWindowProperties(win), SDL_PROP_WINDOW_X11_WINDOW_NUMBER, 0);
	    #endif
	    #elif defined(KUJOGFX_PLATFORM_ANDROID)
	    return (void*)SDL_GetPointerProperty(SDL_GetWindowProperties(win), SDL_PROP_WINDOW_ANDROID_WINDOW_POINTER, NULL);
	    #else
	    return NULL;
	    #endif
	}

	void* getDisplayHandleSDL(SDL_Window *win)
	{
	    #if defined(KUJOGFX_PLATFORM_LINUX) || defined(KUJOGFX_PLATFORM_BSD)
	    #if defined(KUJOGFX_IS_WAYLAND)
	    return (void*)SDL_GetPointerProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_WAYLAND_DISPLAY_POINTER, NULL);
	    #elif defined(KUJOGFX_IS_X11)
	    return (void*)SDL_GetPointerProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_X11_DISPLAY_POINTER, NULL);
	    #else
	    return NULL;
	    #endif
	    #else
	    return NULL;
	    #endif
	}

	void runSDL(function<void()> func)
	{
	    if (!func)
	    {
		return;
	    }

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

		func();
	    }

	}

	#endif
};


#endif // KUJOGFX_HELPER_H