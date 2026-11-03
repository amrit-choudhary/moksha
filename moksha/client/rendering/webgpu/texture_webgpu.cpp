#ifdef ME_WEBGPU

#include "texture_webgpu.h"

ME::TextureWebGPU::TextureWebGPU(const char* path, WGPUDevice device, WGPUQueue queue) {
    Texture::Load(path);
    if (data == nullptr || width == 0 || height == 0) {
        return;
    }

    // sRGB like the other backends.
    WGPUTextureDescriptor descriptor = WGPU_TEXTURE_DESCRIPTOR_INIT;
    descriptor.usage = WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst;
    descriptor.size = {width, height, 1};
    descriptor.format = WGPUTextureFormat_RGBA8UnormSrgb;
    texture = wgpuDeviceCreateTexture(device, &descriptor);
    view = wgpuTextureCreateView(texture, nullptr);

    WGPUTexelCopyTextureInfo destination = WGPU_TEXEL_COPY_TEXTURE_INFO_INIT;
    destination.texture = texture;
    WGPUTexelCopyBufferLayout layout = WGPU_TEXEL_COPY_BUFFER_LAYOUT_INIT;
    layout.bytesPerRow = width * channels;
    layout.rowsPerImage = height;
    wgpuQueueWriteTexture(queue, &destination, data, GetSize(), &layout, &descriptor.size);
}

ME::TextureWebGPU::~TextureWebGPU() {
    if (view != nullptr) {
        wgpuTextureViewRelease(view);
        view = nullptr;
    }
    if (texture != nullptr) {
        wgpuTextureRelease(texture);
        texture = nullptr;
    }
}

WGPUTextureView ME::TextureWebGPU::GetView() const {
    return view;
}

#endif  // ME_WEBGPU
