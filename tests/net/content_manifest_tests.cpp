/**
 * Tests for ContentManifest parsing, editing and path safety.
 */

#include <cstring>
#include <string>

#include "moksha/shared/net/content_manifest.h"
#include "moksha/test_framework/test_framework.h"

using ME::Net::ContentManifest;

static bool ParseText(ContentManifest& manifest, const char* text) {
    return manifest.Parse(text, std::strlen(text));
}

TEST(ContentManifest, ParseValid) {
    ContentManifest manifest;
    ASSERT(
        ParseText(manifest, R"({"files":[{"name":"config/economy.json","version":3},{"name":"a.txt","version":0}]})"));

    EXPECT(manifest.GetCount() == 2);
    const ME::Net::ManifestEntry* entry = manifest.Find("config/economy.json");
    ASSERT(entry != nullptr);
    EXPECT(entry->version == 3);
    EXPECT(manifest.Find("missing.json") == nullptr);
}

TEST(ContentManifest, ParseRejectsInvalidAndLeavesEmpty) {
    ContentManifest manifest;
    manifest.Set("keep.txt", 1);

    EXPECT(!ParseText(manifest, "not json"));
    EXPECT(manifest.GetCount() == 0);
    EXPECT(!ParseText(manifest, R"({"nofiles":[]})"));
    EXPECT(!ParseText(manifest, R"({"files":[{"name":"../secret","version":1}]})"));
    EXPECT(!ParseText(manifest, R"({"files":[{"name":"a.txt","version":1.5}]})"));
    EXPECT(!ParseText(manifest, R"({"files":[{"name":"a.txt","version":-1}]})"));
    EXPECT(!ParseText(manifest, R"({"files":[{"name":"a.txt","version":1},{"name":"a.txt","version":2}]})"));
    EXPECT(manifest.GetCount() == 0);
}

TEST(ContentManifest, SetUpdatesExistingEntry) {
    ContentManifest manifest;
    EXPECT(manifest.Set("a.txt", 1));
    EXPECT(manifest.Set("a.txt", 2));

    EXPECT(manifest.GetCount() == 1);
    EXPECT(manifest.GetEntry(0).version == 2);
}

TEST(ContentManifest, RemoveKeepsOrder) {
    ContentManifest manifest;
    manifest.Set("a.txt", 1);
    manifest.Set("b.txt", 2);
    manifest.Set("c.txt", 3);

    EXPECT(manifest.Remove("b.txt"));
    EXPECT(!manifest.Remove("b.txt"));
    ASSERT(manifest.GetCount() == 2);
    EXPECT(manifest.GetEntry(0).name == "a.txt");
    EXPECT(manifest.GetEntry(1).name == "c.txt");
}

TEST(ContentManifest, SerializeRoundTrip) {
    ContentManifest source;
    source.Set("config/economy.json", 3);
    source.Set("levels/one.json", 7);

    std::string json;
    ASSERT(source.Serialize(json));

    ContentManifest parsed;
    ASSERT(parsed.Parse(json.c_str(), json.size()));
    EXPECT(parsed.GetCount() == 2);
    const ME::Net::ManifestEntry* entry = parsed.Find("levels/one.json");
    ASSERT(entry != nullptr);
    EXPECT(entry->version == 7);
}

TEST(ContentManifest, IsSafePath) {
    EXPECT(ContentManifest::IsSafePath("config/economy.json"));
    EXPECT(ContentManifest::IsSafePath("a.txt"));

    EXPECT(!ContentManifest::IsSafePath(""));
    EXPECT(!ContentManifest::IsSafePath("/etc/passwd"));
    EXPECT(!ContentManifest::IsSafePath("../secret"));
    EXPECT(!ContentManifest::IsSafePath("a/../b"));
    EXPECT(!ContentManifest::IsSafePath("a/./b"));
    EXPECT(!ContentManifest::IsSafePath("a//b"));
    EXPECT(!ContentManifest::IsSafePath("a\\b"));
    EXPECT(!ContentManifest::IsSafePath("C:file"));

    std::string tooLong(ContentManifest::MAX_PATH_LENGTH + 1, 'a');
    EXPECT(!ContentManifest::IsSafePath(tooLong.c_str()));
}
