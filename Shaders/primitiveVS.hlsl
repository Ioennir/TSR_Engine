cbuffer cb : register(b0)
{
    matrix mWorld;
    matrix mWVP; //transform
    matrix normalMatrix;
    matrix lWVP;
};

struct VS_Input
{
    float3 iPos : POSITION;
    float4 iCol : COLOR0;
    float3 iNormal : NORMAL0;
};

struct VS_Output
{
    float4 oPos : SV_POSITION;
    float4 oPosWorld : POSITION;
    float4 oCol : COLOR0;
    float3 oNormal : NORMAL0;
    float3 oLightVP : TEXCOORD0;
    float3 oLightPos : TEXCOORD1;
};

struct DirectionalLight
{
    float4 Position;
    float4 Direction;
    float4 Color;
};
StructuredBuffer<DirectionalLight> DLBuffer : register(t1);

void main(
    VS_Input input,
    out VS_Output output
)
{
    // Calculate position
    output.oPos = mul(float4(input.iPos, 1.0f), mWVP);
    output.oPosWorld = float4(input.iPos, 1.0f); //mul(float4(input.iPos, 1.0f), mWorld);
    output.oPosWorld = mul(float4(input.iPos, 1.0f), mWorld);
    output.oCol = input.iCol;
    output.oNormal = input.iNormal;
    
    output.oLightVP = mul(float4(input.iPos, 1.0f), lWVP);
    output.oLightPos = normalize(DLBuffer[0].Position.xyz - output.oPosWorld.xyz);

}