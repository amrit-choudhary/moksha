#pragma once

/**
 * JSON Utilities Header.
 * This will load a json file and parse it into a game specific class.
 */

// Forward declarations
class cJSON;

namespace ME {

// Forward Declarations
class TextureAtlasProperties;
class SpriteAnimClip;

class JsonUtils {
   public:
    JsonUtils() = default;
    ~JsonUtils() = default;

    /**
     * Load texture atlas properties from a JSON file.
     * Returns True if the properties were loaded successfully, false otherwise.
     */
    static bool LoadTextureAtlasProps(const char* filePath, ME::TextureAtlasProperties& outAtlasProps);

    /**
     * Load sprite animation clips from a JSON file.
     * Returns True if the clips was loaded successfully, false otherwise.
     */
    static bool LoadSpriteAnimClipFromJSON(const char* filePath, ME::SpriteAnimClip** outSpriteAnimClip);

   private:
    /**
     * Common function to load a JSON file and parse it into a cJSON object.
     * Returns nullptr if the file could not be opened or parsed.
     * Responsibility of the caller to delete the returned cJSON object.
     */
    static cJSON* LoadJSONFromFile(const char* filePath);
};

}  // namespace ME
