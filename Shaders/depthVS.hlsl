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
    float4 oLightVP : TEXCOORD0;
};

struct DirectionalLight
{
    float4 Position;
    float4 Direction;
    float4 Color;
};

void main(
    VS_Input input,
    out VS_Output output
)
{
    // Calculate position
    output.oPos = mul(float4(input.iPos, 1.0f), lWVP);
    //output.oPosWorld = float4(input.iPos, 1.0f);
    output.oPosWorld = mul(float4(input.iPos, 1.0f), mWorld);
    
    output.oLightVP = mul(float4(input.iPos, 1.0f), lWVP);

}