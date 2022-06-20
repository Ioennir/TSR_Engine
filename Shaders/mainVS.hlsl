cbuffer cb
{
	matrix mWorld;
	matrix mWVP; //transform
	matrix normalMatrix;
};

struct VS_Input
{
    float3 iPos         : POSITION;
    float4 iCol         : COLOR0;
    float3 iNormal      : NORMAL0;
    float2 iTexcoord    : TEXCOORD0;
};

struct VS_Output
{
    float4 oPos         : SV_POSITION;
    float4 oCol         : COLOR0;
    float3 oNormal      : NORMAL0;
    float2 oTexcoord    : TEXCOORD0;
};

static const float3 LIGHT_DIR = normalize(float3(5.0f, 5.0f, -5.0f));

void main(
    VS_Input input,
    out VS_Output output
)
{
    // Calculate position
	output.oPos = mul(float4(input.iPos, 1.0f), mWVP);
    
    output.oCol = input.iCol;
    output.oNormal = input.iNormal;
    output.oTexcoord = input.iTexcoord;
 }
