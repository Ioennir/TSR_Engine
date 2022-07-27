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
    float4 Fallof;
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
    float3 ambientLight = input.iColor.rgb * 0.04f;
    
    float Range = PLBuffer[0].Fallof.w;
    
    float3 appliedLight = ambientLight;
    
    float3 LightPos = PLBuffer[0].Position.xyz;
    float3 vecToLight = normalize(LightPos - input.iPosWorld.xyz);
    
    float3 diffuseLightIntensity = saturate(dot(vecToLight, normalWS));
    float distanceToLight = distance(LightPos, input.iPosWorld.xyz);
    
    float att0 = PLBuffer[0].Fallof.x;
    float att1 = PLBuffer[0].Fallof.y;
    float att2 = PLBuffer[0].Fallof.z;
    
    float attenuationFactor = 1.0f / (att0 + att1 * distanceToLight + att2 * pow(distanceToLight,2));
    diffuseLightIntensity *= attenuationFactor;
    
    float3 diffuseLight = PLBuffer[0].Color.rgb * (diffuseLightIntensity * PLBuffer[0].Color.a);
    appliedLight += diffuseLight;
    
    float4 pixelColor = float4(input.iColor.xyz * appliedLight, 1.0f);
    
    //float3 LightWorldPos = float4(PLBuffer[0].Position.xyz, 1.0f);//    mul(float4(PLBuffer[0].Position.xyz, 1.0f), mWorld).xyz;
    //float3 WorldPos = mul(float4(input.iPosWorld.xyz, 1.0f), mWorld);
    //float3 lightToPixelVec = normalize(LightWorldPos.xyz - WorldPos);
    //
    //float d = distance(LightWorldPos, WorldPos);
    //if (d > PLBuffer[0].Range)
    //{
    //    pixelColor = float4(ambientLight.xyz, 1.0f);
    //}
    //lightToPixelVec = normalize(lightToPixelVec);
    //float howMuchLight = dot(lightToPixelVec, normalWS);
    //
    //if (howMuchLight > 0.0f)
    //{
    //    pixelColor += howMuchLight * input.iColor;
    //    pixelColor /= PLBuffer[0].Fallof.x + (PLBuffer[0].Fallof.y * d) + (PLBuffer[0].Fallof.z * (d * d));
    //}
    ////float4 lightIntensity = saturate(dot(normalWS, DLBuffer[0].Direction.xyz));
    //pixelColor = saturate(pixelColor + ambientLight);
    
    return float4(pixelColor.xyz, 1.0f);

}
