#include <AK/SoundEngine/Common/IAkPlugin.h>
#include <AK/Plugin/AkMotionGeneratorSourceFactory.h>
#include <AK/Plugin/AkMotionSinkFactory.h>
#include <AK/Plugin/AkMotionSourceSourceFactory.h>
DEFINE_PLUGIN_REGISTER_HOOK

#ifdef _DEBUG
AkAssertHook g_pAssertHook = NULL;
#endif