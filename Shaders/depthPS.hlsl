cbuffer cb : register(b0)
{
    matrix mWorld;
    matrix mWVP; //transform
    matrix normalMatrix;
    matrix lWVP;
};

struct PS_Input
{
    float4 iPosition : SV_POSITION;
    float4 iPosWorld : POSITION;
    float4 lightVP : TEXCOORD0;
};

float4 main(
    PS_Input input
) : SV_TARGET
{
    float depthValue = input.lightVP.z / input.lightVP.w;
    return float4(depthValue, depthValue, depthValue, 1.0f);
}
