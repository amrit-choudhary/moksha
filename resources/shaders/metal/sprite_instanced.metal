#include <metal_stdlib>
using namespace metal;

struct VertexIn {
    float2 position [[attribute(0)]];
    float2 uv       [[attribute(1)]];
};

struct VertexOut {
    float4 position [[position]];
    float2 uv;
    float4 color;
    uint16_t atlasIndex;
};

struct InstanceData {
    float4x4 modelMatrix;
    float4 color;
    uint16_t atlasIndex;
    uint8_t padding[14];
};

struct TextureAtlasProperties {
    uint16_t tileSizeX;
    uint16_t tileSizeY;
    uint16_t padding;
    uint16_t numTextures;
    uint16_t numTilesX;
    uint16_t numTilesY;
    uint16_t width;
    uint16_t height;
};

vertex VertexOut vertexMain(VertexIn in [[stage_in]],
                            constant InstanceData* instanceData [[buffer(1)]],
                            uint instanceId [[instance_id]],
                            constant float4x4& viewMatrix       [[buffer(2)]],
                            constant float4x4& projectionMatrix [[buffer(3)]]) {

    VertexOut out;

    float4x4 modelMatrix = instanceData[instanceId].modelMatrix;
    out.position = projectionMatrix * viewMatrix * modelMatrix * float4(in.position, 0.0f, 1.0f);
    out.uv = float2(in.uv.x, 1.0f - in.uv.y);
    out.color = instanceData[instanceId].color;
    out.atlasIndex = instanceData[instanceId].atlasIndex;
    return out;
}

fragment half4 fragmentMain(VertexOut in [[stage_in]],
                            texture2d<half, access::sample> tex             [[texture(0)]], 
                            sampler textureSampler                          [[sampler(0)]],
                            constant TextureAtlasProperties& atlasProp      [[buffer(1)]]) {

    
    float sizeX = (float)atlasProp.tileSizeX / atlasProp.width;
    float sizeY = (float)atlasProp.tileSizeY / atlasProp.height;
    uint col = in.atlasIndex % atlasProp.numTilesX;
    uint row = (in.atlasIndex / atlasProp.numTilesX ) % atlasProp.numTilesY;
    
    float originX = 0;
    float originY = 0;

    if(col > 0)
        originX = (atlasProp.tileSizeX + atlasProp.padding) * col;
    
    if(row > 0)
        originY = (atlasProp.tileSizeY + atlasProp.padding) * row;

    originX = originX / atlasProp.width;
    originY = originY / atlasProp.height;

    float2 uv = float2(originX, originY) + in.uv * float2(sizeX, sizeY);

    half4 texel =  tex.sample(textureSampler, uv);
    if(texel.a < 0.1f){
        discard_fragment();
    }
    return texel * half4(in.color);
}
