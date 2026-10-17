#include "content_client.h"

#include "moksha/logging/logging.h"
#include "moksha/shared/file_io/vfs.h"
#include "moksha/shared/net/content_protocol.h"
#include "moksha/shared/serialization/byte_reader.h"
#include "moksha/shared/serialization/byte_writer.h"

using ME::Net::TcpResult;
using ME::Net::ContentProtocol::Verb;

namespace {
constexpr const char* MANIFEST_FILE = "manifest.json";
}  // namespace

ME::ContentClient::ContentClient()
    : recvStorage(new uint8_t[Net::ContentProtocol::MAX_MESSAGE_SIZE]),
      receiver(recvStorage, Net::ContentProtocol::MAX_MESSAGE_SIZE, Net::ContentProtocol::MAX_MESSAGE_SIZE) {}

ME::ContentClient::~ContentClient() {
    End();
    delete[] recvStorage;
}

void ME::ContentClient::Init(const char* serverIP, uint16_t serverPort, double now) {
    LogInfo("Content: connecting to ", serverIP, ":", serverPort);
    const TcpResult result = socket.Connect(serverIP, serverPort);
    if (result == TcpResult::Error) {
        Fail("connect failed");
        return;
    }

    if (result == TcpResult::WouldBlock) {
        SetState(ContentSyncState::Connecting, now);
        return;
    }

    // Connected immediately (possible on localhost).
    if (!QueueGetManifest()) {
        Fail("could not build GET_MANIFEST");
        return;
    }
    SetState(ContentSyncState::RequestingManifest, now);
}

void ME::ContentClient::Update(double now) {
    switch (state) {
        case ContentSyncState::Connecting:
            UpdateConnecting(now);
            break;
        case ContentSyncState::RequestingManifest:
            UpdateRequestingManifest(now);
            break;
        case ContentSyncState::DownloadingFiles:
            UpdateDownloadingFiles(now);
            break;
        case ContentSyncState::Idle:
        case ContentSyncState::Done:
        case ContentSyncState::Failed:
            break;
    }
}

void ME::ContentClient::End() {
    socket.Close();
}

ME::ContentSyncState ME::ContentClient::GetState() const {
    return state;
}

void ME::ContentClient::UpdateConnecting(double now) {
    const TcpResult result = socket.PollConnect();
    if (result == TcpResult::Error) {
        Fail("connect failed");
        return;
    }
    if (result == TcpResult::WouldBlock) {
        if (now - stateStartTime > CONNECT_TIMEOUT_SECONDS) {
            Fail("connect timed out");
        }
        return;
    }

    LogInfo("Content: connected");
    if (!QueueGetManifest()) {
        Fail("could not build GET_MANIFEST");
        return;
    }
    SetState(ContentSyncState::RequestingManifest, now);
}

void ME::ContentClient::UpdateRequestingManifest(double now) {
    if (!FlushSend() || !ReceiveAvailable()) {
        return;
    }

    Net::FrameView message;
    const Net::FrameResult frame = receiver.PeekFrame(message);
    if (frame == Net::FrameResult::Invalid) {
        Fail("server sent an invalid frame");
        return;
    }
    if (frame == Net::FrameResult::Incomplete) {
        if (now - stateStartTime > REPLY_TIMEOUT_SECONDS) {
            Fail("timed out waiting for MANIFEST");
        }
        return;
    }

    // message.payload points into the receiver, so use it before PopFrame.
    const bool ok = HandleManifest(message);
    receiver.PopFrame();
    if (!ok) {
        return;
    }

    BuildDownloadList();
    downloadCursor = 0;
    downloadedBytes = 0;
    SetState(ContentSyncState::DownloadingFiles, now);
    StartNextDownload(now);
}

void ME::ContentClient::UpdateDownloadingFiles(double now) {
    if (!FlushSend() || !ReceiveAvailable()) {
        return;
    }

    Net::FrameView message;
    const Net::FrameResult frame = receiver.PeekFrame(message);
    if (frame == Net::FrameResult::Invalid) {
        Fail("server sent an invalid frame");
        return;
    }
    if (frame == Net::FrameResult::Incomplete) {
        if (now - stateStartTime > REPLY_TIMEOUT_SECONDS) {
            Fail("timed out waiting for a file");
        }
        return;
    }

    // message.payload points into the receiver, so use it before PopFrame.
    const bool ok = HandleFile(message);
    receiver.PopFrame();
    if (!ok) {
        return;
    }

    ++downloadCursor;
    StartNextDownload(now);
}

