
Texture2D tex : TEXTURE0 : register(t0);
SamplerState smp : SAMPLER0 : register(s0);

cbuffer cb
{
    matrix mWorld;
    matrix mWVP; //transform
    matrix normalMatrix;
};

struct PS_Input
{
    float4 iPosition    : SV_POSITION;
    float4 iColor       : COLOR0;
    float3 iNormal      : NORMAL0;
    float2 iTexcoord    : TEXCOORD0;
};

static const float3 LIGHT_DIR = normalize(float3(5.0f, 7.0f, -5.0f));
static const float  AMBIENT   = 0.05f;
float4 main(
    PS_Input input
) : SV_TARGET
{
    //Diffuse
    float4 pixelColor = tex.Sample(smp, input.iTexcoord);
    float3 normalWorldSpace = normalize(mul((float3x3)normalMatrix, input.iNormal));
    float4 lightIntensity = saturate(dot(normalWorldSpace, LIGHT_DIR));
    pixelColor = AMBIENT + saturate(pixelColor * lightIntensity);
    return pixelColor;
}
