cbuffer cbPerObject
{
    float4x4 worldViewProj;
}

void VS(
    float3 iPos : POSITION,
    float4 iCol : COLOR,
    out float4 oPos : SV_POSITION,
    out float4 oCol : COLOR
)
{
    oPos = mul(float4(iPos, 1.0f), worldViewProj);

    oCol = iCol;
}

float4 PS(
    float4 iPos : SV_POSITION,
    float4 iCol : COLOR
) : SV_TARGET
{
    return iCol;
}
