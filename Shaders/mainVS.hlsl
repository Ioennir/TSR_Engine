cbuffer cb : register(b0)
{
	matrix mWorld;
	matrix mWVP; //transform
	matrix normalMatrix;
    matrix lWVP;
};

struct VS_Input
{
    float3 iPos         : POSITION;
    float4 iCol         : COLOR0;
    float3 iNormal      : NORMAL0;
    float2 iTexcoord    : TEXCOORD0;
    float4 iTangent     : TANGENT0;
    float4 iBinormal    : BINORMAL0;
};

struct VS_Output
{
    float4 oPos         : SV_POSITION;
    float4 oWorldPos    : POSITION;
    float4 oCol         : COLOR0;
    float3 oNormal      : NORMAL0;
    float2 oTexcoord    : TEXCOORD0;
    float4 oTangent : TANGENT0;
    float4 oBinormal : BINORMAL0;
};

void main(
    VS_Input input,
    out VS_Output output
)
{
    // Calculate position
	output.oPos = mul(float4(input.iPos, 1.0f), mWVP);
    output.oWorldPos = mul(float4(input.iPos, 1.0f), mWorld);
    output.oTangent = input.iTangent;
    output.oBinormal = input.iBinormal;
    output.oCol = input.iCol;
    output.oNormal = input.iNormal;
    output.oTexcoord = input.iTexcoord;
 }