void ME::ContentClient::SetState(ContentSyncState newState, double now) {
    state = newState;
    stateStartTime = now;
}

void ME::ContentClient::Fail(const char* reason) {
    LogWarning("Content: sync failed (", reason, "); continuing with existing content");
    socket.Close();
    state = ContentSyncState::Failed;
}

bool ME::ContentClient::QueueGetManifest() {
    ByteWriter writer(sendStorage, sizeof(sendStorage));
    size_t frameStart = 0;
    if (!Net::BeginFrame(writer, Net::ContentProtocol::VERSION, static_cast<uint8_t>(Verb::GET_MANIFEST), frameStart) ||
        !Net::FinishFrame(writer, frameStart)) {
        return false;
    }
    sendSize = writer.GetSize();
    sendSent = 0;
    return true;
}

bool ME::ContentClient::QueueGetFile(const std::string& path) {
    ByteWriter writer(sendStorage, sizeof(sendStorage));
    size_t frameStart = 0;
    if (!Net::BeginFrame(writer, Net::ContentProtocol::VERSION, static_cast<uint8_t>(Verb::GET_FILE), frameStart) ||
        !writer.WriteU16(static_cast<uint16_t>(path.size())) ||
        !writer.WriteBytes(reinterpret_cast<const uint8_t*>(path.data()), path.size()) ||
        !writer.WriteU32(0) ||  // rangeOffset (reserved)
        !writer.WriteU32(0) ||  // rangeLength (reserved)
        !Net::FinishFrame(writer, frameStart)) {
        return false;
    }
    sendSize = writer.GetSize();
    sendSent = 0;
    return true;
}

bool ME::ContentClient::FlushSend() {
    while (sendSent < sendSize) {
        int sent = 0;
        const TcpResult result = socket.Send(sendStorage + sendSent, static_cast<int>(sendSize - sendSent), sent);
        if (result == TcpResult::Error) {
            Fail("send failed");
            return false;
        }
        if (result == TcpResult::WouldBlock || sent == 0) {
            return true;
        }
        sendSent += static_cast<size_t>(sent);
    }
    return true;
}

bool ME::ContentClient::ReceiveAvailable() {
    // Read until nothing more is waiting this frame, so a large reply arrives as fast as the network allows.
    while (receiver.GetFreeSize() > 0) {
        int received = 0;
        const TcpResult result =
            socket.Recv(receiver.GetFreeData(), static_cast<int>(receiver.GetFreeSize()), received);
        if (result == TcpResult::Ok) {
            receiver.CommitReceived(static_cast<size_t>(received));
            continue;
        }
        if (result == TcpResult::WouldBlock) {
            return true;
        }
        Fail(result == TcpResult::Closed ? "server closed the connection" : "recv failed");
        return false;
    }
    return true;
}

bool ME::ContentClient::HandleManifest(const Net::FrameView& message) {
    if (message.version != Net::ContentProtocol::VERSION || message.verb != static_cast<uint8_t>(Verb::MANIFEST)) {
        Fail("expected a MANIFEST reply");
        return false;
    }
    if (!serverManifest.Parse(reinterpret_cast<const char*>(message.payload), message.payloadSize)) {
        Fail("invalid manifest");
        return false;
    }

    LogInfo("Content: server lists ", serverManifest.GetCount(), " files");
    for (uint32_t i = 0; i < serverManifest.GetCount(); ++i) {
        const Net::ManifestEntry& entry = serverManifest.GetEntry(i);
        LogInfo("Content:   ", entry.name, " v", entry.version);
    }
    return true;
}

