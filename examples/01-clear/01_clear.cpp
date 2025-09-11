#include <iostream>
#include <cstdint>
#include "kujogfx_helper.h"
using namespace std;

KujoGFX gfx;
KujoGFXHelper helper;

bool init()
{
    return helper.init("KujoGFX-clear", 800, 600);
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

    KujoGFXPassAction pass_action(KujoGFXColor(0.0, 0.0, 1.0, 1.0));

    helper.run([&]() -> void {
	gfx.beginPass(pass_action);
	gfx.endPass();
	gfx.commit();
	gfx.frame();
    });

    gfx.shutdown();
    shutdown();
    return 0;
}
