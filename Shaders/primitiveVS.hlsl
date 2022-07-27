cbuffer cb : register(b0)
{
    matrix mWorld;
    matrix mWVP; //transform
    matrix normalMatrix;
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
};

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
}