bool ME::ContentClient::HandleFile(const Net::FrameView& message) {
    const Net::ManifestEntry& expected = serverManifest.GetEntry(downloadList[downloadCursor]);

    ByteReader reader(message.payload, message.payloadSize);
    uint16_t pathLength = 0;
    const uint8_t* path = nullptr;
    if (message.version != Net::ContentProtocol::VERSION || !reader.ReadU16(pathLength) ||
        !reader.ReadBytes(pathLength, path)) {
        Fail("malformed reply to GET_FILE");
        return false;
    }

    // The reply must be for the file we asked for; its name was already checked as a safe path by the manifest.
    if (std::string(reinterpret_cast<const char*>(path), pathLength) != expected.name) {
        Fail("reply is for a different file");
        return false;
    }

    if (message.verb == static_cast<uint8_t>(Verb::FILE_NOT_FOUND)) {
        LogWarning("Content: server is missing ", expected.name);
        Fail("file listed in the manifest is not on the server");
        return false;
    }
    if (message.verb != static_cast<uint8_t>(Verb::FILE)) {
        Fail("expected a FILE reply");
        return false;
    }

    // Everything after the path is the file.
    const size_t fileSize = reader.GetRemaining();
    const uint8_t* fileData = nullptr;
    reader.ReadBytes(fileSize, fileData);

    if (!Vfs::WriteBytes(FileRoot::Dlc, expected.name.c_str(), fileData, fileSize)) {
        Fail("could not write a downloaded file");
        return false;
    }

    // Record the new version only now that the file is fully written.
    if (!localManifest.Set(expected.name.c_str(), expected.version) || !SaveLocalManifest()) {
        Fail("could not save dlc/manifest.json");
        return false;
    }

    downloadedBytes += fileSize;
    LogInfo("Content: downloaded ", expected.name, " v", expected.version, " (", fileSize, " bytes)");
    return true;
}

void ME::ContentClient::BuildDownloadList() {
    std::string text;
    if (!Vfs::ReadText(FileRoot::Dlc, MANIFEST_FILE, text) || !localManifest.Parse(text.data(), text.size())) {
        // First sync, or an unreadable file: treat as nothing downloaded yet.
        localManifest.Clear();
    }

    downloadCount = 0;
    for (uint32_t i = 0; i < serverManifest.GetCount(); ++i) {
        const Net::ManifestEntry& entry = serverManifest.GetEntry(i);
        const Net::ManifestEntry* local = localManifest.Find(entry.name.c_str());

        // Different, not just newer, so rolling a file back on the server also reaches clients.
        size_t size = 0;
        const bool needed = local == nullptr || local->version != entry.version ||
                            !Vfs::GetFileSize(FileRoot::Dlc, entry.name.c_str(), size);
        if (needed) {
            downloadList[downloadCount] = i;
            ++downloadCount;
        }
    }

    LogInfo("Content: ", downloadCount, " of ", serverManifest.GetCount(), " files need downloading");
}

void ME::ContentClient::StartNextDownload(double now) {
    if (downloadCursor >= downloadCount) {
        socket.Close();
        const uint32_t removedCount = RemoveDroppedFiles();
        SetState(ContentSyncState::Done, now);
        LogSuccess("Content: sync done (", downloadCount, " downloaded, ", downloadedBytes, " bytes, ", removedCount,
                   " removed)");
        return;
    }

    if (!QueueGetFile(serverManifest.GetEntry(downloadList[downloadCursor]).name)) {
        Fail("could not build GET_FILE");
        return;
    }
    // Each file gets a fresh reply timeout.
    stateStartTime = now;
}

bool ME::ContentClient::SaveLocalManifest() {
    std::string text;
    if (!localManifest.Serialize(text)) {
        return false;
    }
    return Vfs::WriteBytes(FileRoot::Dlc, MANIFEST_FILE, reinterpret_cast<const uint8_t*>(text.data()), text.size());
}

uint32_t ME::ContentClient::RemoveDroppedFiles() {
    uint32_t removedCount = 0;

    // Backwards, because Remove shifts the later entries down. Only names the client recorded itself are ever
    // deleted, and they passed the manifest's safe-path check, so nothing outside dlc/ can be touched.
    for (uint32_t i = localManifest.GetCount(); i-- > 0;) {
        const std::string name = localManifest.GetEntry(i).name;
        if (serverManifest.Find(name.c_str()) != nullptr) {
            continue;
        }

        if (!Vfs::RemoveFile(FileRoot::Dlc, name.c_str())) {
            // Keep the entry so the next sync tries again (e.g. the file was locked by another program).
            LogWarning("Content: could not remove ", name);
            continue;
        }
        Vfs::RemoveEmptyFolders(FileRoot::Dlc, name.c_str());
        localManifest.Remove(name.c_str());
        ++removedCount;
        LogInfo("Content: removed ", name);
    }

    // Saved once at the end: if the game stops before this, the next sync finds the same entries and
    // RemoveFile succeeds again for files already gone.
    if (removedCount > 0 && !SaveLocalManifest()) {
        LogWarning("Content: could not save dlc/manifest.json after removing files");
    }
    return removedCount;
}
