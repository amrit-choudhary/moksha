// Simple sprite shader for instanced rendering.

#include "structs.hlsl"
#include "helpers.hlsl"

cbuffer CBPerPassUIText : register(b0)
{
    CBPerPassUIText passData;
};

cbuffer TextureAtlasProperties : register(b1)
{
    TextureAtlasProperties atlasProps;
};

Texture2D tex : register(t0);

// Using clamped point sampling for pixel art style atlases.
SamplerState texSampler : register(s1);

StructuredBuffer<TextInstanceData> instanceBuffer : register(t1);

struct VertexIn
{
	float2 position  : POSITION;
    float2 uv : TEXCOORD0;
};

struct VertexOut
{
	float4 position  : SV_POSITION;
    float2 uv : TEXCOORD0;
    float4 color : COLOR0;
};

VertexOut VS(VertexIn input, uint instanceID : SV_InstanceID)
{
	VertexOut output;

    TextInstanceData instanceData = instanceBuffer[instanceID];

    float4 outputPos = mul(float4(input.position, 0.0f, 1.0f), instanceData.modelMatrix);
    // Mapping to NDC space for UI rendering. [-1, 1] range.
    // Screen dimensions are packed in a single uint32_t.
    uint screenWidth = (passData.screenDimension >> 16) & 0xFFFF;
    uint screenHeight = passData.screenDimension & 0xFFFF;
    outputPos.x = (outputPos.x / screenWidth) * 2.0f;
    outputPos.y = (outputPos.y / screenHeight) * 2.0f;
    output.position = outputPos;
    float2 flippedUV = float2(input.uv.x, 1.0f - input.uv.y);
    output.uv = GetAtlasUV(flippedUV, instanceData.atlasIndex, atlasProps, instanceData.flags);
    output.color = instanceData.color;

    return output;
}

float4 PS(VertexOut input) : SV_Target
{
    float4 texColor = tex.Sample(texSampler, input.uv);
    if (texColor.a < 0.1f)
        clip(-1);

    return input.color;
}
