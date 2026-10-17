// Simple sprite shader for instanced rendering.

#include "structs.hlsl"
#include "helpers.hlsl"

cbuffer CBPerPass : register(b0)
{
    CBPerPass passData;
};

cbuffer TextureAtlasProperties : register(b1)
{
    TextureAtlasProperties atlasProps;
};

Texture2D tex : register(t0);

// Using clamped point sampling for pixel art style atlases.
SamplerState texSampler : register(s1);

StructuredBuffer<SpriteInstanceData> instanceBuffer : register(t1);

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

    SpriteInstanceData instanceData = instanceBuffer[instanceID];

    float4 outputPos = mul(float4(input.position, 0.0f, 1.0f), instanceData.modelMatrix);
    outputPos = mul(outputPos, passData.viewMatrix);
    outputPos = mul(outputPos, passData.projectionMatrix);
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

    return texColor * input.color;
}
