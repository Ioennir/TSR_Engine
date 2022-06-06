
Texture2D tex : TEXTURE0 : register(t0);
SamplerState smp : SAMPLER0 : register(s0);

struct PS_Input
{
    float4 iPosition : SV_POSITION;
    float4 iColor : COLOR0;
    float3 iNormal : NORMAL0;
    float2 iTexcoord : TEXCOORD0;
};

float4 main(
    PS_Input input
) : SV_TARGET
{
    float4 pixelColor = tex.Sample(smp, input.iTexcoord);
    return pixelColor;
}
