#include "Transform.fx"
#include "RenderBaseValue.fx"

struct Input
{
    float4 POSITION : POSITION;
    float4 TEXCOORD : TEXCOORD;
};

struct Output
{
    float4 POSITION : SV_POSITION;
    float4 TEXCOORD : TEXCOORD;
};

Output MeshAniTexture_VS(Input _Input)
{
    Output NewOutPut = (Output) 0;
    
    float4 InputPos = _Input.POSITION;
    InputPos.w = 1.0f;
        
    NewOutPut.POSITION = mul(InputPos, WorldViewProjectionMatrix);
    NewOutPut.TEXCOORD = _Input.TEXCOORD;
        
    return NewOutPut;
}

Texture2D DiffuseTexture : register(t0);
Texture2D NoiseTexture : register(t1);

SamplerState ENGINEBASE : register(s0);

float4 MeshAniTexture_PS(Output _Input) : SV_Target0
{
    float2 UV = _Input.TEXCOORD.xy;
    float2 Dir = UV - float2(0.5f, 0.5f);
    
    float4 Noise = NoiseTexture.Sample(ENGINEBASE, UV);
    
    UV += sin(Dir * Noise.x);
    
    float4 Color = DiffuseTexture.Sample(ENGINEBASE, UV);
        
    if (Color.a <= 0.0f)
    {
        clip(-1);
    }
    
    return Color;
}
