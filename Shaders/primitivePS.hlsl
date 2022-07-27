cbuffer cb : register(b0)
{
    matrix mWorld;
    matrix mWVP; //transform
    matrix normalMatrix;
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
StructuredBuffer<PointLight> PLBuffer : register(t4);

struct PS_Input
{
    float4 iPosition : SV_POSITION;
    float4 iPosWorld : POSITION;
    float4 iColor : COLOR0;
    float3 iNormal : NORMAL0;
};

float4 main(
    PS_Input input
) : SV_TARGET
{
    float3 normalWS = normalize(mul((float3x3) normalMatrix, input.iNormal));
    float4 pixelColor = float4(0.0f, 0.0f, 0.0f, 0.0f); //input.iColor;
    float4 ambientLight = input.iColor * 0.04f;
    
    float3 LightWorldPos = mul(float4(PLBuffer[0].Position.xyz, 1.0f), mWorld).xyz;
    float3 lightToPixelVec = LightWorldPos - input.iPosWorld.xyz;
    
    float d = length(lightToPixelVec);
    if (d > PLBuffer[0].Range)
    {
        pixelColor = float4(ambientLight.xyz, 1.0f);
    }
    lightToPixelVec = normalize(lightToPixelVec);
    float howMuchLight = dot(lightToPixelVec, normalWS);

    if (howMuchLight > 0.0f)
    {
        pixelColor += howMuchLight * input.iColor;
        pixelColor /= PLBuffer[0].Fallof.x + (PLBuffer[0].Fallof.y * d) + (PLBuffer[0].Fallof.z * (d * d));
    }
    //float4 lightIntensity = saturate(dot(normalWS, DLBuffer[0].Direction.xyz));
    pixelColor = saturate(pixelColor + ambientLight);
    
    return float4(pixelColor.xyz, 1.0f);

}
