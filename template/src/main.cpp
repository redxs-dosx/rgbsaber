#include "main.hpp"

#include "scotland2/shared/modloader.h"
#include <chrono>
#include <cmath>

static modloader::ModInfo modInfo{MOD_ID, VERSION, 0};
// Stores the ID and version of our mod, and is sent to
// the modloader upon startup

// Loads the config from disk using our modInfo, then returns it for use
// other config tools such as config-utils don't use this config, so it can be
// removed if those are in use
Configuration &getConfig() {
  static Configuration config(modInfo);
  return config;
}

// Called at the early stages of game loading
MOD_EXTERN_FUNC void setup(CModInfo *info) noexcept {
  *info = modInfo.to_c();

  getConfig().Load();

  // File logging
  Paper::Logger::RegisterFileContextId(PaperLogger.tag);

  PaperLogger.info("Completed setup!");
}

// Called later on in the game loading - a good time to install function hooks
MOD_EXTERN_FUNC void late_load() noexcept {
  il2cpp_functions::Init();

  PaperLogger.info("Installing hooks...");
  // Hook ColorManager::ColorForType to return a time-based rainbow color
  MAKE_HOOK_FIND_CLASS(ColorManager_ColorForType, "GlobalNamespace", "ColorManager", "ColorForType", UnityEngine::Color,
                       GlobalNamespace::ColorManager* self, GlobalNamespace::ColorType colorType) {
    // Use steady clock to create a hue that cycles over time
    using namespace std::chrono;
    static const double speed = 0.2; // cycles per second
    double t = duration_cast<duration<double>>(steady_clock::now().time_since_epoch()).count();
    float hue = fmod(static_cast<float>(t * speed), 1.0f);
    return UnityEngine::Color::HSVToRGB(hue, 1.0f, 1.0f);
  }

  INSTALL_HOOK(PaperLogger, ColorManager_ColorForType);

  PaperLogger.info("Installed all hooks!");
}