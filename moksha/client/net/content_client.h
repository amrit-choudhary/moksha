/**
 * Talks to the content server in the background, one step per frame, never blocking the game.
 * Downloads new or changed files straight into dlc/ and keeps dlc/manifest.json describing exactly what is
 * on disk: a file's version is recorded only after the file is fully written, so an interrupted sync resumes.
 */
#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

#include "moksha/shared/net/content_manifest.h"
#include "moksha/shared/net/message_framing.h"
#include "moksha/shared/net/tcp_socket.h"

namespace ME {

enum class ContentSyncState : uint8_t {
    Idle,                // Init not called yet.
    Connecting,          // TCP handshake in progress.
    RequestingManifest,  // GET_MANIFEST sent, waiting for MANIFEST.
    DownloadingFiles,    // One GET_FILE at a time for every new or changed file.
    Done,                // Finished; connection closed.
    Failed,              // Gave up (server down, timeout, bad reply); the game carries on without new content.
};

class ContentClient {
   public:
    ContentClient();
    ~ContentClient();

    // Owns a heap receive buffer that the receiver points into; copying would break both.
    ContentClient(const ContentClient&) = delete;
    ContentClient& operator=(const ContentClient&) = delete;

    /**
     * Starts connecting. now is the game's time in seconds (TimeManager::GetTimeSinceStartup).
     */
    void Init(const char* serverIP, uint16_t serverPort, double now);

    /**
     * Advances the sync by whatever is possible right now. Call once per frame.
     */
    void Update(double now);

    void End();

    ContentSyncState GetState() const;

   private:
    void UpdateConnecting(double now);
    void UpdateRequestingManifest(double now);
    void UpdateDownloadingFiles(double now);

    void SetState(ContentSyncState newState, double now);
    void Fail(const char* reason);

    bool QueueGetManifest();
    bool QueueGetFile(const std::string& path);
    bool FlushSend();

    /**
     * Receives whatever has arrived. Returns false (after calling Fail) if the connection broke.
     */
    bool ReceiveAvailable();

    bool HandleManifest(const Net::FrameView& message);
    bool HandleFile(const Net::FrameView& message);

    /**
     * Reads dlc/manifest.json and lists every server file that is new, has a different version, or is
     * missing on disk.
     */
    void BuildDownloadList();

    /**
     * Requests the next file in the download list, or finishes the sync when none are left.
     */
    void StartNextDownload(double now);

    bool SaveLocalManifest();

    /**
     * Deletes files the local manifest lists but the server no longer does, and their empty folders.
     * Returns how many were removed.
     */
    uint32_t RemoveDroppedFiles();

    // How long one step may take before giving up. The OS alone can take 20+ s to fail a connect.
    static constexpr double CONNECT_TIMEOUT_SECONDS = 5.0;
    static constexpr double REPLY_TIMEOUT_SECONDS = 10.0;
    static constexpr size_t SEND_CAPACITY = 512;

    ContentSyncState state = ContentSyncState::Idle;
    double stateStartTime = 0.0;

    Net::TcpSocket socket;

    // Replies can be whole files (up to MAX_MESSAGE_SIZE), so the receive buffer is on the heap.
    uint8_t* recvStorage = nullptr;
    Net::FrameReceiver receiver;

    // Requests are small and sent from here; Send may take only part per call.
    uint8_t sendStorage[SEND_CAPACITY];
    size_t sendSize = 0;
    size_t sendSent = 0;

    Net::ContentManifest serverManifest;

    // What is on disk in dlc/, saved to dlc/manifest.json after every downloaded file.
    Net::ContentManifest localManifest;

    // Indices into serverManifest of the files to download, worked through one at a time.
    uint32_t downloadList[Net::ContentManifest::MAX_ENTRIES];
    uint32_t downloadCount = 0;
    uint32_t downloadCursor = 0;
    size_t downloadedBytes = 0;
};

}  // namespace ME
