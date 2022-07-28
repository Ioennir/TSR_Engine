
Texture2D tex : TEXTURE0 : register(t0);
Texture2D nor : TEXTURE1 : register(t1);

SamplerState smp : SAMPLER0 : register(s0);

cbuffer cb : register(b0)
{
    matrix mWorld;
    matrix mWVP; //transform
    matrix normalMatrix;
    matrix lWVP;
};

struct DirectionalLight
{
    float4 Position;
    float4 Direction;
    float4 Color;
};
struct PointLight
{
    float4 Position;
    float4 Color;
    float3 Fallof;
    float Range;
};
StructuredBuffer<DirectionalLight> DLBuffer : register(t3);
StructuredBuffer<DirectionalLight> PLBuffer : register(t4);

struct PS_Input
{
    float4 iPosition    : SV_POSITION;
    float4 iWorldPos    : POSITION;
    float4 iColor       : COLOR0;
    float3 iNormal      : NORMAL0;
    float2 iTexcoord    : TEXCOORD0;
    float4 iTangent : TANGENT0;
    float4 iBinormal : BINORMAL0;
};


half3 sRGBToLinear( half3 Color ) 
{
	Color = max(6.10352e-5, Color); // minimum positive non-denormal (fixes black problem on DX11 AMD and NV)
	return Color > 0.04045 ? pow( Color * (1.0 / 1.055) + 0.0521327, 2.4 ) : Color * (1.0 / 12.92);
}

half3 LinearTosRGB(half3 lin)
{
    lin = max(6.10352e-5, lin); // minimum positive non-denormal (fixes black problem on DX11 AMD and NV)
    return min(lin * 12.92, pow(max(lin, 0.00313067), 1.0 / 2.4) * 1.055 - 0.055);
    // Possible that mobile GPUs might have native pow() function?
    //return min(lin * 12.92, exp2(log2(max(lin, 0.00313067)) * (1.0/2.4) + log2(1.055)) - 0.055);
}

float4 main(
    PS_Input input
) : SV_TARGET
{
    float3 normalTex = nor.Sample(smp, input.iTexcoord);
    //Unpack normal
    normalTex = normalTex * 2.0f - 1.0f;
    float4 texColor = tex.Sample(smp, input.iTexcoord);
    //Diffuse
    float3 normalWS = normalize(mul((float3x3)normalMatrix, input.iNormal));
    float3 tangentWS = normalize(mul((float3x3) normalMatrix, input.iTangent.xyz));
    float3 binormalWS = normalize(mul((float3x3) normalMatrix, input.iBinormal.xyz));
    
    float3 bumpNormal = (normalTex.x * tangentWS) + (normalTex.y * binormalWS) + (normalTex.z * normalWS);
    bumpNormal = normalize(bumpNormal);
    
    float4 pixelColor = texColor.rgba;
    float4 lightIntensity = saturate(dot(bumpNormal, DLBuffer[0].Direction.xyz));
    pixelColor = saturate(pixelColor * lightIntensity);
    
    return pixelColor;
}
