
Texture2D tex : TEXTURE0 : register(t0);
Texture2D nor : TEXTURE1 : register(t1);

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

float4 sRGBtoLinear(float4 sRGBcolor)
{
    float4 Result;
    return Result;
}

/*
half3 sRGBToLinear( half3 Color ) 
{
	Color = max(6.10352e-5, Color); // minimum positive non-denormal (fixes black problem on DX11 AMD and NV)
	return Color > 0.04045 ? pow( Color * (1.0 / 1.055) + 0.0521327, 2.4 ) : Color * (1.0 / 12.92);
}
*/

static const float3 LIGHT_DIR = normalize(float3(5.0f, 7.0f, -5.0f));
static const float  AMBIENT   = 0.05f;
float4 main(
    PS_Input input
) : SV_TARGET
{
    float3 normalTex = nor.Sample(smp, input.iTexcoord);
    
    //Diffuse
    float4 pixelColor = tex.Sample(smp, input.iTexcoord);
    float3 normalWorldSpace = normalize(mul((float3x3)normalMatrix, input.iNormal));
    float4 lightIntensity = saturate(dot(normalWorldSpace, LIGHT_DIR));
    pixelColor = AMBIENT + saturate(pixelColor * lightIntensity);
    
    return pixelColor;
}
