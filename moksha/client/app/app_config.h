/**
 * Startup settings of a client: the command line (AppArgs), the requested configuration (AppConfig) and the
 * actual window (WindowState). AppConfig is filled in layers, later ones winning: engine defaults, the game's
 * CreateGame, settings.ini, then the command line.
 */
#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

#include "moksha/shared/file_io/ini/ini_parser.h"

namespace ME {

/**
 * The process's command-line arguments. Holds pointers to argv's strings, which live for the whole process.
 */
class AppArgs {
   public:
    // Arguments past this are ignored.
    static constexpr int MaxCount = 64;

    // Keeps argv[0 .. argc-1]; argv[0] is the executable.
    void Set(int argc, char** argv);

    int GetCount() const;

    // The argument at index, or nullptr when out of range.
    const char* Get(int index) const;

    // Value of "--name=value", or nullptr when that argument is absent.
    const char* GetValue(const char* name) const;

   private:
    int count = 0;
    const char* values[MaxCount] = {};
};

/**
 * What the app asks for at startup. Window size is a request: desktop uses it as the client area, mobile and
 * web decide the size themselves, headless keeps it for layout. Settings a platform can't honour are ignored.
 */
class AppConfig {
   public:
    std::string windowTitle = "Moksha";
    uint16_t windowWidth = 1280;
    uint16_t windowHeight = 720;
    bool vsync = true;
    bool fullscreen = false;  // not applied yet on any platform

    double fixedStepFPS = 60.0;

    std::string serverIP = "127.0.0.1";
    uint16_t serverPort = 9310;
    std::string contentServerIP = "127.0.0.1";
    uint16_t contentServerPort = 9311;

    // Applies [window] (title, width, height, vsync, fullscreen), [time] (fixedStepFPS) and
    // [net] (serverIP, serverPort, contentServerIP, contentServerPort). Missing keys keep their current value.
    void ApplySettings(const INIMap& settings);

    // Applies "--key=value" arguments, using the same key names as ApplySettings.
    void ApplyArgs(const AppArgs& args);

   private:
    // Sets the field named key. False for an unknown key or an unreadable value.
    bool ApplyValue(const std::string& key, const std::string& value);
};

/**
 * The window the app actually got. Systems that lay out or render against the screen read it from here.
 */
class WindowState {
   public:
    uint16_t width = 0;
    uint16_t height = 0;
    float dpiScale = 1.0f;
};

}  // namespace ME
