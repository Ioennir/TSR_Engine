cbuffer cbPerObject
{
    float4x4 worldViewProj;
}

void main(
    float3 iPos : POSITION,
    float4 iCol : COLOR,
    out float4 oPos : SV_POSITION,
    out float4 oCol : COLOR
)
{
    oPos = mul(float4(iPos, 1.0f), worldViewProj);

    oCol = iCol;
}
